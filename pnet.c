#include "pnet.h"
#include "pnet_error.h"
#include "pnet_error_priv.h"

// #define _PNET_DEBUG_

// ------------------------------ Private functions --------------------------------

// creates a w_minus amtrix, that contains only information about the tokens to be removed after a transition
void pnet_w_minus_new(pnet_t *pnet){
    for(size_t i = 0; i < pnet->arcs_map->y; i++){
        for(size_t j = 0; j < pnet->arcs_map->x; j++){
            if(pnet->w_minus->m[i][j] >= 0)
                pnet->w_minus->m[i][j] = 0;
        }
    }
}

// validate input arguments to pnet_new() and get the number of places, transitions, inputs and outputs
bool validate_pnet_args(
    pnet_t *pnet, 
    pnet_arcs_map_t *arcs_map, 
    pnet_arcs_map_t *inhibit_arcs_map, 
    pnet_arcs_map_t *reset_arcs_map,
    pnet_places_t *places_init, 
    pnet_transitions_t *transitions_delay,
    pnet_inputs_map_t *inputs_map,
    pnet_outputs_map_t *outputs_map
){
    // check for non nullable values
    if(places_init == NULL){
        pnet_set_error(pnet_error_places_init_must_not_be_null);
        return true;
    }

    size_t transitions_num = 0;
    size_t places_num = 0;
    size_t inputs_num = 0;
    size_t outputs_num = 0;

    // checks for each argument
    if(arcs_map != NULL && transitions_num == 0 && places_num == 0){
        transitions_num = arcs_map->values->x;
        places_num = arcs_map->values->y;
    }
    
    if(inhibit_arcs_map != NULL){
        if(transitions_num == 0 && places_num == 0){
            transitions_num = inhibit_arcs_map->values->x;
            places_num = inhibit_arcs_map->values->y;
        }
        else{
            // check for incorrect size
            if(transitions_num != inhibit_arcs_map->values->x){
                pnet_set_error(pnet_error_inhibit_arcs_has_incorrect_number_of_transitions);
                return true;
            }
            if(places_num != inhibit_arcs_map->values->y){
                pnet_set_error(pnet_error_inhibit_arcs_has_incorrect_number_of_places);
                return true;
            }
        }

        // make all values 0 or 1 
        for(size_t i = 0; i < inhibit_arcs_map->values->y; i++){
            for(size_t j = 0; j < inhibit_arcs_map->values->x; j++){
                inhibit_arcs_map->values->m[i][j] = !!inhibit_arcs_map->values->m[i][j];
            }
        }
    }

    if(reset_arcs_map != NULL){
        if(transitions_num == 0 && places_num == 0){
            transitions_num = reset_arcs_map->values->x;
            places_num = reset_arcs_map->values->y;
        }
        else{
            // check for incorrect size
            if(transitions_num != reset_arcs_map->values->x){
                pnet_set_error(pnet_error_reset_arcs_has_incorrect_number_of_transitions);
                return true;
            }
            if(places_num != reset_arcs_map->values->y){
                pnet_set_error(pnet_error_reset_arcs_has_incorrect_number_of_places);
                return true;
            }
        }

        // make all values 0 or 1 
        for(size_t i = 0; i < reset_arcs_map->values->y; i++){
            for(size_t j = 0; j < reset_arcs_map->values->x; j++){
                reset_arcs_map->values->m[i][j] = !!reset_arcs_map->values->m[i][j];
            }
        }
    }

    // if no arcs given
    if(transitions_num == 0 && places_num == 0){
        pnet_set_error(pnet_error_no_arcs_were_given);
        return true;
    }
    
    if(places_init != NULL){
        if(transitions_num == 0 && places_num == 0){
            places_num = places_init->values->x;
        }
        else{
            // check for incorrect size
            if(places_num != places_init->values->x){
                pnet_set_error(pnet_error_places_init_has_incorrect_number_of_places_on_its_first_row);
                return true;
            }
        }
        
        // check if single row
        if(places_init->values->y != 1){
            pnet_set_error(pnet_error_place_init_must_have_only_one_row);
            return true;
        }

        // check for negative values
        for(size_t i = 0; i < places_init->values->y; i++){
            for(size_t j = 0; j < places_init->values->x; j++){
                if(places_init->values->m[i][j] < 0) 
                    places_init->values->m[i][j] = 0;
            }
        }
    }

    if(transitions_delay != NULL){
        // check for incorrect size
        if(transitions_num != transitions_delay->values->x){
            pnet_set_error(pnet_error_transitions_delay_has_different_number_of_transitions_in_its_first_row_than_in_the_arcs);
            return true;
        }
        // check if single row
        if(transitions_delay->values->y != 1){
            pnet_set_error(pnet_error_transitions_delay_must_have_only_one_row);
            return true;
        }

        // check for negative values
        for(size_t i = 0; i < transitions_delay->values->y; i++){
            for(size_t j = 0; j < transitions_delay->values->x; j++){
                if(transitions_delay->values->m[i][j] < 0) transitions_delay->values->m[i][j] = 0;
            }
        }
    }

    if(inputs_map != NULL){
        // check for incorrect size
        if(transitions_num != inputs_map->values->x){
            pnet_set_error(pnet_error_inputs_has_different_number_of_transitions_in_its_first_row_than_in_the_arcs);
            return true;
        }

        // check and correct invalid values for event_non pnet_event_t values
        for(size_t i = 0; i < inputs_map->values->y; i++){
            for(size_t j = 0; j < inputs_map->values->x; j++){
                if(inputs_map->values->m[i][j] < 0 || inputs_map->values->m[i][j] >= pnet_event_t_max) inputs_map->values->m[i][j] = pnet_event_none;
            }
        }

        // check for multiple inputs in the same transition
        for(size_t transition = 0; transition < inputs_map->values->x; transition++){
            bool flag = false;
            for(size_t input = 0; input < inputs_map->values->y; input++){

                // check if value is enum pnet_event_t
                if(
                    (inputs_map->values->m[input][transition] > pnet_event_none) && 
                    (inputs_map->values->m[input][transition] < pnet_event_t_max)
                ){
                    // if flag was marked true before, then this is another input to the same transition
                    if(flag){
                        pnet_set_error(pnet_error_inputs_there_are_more_than_one_input_per_transition);
                        return true;
                    }

                    flag = true;
                }
            }
        }

        inputs_num = inputs_map->values->y;
    }

    if(outputs_map != NULL){
        // check for incorrect size
        if(places_num != outputs_map->values->y){
            pnet_set_error(pnet_error_outputs_has_different_number_of_places_in_its_first_columns_than_in_the_arcs);
            return true;
        }

        // make all values 0 or 1 
        for(size_t i = 0; i < outputs_map->values->y; i++){
            for(size_t j = 0; j < outputs_map->values->x; j++){
                outputs_map->values->m[i][j] = !!outputs_map->values->m[i][j];
            }
        }

        outputs_num = outputs_map->values->x;
    }
    
    pnet->num_places = places_num;
    pnet->num_transitions = transitions_num;
    pnet->num_inputs = inputs_num;
    pnet->num_outputs = outputs_num;

    return false;
}

