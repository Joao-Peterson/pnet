#ifndef _DATA_HEADER_
#define _DATA_HEADER_

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <pthread.h>

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
	bool _threadsafe;
}queue_t;

typedef struct{
	queue_t *queue;
	pthread_mutex_t lock;
}queue_thread_safe_t;

queue_t *queue_new(void);

queue_t *queue_new_thread_safe(void);

void queue_delete(queue_t *queue);

void _queue_push(queue_t *queue, void *value);

void _queue_push_unique(queue_t *queue, void *value, size_t size);

void* _queue_pop(queue_t *queue);

#define queue_node_value(node, type) *((type*)(node->value))

#define queue_foreach(node, queue) queue_node_t *node; node != NULL; node = node->next

#define queue_push(queue, type, val) \
	_queue_push(queue, calloc(1, sizeof(val))); \
	*((type*)(queue->last->value)) = val

#define queue_push_unique(queue, type, val) \
	_queue_push_unique(queue, calloc(1, sizeof(val)), sizeof(val)); \
	*((type*)(queue->last->value)) = val

#define queue_pop(queue, type) \
	queue->first != NULL ? (*((type*)(queue->first->value))) : 0; free(_queue_pop(queue))

#define queue_last(queue, type) \
	queue->first != NULL ? (*((type*)(queue->first->value))) : 0

#endif