#include "pnet.h"
#include "pnet_error.h"
#include "pnet_error_priv.h"

// #define _PNET_DEBUG_

// ------------------------------ Private functions --------------------------------

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

// move tokens around
void pnet_move(pnet_t *pnet){

    // will hold the results
    matrix_int_t *places_res = matrix_duplicate(pnet->places);
    matrix_int_t *arcs_transposed = NULL;
    matrix_int_t *buffer = NULL;
    matrix_int_t *buffer2 = NULL;

    // for weighted arcs, only if at least one of them is not null
    if(pnet->pos_arcs_map != NULL || pnet->neg_arcs_map != NULL){
        
        matrix_int_t *weighted_matrix = NULL;

        // sum pos and neg arcs first, if null then use the non null one
        if(pnet->pos_arcs_map != NULL && pnet->neg_arcs_map != NULL)
            weighted_matrix = matrix_add(pnet->neg_arcs_map, pnet->pos_arcs_map);
        else if(pnet->pos_arcs_map != NULL)
            weighted_matrix = matrix_duplicate(pnet->pos_arcs_map);
        else if(pnet->neg_arcs_map != NULL)
            weighted_matrix = matrix_duplicate(pnet->neg_arcs_map);

        // multiply the transitions by the arcs matrix, the effect is that the result matrix gives
        // the amount of tokens to be added/subtracted by every place
        arcs_transposed = matrix_transpose(weighted_matrix);
        matrix_int_t *buffer = matrix_mul(pnet->transition_to_fire, arcs_transposed);

        // finnaly adding the difference in tokens for the places, effectly moving the tokens
        buffer2 = matrix_add(pnet->places, buffer);
        matrix_copy(places_res, buffer2);
        matrix_delete(buffer);
        matrix_delete(buffer2);
        matrix_delete(weighted_matrix);
        matrix_delete(arcs_transposed);
    }

    // for reset arcs
    if(pnet->reset_arcs_map != NULL){
        // multiply the transitions by the arcs matrix, the effect is that the result matrix marks
        // with 1, or more, if a place must be set to zero
        arcs_transposed = matrix_transpose(pnet->reset_arcs_map);
        buffer = matrix_mul(pnet->transition_to_fire, arcs_transposed);

        // negate so its a 0 for a place to be reset, in that way we can then multiply by it element by element, 
        // zeroing out the places where needed
        buffer2 = matrix_neg(buffer);
        matrix_delete(buffer); 
        buffer = matrix_mul_by_element(buffer2, places_res);
        matrix_copy(places_res, buffer);

        matrix_delete(buffer);
        matrix_delete(buffer2);
        matrix_delete(arcs_transposed);
    }

    // copy values to the pnet
    matrix_copy(pnet->places, places_res);

    // do the output logic
    pnet_output_set(pnet);

    matrix_delete(places_res);
}

// timed thread cleanup function
static void timed_thread_cleanup(void *arg){
    clock_t *time = (clock_t*)arg;
    free(time);
    return;
}

// timed thread function
static void *timed_thread_main(void *arg){
    pnet_t *pnet = (pnet_t*)arg;
    clock_t *time = (clock_t*)calloc(pnet->num_transitions, sizeof(clock_t));

    pthread_cleanup_push(timed_thread_cleanup, time);

    while(1){
        pthread_testcancel();

        if(pnet->transitions_delay != NULL){
            for(size_t transition = 0; transition < pnet->num_transitions; transition++){
                // only on timed transitions
                if(pnet->transitions_delay->m[0][transition] == 0)
                    continue;

                // start counting
                if(pnet->transition_to_fire->m[0][transition] == 1 && time[transition] == 0){
                    time[transition] = clock();
                    pnet->transition_to_fire->m[0][transition] = 0;
                    continue;
                }

                // count
                if(time[transition] > 0){
                    clock_t now = clock();
                    // after elapsed time 
                    if( (CLOCK_TO_MS(now - time[transition])) >= pnet->transitions_delay->m[0][transition] ){
                        
                        time[transition] = 0;

                        // re check sensibility
                        pnet_sense(pnet);
                        if(pnet->sensitive_transitions->m[0][transition] == 1){
                            // move tokens
                            pnet_move(pnet);
                            // call callback
                            if(pnet->function != NULL) pnet->function(pnet);
                        }
                    }
                }
            }
        }
    }

    pthread_cleanup_pop(timed_thread_cleanup);
}

