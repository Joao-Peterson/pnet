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

typedef enum{
    end_options_pnet    = -1,
    places_name_pnet,
    places_init_pnet,
    transitions_pnet,
    arcs_pnet,
    inhibit_arcs_pnet,
    conditions_name_pnet,
    input_name_pnet,
    output_name_pnet,
    conditions_map_pnet,
    input_map_pnet,
    output_map_pnet,
    transitions_delay_pnet,
    pnet_options_max
}pnet_options_t;

typedef enum{
    pos_edge_pnet               = 0x01,
    rise_edge_pnet              = 0x01,
    neg_edge_pnet               = 0x02,
    fall_edge_pnet              = 0x02,
    any_edge_pnet               = 0x04,
    toggle_up_down_pnet         = 0x08,
    change_up_down_pnet         = 0x08,
    toggle_down_up_pnet         = 0x10,
    change_down_up_pnet         = 0x10,
    toggle_pnet                 = 0x20,
    change_pnet                 = 0x20,
    toggle_any_pnet             = 0x20,
    change_any_pnet             = 0x20
}pnet_edge_t;

typedef struct{
    matrix_string_t *places_name;
    matrix_string_t *transitions;
    matrix_string_t *conditions_name;
    matrix_string_t *input_name;
    matrix_string_t *output_name;

    matrix_int_t *init_state;
    matrix_int_t *arcs;
    matrix_int_t *arcs_inhibit;
    matrix_int_t *transitions_delay;
    matrix_int_t *input;
    matrix_int_t *input_edge;
    matrix_int_t *input_last;
    matrix_int_t *input_last_edge;
    matrix_int_t *conditions;
    matrix_int_t *output;
    matrix_int_t *input_map;
    matrix_int_t *conditions_map;
    matrix_int_t *output_map;

    matrix_int_t *w_minus;
    matrix_int_t *places;
    matrix_int_t *sensitive_transitions;
}pnet_t;

void pnet_w_minus_new(pnet_t *pnet);

void pnet_delete(pnet_t *pnet);

void pnet_print(pnet_t *pnet);

void pnet_sense(pnet_t *pnet);

matrix_int_t *pnet_input_detection(pnet_t *pnet);

void pnet_output_set(pnet_t *pnet);

void pnet_fire(pnet_t *pnet, matrix_int_t *input, matrix_int_t *conditions);

pnet_t *pnet_new(size_t places_num, size_t transitions_num, size_t conditions_num, size_t inputs_num, size_t output_num, ...);

#endif