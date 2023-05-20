#ifndef _QUEUE_HEADER_
#define _QUEUE_HEADER_

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdarg.h>
#include <time.h>

// ------------------------------------------------------------ Defines --------------------------------------------------------------

#define CLOCK_TO_MS(x) ((int)((x) * 1000 / CLOCKS_PER_SEC))

// ------------------------------------------------------------ Queue --------------------------------------------------------------

typedef struct{
	size_t transition;
	int start;
	int delay;
}transition_t;

typedef struct transition_queue_t transition_queue_t;

transition_queue_t *transition_queue_new(void);

void transition_queue_destroy(transition_queue_t *queue);

void transition_queue_push(transition_queue_t *queue, size_t transition, int delay);

bool transition_queue_pop(transition_queue_t *queue, transition_t *transition);

#endif