// check for sensibilized transitions 
void pnet_sense(pnet_t *pnet){
    if(pnet == NULL){
        pnet_set_error(pnet_error_pnet_struct_pointer_passed_as_argument_is_null);
        return;
    } 

    if(pnet->arcs_map == NULL && pnet->inhibit_arcs_map == NULL){                   // if no arcs to fire 
        pnet_set_error(pnet_info_no_weighted_arcs_nor_inhibit_arcs_provided_no_transition_will_be_sensibilized);
        return;
    } 

    for(size_t transition = 0; transition < pnet->num_transitions; transition++){
        pnet->sensitive_transitions->m[0][transition] = 1;                          // set transition to sensibilized
        for(size_t place = 0; place < pnet->num_places; place++){

            /**
             * to fire, sufficient tokens must be available, 
             * by adding the existing token to the required tokens, w_minus, we can compare to see
             * if the transition is firable, that is, after the sum, the transition column should
             * be zero or bigger, indicating that there are enought or more tokens to satisfy the
             * subtraction.
             */
            
            if(
                // weighted arcs
                (
                    (pnet->w_minus != NULL) &&                                                  // when there are weighted arcs
                    (pnet->w_minus->m[place][transition] != 0) &&                               // when there is a requirement for this transition/place
                    ((pnet->w_minus->m[place][transition] + pnet->places->m[0][place]) < 0)     // and there is not enough tokens
                ) ||
                // inhibit arcs          
                (
                    (pnet->inhibit_arcs_map != NULL) &&                                         // when there are inhibit arcs
                    (pnet->inhibit_arcs_map->m[place][transition] == 1) &&                      // if there is a inhibit arc
                    (pnet->places->m[0][place] != 0)                                            // and place has token, then do not trigger, otherwise trigger
                )
            ){
                pnet->sensitive_transitions->m[0][transition] = 0;                              // desensibilize
                break;
            }                                                                        
        }
    }
}

