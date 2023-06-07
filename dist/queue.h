/**
 * @file queue.h
 * 
 * pnet - easly make petri nets in C/C++ code. This library can create high level timed petri nets, with support for nesting,
 * negated arcs, reset arcs, inputs and outputs and tools for analisys, simulation and compiling petri nets to other forms of code.
 * Is intended for embedding!
 * 
 * Created by João Peterson Scheffer - 2023. Version 1.1.2.
 * 
 * Licensed under the MIT License. Please refeer to the LICENSE file in the project root for license information.
 * 
 * A queue for use with timed transitions on threads  
 */

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
