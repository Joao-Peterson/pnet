/**
 * @file pnet.h
 * 
 * pnet - easly make petri nets in C/C++ code. This library can create high level timed petri nets, with support for nesting,
 * negated arcs, reset arcs, inputs and outputs and tools for analisys, simulation and compiling petri nets to other forms of code.
 * Is intended for embedding!
 * Supports the PNML standard with read/write functions!
 * 
 * Created by João Peterson Scheffer - 2022. Version 1.0-0.
 * 
 * Licensed under the MIT License. Please refeer to the LICENSE file in the project root for license information.
 */

#ifndef _PNET_HEADER_
#define _PNET_HEADER_

#include <stddef.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdbool.h>
#include "pnet_error.h"
#include "matrix.h"

/**
 * @brief type of events used in the input/transition mapping 
 */
typedef enum{
    pnet_event_none         = 0b00,                                                 /**< No input event, transition will trigger if sensibilized. Same as 0 */
    pnet_event_pos_edge     = 0b01,                                                 /**< The input must be 0 then 1 so the transition can trigger */
    pnet_event_neg_edge     = 0b10,                                                 /**< The input must be 1 then 0 so the transition can trigger */
    pnet_event_any_edge     = 0b11,                                                 /**< The input must be change state from 1 to 0 or vice versa */
    pnet_event_t_max                                                                /**< Enumerator check value, don't use! */
}pnet_event_t;

/**
 * @brief pnet places list, created by calling pnet_places_new()
 */
typedef struct{
    matrix_int_t *values;
}pnet_places_t;

/**
 * @brief pnet transitions list, created by calling pnet_transitions_new()
 */
typedef struct{
    matrix_int_t *values;
}pnet_transitions_t;

/**
 * @brief pnet arcs map list, created by calling pnet_arcs_new()
 */
typedef struct{
    matrix_int_t *values;
}pnet_arcs_map_t;

/**
 * @brief pnet inputs map list, created by calling pnet_inputs_new()
 */
typedef struct{
    matrix_int_t *values;
}pnet_inputs_map_t;

/**
 * @brief pnet outputs map list, created by calling pnet_outputs_new()
 */
typedef struct{
    matrix_int_t *values;
}pnet_outputs_map_t;

/**
 * @brief pnet inputs, created by calling pnet_inputs_new()
 */
typedef struct{
    matrix_int_t *values;
}pnet_inputs_t;

/**
 * @brief struct that represents a petri net
 */
typedef struct{
    // size
    size_t num_places;
    size_t num_transitions;
    size_t num_inputs;
    size_t num_outputs;

    // maps
    matrix_int_t *neg_arcs_map; 
    matrix_int_t *pos_arcs_map; 
    matrix_int_t *inhibit_arcs_map; 
    matrix_int_t *reset_arcs_map;
    matrix_int_t *places_init; 
    matrix_int_t *transitions_delay;
    matrix_int_t *inputs_map;
    matrix_int_t *outputs_map;

    // net state
    matrix_int_t *places;                                                           /**< The actual places that hold tokens */
    matrix_int_t *sensitive_transitions;                                            /**< Currently firable transitions */

    // input edges state
    matrix_int_t *inputs_last;                                                      /**< The last state of the inputs, used to make edge events */

    // output values
    matrix_int_t *outputs;                                                          /**< The actual output values produced by the petri net */
}pnet_t;

/**
 * @brief Create a new petri net. All values from the inputs are freed automatically
 * @param neg_arcs_map: matrix of arcs weight/direction, where the rows are the places and the columns are the transitions. Represents the number of tokens to be removed from a place. Only negative values. Can be null
 * @param pos_arcs_map: matrix of arcs weight/direction, where the rows are the places and the columns are the transitions. Represents the number of tokens to be moved onto a place. Only positive values. Can be null
 * @param inhibit_arcs_map: matrix of arcs, where the coluns are the places and the rows are the transitions. Dictates the firing of a transition when a place has zero tokens. Values must be 0 or 1, any non zero number counts as 1. Can be null
 * @param reset_arcs_map: matrix of arcs, where the coluns are the places and the rows are the transitions. When a transition occurs it zeroes out the place tokens. Values must be 0 or 1, any non zero number counts as 1. Can be null
 * @param places_init: matrix of values, where the columns are the places. The initial values for the places. Values must be a positive value. Must be not null
 * @param transitions_delay: matrix of values, were the columns are the transitions. While a place has enough tokens, the transitions will delay it's firing. Values must be positive, given is micro seconds (us). Can be null
 * @param inputs_map: matrix where the columns are the transitions and the rows are inputs. Represents the type of event that will fire that transistion, given by the enumerator pnet_event_t. Can be null
 * @param outputs_map: matrix where the columns are the outputs and the rows are places. An output is true when a place has one or more tokens. Values must be 0 or 1, any non zero number counts as 1. Can be null
 * @return pnet_t struct pointer
 */
