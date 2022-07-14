#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdbool.h>
#include "pnet_error.h"
#include "pnet_error_priv.h"

// put enum name to string
#define PNET_DEF_ERR(enum) #enum

// global error code
static pnet_error_t error = pnet_info_ok;

// global error message
char* error_msg = NULL;
bool error_msg_free = false;

// static array error message list
static const char* error_msg_list[] = {
    PNET_DEF_ERR(pnet_info_ok),
    PNET_DEF_ERR(pnet_info_no_neg_arcs_nor_inhibit_arcs_provided_no_transition_will_be_sensibilized),
    PNET_DEF_ERR(pnet_info_no_weighted_arcs_nor_reset_arcs_provided_no_token_will_be_moved_or_set),
    PNET_DEF_ERR(pnet_info_inputs_were_passed_but_no_input_map_was_set_when_the_petri_net_was_created),
    PNET_DEF_ERR(pnet_error_no_arcs_were_given),
    PNET_DEF_ERR(pnet_error_place_init_must_have_only_one_row),
    PNET_DEF_ERR(pnet_error_transitions_delay_must_have_only_one_row),
    PNET_DEF_ERR(pnet_error_places_init_must_not_be_null),
    PNET_DEF_ERR(pnet_error_pos_arcs_has_incorrect_number_of_places),
    PNET_DEF_ERR(pnet_error_pos_arcs_has_incorrect_number_of_transitions),
    PNET_DEF_ERR(pnet_error_inhibit_arcs_has_incorrect_number_of_places),
    PNET_DEF_ERR(pnet_error_inhibit_arcs_has_incorrect_number_of_transitions),
    PNET_DEF_ERR(pnet_error_reset_arcs_has_incorrect_number_of_places),
    PNET_DEF_ERR(pnet_error_reset_arcs_has_incorrect_number_of_transitions),
    PNET_DEF_ERR(pnet_error_places_init_has_incorrect_number_of_places_on_its_first_row),
    PNET_DEF_ERR(pnet_error_transitions_delay_has_different_number_of_transitions_in_its_first_row_than_in_the_arcs),
    PNET_DEF_ERR(pnet_error_inputs_has_different_number_of_transitions_in_its_first_row_than_in_the_arcs),
    PNET_DEF_ERR(pnet_error_inputs_there_are_more_than_one_input_per_transition),
    PNET_DEF_ERR(pnet_error_outputs_has_different_number_of_places_in_its_first_columns_than_in_the_arcs),
    PNET_DEF_ERR(pnet_error_pnet_struct_pointer_passed_as_argument_is_null),
    PNET_DEF_ERR(pnet_error_input_matrix_argument_size_doesnt_match_the_input_size_on_the_pnet_provided)
};

// return global error code
pnet_error_t pnet_get_error(void){
    return error;
}

// return global error message
char *pnet_get_error_msg(void){
    return error_msg;
}

// set global error
void pnet_set_error(pnet_error_t code){
    error = code;

    if(error_msg != NULL && error_msg_free){
        free(error_msg);    
    }
    
    error_msg = (char*)error_msg_list[code];
    error_msg_free = false;
}

// set global error message
void pnet_set_error_msg(char* format, ...){
    va_list args;
    va_start(args, format);

    if(error_msg != NULL && error_msg_free){
        free(error_msg);
    }

    error_msg = calloc(ERROR_MSG_MAX_LEN + 1, sizeof(char));
    vsnprintf(error_msg, ERROR_MSG_MAX_LEN, format, args);
    error_msg_free = true;

    va_end(args);
}
