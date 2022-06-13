/**
 * @file matrix.h
 * 
 * pnet - easly make petri nets in C/C++ code. This library can create high level timed petri nets, with support for nesting,
 * negated arcs, reset arcs, inputs and outputs and tools for analisys, simulation and compiling petri nets to other forms of code.
 * Is intended for embedding!
 * Supports the PNML standard with read/write functions!
 * 
 * Created by João Peterson Scheffer - 2022. Version 1.0-0.
 * 
 * Licensed under the MIT License. Please refeer to the LICENSE file in the project root for license information.
 * 
 * This file was made as 
 */

#ifndef _PNET_ERROR_HEADER_
#define _PNET_ERROR_HEADER_

/**
 * @brief enum of possible errors genrated bu the library
 */
typedef enum{
    pnet_error_ok = 0,                                                              /**< Everything is OK */
    pnet_error_invalid_number_of_places_or_transitions                              /**< Places/trasitions have invalid value. They must be bigger than 0 */
}pnet_error_t;

/**
 * @brief get error code from latest execution
 * @return pnet_error_t enumerator type
 */
pnet_error_t pnet_get_error(void);

/**
 * @brief get error code message as string from latest execution
 * @return char* string. Do not free!
 */
char *pnet_get_error_msg(void);

#endif