// process input data for edge events
matrix_int_t *pnet_input_detection(pnet_t *pnet, matrix_int_t *inputs){
    matrix_int_t *transitions = matrix_new_zero(pnet->num_transitions, 1);
    matrix_int_t *edges = matrix_new_zero(pnet->num_inputs, 1);

    // process inputs
    // only check for inputs when there are
    if(inputs != NULL){
        // run for every input given
        for(size_t input = 0; input < pnet->num_inputs; input++){
            // check for pos edges
            if(pnet->inputs_last->m[0][input] == 0 && inputs->m[0][input] == 1){
                edges->m[0][input] = pnet_event_pos_edge;
            }
            // check for neg edges
            else if(pnet->inputs_last->m[0][input] == 1 && inputs->m[0][input] == 0){
                edges->m[0][input] = pnet_event_neg_edge;
            }
        }

        // store last inputs
        matrix_copy(pnet->inputs_last, inputs);
    }

    // process wich transitions should be sensibilized
    // check edges againts input/transition map and set transitions to fire
    for(size_t transition = 0; transition < pnet->num_transitions; transition++){
        transitions->m[0][transition] = 0;
        
        // if input map is null all transitions can occurr
        if(pnet->inputs_map == NULL){
            transitions->m[0][transition] = 1;
            continue;
        }
        
        bool all_zero = false;
        for(size_t input = 0; input < pnet->num_inputs; input++){


            // if event type is none mark as firable, run until the end of inputs
            if(pnet->inputs_map->m[input][transition] == pnet_event_none){
                transitions->m[0][transition] = 1;
            }
            // if the transitions has an event. When a single event is found then this event must be satisfied, 
            // otherwise the transition stay desensibilized, so we exit the loop when we reach it
            else{
                // using the & operator to check edge type, see pnet_event_t for why
                if(pnet->inputs_map->m[input][transition] & edges->m[0][input]){
                    transitions->m[0][transition] = 1;
                }
                else{
                    transitions->m[0][transition] = 0;
                }

                break;
            }
        }
    }

    matrix_delete(edges);
    return transitions;
}

// set outputs accordingly to the places
void pnet_output_set(pnet_t *pnet){
    for(size_t output = 0; output < pnet->num_outputs; output++){
        pnet->outputs->m[0][output] = 0;
        for(size_t place = 0; place < pnet->num_places; place++){

            if(
                (pnet->outputs_map != NULL) &&
                (pnet->outputs_map->m[place][output] == 1) &&
                (pnet->places->m[0][place] > 0)
            ){
                pnet->outputs->m[0][output] = 1;
            }
        }
    }
}

// ------------------------------ Public functions ---------------------------------

// create pnet
pnet_t *pnet_new(
    pnet_arcs_map_t *arcs_map, 
    pnet_arcs_map_t *inhibit_arcs_map, 
    pnet_arcs_map_t *reset_arcs_map,
    pnet_places_t *places_init, 
    pnet_transitions_t *transitions_delay,
    pnet_inputs_map_t *inputs_map,
    pnet_outputs_map_t *outputs_map
){
    pnet_t *pnet = (pnet_t*)calloc(1, sizeof(pnet_t)); 

    // check for errors and get sizes
    if(validate_pnet_args(
        pnet,
        arcs_map, 
        inhibit_arcs_map, 
        reset_arcs_map,
        places_init, 
        transitions_delay,
        inputs_map,
        outputs_map
    )){
        free(pnet);
        return NULL;
    } 
    
    // copy matrixes
    if(arcs_map != NULL){
        pnet->arcs_map = arcs_map->values;  
        free(arcs_map);
    }
    if(inhibit_arcs_map != NULL){
        pnet->inhibit_arcs_map = inhibit_arcs_map->values;  
        free(inhibit_arcs_map);
    }
    if(reset_arcs_map != NULL){
        pnet->reset_arcs_map = reset_arcs_map->values; 
        free(reset_arcs_map);
    }
    if(places_init != NULL){
        pnet->places_init = places_init->values;  
        free(places_init);
    }
    if(transitions_delay != NULL){
        pnet->transitions_delay = transitions_delay->values; 
        free(transitions_delay);
    }
    if(inputs_map != NULL){
        pnet->inputs_map = inputs_map->values; 
        free(inputs_map);
    }
    if(outputs_map != NULL){
        pnet->outputs_map = outputs_map->values; 
        free(outputs_map);
    }

    // create subtration w_minus matrix
    if(pnet->arcs_map != NULL){
        pnet->w_minus = matrix_duplicate(pnet->arcs_map);
        pnet_w_minus_new(pnet);
    }

    pnet->places = matrix_duplicate(pnet->places_init);
    pnet->sensitive_transitions = matrix_new_zero(pnet->num_transitions, 1);
    pnet->inputs_last = matrix_new_zero(pnet->num_inputs, 1);
    pnet->outputs = matrix_new_zero(pnet->num_outputs, 1);

    pnet_set_error(pnet_info_ok);
    return pnet;
}

