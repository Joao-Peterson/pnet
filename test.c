#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "pnet.h"

// global test count
size_t test_counter = 1;

//failed test count
size_t fail_counter = 0;

// test call. dummy its just to receive __COUNTER__
void test_call(bool condition, char* text, char *file, int line, int dummy);

// summary of succeeded and failed tests
void test_summary(void);

// takes a condition to test, if true passes if false fails. COUNTER increments with every call
#define test(condition, text) test_call(condition, text, __FILE__, __LINE__, __COUNTER__);

int main(int argc, char **argv){

    // #############################################################################
    // simple petrinet with two places and a single transition between them
    pnet_t *pnet = pnet_new(
        pnet_arcs_map_new(1,2,
            -1,
             1
        ),
        NULL,
        NULL,
        pnet_places_init_new(2,
            1, 0
        ),
        NULL,
        NULL,
        NULL
    );
    test((pnet != NULL) && (pnet_get_error() == pnet_info_ok), "Test for non null return and err code ok");
    if(pnet != NULL) pnet_delete(pnet);

    // #############################################################################
    // simple petrinet with 3 places and 2 transitions between them, uses all avaiable arcs types and inputs/outputs
    pnet = pnet_new(
        pnet_arcs_map_new(2,3,
            -1, 0,
             1, 0,
             0, 1
        ),
        pnet_arcs_map_new(2,3,
             0, 1,
             0, 0,
             0, 0
        ),
        pnet_arcs_map_new(2,3,
             0, 0,
             0, 1,
             0, 0
        ),
        pnet_places_init_new(3,
            1, 0, 0
        ),
        pnet_transitions_delay_new(2,
            0, 0
        ),
        pnet_inputs_map_new(2,2,
            pnet_event_none, pnet_event_pos_edge,
            0,0
        ),
        pnet_outputs_map_new(3,3,
            1,0,0,
            0,1,0,
            0,0,1
        )
    );
    test((pnet != NULL) && (pnet_get_error() == pnet_info_ok), "Test for creation with all arguments");
    if(pnet != NULL) pnet_delete(pnet);

    // #############################################################################
    // test no args
    pnet = pnet_new(
        pnet_arcs_map_new(1,2,
            -1,
             1
        ),
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL
    );
    test((pnet == NULL) && (pnet_get_error() == pnet_error_places_init_must_not_be_null), "Test for all null args");
    if(pnet != NULL) pnet_delete(pnet);

    // #############################################################################
    // test with no arcs
    pnet = pnet_new(
        NULL,
        NULL,
        NULL,
        pnet_places_init_new(5, 1,0,0,0,0),
        NULL,
        NULL,
        NULL
    );
    test((pnet == NULL) && (pnet_get_error() == pnet_error_no_arcs_were_given), "Test for only places, no arcs");
    if(pnet != NULL) pnet_delete(pnet);

    // #############################################################################
    // Test for autocorrection of values 
    pnet = pnet_new(
        pnet_arcs_map_new(2,3,
            -1, 0,
             1, 0,
             0, 1
        ),
        pnet_arcs_map_new(2,3,
            -5, 1,
             0, 10,
             0, 0
        ),
        pnet_arcs_map_new(2,3,
             0, 0,
            -15, 1,
             0, 30
        ),
        pnet_places_init_new(3,
            1, -10, 0
        ),
        pnet_transitions_delay_new(2,
            -30, 0
        ),
        pnet_inputs_map_new(2,2,
            -15, 34,
            0,0
        ),
        pnet_outputs_map_new(3,3,
            -11,0,0,
            0,14,0,
            0,0,1
        )
    );

    matrix_int_t *inhibit_arcs_map = matrix_new(2,3, 1,1, 0,1, 0,0);
    matrix_int_t *reset_arcs_map = matrix_new(2,3, 0,0, 1,1, 0,1);
    matrix_int_t *places_init = matrix_new(3,1, 1,0,0);
    matrix_int_t *transitions_delay = matrix_new(2,1, 0,0);
    matrix_int_t *inputs_map = matrix_new(2,2, 0,0,0,0);
    matrix_int_t *outputs_map = matrix_new(3,3, 1,0,0, 0,1,0, 0,0,1);

    test(
        (pnet != NULL) && 
        (pnet_get_error() == pnet_info_ok) &&
        matrix_cmp_eq(pnet->inhibit_arcs_map, inhibit_arcs_map) &&
        matrix_cmp_eq(pnet->reset_arcs_map, reset_arcs_map) &&
        matrix_cmp_eq(pnet->places_init, places_init) &&
        matrix_cmp_eq(pnet->transitions_delay, transitions_delay) &&
        matrix_cmp_eq(pnet->inputs_map, inputs_map) &&
        matrix_cmp_eq(pnet->outputs_map, outputs_map),
        "Test for autocorrection of values "
    );
    
    if(pnet != NULL) pnet_delete(pnet);
    matrix_delete(inhibit_arcs_map);
    matrix_delete(reset_arcs_map);
    matrix_delete(places_init);
    matrix_delete(transitions_delay);
    matrix_delete(inputs_map);
    matrix_delete(outputs_map);

    // #############################################################################
    // test for no weighted or inhibit arcs
    pnet = pnet_new(
        NULL,
        NULL,
        pnet_arcs_map_new(2,3,
             0, 0,
             0, 1,
             0, 0
        ),
        pnet_places_init_new(3,
            1, 0, 0
        ),
        pnet_transitions_delay_new(2,
            0, 0
        ),
        pnet_inputs_map_new(2,2,
            pnet_event_none, pnet_event_pos_edge,
            0,0
        ),
        pnet_outputs_map_new(3,3,
            1,0,0,
            0,1,0,
            0,0,1
        )
    );

    if(pnet != NULL){
        pnet_sense(pnet);
    }

    test(
        (pnet != NULL) && 
        (pnet_get_error() == pnet_info_no_weighted_arcs_nor_inhibit_arcs_provided_no_transition_will_be_sensibilized), 
        "Test for no weighted or inhibit arcs"
    );
    if(pnet != NULL) pnet_delete(pnet);

    // #############################################################################
    // Test for multiple inputs on a single transition
    pnet = pnet_new(
        NULL,
        NULL,
        pnet_arcs_map_new(2,3,
             0, 0,
             0, 1,
             0, 0
        ),
        pnet_places_init_new(3,
            1, 0, 0
        ),
        pnet_transitions_delay_new(2,
            0, 0
        ),
        pnet_inputs_map_new(2,2,
            pnet_event_none, pnet_event_pos_edge,
            pnet_event_none, pnet_event_any_edge
        ),
        pnet_outputs_map_new(3,3,
            1,0,0,
            0,1,0,
            0,0,1
        )
    );

    test(
        (pnet == NULL) && 
        (pnet_get_error() == pnet_error_inputs_there_are_more_than_one_input_per_transition), 
        "Test for multiple inputs on a single transition"
    );
    if(pnet != NULL) pnet_delete(pnet);

    // #############################################################################
    // Test sense 1
    pnet = pnet_new(
        pnet_arcs_map_new(2,3,
            -1, 0,
             1, 0,
             0, 1
        ),
        pnet_arcs_map_new(2,3,
             0, 1,
             0, 0,
             0, 0
        ),
        pnet_arcs_map_new(2,3,
             0, 0,
             0, 1,
             0, 0
        ),
        pnet_places_init_new(3,
            1, 0, 0
        ),
        pnet_transitions_delay_new(2,
            0, 0
        ),
        pnet_inputs_map_new(2,2,
            pnet_event_none, pnet_event_pos_edge,
            0,0
        ),
        pnet_outputs_map_new(3,3,
            1,0,0,
            0,1,0,
            0,0,1
        )
    );

    if(pnet != NULL){
        pnet_sense(pnet);
    }

    matrix_int_t *sense = matrix_new(2, 1,  1,0);

    test(
        (pnet != NULL) && 
        (pnet_get_error() == pnet_info_ok) &&
        (matrix_cmp_eq(pnet->sensitive_transitions, sense)),
        "Test sense 1"
    );

    if(pnet != NULL) pnet_delete(pnet);
    matrix_delete(sense);

    // #############################################################################
    // Test fire without weighted or reset arcs
    pnet = pnet_new(
        NULL,
        pnet_arcs_map_new(2,3,
             0, 1,
             0, 0,
             0, 0
        ),
        NULL,
        pnet_places_init_new(3,
            1, 0, 0
        ),
        pnet_transitions_delay_new(2,
            0, 0
        ),
        pnet_inputs_map_new(2,2,
            pnet_event_none, pnet_event_pos_edge,
            0,0
        ),
        pnet_outputs_map_new(3,3,
            1,0,0,
            0,1,0,
            0,0,1
        )
    );

    if(pnet != NULL){
        pnet_fire(pnet, pnet_inputs_new(2,
            0,0
        ));
    }

    test(
        (pnet != NULL) && 
        (pnet_get_error() == pnet_info_no_weighted_arcs_nor_reset_arcs_provided_no_token_will_be_moved_or_set),
        "Test fire without weighted or reset arcs"
    );
    
    if(pnet != NULL) pnet_delete(pnet);

    // #############################################################################
    // Test firing logic
    pnet = pnet_new(
        pnet_arcs_map_new(2,3,
            -1, 0,
             1, 0,
             0, 1
        ),
        pnet_arcs_map_new(2,3,
             0, 1,
             0, 0,
             0, 0
        ),
        pnet_arcs_map_new(2,3,
             0, 0,
             0, 1,
             0, 0
        ),
        pnet_places_init_new(3,
            1, 0, 0
        ),
        pnet_transitions_delay_new(2,
            0, 0
        ),
        pnet_inputs_map_new(2,2,
            pnet_event_none, pnet_event_pos_edge,
            pnet_event_none, pnet_event_none
        ),
        pnet_outputs_map_new(3,3,
            1,0,0,
            0,1,0,
            0,0,1
        )
    );

    // Test firing of transition without input map
    pnet_fire(pnet, pnet_inputs_new(2, 0,0));
    // matrix_print(pnet->places,"places");
    matrix_int_t *places = matrix_new(3,1, 0,1,0);

    test(
        (pnet != NULL) && 
        (pnet_get_error() == pnet_info_ok) &&
        (matrix_cmp_eq(pnet->places, places)),
        "Test firing of transition without input map"
    );

    matrix_delete(places);

    // Test missfire
    pnet_fire(pnet, pnet_inputs_new(2, 0,0));
    places = matrix_new(3,1, 0,1,0);

    test(
        (pnet != NULL) && 
        (pnet_get_error() == pnet_info_ok) &&
        (matrix_cmp_eq(pnet->places, places)),
        "Test missfire"
    );
    
    matrix_delete(places);

    // Test transition with weighted, inhibit and reset arc and input event
    pnet_fire(pnet, pnet_inputs_new(2, 1,0));
    places = matrix_new(3,1, 0,0,1);
    matrix_int_t *transitions = matrix_new(2,1, 0,1);

    test(
        (pnet != NULL) && 
        (pnet_get_error() == pnet_info_ok) &&
        (matrix_cmp_eq(pnet->sensitive_transitions, transitions)) &&
        (matrix_cmp_eq(pnet->places, places)),
        "Test transition with weighted, inhibit and reset arc and input event"
    );
    
    matrix_delete(places);
    matrix_delete(transitions);
    pnet_delete(pnet);

    // #############################################################################
    // Test for mutual fire
    pnet = pnet_new(
        pnet_arcs_map_new(3,2,
            -1, -1, -1,
             1,  1,  1
        ),
        NULL,
        NULL,
        pnet_places_init_new(2,
            1, 0
        ),
        NULL,
        NULL,
        NULL
    );

    places = matrix_new(2,1, 0, 1);
    transitions = matrix_new(3,1, 0,0,0);

    pnet_fire(pnet, NULL);
    pnet_sense(pnet);

    test(
        (pnet_get_error() == pnet_info_ok) &&
        matrix_cmp_eq(pnet->sensitive_transitions, transitions), 
        "Test for transitions sensibilization for no input_map and null input for pnet_fire"
    );
    
    test(
        (pnet_get_error() == pnet_info_ok) &&
        matrix_cmp_eq(pnet->places, places), 
        "Test for mutual fire"
    );

    pnet_delete(pnet);
    matrix_delete(places);
    matrix_delete(transitions);

    // #############################################################################
    // Test for multiple mutual transitions and 
    pnet = pnet_new(
        pnet_arcs_map_new(3,2,
            -1, -1, -1,
             1,  1,  1
        ),
        NULL,
        NULL,
        pnet_places_init_new(2,
            1, 0
        ),
        NULL,
        NULL,
        NULL
    );

    places = matrix_new(2,1, 0, 1);
    transitions = matrix_new(3,1, 0,0,0);

    pnet_fire(pnet, NULL);
    pnet_sense(pnet);

    test(
        (pnet_get_error() == pnet_info_ok) &&
        matrix_cmp_eq(pnet->sensitive_transitions, transitions), 
        "Test for transitions sensibilization for no input_map and null input for pnet_fire"
    );
    
    test(
        (pnet_get_error() == pnet_info_ok) &&
        matrix_cmp_eq(pnet->places, places), 
        "Test for multiple mutual transitions and "
    );

    pnet_delete(pnet);
    matrix_delete(places);
    matrix_delete(transitions);


    // #############################################################################
    test_summary();

    // test for simultaneous trasition firing. Do some research on how to implement correctly
    // test for possible negative tokens
    // test for rush conditions with the timed transitions
    // test outputs 
    // test for no input map and pass inputs to pnet fire
    // test for input map and pass null inputs to pnet fire

    // pnet_t *pnet = pnet_new(5, 4, 1, 2, 3,
    //     places_name_pnet,
    //     "p1", "p2", "p3", "p4", "p5",
    //     places_init_pnet,
    //     1, 0, 0, 0, 1,
    //     transitions_pnet,
    //     "t1", "t2", "t3", "t4",
    //     arcs_pnet,
    //     -1,  0,  0,  0,
    //      1, -1,  0,  1,
    //      0,  1, -1,  0,
    //      0,  0,  1, -1,
    //      0,  0, -1,  0,
    //     inhibit_arcs_pnet,
    //      0,  0,  0,  0,
    //      0,  0,  0,  0,
    //      0,  0,  0,  0,
    //      0,  0,  0,  0,
    //      1,  0,  0,  0,
    //     transitions_delay_pnet,
    //      2,  0,  3,  0,
    //     input_name_pnet,
    //     "start", "go",
    //     input_map_pnet,
    //      pnet_event_pos_edge,  0,  0,  0,
    //      0,  0,  0,  pnet_event_pos_edge,
    //     output_name_pnet,
    //     "s1", "s2", "s3",
    //     output_map_pnet,
    //      0,  0,  0,
    //      1,  0,  0, 
    //      0,  1,  0, 
    //      0,  0,  1, 
    //      0,  0,  0,
    //     conditions_name_pnet,
    //     "power",
    //     conditions_map_pnet,
    //      1,  0,  0,  0,
    //     end_options_pnet
    // ); 

    // if(pnet == NULL){
    //     printf("[%s.%i] [DEBUG] [pnet_new() failed!] \n", __FILE__, __LINE__);
    //     return 0;
    // }

    // pnet_sense(pnet);
    // pnet_print(pnet);

    // matrix_int_t *inputs = matrix_new(2, 1, 0, 0);
    // matrix_int_t *conditions = matrix_new(1, 1, 1);

    // pnet_fire(pnet, inputs, conditions);
    // pnet_sense(pnet);
    // pnet_print(pnet);
    // matrix_delete(inputs);

    // inputs = matrix_new(2, 1, 1, 0);

    // pnet_fire(pnet, inputs, conditions);
    // pnet_sense(pnet);
    // pnet_print(pnet);
    // matrix_delete(inputs);


    // matrix_delete(conditions);
    // pnet_delete(pnet);

    return 0;
}

void make_bar(double value, double max, char *array, size_t size, char chr){

    size_t lim = (size_t)((value*size)/max);
    
    for(size_t i = 0; i < size; i++){
        if(i <= lim)
            array[i] = chr;
        else 
            array[i] = ' ';
    }
}

void test_call(bool condition, char *text, char *file, int line, int dummy){
    // calls COUNTER a last time, thus it will be the total number of tests
    int total = __COUNTER__;

    char bar[21] = {0};
    make_bar(test_counter, total, bar, 20, '#');
    
    if(condition){
        printf("[TEST: %3d / %d] [%s] [%s:%-4d] [PASSED]: %s\n", test_counter, total, bar, file, line, text);
    }
    else{
        printf("[TEST: %3d / %d] [%s] [%s:%-4d] [FAILED] [PNET_ERR: %s]: %s\n", test_counter, total, bar, file, line, pnet_get_error_msg(), text);
        fail_counter++;
    }

    test_counter++;
}

void test_summary(void){
    printf("[SUMMARY]: %d succeeded, %d failed\n", test_counter - fail_counter, fail_counter);
}
