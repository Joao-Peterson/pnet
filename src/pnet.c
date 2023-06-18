#include "pnet.h"
#include "pnet_error_priv.h"
#include "queue.h"
#include <string.h>

// ------------------------------ Private Types ------------------------------------

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
// thread safe
void pnet_move(pnet_t *pnet, size_t transition){

    pthread_mutex_lock(&(pnet->lock));
    
    // will hold the results
    pnet_matrix_t *places_res = pnet_matrix_duplicate(pnet->places);

    // for weighted arcs, only if at least one of them is not null
    if(pnet->pos_arcs_map != NULL || pnet->neg_arcs_map != NULL){
        
        pnet_matrix_t *weighted_matrix = NULL;
        pnet_matrix_t *pos_weighted_matrix = NULL;
        pnet_matrix_t *neg_weighted_matrix = NULL;

        // extract the values from the arcs for the transition 
        if(pnet->pos_arcs_map != NULL)
            pos_weighted_matrix = pnet_matrix_extract_col(pnet->pos_arcs_map, transition);
        if(pnet->neg_arcs_map != NULL)
            neg_weighted_matrix = pnet_matrix_extract_col(pnet->neg_arcs_map, transition);

        // sum the pos and neg values or returning only the non null maps  
        if(pos_weighted_matrix != NULL && neg_weighted_matrix != NULL){
            weighted_matrix = pnet_matrix_add(pos_weighted_matrix, neg_weighted_matrix);
            pnet_matrix_delete(pos_weighted_matrix);
            pnet_matrix_delete(neg_weighted_matrix);
        }
        else if(neg_weighted_matrix != NULL)
            weighted_matrix = neg_weighted_matrix;
        else
            weighted_matrix = pos_weighted_matrix;
            
        // finnaly adding the difference in tokens for the places, effectly moving the tokens
        pnet_matrix_t *buffer = pnet_matrix_add(places_res, weighted_matrix);
        pnet_matrix_copy(places_res, buffer);
        pnet_matrix_delete(weighted_matrix);
        pnet_matrix_delete(buffer);
    }

    // for reset arcs
    if(pnet->reset_arcs_map != NULL){
        // get places to reset for a given transition        
        pnet_matrix_t *places_reset = pnet_matrix_extract_col(pnet->reset_arcs_map, transition);

        // negate so its a 0 for a place to be reset, that way we can then multiply it with the places element by element, zeroing out the places where needed
        pnet_matrix_t *places_reset_neg = pnet_matrix_neg(places_reset);
        pnet_matrix_delete(places_reset);

        pnet_matrix_t *buffer = pnet_matrix_mul_by_element(places_res, places_reset_neg);
        pnet_matrix_copy(places_res, buffer);

        pnet_matrix_delete(places_reset_neg);
        pnet_matrix_delete(buffer);
    }

    // copy values to the pnet
    pnet_matrix_copy(pnet->places, places_res);

    // do the output logic
    pnet_output_set(pnet);

    pnet_matrix_delete(places_res);

    pthread_mutex_unlock(&(pnet->lock));
}

// timed thread cleanup function
// static void timed_thread_cleanup(void *arg){
//     return;
// }

// timed thread function
static void *timed_thread_main(void *arg){
    pnet_t *pnet = (pnet_t*)arg;

    // pthread_cleanup_push(timed_thread_cleanup, time);                               // exec cleanup after thread exits

    while(1){
        pthread_testcancel();

        transition_t transition;
        if(transition_queue_pop(pnet->transition_to_fire, &transition)){
            pnet_sense(pnet);
            if(pnet->sensitive_transitions->m[0][transition.transition] == 1){      // re check sensibility
                pnet_move(pnet, transition.transition);                             // FIRE!! move tokens and call callback
                if(pnet->function != NULL) 
                    pnet->function(pnet, transition.transition, pnet->user_data);
            }
        }
    }

    // pthread_cleanup_pop(timed_thread_cleanup);

    return NULL;
}

// process input data for edge events
pnet_matrix_t *pnet_input_detection(pnet_t *pnet, pnet_matrix_t *inputs){
    pnet_matrix_t *transitions = pnet_matrix_new_zero(pnet->num_transitions, 1);
    pnet_matrix_t *edges = pnet_matrix_new_zero(pnet->num_inputs, 1);

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
        pnet_matrix_copy(pnet->inputs_last, inputs);
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

    pnet_matrix_delete(edges);
    return transitions;
}

