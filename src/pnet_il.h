/**
 * @file pnet_il.h
 * 
 * pnet - easly make petri nets in C/C++ code. This library can create high level timed petri nets, with support for nesting,
 * negated arcs, reset arcs, inputs and outputs and tools for analisys, simulation and compiling petri nets to other forms of code.
 * Is intended for embedding!
 * 
 * Created by {AUTHOR} - {YEAR}. Version {VERSION}.
 * 
 * Licensed under the MIT License. Please refeer to the LICENSE file in the project root for license information.
 * 
 * This file contains compile utilities relating to IL (instruction list), a common standard for PLC's in industry
 */

#ifndef _PNET_IL_HEADER_
#define _PNET_IL_HEADER_

#include "pnet.h"

// ------------------------------------------------------------ Calls --------------------------------------------------------------

/**
 * @brief compile pnet to il weg tpw04. See il_weg_tpw04.c
 * @param pnet: a pointer to a pnet structure
 * @param input_offset: offset in memory for inputs X 
 * @param output_offset: offset in memory for outputs Y
 * @param trasnsitions_offset: offset in memory for transitions M 
 * @param places_offset: offset in memory for places D
 */
char *pnet_compile_il_weg_tpw04(pnet_t *pnet, int input_offset, int output_offset, int transition_offset, int place_offset, int timer_offset, int timer_min);

#endif