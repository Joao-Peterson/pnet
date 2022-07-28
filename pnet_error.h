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