/**
 * @mainpage
 * 
 * # pnet - a petri net library for C/C++
 * 
 * Easily make petri nets in C/C++. This library can create high level timed petri nets, with negated arcs and reset arcs, input events and outputs.
 * 
 * Is intended for embedding!
 * 
 * Created by João Peterson Scheffer - 2023. Version 1.1.1.
 * 
 * Licensed under the MIT License. Please reefer to the LICENSE file in the project root for license information.
 * 
 * # Usage
 * 
 * To start include the header file:
 * 
 * ```c
 * #include "pnet.h"
 * ```
 * 
 * The you can create a new petri net:
 * 
 * ```c
 * pnet_t *pnet = pnet_new(
 *     pnet_arcs_map_new(2,3,
 *         -1, 0,
 *          0, 0,
 *          0, 0
 *     ),
 *     pnet_arcs_map_new(2,3,
 *          0, 0,
 *          1, 0,
 *          0, 1
 *     ),
 *     pnet_arcs_map_new(2,3,
 *             0, 1,
 *             0, 0,
 *             0, 0
 *     ),
 *     pnet_arcs_map_new(2,3,
 *             0, 0,
 *             0, 1,
 *             0, 0
 *     ),
 *     pnet_places_init_new(3,
 *         1, 0, 0
 *     ),
 *     pnet_transitions_delay_new(2,
 *         0, 0
 *     ),
 *     pnet_inputs_map_new(2,2,
 *         pnet_event_none, pnet_event_pos_edge,
 *         pnet_event_none, pnet_event_none
 *     ),
 *     pnet_outputs_map_new(3,3,
 *         1,0,0,
 *         0,1,0,
 *         0,0,1
 *     ),
 *     NULL,
 *     NULL
 * ); 
 * ```
 * 
 * Note how you can pass weighted arcs, inhibit arcs, reset arcs, the initial tokens for the places, delay for transitions, inputs events and outputs in order, however, only the `places_init` and **at least one** type of arc are required, so very simple declarations can be made, like this:
 * 
 * ```c    
 * pnet_t *pnet = pnet_new(
 *     pnet_arcs_map_new(1,2,
 *         -1,
 *         0
 *     ),
 *     pnet_arcs_map_new(1,2,
 *         0, 
 *         1
 *     ),
 *     NULL,
 *     NULL,
 *     pnet_places_init_new(2,
 *         1, 0
 *     ),
 *     NULL,
 *     NULL,
 *     NULL,
 *     NULL,
 *     NULL
 * );
 * ```
 * 
 * To execute your petri net just call `pnet_fire`:
 * 
 * ```c
 * pnet_fire(pnet, pnet_inputs_new(2, 1,0))
 * ```
 * 
 * This will execute **one and only transition** at a time, so the execution is made in stepped manner.
 * 
 * ## Arguments
 * 
 * ### Weighted arcs
 * 
 * Weighted arcs define the amount of token that are consumed and given by some transition. Represented in matrix form by two matrices, it's shape should be like:
 * 
 * ```c
 * pnet_arcs_map_new(2,3,
 *     -1, 0,
 *      0, 0,
 *      0, 0
 * ),
 * pnet_arcs_map_new(2,3,
 *      0, 0,
 *      1, 0,
 *      0, 1
 * ),
 * 
 * ```
 * 
 * The first matrix are the positive weights, the second are the negative weights. Where 2 is the amount of transitions, and 3 the amount of places, notice how the columns represent the transitions and rows the places. In the example we are telling that for the first transition, 1 token will be consumed from the first place, and 1 token will be given to the second place, and for the second transition, a token will be given to the third place.
 * 
 * When there are no negative weights, a transition can fire at any time, so negative weights act as conditions/restrictions for a transition to fire.
 * 
 * ### Inhibit arcs
 * 
 * Inhibit arcs define that a transition shall occur when there are no token in the specified place. Represented in matrix form, values can be only 1 or 0:
 * 
 * ```c
 * pnet_arcs_map_new(2,3,
 *     0, 1,
 *     0, 0,
 *     0, 0
 * ),
 * ```
 * 
 * In the example we are saying that for the second transition to fire, no tokens can be present in the first place. This type of arc doesn't move any tokens like the reset or weighted arcs, it solely represents a condition, like the negative weights in the weighted arcs.
 * 
 * 
 * ### Reset arcs
 * 
 * Reset arcs express the act of setting the number of tokens in a place to 0 if the transition specified is fired. Represented in matrix form, values can be only 1 or 0:
 * 
 * ```c
 * pnet_arcs_map_new(2,3,
 *     0, 0,
 *     0, 1,
 *     0, 0
 * ),
 * ```
 * 
 * In the example we are saying that the second transition will reset the tokens in the second place when fired. This type of arc expresses change, like the weighted arcs, but no condition/restriction.
 * 
 * ### Input events
 * 
 * Inputs can be passed to `pnet_fire` and based on the events set by the `inputs_map` argument in `pnet_new` can dictate the triggering of transitions. 
 * 
 * `inputs_map` is given in matrix form, only the values of the enumerator `pnet_event_t` are valid.
 * 
 * ```c
 * pnet_inputs_map_new(2,2,
 *     pnet_event_none, pnet_event_pos_edge,
 *     pnet_event_none, pnet_event_none
 * ),
 * ```
 * 
 * The columns are the transitions and the rows are the inputs. 
 * 
 * Only one input event can be assigned to a single transition.
 * 
 * `pnet_event_none` and 0 are the same.
 * 
 * The events are as follow:
 * 
 * ```c
 * pnet_event_none         // no event, always trigger if arc conditions are met
 * pnet_event_pos_edge     // when input changes from 0 to 1 between fires
 * pnet_event_neg_edge     // when input changes from 1 to 0 between fires
 * pnet_event_any_edge     // when input changes value between fires
 * ```
 * 
 * Firing can be called with or without inputs:
 * 
 * ```c
 * pnet_fire(pnet, pnet_inputs_new(2, 1,0))
 * ```
 * 
 * ```c
 * pnet_fire(pnet, NULL)
 * ```
 * 
 * ### Delay
 * 
 * You can add delay to transitions by mapping the value in milliseconds to every transition, a 0 represents a instant transition. Given in matrix form, one row and the columns are the transitions.
 * 
 * ```c
 * // 500 ms delay on transition 0
 * pnet_transitions_delay_new(2,
 *     500, 0                                                      
 * ),
 * ```
 * 
 * Note that when using instant transitions, after the `pnet_fire()` call, the tokens would have moved already, but when using a delay you can only expect the net state after the define time, so to react accordingly you have to provide a callback, see section [Callback](#callback). When a callback is given it will be called after a delayed transition is fired.
 * 
 * ### Outputs
 * 
 * Outputs are given in matrix form, values can be only 1 and 0.
 * 
 * ```c
 * pnet_outputs_map_new(3,3,
 *     1,0,0,
 *     0,1,0,
 *     0,0,1
 * )
 * ```
 * 
 * The columns are the outputs and the rows are the places. A output is only 1 when there >0 tokens inside the respective place.
 * 
 * The state of the outputs can be accessed reading the `outputs` member of the `pnet_t` struct.
 * 
 * ```c
 * pnet->outputs
 * ```
 * 
 * ### Callback
 * 
 * A callback of type `pnet_callback_t` must be provided as argument when using timed transitions. **It will** be called after the execution of the delay for a given transition and that transition is still sensible. **It will not be called** when a instant transition was fired.
 * 
 * It's form is as follows:
 * 
 * ```c
 * void cb(pnet_t *pnet, size_t transition, void *data){
 *     your_data_struct *my_data = (your_data_struct*)data;
 *     
 *     // you code here
 * }
 * ```
 * 
 * You can access the pnet directly through the callback as well a user data passed in the `data` parameter on the `pnet_new()` and `m_pnet_new()` calls.
 * 
 * ## Error handling
 * 
 * Errors are bound to occur when defining the petri net, we can check for then by comparing the pointer return value from the calls and by using the `pnet_get_error` and `pnet_get_error_msg` calls.
 * 
 * ```c
 * pnet_t *pnet = pnet_new();
 * 
 * if(pnet == NULL || pnet_get_error() != pnet_info_ok){
 *     // error handling
 * }
 * 
 * // or 
 * 
 * if(pnet != NULL && pnet_get_error() == pnet_info_ok){
 *     // do petri net stuff 
 * }
 * ```
 * 
 * # Compile and install
 * 
 * Compilation is done by executing:
 * 
 * ```
 * $ make release
 * ```
 * 
 * Installation is done by executing:
 * 
 * ```
 * $ make install
 * ```
 * 
 * The location of installation can be configured in the [Makefile](Makefile) by altering the following variables:
 * 
 * ```
 * INSTALL_BIN_DIR
 * INSTALL_LIB_DIR
 * INSTALL_INC_DIR
 * ```
 * 
 * # Implementation details
 * 
 * This implementation uses matrix representation and custom independent algorithms by the author for sensing and firing the petri net.
 * 
 * # TODO
 * 
 * - Prioritized petri net, add priority to transitions
 * - Make special calls for reading the output, or make up another type of abstraction that don't involves matrix_int_t 
 * - Callback for output change
 * - Analysis tools
 * - PNML support
 * - Doxygen doc page
 * - Badges on readme
 * - Better abstraction for embedding purposes
 * - Timed implementation for embedded systems, custom timers
 * - Custom memory allocator option
 * - Compile macro for minimal build, used for excluding some logic checking and abstraction so the library could be compiled for minimal space keeping in mind embedded systems

 */