// ------------------------------ Public functions ---------------------------------

// create pnet from matrices
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
){
    pnet_t *pnet = (pnet_t*)calloc(1, sizeof(pnet_t)); 

    pnet_set_error(pnet_info_ok);

    // copy matrices
    pnet->neg_arcs_map = neg_arcs_map;  
    pnet->pos_arcs_map = pos_arcs_map;  
    pnet->inhibit_arcs_map = inhibit_arcs_map;  
    pnet->reset_arcs_map = reset_arcs_map; 
    pnet->places_init = places_init;  
    pnet->transitions_delay = transitions_delay; 
    pnet->inputs_map = inputs_map; 
    pnet->outputs_map = outputs_map; 

    // check for errors and get sizes
    pnet_check(pnet);
    if(pnet->valid == false){
        pnet_matrix_delete(neg_arcs_map);
        pnet_matrix_delete(pos_arcs_map);
        pnet_matrix_delete(inhibit_arcs_map);
        pnet_matrix_delete(reset_arcs_map);
        pnet_matrix_delete(places_init);
        pnet_matrix_delete(transitions_delay);
        pnet_matrix_delete(inputs_map);
        pnet_matrix_delete(outputs_map);
        free(pnet);
        return NULL;
    } 

    pnet->places = pnet_matrix_duplicate(pnet->places_init);
    pnet->sensitive_transitions = pnet_matrix_new_zero(pnet->num_transitions, 1);
    pnet->inputs_last = pnet->num_inputs ? pnet_matrix_new_zero(pnet->num_inputs, 1) : NULL;
    pnet->outputs = pnet->num_outputs ? pnet_matrix_new_zero(pnet->num_outputs, 1) : NULL;
    
    if(transitions_delay != NULL && function == NULL){
        pnet_set_error(pnet_info_no_callback_function_was_passed_while_using_timed_transitions_watch_out);
    }

    // async
    pnet->function = function;
    pnet->user_data = data;
    pnet->transition_to_fire = transition_queue_new();
    pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
    pnet->lock = lock;
    int res = pthread_create(&(pnet->thread), NULL, timed_thread_main, pnet);

    // on thread create error
    if(res != 0){
        pnet_set_error(pnet_error_thread_could_not_be_created);
        pnet_set_error_msg("pthread_create could not create a new thread. LIBC: \"%s\"\n", strerror(errno));
        pnet_matrix_delete(neg_arcs_map);
        pnet_matrix_delete(pos_arcs_map);
        pnet_matrix_delete(inhibit_arcs_map);
        pnet_matrix_delete(reset_arcs_map);
        pnet_matrix_delete(places_init);
        pnet_matrix_delete(transitions_delay);
        pnet_matrix_delete(inputs_map);
        pnet_matrix_delete(outputs_map);
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
    pnet_callback_t function,
    void *data
){

    // create with matrices
    pnet_t *pnet = m_pnet_new(
        neg_arcs_map        != NULL ? neg_arcs_map->values          : NULL,
        pos_arcs_map        != NULL ? pos_arcs_map->values          : NULL,
        inhibit_arcs_map    != NULL ? inhibit_arcs_map->values      : NULL,
        reset_arcs_map      != NULL ? reset_arcs_map->values        : NULL,
        places_init         != NULL ? places_init->values           : NULL,
        transitions_delay   != NULL ? transitions_delay->values     : NULL,
        inputs_map          != NULL ? inputs_map->values            : NULL,
        outputs_map         != NULL ? outputs_map->values           : NULL,
        function,
        data
    );

    // free input structs
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

// validate a given pnet
void pnet_check(pnet_t *pnet){
    // if null
    if(pnet == NULL){
        pnet_set_error(pnet_error_pnet_struct_pointer_passed_as_argument_is_null);
        return;
    }

    // if already checked
    if(pnet->valid == true){
        pnet_set_error(pnet_info_ok);
        return;
    } 
    
    // check for non nullable values
    if(pnet->places_init == NULL){
        pnet_set_error(pnet_error_places_init_must_not_be_null);
        pnet->valid = false;
        return;
    }

    pnet->valid = true;
    pnet_set_error(pnet_info_ok);

    size_t transitions_num = 0;
    size_t places_num = 0;
    size_t inputs_num = 0;
    size_t outputs_num = 0;

    // checks negative arcs
    if(pnet->neg_arcs_map != NULL && transitions_num == 0 && places_num == 0){
        transitions_num = pnet->neg_arcs_map->x;
        places_num = pnet->neg_arcs_map->y;

        // check for positive values
        for(size_t i = 0; i < pnet->neg_arcs_map->y; i++){
            for(size_t j = 0; j < pnet->neg_arcs_map->x; j++){
                if(pnet->neg_arcs_map->m[i][j] > 0) pnet->neg_arcs_map->m[i][j] = 0;
            }
        }
    }
    
    // positive arcs
    if(pnet->pos_arcs_map != NULL){
        if(transitions_num == 0 && places_num == 0){
            transitions_num = pnet->pos_arcs_map->x;
            places_num = pnet->pos_arcs_map->y;
        }
        else{
            // check for incorrect size
            if(transitions_num != pnet->pos_arcs_map->x){
                pnet_set_error(pnet_error_pos_arcs_has_incorrect_number_of_transitions);
                pnet->valid = false;
            }
            if(places_num != pnet->pos_arcs_map->y){
                pnet_set_error(pnet_error_pos_arcs_has_incorrect_number_of_places);
                pnet->valid = false;
            }
        }

        // check for positive values
        for(size_t i = 0; i < pnet->pos_arcs_map->y; i++){
            for(size_t j = 0; j < pnet->pos_arcs_map->x; j++){
                if(pnet->pos_arcs_map->m[i][j] < 0) pnet->pos_arcs_map->m[i][j] = 0;
            }
        }
    }

    // inhibit arcs
    if(pnet->inhibit_arcs_map != NULL){
        if(transitions_num == 0 && places_num == 0){
            transitions_num = pnet->inhibit_arcs_map->x;
            places_num = pnet->inhibit_arcs_map->y;
        }
        else{
            // check for incorrect size
            if(transitions_num != pnet->inhibit_arcs_map->x){
                pnet_set_error(pnet_error_inhibit_arcs_has_incorrect_number_of_transitions);
                pnet->valid = false;
            }
            if(places_num != pnet->inhibit_arcs_map->y){
                pnet_set_error(pnet_error_inhibit_arcs_has_incorrect_number_of_places);
                pnet->valid = false;
            }
        }

        // make all values 0 or 1 
        for(size_t i = 0; i < pnet->inhibit_arcs_map->y; i++){
            for(size_t j = 0; j < pnet->inhibit_arcs_map->x; j++){
                pnet->inhibit_arcs_map->m[i][j] = !!pnet->inhibit_arcs_map->m[i][j];
            }
        }
    }

    // reset arcs
    if(pnet->reset_arcs_map != NULL){
        if(transitions_num == 0 && places_num == 0){
            transitions_num = pnet->reset_arcs_map->x;
            places_num = pnet->reset_arcs_map->y;
        }
        else{
            // check for incorrect size
            if(transitions_num != pnet->reset_arcs_map->x){
                pnet_set_error(pnet_error_reset_arcs_has_incorrect_number_of_transitions);
                pnet->valid = false;
            }
            if(places_num != pnet->reset_arcs_map->y){
                pnet_set_error(pnet_error_reset_arcs_has_incorrect_number_of_places);
                pnet->valid = false;
            }
        }

        // make all values 0 or 1 
        for(size_t i = 0; i < pnet->reset_arcs_map->y; i++){
            for(size_t j = 0; j < pnet->reset_arcs_map->x; j++){
                pnet->reset_arcs_map->m[i][j] = !!pnet->reset_arcs_map->m[i][j];
            }
        }
    }

    // if no arcs given
    if(transitions_num == 0 && places_num == 0){
        pnet_set_error(pnet_error_no_arcs_were_given);
        pnet->valid = false;
    }

    // places init    
    if(pnet->places_init != NULL){
        if(transitions_num == 0 && places_num == 0){
            places_num = pnet->places_init->x;
        }
        else{
            // check for incorrect size
            if(places_num != pnet->places_init->x){
                pnet_set_error(pnet_error_places_init_has_incorrect_number_of_places_on_its_first_row);
                pnet->valid = false;
            }
        }
        
        // check if single row
        if(pnet->places_init->y != 1){
            pnet_set_error(pnet_error_place_init_must_have_only_one_row);
            pnet->valid = false;
        }

        // check for negative values
        for(size_t i = 0; i < pnet->places_init->y; i++){
            for(size_t j = 0; j < pnet->places_init->x; j++){
                if(pnet->places_init->m[i][j] < 0) 
                    pnet->places_init->m[i][j] = 0;
            }
        }
    }

    // transition delay
    if(pnet->transitions_delay != NULL){
        // check for incorrect size
        if(transitions_num != pnet->transitions_delay->x){
            pnet_set_error(pnet_error_transitions_delay_has_different_number_of_transitions_in_its_first_row_than_in_the_arcs);
            pnet->valid = false;
        }
        // check if single row
        if(pnet->transitions_delay->y != 1){
            pnet_set_error(pnet_error_transitions_delay_must_have_only_one_row);
            pnet->valid = false;
        }

        // check for negative values
        for(size_t i = 0; i < pnet->transitions_delay->y; i++){
            for(size_t j = 0; j < pnet->transitions_delay->x; j++){
                if(pnet->transitions_delay->m[i][j] < 0) pnet->transitions_delay->m[i][j] = 0;
            }
        }
    }

    // inputs
    if(pnet->inputs_map != NULL){
        // check for incorrect size
        if(transitions_num != pnet->inputs_map->x){
            pnet_set_error(pnet_error_inputs_has_different_number_of_transitions_in_its_first_row_than_in_the_arcs);
            pnet->valid = false;
        }

        // check and correct invalid values for event_non pnet_event_t values
        for(size_t i = 0; i < pnet->inputs_map->y; i++){
            for(size_t j = 0; j < pnet->inputs_map->x; j++){
                if(pnet->inputs_map->m[i][j] < 0 || pnet->inputs_map->m[i][j] >= pnet_event_t_max) pnet->inputs_map->m[i][j] = pnet_event_none;
            }
        }

        // check for multiple inputs in the same transition
        for(size_t transition = 0; transition < pnet->inputs_map->x; transition++){
            bool flag = false;
            for(size_t input = 0; input < pnet->inputs_map->y; input++){

                // check if value is enum pnet_event_t
                if(
                    (pnet->inputs_map->m[input][transition] > pnet_event_none) && 
                    (pnet->inputs_map->m[input][transition] < pnet_event_t_max)
                ){
                    // if flag was marked true before, then this is another input to the same transition
                    if(flag){
                        pnet_set_error(pnet_error_inputs_there_are_more_than_one_input_per_transition);
                        pnet->valid = false;
                    }

                    flag = true;
                }
            }
        }

        inputs_num = pnet->inputs_map->y;
    }

    // outputs
    if(pnet->outputs_map != NULL){
        // check for incorrect size
        if(places_num != pnet->outputs_map->y){
            pnet_set_error(pnet_error_outputs_has_different_number_of_places_in_its_first_columns_than_in_the_arcs);
            pnet->valid = false;
        }

        // make all values 0 or 1 
        for(size_t i = 0; i < pnet->outputs_map->y; i++){
            for(size_t j = 0; j < pnet->outputs_map->x; j++){
                pnet->outputs_map->m[i][j] = !!pnet->outputs_map->m[i][j];
            }
        }

        outputs_num = pnet->outputs_map->x;
    }
    
    // save sizes
    pnet->num_places = places_num;
    pnet->num_transitions = transitions_num;
    pnet->num_inputs = inputs_num;
    pnet->num_outputs = outputs_num;
}

// create new arcs map object  
pnet_arcs_map_t *pnet_arcs_map_new(size_t transitions_num, size_t places_num, ...){
    va_list args;
    va_start(args, places_num);
    pnet_arcs_map_t *obj = (pnet_arcs_map_t*)calloc(1,sizeof(pnet_arcs_map_t));
    obj->values = v_pnet_matrix_new(transitions_num, places_num, &args);
    va_end(args);
    return obj;
}

// create new places init object  
pnet_places_t *pnet_places_init_new(size_t places_num, ...){
    va_list args;
    va_start(args, places_num);
    pnet_places_t *obj = (pnet_places_t*)calloc(1,sizeof(pnet_places_t));
    obj->values = v_pnet_matrix_new(places_num, 1, &args);
    va_end(args);
    return obj;
}

// create new transitions delay object  
pnet_transitions_t *pnet_transitions_delay_new(size_t transitions_num, ...){
    va_list args;
    va_start(args, transitions_num);
    pnet_transitions_t *obj = (pnet_transitions_t*)calloc(1,sizeof(pnet_transitions_t));
    obj->values = v_pnet_matrix_new(transitions_num, 1, &args);
    va_end(args);
    return obj;
}

// create new inputs map object  
pnet_inputs_map_t *pnet_inputs_map_new(size_t transitions_num, size_t inputs_num, ...){
    va_list args;
    va_start(args, inputs_num);
    pnet_inputs_map_t *obj = (pnet_inputs_map_t*)calloc(1,sizeof(pnet_inputs_map_t));
    obj->values = v_pnet_matrix_new(transitions_num, inputs_num, &args);
    va_end(args);
    return obj;
}

// create new outputs map object  
pnet_outputs_map_t *pnet_outputs_map_new(size_t outputs_num, size_t places_num, ...){
    va_list args;
    va_start(args, places_num);
    pnet_outputs_map_t *obj = (pnet_outputs_map_t*)calloc(1,sizeof(pnet_outputs_map_t));
    obj->values = v_pnet_matrix_new(outputs_num, places_num, &args);
    va_end(args);
    return obj;
}

// create new inputs object  
pnet_inputs_t *pnet_inputs_new(size_t inputs_num, ...){
    va_list args;
    va_start(args, inputs_num);
    pnet_inputs_t *obj = (pnet_inputs_t*)calloc(1,sizeof(pnet_inputs_t));
    obj->values = v_pnet_matrix_new(inputs_num, 1, &args);
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
    pthread_mutex_destroy(&(pnet->lock));

    pnet_matrix_delete(pnet->pos_arcs_map); 
    pnet_matrix_delete(pnet->neg_arcs_map); 
    pnet_matrix_delete(pnet->inhibit_arcs_map); 
    pnet_matrix_delete(pnet->reset_arcs_map);
    pnet_matrix_delete(pnet->places_init); 
    pnet_matrix_delete(pnet->transitions_delay);
    pnet_matrix_delete(pnet->inputs_map);
    pnet_matrix_delete(pnet->outputs_map);
    pnet_matrix_delete(pnet->places);
    pnet_matrix_delete(pnet->sensitive_transitions);
    pnet_matrix_delete(pnet->inputs_last);
    pnet_matrix_delete(pnet->outputs);
    transition_queue_destroy(pnet->transition_to_fire);
    free(pnet);
}

// check for sensibilized transitions
// thread safe
void pnet_sense(pnet_t *pnet){
    if(pnet == NULL){
        pnet_set_error(pnet_error_pnet_struct_pointer_passed_as_argument_is_null);
        return;
    } 

    if(pnet->neg_arcs_map == NULL && pnet->inhibit_arcs_map == NULL){               // if no arcs to fire 
        pnet_set_error(pnet_info_no_neg_arcs_nor_inhibit_arcs_provided_no_transition_will_be_sensibilized);
        return;
    } 

    pthread_mutex_lock(&(pnet->lock));

    // zero sensibilized transitions
    pnet_matrix_set_all(pnet->sensitive_transitions, 0);

    for(size_t transition = 0; transition < pnet->num_transitions; transition++){
        pnet->sensitive_transitions->m[0][transition] = 1;                                          // set transition to sensibilized
        for(size_t place = 0; place < pnet->num_places; place++){

            /**
             * to fire, sufficient tokens must be available, 
             * by adding the existing token to the required tokens, we can compare to see
             * if the transition is firable, that is, after the sum, the transition column should
             * be zero or bigger, indicating that there are enought or more tokens to satisfy the
             * subtraction.
             */
            
            if(                                                                                     // check desensibilization
                // negative arcs
                (
                    (pnet->neg_arcs_map != NULL) &&                                                 // when there are negative arcs
                    (pnet->neg_arcs_map->m[place][transition] != 0) &&                              // when there is a neg arc for this transition/place
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

        // if(pnet->sensitive_transitions->m[0][transition] == 1) break;                               // return after the first transition is sensibilzed
    }

    pthread_mutex_unlock(&(pnet->lock));
}

// fire the transitions
void m_pnet_fire(pnet_t *pnet, pnet_matrix_t *inputs){
    if(pnet == NULL){
        pnet_set_error(pnet_error_pnet_struct_pointer_passed_as_argument_is_null);
        pnet_matrix_delete(inputs);
        return;
    } 

    // if inputs were given but no input map was set
    if(
        (inputs != NULL) && 
        (pnet->num_inputs == 0)
    ){
        pnet_set_error(pnet_info_inputs_were_passed_but_no_input_map_was_set_when_the_petri_net_was_created);
        pnet_matrix_delete(inputs);
        return;
    }
    else{
        pnet_set_error(pnet_info_ok);
    }

    // return if inputs is not the same size as needed
    if(
        (inputs != NULL) && 
        (pnet->num_inputs != 0) && 
        (inputs->x != pnet->num_inputs))
    {                
        pnet_set_error(pnet_error_input_matrix_argument_size_doesnt_match_the_input_size_on_the_pnet_provided);
        pnet_matrix_delete(inputs);
        return;                                        
    }

    // if no arcs, then no tokens will be moved/set
    if(                                                                             
        pnet->neg_arcs_map == NULL && 
        pnet->pos_arcs_map == NULL && 
        pnet->reset_arcs_map == NULL
    ){                 
        pnet_set_error(pnet_info_no_weighted_arcs_nor_reset_arcs_provided_no_token_will_be_moved_or_set);
        pnet_matrix_delete(inputs);
        return;
    }

    // get input events, the result are the transitions that where activated by the configured input/transitions event type
    pnet_matrix_t *input_event_transitions;
    if(inputs != NULL){
        input_event_transitions = pnet_input_detection(pnet, inputs);
        pnet_matrix_delete(inputs);
    }
    else{
        input_event_transitions = pnet_input_detection(pnet, NULL);
    }

    // sense transitions
    pnet_sense(pnet);

    // transitions that are sensibilized and got the event 
    pnet_matrix_t *transitions_able_to_fire = pnet_matrix_and(input_event_transitions, pnet->sensitive_transitions);

    // fire transitions
    for(size_t transition = 0; transition < pnet->num_transitions; transition++){
        if(transitions_able_to_fire->m[0][transition] == 1){                        // firable transition
            if(
                (pnet->transitions_delay == NULL) ||                                // not timed
                (
                    (pnet->transitions_delay != NULL) &&                            // timed 
                    (pnet->transitions_delay->m[0][transition] == 0)                // but instant
                )
            ){
                // move and callback
                pnet_move(pnet, transition);        
                if(pnet->function != NULL) pnet->function(pnet, transition, pnet->user_data);
                break;                                                              // only one instant transitions
            }
            else{
                // add to queue
                transition_queue_push(pnet->transition_to_fire, transition, pnet->transitions_delay->m[0][transition]);
            }

        }
    }

    pnet_matrix_delete(transitions_able_to_fire);
    pnet_matrix_delete(input_event_transitions);
}

// fire the transitions
void pnet_fire(pnet_t *pnet, pnet_inputs_t *inputs){
    m_pnet_fire(pnet, inputs != NULL ? inputs->values : NULL);
    free(inputs);
}

// reset pnet state
void pnet_reset(pnet_t *pnet){
    if(pnet == NULL){
        pnet_set_error(pnet_error_pnet_struct_pointer_passed_as_argument_is_null);
        return;
    };

    if(pnet->places != NULL)                pnet_matrix_copy(pnet->places_init, pnet->places);
    if(pnet->inputs_last != NULL)           pnet_matrix_set_all(pnet->inputs_last, 0);
    if(pnet->outputs != NULL)               pnet_matrix_set_all(pnet->outputs, 0);
    if(pnet->sensitive_transitions != NULL) pnet_matrix_set_all(pnet->sensitive_transitions, 0);
}

// print the petri net 
void pnet_print(pnet_t *pnet){
    // sense before printing because sensibilized transitions are not updated after the last firing
    pnet_sense(pnet);                                                               

    printf("################# Petri net #################\n");
    pnet_matrix_print(pnet->places, "state");
    printf("\n");
    pnet_matrix_print(pnet->sensitive_transitions, "sensible");
    printf("\n");
    pnet_matrix_print(pnet->outputs, "output");
    printf("#############################################\n");
}
