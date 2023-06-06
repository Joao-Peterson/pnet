/**
 * @file crc32.h
 * 
 * pnet - easly make petri nets in C/C++ code. This library can create high level timed petri nets, with support for nesting,
 * negated arcs, reset arcs, inputs and outputs and tools for analisys, simulation and compiling petri nets to other forms of code.
 * Is intended for embedding!
 * 
 * Created by {AUTHOR} - {YEAR}. Version {VERSION}.
 * 
 * Licensed under the MIT License. Please refeer to the LICENSE file in the project root for license information.
 * 
 * Implementation of crc32 based on GNU glib/libibera
 */

#ifndef _CRC32_HEADER_
#define _CRC32_HEADER_

#include <stdint.h>
#include <string.h>

/**
 * @brief compute crc32
 * @details This CRC can be specified as:
 *   Width  : 32
 *   Poly   : 0x04c11db7
 *   Init   : parameter, typically 0xffffffff
 *   RefIn  : false
 *   RefOut : false
 *   XorOut : 0
 * 
 * This differs from the "standard" CRC-32 algorithm in that the values
 * are not reflected, and there is no final XOR value.  These differences
 * make it easy to compose the values of multiple blocks.
 * @note based on the Gnulib/libiberty implementation: https://github.com/gcc-mirror/gcc/blob/master/libiberty/crc32.c
 */
uint32_t crc32(uint8_t *buf, size_t len, uint32_t init);

#endif