# pnet - a petri net library for C/C++

Easly make petri nets in C/C++ code. This library can create high level timed petri nets, with support for nesting,
negated arcs, reset arcs, inputs and outputs and tools for analisys, simulation and compiling petri nets to other forms of code.

Is intended for embedding!

Supports the PNML standard with read/write functions!

Created by João Peterson Scheffer - 2022. Version 1.0-0.

Licensed under the MIT License. Please refeer to the LICENSE file in the project root for license information.

# TODO

- Don't make matrix.h visible to end user
- Make special calls for readind the output, or make up another type of abstraction that don't involves matrix_int_t 