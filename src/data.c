#include "data.h"

#define queue_lock() if(queue->_threadsafe) pthread_mutex_lock(&(((queue_thread_safe_t*)queue)->lock))
#define queue_unlock() if(queue->_threadsafe) pthread_mutex_unlock(&(((queue_thread_safe_t*)queue)->lock))

queue_t *queue_new(void){
	queue_t *queue = calloc(1, sizeof(queue_t));

	queue->size = 0;
	queue->first = NULL;
	queue->last = NULL;
	queue->_threadsafe = false;

	return queue;
}

queue_t *queue_new_thread_safe(void){
	queue_t *queue = calloc(1, sizeof(queue_thread_safe_t));

	queue->size = 0;
	queue->first = NULL;
	queue->last = NULL;
	queue->_threadsafe = true;

	pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
	((queue_thread_safe_t*)queue)->lock = lock;

	return queue;
}

void queue_delete(queue_t *queue){
	if(queue->first != NULL){
		for(queue_node_t *node = queue->first->next; node != NULL; node = node->next){
			free(node->prev->value);
			free(node->prev);
		}
		free(queue->last);
	}
	free(queue);
}

void _queue_push(queue_t *queue, void *value){
	if(queue == NULL || value == NULL) return;

	queue_node_t *node = calloc(1, sizeof(node));

	queue_lock();

	node->next = NULL;
	node->value = value;
	
	if(queue->size == 0){
		queue->last = node; 
		queue->first = node; 
		node->prev = NULL;
	}
	else{
		queue->last->next = node; 
		node->prev = queue->last; 
		queue->last = node;
	}
	
	queue->size++;

	queue_unlock();
}

void _queue_push_unique(queue_t *queue, void *value, size_t size){
	if(queue == NULL || value == NULL) return;

	queue_node_t *node = calloc(1, sizeof(node));

	queue_lock();

	for(queue_foreach(cursor, queue)){
		if(!memcmp(cursor->value, value, size)){
			free(value);
			queue_unlock();
			return;
		}
	}

	queue_unlock();

	_queue_push(queue, value);
}

void* _queue_pop(queue_t *queue){
	if(queue == NULL) return NULL;
	if(queue->size == 0) return NULL;

	void *value = queue->first->value;

	queue_lock();

	if(queue->size == 1){
		free(queue->first);
		queue->first = NULL;
		queue->last = NULL;
	}
	else{
		queue->first = queue->first->next;
		free(queue->first->prev);
		queue->first->prev = NULL;
	}

	queue->size--;

	queue_unlock();
	return value;
}

void* _queue_pop_value(queue_t *queue){
	if(queue == NULL) return NULL;
	if(queue->size == 0) return NULL;

	void *value;

	queue_lock();

	// 

	queue->size--;

	queue_unlock();
	return value;
}