// validate input arguments to pnet_new() and get the number of places, transitions, inputs and outputs
bool validate_pnet_args(
    pnet_t *pnet, 
    matrix_int_t *neg_arcs_map, 
    matrix_int_t *pos_arcs_map, 
    matrix_int_t *inhibit_arcs_map, 
    matrix_int_t *reset_arcs_map,
    matrix_int_t *places_init, 
    matrix_int_t *transitions_delay,
    matrix_int_t *inputs_map,
    matrix_int_t *outputs_map
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

    // checks negative arcs
    if(neg_arcs_map != NULL && transitions_num == 0 && places_num == 0){
        transitions_num = neg_arcs_map->x;
        places_num = neg_arcs_map->y;

        // check for positive values
        for(size_t i = 0; i < neg_arcs_map->y; i++){
            for(size_t j = 0; j < neg_arcs_map->x; j++){
                if(neg_arcs_map->m[i][j] > 0) neg_arcs_map->m[i][j] = 0;
            }
        }
    }
    
    // positive arcs
    if(pos_arcs_map != NULL){
        if(transitions_num == 0 && places_num == 0){
            transitions_num = pos_arcs_map->x;
            places_num = pos_arcs_map->y;
        }
        else{
            // check for incorrect size
            if(transitions_num != pos_arcs_map->x){
                pnet_set_error(pnet_error_pos_arcs_has_incorrect_number_of_transitions);
                return true;
            }
            if(places_num != pos_arcs_map->y){
                pnet_set_error(pnet_error_pos_arcs_has_incorrect_number_of_places);
                return true;
            }
        }

        // check for positive values
        for(size_t i = 0; i < pos_arcs_map->y; i++){
            for(size_t j = 0; j < pos_arcs_map->x; j++){
                if(pos_arcs_map->m[i][j] < 0) pos_arcs_map->m[i][j] = 0;
            }
        }
    }

    // inhibit arcs
    if(inhibit_arcs_map != NULL){
        if(transitions_num == 0 && places_num == 0){
            transitions_num = inhibit_arcs_map->x;
            places_num = inhibit_arcs_map->y;
        }
        else{
            // check for incorrect size
            if(transitions_num != inhibit_arcs_map->x){
                pnet_set_error(pnet_error_inhibit_arcs_has_incorrect_number_of_transitions);
                return true;
            }
            if(places_num != inhibit_arcs_map->y){
                pnet_set_error(pnet_error_inhibit_arcs_has_incorrect_number_of_places);
                return true;
            }
        }

        // make all values 0 or 1 
        for(size_t i = 0; i < inhibit_arcs_map->y; i++){
            for(size_t j = 0; j < inhibit_arcs_map->x; j++){
                inhibit_arcs_map->m[i][j] = !!inhibit_arcs_map->m[i][j];
            }
        }
    }

    // reset arcs
    if(reset_arcs_map != NULL){
        if(transitions_num == 0 && places_num == 0){
            transitions_num = reset_arcs_map->x;
            places_num = reset_arcs_map->y;
        }
        else{
            // check for incorrect size
            if(transitions_num != reset_arcs_map->x){
                pnet_set_error(pnet_error_reset_arcs_has_incorrect_number_of_transitions);
                return true;
            }
            if(places_num != reset_arcs_map->y){
                pnet_set_error(pnet_error_reset_arcs_has_incorrect_number_of_places);
                return true;
            }
        }

        // make all values 0 or 1 
        for(size_t i = 0; i < reset_arcs_map->y; i++){
            for(size_t j = 0; j < reset_arcs_map->x; j++){
                reset_arcs_map->m[i][j] = !!reset_arcs_map->m[i][j];
            }
        }
    }

    // if no arcs given
    if(transitions_num == 0 && places_num == 0){
        pnet_set_error(pnet_error_no_arcs_were_given);
        return true;
    }

    // places init    
    if(places_init != NULL){
        if(transitions_num == 0 && places_num == 0){
            places_num = places_init->x;
        }
        else{
            // check for incorrect size
            if(places_num != places_init->x){
                pnet_set_error(pnet_error_places_init_has_incorrect_number_of_places_on_its_first_row);
                return true;
            }
        }
        
        // check if single row
        if(places_init->y != 1){
            pnet_set_error(pnet_error_place_init_must_have_only_one_row);
            return true;
        }

        // check for negative values
        for(size_t i = 0; i < places_init->y; i++){
            for(size_t j = 0; j < places_init->x; j++){
                if(places_init->m[i][j] < 0) 
                    places_init->m[i][j] = 0;
            }
        }
    }

    // transition delay
    if(transitions_delay != NULL){
        // check for incorrect size
        if(transitions_num != transitions_delay->x){
            pnet_set_error(pnet_error_transitions_delay_has_different_number_of_transitions_in_its_first_row_than_in_the_arcs);
            return true;
        }
        // check if single row
        if(transitions_delay->y != 1){
            pnet_set_error(pnet_error_transitions_delay_must_have_only_one_row);
            return true;
        }

        // check for negative values
        for(size_t i = 0; i < transitions_delay->y; i++){
            for(size_t j = 0; j < transitions_delay->x; j++){
                if(transitions_delay->m[i][j] < 0) transitions_delay->m[i][j] = 0;
            }
        }
    }

    // inputs
    if(inputs_map != NULL){
        // check for incorrect size
        if(transitions_num != inputs_map->x){
            pnet_set_error(pnet_error_inputs_has_different_number_of_transitions_in_its_first_row_than_in_the_arcs);
            return true;
        }

        // check and correct invalid values for event_non pnet_event_t values
        for(size_t i = 0; i < inputs_map->y; i++){
            for(size_t j = 0; j < inputs_map->x; j++){
                if(inputs_map->m[i][j] < 0 || inputs_map->m[i][j] >= pnet_event_t_max) inputs_map->m[i][j] = pnet_event_none;
            }
        }

        // check for multiple inputs in the same transition
        for(size_t transition = 0; transition < inputs_map->x; transition++){
            bool flag = false;
            for(size_t input = 0; input < inputs_map->y; input++){

                // check if value is enum pnet_event_t
                if(
                    (inputs_map->m[input][transition] > pnet_event_none) && 
                    (inputs_map->m[input][transition] < pnet_event_t_max)
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

        inputs_num = inputs_map->y;
    }

    // outputs
    if(outputs_map != NULL){
        // check for incorrect size
        if(places_num != outputs_map->y){
            pnet_set_error(pnet_error_outputs_has_different_number_of_places_in_its_first_columns_than_in_the_arcs);
            return true;
        }

        // make all values 0 or 1 
        for(size_t i = 0; i < outputs_map->y; i++){
            for(size_t j = 0; j < outputs_map->x; j++){
                outputs_map->m[i][j] = !!outputs_map->m[i][j];
            }
        }

        outputs_num = outputs_map->x;
    }
    
    // save sizes
    pnet->num_places = places_num;
    pnet->num_transitions = transitions_num;
    pnet->num_inputs = inputs_num;
    pnet->num_outputs = outputs_num;

    return false;
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

// ------------------------------ Public functions ---------------------------------

// create pnet
pnet_t *m_pnet_new(
    matrix_int_t *neg_arcs_map, 
    matrix_int_t *pos_arcs_map, 
    matrix_int_t *inhibit_arcs_map, 
    matrix_int_t *reset_arcs_map,
    matrix_int_t *places_init, 
    matrix_int_t *transitions_delay,
    matrix_int_t *inputs_map,
    matrix_int_t *outputs_map,
    pnet_callback_t function
){
    pnet_t *pnet = (pnet_t*)calloc(1, sizeof(pnet_t)); 

    pnet_set_error(pnet_info_ok);

    // check for errors and get sizes
    if(validate_pnet_args(
        pnet,
        neg_arcs_map, 
        pos_arcs_map, 
        inhibit_arcs_map, 
        reset_arcs_map,
        places_init, 
        transitions_delay,
        inputs_map,
        outputs_map
    )){
        matrix_delete(neg_arcs_map);
        matrix_delete(pos_arcs_map);
        matrix_delete(inhibit_arcs_map);
        matrix_delete(reset_arcs_map);
        matrix_delete(places_init);
        matrix_delete(transitions_delay);
        matrix_delete(inputs_map);
        matrix_delete(outputs_map);
        free(pnet);
        return NULL;
    } 

    // copy matrices
    pnet->neg_arcs_map = neg_arcs_map;  
    pnet->pos_arcs_map = pos_arcs_map;  
    pnet->inhibit_arcs_map = inhibit_arcs_map;  
    pnet->reset_arcs_map = reset_arcs_map; 
    pnet->places_init = places_init;  
    pnet->transitions_delay = transitions_delay; 
    pnet->inputs_map = inputs_map; 
    pnet->outputs_map = outputs_map; 

    pnet->places = matrix_duplicate(pnet->places_init);
    pnet->sensitive_transitions = matrix_new_zero(pnet->num_transitions, 1);
    pnet->inputs_last = matrix_new_zero(pnet->num_inputs, 1);
    pnet->outputs = matrix_new_zero(pnet->num_outputs, 1);
    pnet->transition_to_fire = matrix_new_zero(pnet->num_transitions, 1);

    if(transitions_delay != NULL && function == NULL){
        pnet_set_error(pnet_info_no_callback_function_was_passed_while_using_timed_transitions_watch_out);
    }

    // async
    pnet->function = function;
    int res = pthread_create(&(pnet->thread), NULL, timed_thread_main, pnet);

    // on thread create error
    if(res != 0){
        pnet_set_error(pnet_error_thread_could_not_be_created);
        pnet_set_error_msg("pthread_create could not create a new thread. LIBC: \"%s\"\n", strerror(errno));
        matrix_delete(neg_arcs_map);
        matrix_delete(pos_arcs_map);
        matrix_delete(inhibit_arcs_map);
        matrix_delete(reset_arcs_map);
        matrix_delete(places_init);
        matrix_delete(transitions_delay);
        matrix_delete(inputs_map);
        matrix_delete(outputs_map);
        free(pnet);
        return NULL;
    }

    return pnet;
}

// create pnet
pnet_t *pnet_new(
    pnet_arcs_map_t *neg_arcs_map, 
    pnet_arcs_map_t *pos_arcs_map, 
    pnet_arcs_map_t *inhibit_arcs_map, 
    pnet_arcs_map_t *reset_arcs_map,
    pnet_places_t *places_init, 
    pnet_transitions_t *transitions_delay,
    pnet_inputs_map_t *inputs_map,
    pnet_outputs_map_t *outputs_map,
    pnet_callback_t function
){

    pnet_t *pnet = m_pnet_new(
        neg_arcs_map        != NULL ? neg_arcs_map->values          : NULL,
        pos_arcs_map        != NULL ? pos_arcs_map->values          : NULL,
        inhibit_arcs_map    != NULL ? inhibit_arcs_map->values      : NULL,
        reset_arcs_map      != NULL ? reset_arcs_map->values        : NULL,
        places_init         != NULL ? places_init->values           : NULL,
        transitions_delay   != NULL ? transitions_delay->values     : NULL,
        inputs_map          != NULL ? inputs_map->values            : NULL,
        outputs_map         != NULL ? outputs_map->values           : NULL,
        function
    );

    if(neg_arcs_map != NULL){
        free(neg_arcs_map);
    }
    if(pos_arcs_map != NULL){
        free(pos_arcs_map);
    }
    if(inhibit_arcs_map != NULL){
        free(inhibit_arcs_map);
    }
    if(reset_arcs_map != NULL){
        free(reset_arcs_map);
    }
    if(places_init != NULL){
        free(places_init);
    }
    if(transitions_delay != NULL){
        free(transitions_delay);
    }
    if(inputs_map != NULL){
        free(inputs_map);
    }
    if(outputs_map != NULL){
        free(outputs_map);
    }
    
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
    // cancel thread before freeing stuff
    pthread_cancel(pnet->thread);
    pthread_join(pnet->thread,NULL);

    matrix_delete(pnet->pos_arcs_map); 
    matrix_delete(pnet->neg_arcs_map); 
    matrix_delete(pnet->inhibit_arcs_map); 
    matrix_delete(pnet->reset_arcs_map);
    matrix_delete(pnet->places_init); 
    matrix_delete(pnet->transitions_delay);
    matrix_delete(pnet->inputs_map);
    matrix_delete(pnet->outputs_map);
    matrix_delete(pnet->places);
    matrix_delete(pnet->sensitive_transitions);
    matrix_delete(pnet->inputs_last);
    matrix_delete(pnet->outputs);
    matrix_delete(pnet->transition_to_fire);
    free(pnet);
}

// check for sensibilized transitions, one transition is retuned at a time
void pnet_sense(pnet_t *pnet){
    if(pnet == NULL){
        pnet_set_error(pnet_error_pnet_struct_pointer_passed_as_argument_is_null);
        return;
    } 

    if(pnet->neg_arcs_map == NULL && pnet->inhibit_arcs_map == NULL){               // if no arcs to fire 
        pnet_set_error(pnet_info_no_neg_arcs_nor_inhibit_arcs_provided_no_transition_will_be_sensibilized);
        return;
    } 

    // zero sensibilized transitions
    matrix_set(pnet->sensitive_transitions, 0);

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
                // negative arcs
                (
                    (pnet->neg_arcs_map != NULL) &&                                                 // when there are negative arcs
                    (pnet->neg_arcs_map->m[place][transition] != 0) &&                              // when there is a requirement for this transition/place
                    ((pnet->neg_arcs_map->m[place][transition] + pnet->places->m[0][place]) < 0)    // and there is not enough tokens
                ) ||
                // inhibit arcs          
                (
                    (pnet->inhibit_arcs_map != NULL) &&                                             // when there are inhibit arcs
                    (pnet->inhibit_arcs_map->m[place][transition] == 1) &&                          // if there is a inhibit arc
                    (pnet->places->m[0][place] != 0)                                                // and place has token, then do not trigger, otherwise trigger
                )
            ){
                pnet->sensitive_transitions->m[0][transition] = 0;                                  // desensibilize
                break;
            }                                                                        
        }

        if(pnet->sensitive_transitions->m[0][transition] == 1) break;                               // return after the first transition is sensibilzed
    }
}

// fire the transitions
void pnet_fire(pnet_t *pnet, pnet_inputs_t *inputs){
    if(pnet == NULL){
        pnet_set_error(pnet_error_pnet_struct_pointer_passed_as_argument_is_null);
        matrix_delete(inputs->values);
        free(inputs);
        return;
    } 

    // if inputs were given but no input map was set
    if(
        (inputs != NULL) && 
        (pnet->num_inputs == 0)
    ){
        pnet_set_error(pnet_info_inputs_were_passed_but_no_input_map_was_set_when_the_petri_net_was_created);
    }
    else{
        pnet_set_error(pnet_info_ok);
    }

    // return if inputs is not the same size as needed
    if(
        (inputs != NULL) && 
        (pnet->num_inputs != 0) && 
        (inputs->values->x != pnet->num_inputs))
    {                
        pnet_set_error(pnet_error_input_matrix_argument_size_doesnt_match_the_input_size_on_the_pnet_provided);
        matrix_delete(inputs->values);
        free(inputs);
        return;                                        
    }

    // if no arcs, then no tokens will be moved/set
    if(                                                                             
        pnet->neg_arcs_map == NULL && 
        pnet->pos_arcs_map == NULL && 
        pnet->reset_arcs_map == NULL
    ){                 
        pnet_set_error(pnet_info_no_weighted_arcs_nor_reset_arcs_provided_no_token_will_be_moved_or_set);
        matrix_delete(inputs->values);
        free(inputs);
        return;
    }

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

    // sense transitions
    pnet_sense(pnet);
    
    // transitions that are sensibilized and got the event 
    matrix_int_t *transitions_able_to_fire = matrix_and(input_event_transitions, pnet->sensitive_transitions);
    // copy to pnet so the thread can fire them
    matrix_copy(pnet->transition_to_fire, transitions_able_to_fire);

    // fire on instant transitions
    for(size_t transition = 0; transition < pnet->num_transitions; transition++){
        if(
            (pnet->transition_to_fire->m[0][transition] == 1) &&                    // firable transition
            (
                (pnet->transitions_delay == NULL) ||                                // not timed
                (
                    (pnet->transitions_delay != NULL) &&                            // timed 
                    (pnet->transitions_delay->m[0][transition] == 0)                // but instant
                )
            )
        ){
            // move and callback
            pnet_move(pnet);        
            pnet->transition_to_fire->m[0][transition] = 0;
            if(pnet->function != NULL) pnet->function(pnet);
            break;
        }
    }

    #ifdef _PNET_DEBUG_
        matrix_print(transitions_able_to_fire, "transitions_able_to_fire");
    #endif
    matrix_delete(transitions_able_to_fire);
    matrix_delete(input_event_transitions);
}

// print the petri net 
void pnet_print(pnet_t *pnet){
    // sense before printing because sensibilized transitions are not updated after the last firing
    pnet_sense(pnet);                                                               

    printf("################# Petri net #################\n");
    matrix_print(pnet->places, "state");
    printf("\n");
    matrix_print(pnet->sensitive_transitions, "sensible");
    printf("\n");
    matrix_print(pnet->outputs, "output");
    printf("#############################################\n");
}