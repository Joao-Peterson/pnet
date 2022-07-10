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

#ifndef _MATRIX_HEADER_
#define _MATRIX_HEADER_

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>

// ------------------------------ Structs ----------------------------------------

/**
 * @brief matrix of type int, can be constructed by calling matrix_new(), v_matrix_new() or matrix_new_zero()
 */
typedef struct{
    size_t x;                                                                       /**< matrix columns size */
    size_t y;                                                                       /**< matrix rows size */
    int **m;                                                                        /**< pointer to an array of size y that contains pointers to rows of size x */
}matrix_int_t;

/**
 * @brief matrix of string type, can be created by calling matrix_string_new() or v_matrix_string_new()
 */
typedef struct{
    size_t x;
    size_t y;
    char ***m;
}matrix_string_t;

matrix_string_t *v_matrix_string_new(size_t x, size_t y, va_list *args);

matrix_string_t *matrix_string_new(size_t x, size_t y, ...);

void matrix_string_delete(matrix_string_t *matrix);

void matrix_string_print(matrix_string_t *matrix, char *name);

matrix_int_t *v_matrix_new(size_t x, size_t y, va_list *args);

matrix_int_t *matrix_new(size_t x, size_t y, ...);

matrix_int_t *matrix_new_zero(size_t x, size_t y);

void matrix_delete(matrix_int_t *matrix);

void matrix_print(matrix_int_t *matrix, char *name);

matrix_int_t *matrix_mul(matrix_int_t *a, matrix_int_t *b);

matrix_int_t *matrix_mul_by_element(matrix_int_t *a, matrix_int_t *b);

matrix_int_t *matrix_mul_scalar(matrix_int_t *a, int c);

matrix_int_t *matrix_add(matrix_int_t *a, matrix_int_t *b);

matrix_int_t *matrix_and(matrix_int_t *a, matrix_int_t *b);

matrix_int_t *matrix_neg(matrix_int_t *a);

matrix_int_t *matrix_duplicate(matrix_int_t *a);

void matrix_copy(matrix_int_t *dest, matrix_int_t *src);

matrix_int_t *matrix_transpose(matrix_int_t *matrix);

bool matrix_cmp_eq(matrix_int_t *a, matrix_int_t *b);

#endif