/**
 * @file pnet.h
 * 
 * pnet - easly make petri nets in C/C++ code. This library can create high level timed petri nets, with support for nesting,
 * negated arcs, reset arcs, inputs and outputs and tools for analisys, simulation and compiling petri nets to other forms of code.
 * Is intended for embedding!
 * 
 * Created by João Peterson Scheffer - 2023. Version 1.1.1.
 * 
 * Licensed under the MIT License. Please refeer to the LICENSE file in the project root for license information.
 * 
 * The main file on the lib, contains the main data structure for the petri net, types, error handling and functions to use on the petri net.
 */

#ifndef _PNET_HEADER_
#define _PNET_HEADER_

#include <stddef.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdbool.h>
#include <pthread.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include "pnet_matrix.h"
#include "queue.h"

// ------------------------------------------------------------ Error handling -----------------------------------------------------

/**
 * @brief enum of possible errors genrated bu the library
 */
typedef enum{
    pnet_info_ok = 0,
    pnet_info_no_neg_arcs_nor_inhibit_arcs_provided_no_transition_will_be_sensibilized,
    pnet_info_no_weighted_arcs_nor_reset_arcs_provided_no_token_will_be_moved_or_set,
    pnet_info_inputs_were_passed_but_no_input_map_was_set_when_the_petri_net_was_created,
    pnet_info_no_callback_function_was_passed_while_using_timed_transitions_watch_out,
    pnet_error_no_arcs_were_given,                                    
    pnet_error_place_init_must_have_only_one_row,                                    
    pnet_error_transitions_delay_must_have_only_one_row,               
    pnet_error_places_init_must_not_be_null,               
    pnet_error_pos_arcs_has_incorrect_number_of_places,               
    pnet_error_pos_arcs_has_incorrect_number_of_transitions,               
    pnet_error_inhibit_arcs_has_incorrect_number_of_places,               
    pnet_error_inhibit_arcs_has_incorrect_number_of_transitions,               
    pnet_error_reset_arcs_has_incorrect_number_of_places,               
    pnet_error_reset_arcs_has_incorrect_number_of_transitions,               
    pnet_error_places_init_has_incorrect_number_of_places_on_its_first_row,               
    pnet_error_transitions_delay_has_different_number_of_transitions_in_its_first_row_than_in_the_arcs,               
    pnet_error_inputs_has_different_number_of_transitions_in_its_first_row_than_in_the_arcs,               
    pnet_error_inputs_there_are_more_than_one_input_per_transition,               
    pnet_error_outputs_has_different_number_of_places_in_its_first_columns_than_in_the_arcs,
    pnet_error_pnet_struct_pointer_passed_as_argument_is_null,
    pnet_error_input_matrix_argument_size_doesnt_match_the_input_size_on_the_pnet_provided,
    pnet_error_thread_could_not_be_created,
    pnet_error_matrix_passed_is_null,
    pnet_error_matrix_index_x_y_out_of_range,
    pnet_error_matrices_should_be_of_the_same_size,
    pnet_error_matrices_should_be_square_matrices,
    pnet_error_matrices_should_be_tranposed_equivalents,
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

// ------------------------------------------------------------ Enumerators ----------------------------------------------------------

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

// ------------------------------------------------------------ Typedef's ----------------------------------------------------------

/**
 * @brief typedef for pnet_t struct
 */
typedef struct pnet_t pnet_t;

/**
 * @brief typedef for a callback function signature
 */
typedef void (*pnet_callback_t)(pnet_t *pnet, size_t transition, void *data);

// ------------------------------------------------------------ Structs ------------------------------------------------------------

/**
 * @brief pnet places list, created by calling pnet_places_init_new()
 */
typedef struct{
    pnet_matrix_t *values;
}pnet_places_t;

/**
 * @brief pnet transitions list, created by calling pnet_transitions_delay_new()
 */
typedef struct{
    pnet_matrix_t *values;
}pnet_transitions_t;

/**
 * @brief pnet arcs map list, created by calling pnet_arcs_map_new()
 */
typedef struct{
    pnet_matrix_t *values;
}pnet_arcs_map_t;

/**
 * @brief pnet inputs map list, created by calling pnet_inputs_map_new()
 */
typedef struct{
    pnet_matrix_t *values;
}pnet_inputs_map_t;

/**
 * @brief pnet outputs map list, created by calling pnet_outputs_map_new()
 */
typedef struct{
    pnet_matrix_t *values;
}pnet_outputs_map_t;

/**
 * @brief pnet inputs, created by calling pnet_inputs_new()
 */
typedef struct{
    pnet_matrix_t *values;
}pnet_inputs_t;

/**
 * @brief struct that represents a petri net
 */
struct pnet_t{
    // size
    size_t num_places;                                                              /**< The number of places in the petri net */
    size_t num_transitions;                                                         /**< The number of transitions in the petri net */
    size_t num_inputs;                                                              /**< The number of inputs in the petri net */
    size_t num_outputs;                                                             /**< The number of outputs in the petri net */

    // maps
    pnet_matrix_t *neg_arcs_map;                                                    /**< Matrix map of negative weighted arcs */            
    pnet_matrix_t *pos_arcs_map;                                                    /**< Matrix map of positive weighted arcs */            
    pnet_matrix_t *inhibit_arcs_map;                                                /**< Matrix map of inhibit arcs */                
    pnet_matrix_t *reset_arcs_map;                                                  /**< Matrix map of reset arcs */            
    pnet_matrix_t *places_init;                                                     /**< Matrix of the initial places tokens */            
    pnet_matrix_t *transitions_delay;                                               /**< Matrix map of transitions delays in milliseconds */                
    pnet_matrix_t *inputs_map;                                                      /**< Matrix map of inputs to transitions */        
    pnet_matrix_t *outputs_map;                                                     /**< Matrix map of places to outputs */        

    // validation
    bool valid;                                                                     /**< If true, the patri is able to to fire, if not the it doesnt. Call pnet_check() to validate beforehand */

    // net state
    pnet_matrix_t *places;                                                          /**< The actual places that hold tokens */
    pnet_matrix_t *sensitive_transitions;                                           /**< Currently firable transitions */

    // input edges state
    pnet_matrix_t *inputs_last;                                                     /**< The last state of the inputs, used to make edge events */

    // output values
    pnet_matrix_t *outputs;                                                         /**< The actual output values produced by the petri net */

    // async
    pnet_callback_t function;                                                       /**< Callback called by the timed thread on state change */
    void *user_data;                                                                /**< Data given by the user to passed on call to the callback function */
    pthread_t thread;                                                               /**< Thread used to time timed transitions */
    pthread_mutex_t lock;                                                           /**< Mutex used by the timed thread */
    transition_queue_t *transition_to_fire;                                         /**< Queue used to by the timed thread to fire transitions */
};

// ------------------------------------------------------------ Functions ------------------------------------------------------------

/**
 * @brief Create a new petri net. All values from the inputs are freed automatically
 * @param neg_arcs_map: matrix of arcs weight/direction, where the rows are the places and the columns are the transitions. Represents the number of tokens to be removed from a place. Only negative values. Can be null
 * @param pos_arcs_map: matrix of arcs weight/direction, where the rows are the places and the columns are the transitions. Represents the number of tokens to be moved onto a place. Only positive values. Can be null
 * @param inhibit_arcs_map: matrix of arcs, where the coluns are the places and the rows are the transitions. Dictates the firing of a transition when a place has zero tokens. Values must be 0 or 1, any non zero number counts as 1. Can be null
 * @param reset_arcs_map: matrix of arcs, where the coluns are the places and the rows are the transitions. When a transition occurs it zeroes out the place tokens. Values must be 0 or 1, any non zero number counts as 1. Can be null
 * @param places_init: matrix of values, where the columns are the places. The initial values for the places. Values must be a positive value. Must be not null
 * @param transitions_delay: matrix of values, were the columns are the transitions. While a place has enough tokens, the transitions will delay it's firing. Values must be positive, given in milli seconds (ms). Can be null
 * @param inputs_map: matrix where the columns are the transitions and the rows are inputs. Represents the type of event that will fire that transistion, given by the enumerator pnet_event_t. Can be null
 * @param outputs_map: matrix where the columns are the outputs and the rows are places. An output is true when a place has one or more tokens. Values must be 0 or 1, any non zero number counts as 1. Can be null
 * @param function: callback function of type pnet_callback_t that is called after firing operations asynchronously, useful for timed transitions. Can be NULL
 * @param data: data given by the user to passed on call to the callback function in it's data parameter. A void pointer. Can be NULL
 * @return pnet_t struct pointer
 */
pnet_t *pnet_new(
    pnet_arcs_map_t *neg_arcs_map, 
    pnet_arcs_map_t *pos_arcs_map, 
    pnet_arcs_map_t *inhibit_arcs_map, 
    pnet_arcs_map_t *reset_arcs_map,
    pnet_places_t *places_init, 
    pnet_transitions_t *transitions_delay,
    pnet_inputs_map_t *inputs_map,
    pnet_outputs_map_t *outputs_map,
    pnet_callback_t function,
    void *data
);

/**
 * @brief Create a new petri net, uses pnet_matrix_t as argument type. All values from the inputs are freed automatically
 * @param neg_arcs_map: matrix of arcs weight/direction, where the rows are the places and the columns are the transitions. Represents the number of tokens to be removed from a place. Only negative values. Can be null
 * @param pos_arcs_map: matrix of arcs weight/direction, where the rows are the places and the columns are the transitions. Represents the number of tokens to be moved onto a place. Only positive values. Can be null
 * @param inhibit_arcs_map: matrix of arcs, where the coluns are the places and the rows are the transitions. Dictates the firing of a transition when a place has zero tokens. Values must be 0 or 1, any non zero number counts as 1. Can be null
 * @param reset_arcs_map: matrix of arcs, where the coluns are the places and the rows are the transitions. When a transition occurs it zeroes out the place tokens. Values must be 0 or 1, any non zero number counts as 1. Can be null
 * @param places_init: matrix of values, where the columns are the places. The initial values for the places. Values must be a positive value. Must be not null
 * @param transitions_delay: matrix of values, were the columns are the transitions. While a place has enough tokens, the transitions will delay it's firing. Values must be positive, given in milli seconds (ms). Can be null
 * @param inputs_map: matrix where the columns are the transitions and the rows are inputs. Represents the type of event that will fire that transistion, given by the enumerator pnet_event_t. Can be null
 * @param outputs_map: matrix where the columns are the outputs and the rows are places. An output is true when a place has one or more tokens. Values must be 0 or 1, any non zero number counts as 1. Can be null
 * @param function: callback function of type pnet_callback_t that is called after firing operations asynchronously, useful for timed transitions. Can be NULL
 * @param data: data given by the user to passed on call to the callback function in it's data parameter. A void pointer. Can be NULL
 * @return pnet_t struct pointer
 */
pnet_t *m_pnet_new(
    pnet_matrix_t *neg_arcs_map, 
    pnet_matrix_t *pos_arcs_map, 
    pnet_matrix_t *inhibit_arcs_map, 
    pnet_matrix_t *reset_arcs_map,
    pnet_matrix_t *places_init,
    pnet_matrix_t *transitions_delay,
    pnet_matrix_t *inputs_map,
    pnet_matrix_t *outputs_map,
    pnet_callback_t function,
    void *data
);

/**
 * @brief Check if a given pnet is valid. This is a necessary call after creating a newe pnet, because if not called, you can't fire,
 * sense or print with this petri net.
 * @param pnet: pointer to a newly created pnet_t
 */
void pnet_check(pnet_t *pnet);

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
void m_pnet_fire(pnet_t *pnet, pnet_matrix_t *inputs);

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
 * @brief compute if the transitions are sensibilized. 
 * @param pnet: the pnet struct pointer
 */
void pnet_sense(pnet_t *pnet);

#endif