// create new arcs map object  
pnet_arcs_map_t *pnet_arcs_map_new(size_t transitions_num, size_t places_num, ...){
    va_list args;
    va_start(args, places_num);
    pnet_arcs_map_t *obj = (pnet_arcs_map_t*)calloc(1,sizeof(pnet_arcs_map_t));
    obj->values = v_matrix_new(transitions_num, places_num, &args);
    va_end(args);
    return obj;
}

// create new places init object  
pnet_places_t *pnet_places_init_new(size_t places_num, ...){
    va_list args;
    va_start(args, places_num);
    pnet_places_t *obj = (pnet_places_t*)calloc(1,sizeof(pnet_places_t));
    obj->values = v_matrix_new(places_num, 1, &args);
    va_end(args);
    return obj;
}

// create new transitions delay object  
pnet_transitions_t *pnet_transitions_delay_new(size_t transitions_num, ...){
    va_list args;
    va_start(args, transitions_num);
    pnet_transitions_t *obj = (pnet_transitions_t*)calloc(1,sizeof(pnet_transitions_t));
    obj->values = v_matrix_new(transitions_num, 1, &args);
    va_end(args);
    return obj;
}

// create new inputs map object  
pnet_inputs_map_t *pnet_inputs_map_new(size_t transitions_num, size_t inputs_num, ...){
    va_list args;
    va_start(args, inputs_num);
    pnet_inputs_map_t *obj = (pnet_inputs_map_t*)calloc(1,sizeof(pnet_inputs_map_t));
    obj->values = v_matrix_new(transitions_num, inputs_num, &args);
    va_end(args);
    return obj;
}

// create new outputs map object  
pnet_outputs_map_t *pnet_outputs_map_new(size_t outputs_num, size_t places_num, ...){
    va_list args;
    va_start(args, places_num);
    pnet_outputs_map_t *obj = (pnet_outputs_map_t*)calloc(1,sizeof(pnet_outputs_map_t));
    obj->values = v_matrix_new(outputs_num, places_num, &args);
    va_end(args);
    return obj;
}

// create new inputs object  
pnet_inputs_t *pnet_inputs_new(size_t inputs_num, ...){
    va_list args;
    va_start(args, inputs_num);
    pnet_inputs_t *obj = (pnet_inputs_t*)calloc(1,sizeof(pnet_inputs_t));
    obj->values = v_matrix_new(inputs_num, 1, &args);
    va_end(args);
    return obj;
}

// delete pnet
void pnet_delete(pnet_t *pnet){
    if(pnet == NULL){
        pnet_set_error(pnet_error_pnet_struct_pointer_passed_as_argument_is_null);
        return;
    } 
    matrix_delete(pnet->arcs_map); 
    matrix_delete(pnet->inhibit_arcs_map); 
    matrix_delete(pnet->reset_arcs_map);
    matrix_delete(pnet->places_init); 
    matrix_delete(pnet->transitions_delay);
    matrix_delete(pnet->inputs_map);
    matrix_delete(pnet->outputs_map);
    matrix_delete(pnet->w_minus);
    matrix_delete(pnet->places);
    matrix_delete(pnet->sensitive_transitions);
    matrix_delete(pnet->inputs_last);
    matrix_delete(pnet->outputs);
    free(pnet);
}

