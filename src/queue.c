#include "queue.h"
#include <stdarg.h>
#include <stdio.h>
#include <pthread.h>

// ------------------------------------------------------------ Queue --------------------------------------------------------------

typedef struct queue_node_t queue_node_t;

struct queue_node_t{
	queue_node_t *next;
	queue_node_t *prev;
	void *value;
};

typedef struct{
	queue_node_t *first;
	queue_node_t *last;
	size_t size;
}queue_t;

struct transition_queue_t{
	queue_t *q;
	pthread_mutex_t lock;
};

queue_t *queue_new(void);

void queue_delete(queue_t *queue);

#define queue_value(node, type) \
	(*((type*)(node->value)))

queue_t *queue_new(void){
	queue_t *queue = calloc(1, sizeof(queue_t));

	queue->size = 0;
	queue->first = NULL;
	queue->last = NULL;

	return queue;
}

void queue_delete(queue_t *queue){
	if(queue->first != NULL){
		for(queue_node_t *node = queue->first->next; node != NULL; node = node->next){
			free(node->prev->value);
			free(node->prev);
		}
		free(queue->last->value);
		free(queue->last);
	}
	free(queue);
}

#define queue_lock() pthread_mutex_lock(&(queue->lock))
#define queue_unlock() pthread_mutex_unlock(&(queue->lock))

transition_queue_t *transition_queue_new(void){
	transition_queue_t *queue = calloc(1, sizeof(transition_queue_t));
	queue->q = queue_new();
	pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
	queue->lock = lock;
	return queue;
}

void transition_queue_destroy(transition_queue_t *queue){
	pthread_mutex_destroy(&(queue->lock));
	queue_delete(queue->q);
	free(queue);
}

void transition_queue_push(transition_queue_t *queue, size_t transition, int delay){
	if(queue == NULL) return;

	queue_lock();
	
	transition_t tran_data = {
		.transition = transition,
		.start = CLOCK_TO_MS(clock()),
		.delay = delay,
	};

	transition_t *tran = calloc(1, sizeof(transition_t));
	*tran = tran_data;

	queue_node_t *tran_node = calloc(1, sizeof(queue_node_t));
	tran_node->value = (void*)tran;

	if(queue->q->size == 0){
		queue->q->first = tran_node;
		queue->q->last = tran_node;
		queue->q->size++;
	}
	else{
		queue_node_t *node = NULL;
		for(node = queue->q->last; node != NULL; node = node->prev){
			transition_t node_value = queue_value(node, transition_t);

			if(node_value.transition == tran_data.transition){
				queue_unlock();
				free(tran_node);
				free(tran);
				return;
			}
			
			if(
				(tran_data.start + tran_data.delay) <
				(node_value.start + node_value.delay) 
			){
				tran_node->prev = node;
				tran_node->next = node->next;

				if(node->next != NULL) 
					node->next->prev = tran_node;
				
				node->next = tran_node;
				
				if(queue->q->last == node)
					queue->q->last = tran_node;

				queue->q->size++;
				break;
			}
		}

		if(node == NULL){
			node = queue->q->first;
			queue->q->first = tran_node;
			tran_node->next = node;
			node->prev = tran_node;
		}
	}

	queue_unlock();
}

bool transition_queue_pop(transition_queue_t *queue, transition_t *transition){
	if(queue == NULL || queue->q->size == 0 || transition == NULL) return false;
	queue_lock();
	int now = CLOCK_TO_MS(clock());

	transition_t node_value = queue_value(queue->q->last, transition_t);
	if((now - node_value.start) >= node_value.delay){
		*transition = node_value;

		queue_node_t *node = queue->q->last;
		queue->q->last = node->prev;

		if(node->prev != NULL)
			node->prev->next = NULL;

		queue->q->size--;

		if(queue->q->first == node)
			queue->q->first = NULL;

		free(node->value);
		free(node);
		
		queue_unlock();
		return true;
	}

	queue_unlock();
	return false;
}