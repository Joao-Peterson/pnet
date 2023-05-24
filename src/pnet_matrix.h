/**
 * @file pnet_matrix.h
 * 
 * pnet - easly make petri nets in C/C++ code. This library can create high level timed petri nets, with support for nesting,
 * negated arcs, reset arcs, inputs and outputs and tools for analisys, simulation and compiling petri nets to other forms of code.
 * Is intended for embedding!
 * 
 * Created by {AUTHOR} - {YEAR}. Version {VERSION}.
 * 
 * Licensed under the MIT License. Please refeer to the LICENSE file in the project root for license information.
 * 
 * Pnet matrices are simple matrices made using dynamic allocated int's that are used inside the pnet_t type to represent places, 
 * transitions, inputs and outputs and all internal logic. Some helper function are implemented for internal use but exposed here if needed.  
 */

#ifndef _PNET_MATRIX_HEADER_
#define _PNET_MATRIX_HEADER_

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>

// ------------------------------ Structs ----------------------------------------

/**
 * @brief matrix of type int, can be constructed by calling pnet_matrix_new(), v_pnet_matrix_new() or pnet_matrix_new_zero()
 */
typedef struct{
    size_t x;                                                                       /**< matrix columns size */
    size_t y;                                                                       /**< matrix rows size */
    int **m;                                                                        /**< pointer to an array of size y that contains pointers to rows of size x */
}pnet_matrix_t;

// /**
//  * @brief matrix of string type, can be created by calling matrix_string_new() or v_matrix_string_new()
//  */
// typedef struct{
//     size_t x;
//     size_t y;
//     char ***m;
// }matrix_string_t;

// matrix_string_t *v_matrix_string_new(size_t x, size_t y, va_list *args);

// matrix_string_t *matrix_string_new(size_t x, size_t y, ...);

// void matrix_string_delete(matrix_string_t *matrix);

// void matrix_string_print(matrix_string_t *matrix, char *name);

/**
 * @brief creates a new matrix based a vlist of arguments and size
 */
pnet_matrix_t *v_pnet_matrix_new(size_t x, size_t y, va_list *args);

/**
 * @brief creates a new matrix given it's size and values as var args
 */
pnet_matrix_t *pnet_matrix_new(size_t x, size_t y, ...);

/**
 * @brief creates a new matrix fille dwith zeroes
 */
pnet_matrix_t *pnet_matrix_new_zero(size_t x, size_t y);

/**
 * @brief deletes a matrix
 */
void pnet_matrix_delete(pnet_matrix_t *matrix);

/**
 * @brief get a value from a matrix
 * @param m: pointer to matrix
 * @param x: column index
 * @param y: row index
 * @return 0 if value not found
 */
int pnet_matrix_get(pnet_matrix_t *m, size_t x, size_t y);

/**
 * @brief set a value on a matrix
 * @param m: pointer to matrix
 * @param x: column index
 * @param y: row index
 * @param value: new int value
 */
void pnet_matrix_set(pnet_matrix_t *m, size_t x, size_t y, int value);

/**
 * @brief prints a matrix in ascii form
 */
void pnet_matrix_print(pnet_matrix_t *matrix, char *name);

/**
 * @brief multiply two matrices and returns a new one
 */
pnet_matrix_t *pnet_matrix_mul(pnet_matrix_t *a, pnet_matrix_t *b);

/**
 * @brief multiply two matrices of same size element by element, returns a new one
 */
pnet_matrix_t *pnet_matrix_mul_by_element(pnet_matrix_t *a, pnet_matrix_t *b);

/**
 * @brief multiply a matrices by a scalar and returns a new one
 */
pnet_matrix_t *pnet_matrix_mul_scalar(pnet_matrix_t *a, int c);

/**
 * @brief adds two matrices and returns a new one
 */
pnet_matrix_t *pnet_matrix_add(pnet_matrix_t *a, pnet_matrix_t *b);

/**
 * @brief executes a and operations on two matrices of same size element by element, returns a new one
 */
pnet_matrix_t *pnet_matrix_and(pnet_matrix_t *a, pnet_matrix_t *b);

/**
 * @brief returns a new matrix that is the boolean negation of the input
 */
pnet_matrix_t *pnet_matrix_neg(pnet_matrix_t *a);

/**
 * @brief makes a new matrix which is a copy of the input
 */
pnet_matrix_t *pnet_matrix_duplicate(pnet_matrix_t *a);

/**
 * @brief copies the value of the src matrix to the dest matrix
 */
void pnet_matrix_copy(pnet_matrix_t *dest, pnet_matrix_t *src);

/**
 * @brief transpose a matrix and returns a new one
 */
pnet_matrix_t *pnet_matrix_transpose(pnet_matrix_t *matrix);

/**
 * @brief compares to see if two matrices are equal in a element by element manner
 */
bool pnet_matrix_cmp_eq(pnet_matrix_t *a, pnet_matrix_t *b);

/**
 * @brief sets all values of a matrix to the specified number 
 */
void pnet_matrix_set_all(pnet_matrix_t *m, int number);

/**
 * @brief returns a columns from matrix
 */
pnet_matrix_t *pnet_matrix_extract_col(pnet_matrix_t *m, size_t x);

#endif