// fire the transitions
void pnet_fire(pnet_t *pnet, pnet_inputs_t *inputs){
    if(pnet == NULL){
        pnet_set_error(pnet_error_pnet_struct_pointer_passed_as_argument_is_null);
        return;
    } 

    if((inputs != NULL) && (inputs->values->x != pnet->num_inputs)){                                      // return if inputs is not the same size as needed
        pnet_set_error(pnet_error_input_matrix_argument_size_doesnt_match_the_input_size_on_the_pnet_provided);
        return;                                        
    }

    if(pnet->arcs_map == NULL && pnet->reset_arcs_map == NULL){                     // if no arcs, then no tokens will be moved/set
        pnet_set_error(pnet_info_no_weighted_arcs_nor_reset_arcs_provided_no_token_will_be_moved_or_set);
        return;
    }

    pnet_sense(pnet);

    // get input events, the result are the transitions that where activated by the configured input/transitions event type
    matrix_int_t *input_event_transitions;
    if(inputs != NULL){
        input_event_transitions = pnet_input_detection(pnet, inputs->values);
        matrix_delete(inputs->values);
        free(inputs);
    }
    else{
        input_event_transitions = pnet_input_detection(pnet, NULL);
    }

    #ifdef _PNET_DEBUG_
    matrix_print(pnet->sensitive_transitions, "sensitive_transitions");
    matrix_print(input_event_transitions, "input_event_transitions");
    #endif
    
    // transitions that are sensibilized and got the event 
    matrix_int_t *transitions_able_to_fire = matrix_and(input_event_transitions, pnet->sensitive_transitions);
    matrix_int_t *transitions_able_to_fire_T = matrix_transpose(transitions_able_to_fire);
    matrix_delete(transitions_able_to_fire);
    matrix_delete(input_event_transitions);

    #ifdef _PNET_DEBUG_
    matrix_print(transitions_able_to_fire_T, "transitions_able_to_fire_T");
    #endif

    matrix_int_t *places_res = matrix_duplicate(pnet->places);
    matrix_int_t *buffer = NULL;
    matrix_int_t *buffer2 = NULL;

    // for weighted arcs
    if(pnet->arcs_map != NULL){
        // multiply the transitions by the arcs matrix, the effect is that the result matrix gives
        // the amount of tokens to be added/subtracted by every place
        matrix_int_t *buffer = matrix_mul(pnet->arcs_map, transitions_able_to_fire_T);
        matrix_int_t *buffer2 = matrix_transpose(buffer);
        matrix_delete(buffer);

        // finnaly adding the difference in tokens for the places, effectly moving the tokens
        buffer = matrix_add(pnet->places, buffer2);
        matrix_copy(places_res, buffer);
        matrix_delete(buffer);
        matrix_delete(buffer2);

        #ifdef _PNET_DEBUG_
        matrix_print(places_res, "places_res");
        #endif
    }

    // for reset arcs
    if(pnet->reset_arcs_map != NULL){
        // multiply the transitions by the arcs matrix, the effect is that the result matrix marks
        // with 1, or more, if a place must be set to zero
        buffer = matrix_mul(pnet->reset_arcs_map, transitions_able_to_fire_T);
        buffer2 = matrix_transpose(buffer);
        matrix_delete(buffer);

        #ifdef _PNET_DEBUG_
        matrix_print(buffer2, "buffer2");
        #endif
        
        // negate so its a 0 for a place to be reset, in that way we can then multiply by it element by element, 
        // zeroing out the places where needed
        buffer = matrix_neg(buffer2);
        matrix_delete(buffer2); 
        buffer2 = matrix_mul_by_element(buffer, places_res);
        matrix_delete(buffer);
        matrix_copy(places_res, buffer2);
        matrix_delete(buffer2);
    }

    #ifdef _PNET_DEBUG_
    matrix_print(places_res, "places_res");
    #endif

    // copy values to the pnet
    matrix_copy(pnet->places, places_res);
    matrix_delete(places_res);
    matrix_delete(transitions_able_to_fire_T);

    // do the output logic
    pnet_output_set(pnet);

    pnet_set_error(pnet_info_ok);
}

// print the petri net 
void pnet_print(pnet_t *pnet){
    printf("################# Petri net #################\n");
    matrix_print(pnet->places, "state");
    printf("\n");
    matrix_print(pnet->sensitive_transitions, "sensible");
    printf("\n");
    matrix_print(pnet->outputs, "output");
    printf("#############################################\n");
}