pnet_t *pnet_new(
    pnet_arcs_map_t *pos_arcs_map, 
    pnet_arcs_map_t *neg_arcs_map, 
    pnet_arcs_map_t *inhibit_arcs_map, 
    pnet_arcs_map_t *reset_arcs_map,
    pnet_places_t *places_init, 
    pnet_transitions_t *transitions_delay,
    pnet_inputs_map_t *inputs_map,
    pnet_outputs_map_t *outputs_map
);

/**
 * @brief Create a new petri net, uses matrix_int_t as argument type. All values from the inputs are freed automatically
 * @param neg_arcs_map: matrix of arcs weight/direction, where the rows are the places and the columns are the transitions. Represents the number of tokens to be removed from a place. Only negative values. Can be null
 * @param pos_arcs_map: matrix of arcs weight/direction, where the rows are the places and the columns are the transitions. Represents the number of tokens to be moved onto a place. Only positive values. Can be null
 * @param inhibit_arcs_map: matrix of arcs, where the coluns are the places and the rows are the transitions. Dictates the firing of a transition when a place has zero tokens. Values must be 0 or 1, any non zero number counts as 1. Can be null
 * @param reset_arcs_map: matrix of arcs, where the coluns are the places and the rows are the transitions. When a transition occurs it zeroes out the place tokens. Values must be 0 or 1, any non zero number counts as 1. Can be null
 * @param places_init: matrix of values, where the columns are the places. The initial values for the places. Values must be a positive value. Must be not null
 * @param transitions_delay: matrix of values, were the columns are the transitions. While a place has enough tokens, the transitions will delay it's firing. Values must be positive, given is micro seconds (us). Can be null
 * @param inputs_map: matrix where the columns are the transitions and the rows are inputs. Represents the type of event that will fire that transistion, given by the enumerator pnet_event_t. Can be null
 * @param outputs_map: matrix where the columns are the outputs and the rows are places. An output is true when a place has one or more tokens. Values must be 0 or 1, any non zero number counts as 1. Can be null
 * @return pnet_t struct pointer
 */
pnet_t *m_pnet_new(
    matrix_int_t *pos_arcs_map, 
    matrix_int_t *neg_arcs_map, 
    matrix_int_t *inhibit_arcs_map, 
    matrix_int_t *reset_arcs_map,
    matrix_int_t *places_init, 
    matrix_int_t *transitions_delay,
    matrix_int_t *inputs_map,
    matrix_int_t *outputs_map
);

/**
 * @brief create new arcs map object. It's freed by the calls that receive it as argument
 * @param transitions_num: number of transitions for the petri net 
 * @param places_num: number of places for the petri net
 * @param ...: the values for each index in the matrix, comma separeted
 */
pnet_arcs_map_t *pnet_arcs_map_new(size_t transitions_num, size_t places_num, ...);

/**
 * @brief create new places init object. It's freed by the calls that receive it as argument
 * @param places_num: number of places for the petri net
 * @param ...: the values for each index in the matrix, comma separeted
 */
pnet_places_t *pnet_places_init_new(size_t places_num, ...);

/**
 * @brief create new transitions delay object. It's freed by the calls that receive it as argument
 * @param transitions_num: number of transitions for the petri net 
 * @param ...: the values for each index in the matrix, comma separeted
 */
pnet_transitions_t *pnet_transitions_delay_new(size_t transitions_num, ...);

/**
 * @brief create new inputs map object. It's freed by the calls that receive it as argument
 * @param transitions_num: number of transitions for the petri net 
 * @param inputs_num: number of inputs for the petri net
 * @param ...: the values for each index in the matrix, comma separeted
 */
pnet_inputs_map_t *pnet_inputs_map_new(size_t transitions_num, size_t inputs_num, ...);

/**
 * @brief create new outputs map object. It's freed by the calls that receive it as argument
 * @param outputs_num: number of outputs for the petri net 
 * @param places_num: number of places for the petri net
 * @param ...: the values for each index in the matrix, comma separeted
 */
pnet_outputs_map_t *pnet_outputs_map_new(size_t outputs_num, size_t places_num, ...);

/**
 * @brief create new inputs object. It's freed by the calls that receive it as argument
 * @param inputs_num: number of outputs for the petri net 
 * @param ...: the values for each index in the matrix, comma separeted
 */
pnet_inputs_t *pnet_inputs_new(size_t inputs_num, ...);

/**
 * @brief delete a pnet
 * @param pnet: the pnet struct pointer
 */
void pnet_delete(pnet_t *pnet);

/**
 * @brief fire the transitions based on the inputs and internal state. Sensitive transitions are NOT updated after firing
 * @param pnet: the pnet struct pointer
 * @param inputs: matrix of one row and columns the same size of the inputs given on pnet_new()
 */
void pnet_fire(pnet_t *pnet, pnet_inputs_t *inputs);

/**
 * @brief print the current state of the petri net to the stdout. Places, sensibilized transitions and outputs are printed
 * @param pnet: the pnet struct pointer
 */
void pnet_print(pnet_t *pnet);

/**
 * @brief compute if the transitions are sensibilized
 * @param pnet: the pnet struct pointer
 */
void pnet_sense(pnet_t *pnet);

#endif