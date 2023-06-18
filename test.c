#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <errno.h>
#include "src/pnet.h"
#include "src/pnet_il.h"

// time precision for testing
#define TIME_PRECISION_MS (10)
#define TIME_TEST_DELAY_MS (1)
#define TIME_TEST_LOOP_MAX (3000)

// global test count
size_t test_counter = 1;

//failed test count
size_t fail_counter = 0;

// test call. dummy its just to receive __COUNTER__
void test_call(bool condition, char* text, char *file, int line, int dummy);

// summary of succeeded and failed tests
void test_summary(void);

// takes a condition to test, if true passes if false fails. COUNTER increments with every call
#define test(condition, text) test_call((condition), text, __FILE__, __LINE__, __COUNTER__);

// callback and global flag
void cb(pnet_t *pnet, size_t transition, void *data);
bool cb_flag = false;


// Main #############################################################################
int main(int argc, char **argv){
    
    // #############################################################################
    // simple petrinet with two places and a single transition between them
    pnet_t *pnet = pnet_new(
        pnet_arcs_map_new(1,2,
            -1,
             0
        ),
        pnet_arcs_map_new(1,2,
             0,
             1
        ),
        NULL,
        NULL,
        pnet_places_init_new(2,
            1, 0
        ),
        NULL,
        NULL,
        NULL,
        NULL,
        NULL
    );
    test((pnet != NULL) && (pnet_get_error() == pnet_info_ok), "Test for non null return and err code ok");
    pnet_delete(pnet);

    // #############################################################################
    // simple petrinet with 3 places and 2 transitions between them, uses all avaiable arcs types and inputs/outputs
    pnet = pnet_new(
        pnet_arcs_map_new(2,3,
            -1, 0,
             0, 0,
             0, 0
        ),
        pnet_arcs_map_new(2,3,
             0, 0,
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
        ),
        cb,
        NULL
    );
    test((pnet != NULL) && (pnet_get_error() == pnet_info_ok), "Test for creation with all arguments");
    pnet_delete(pnet);

    // #############################################################################
    // test no args
    pnet = pnet_new(
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL
    );
    test((pnet == NULL) && (pnet_get_error() == pnet_error_places_init_must_not_be_null), "Test for all null args");
    pnet_delete(pnet);

    // #############################################################################
    // test with no arcs
    pnet = pnet_new(
        NULL,
        NULL,
        NULL,
        NULL,
        pnet_places_init_new(5, 1,0,0,0,0),
        NULL,
        NULL,
        NULL,
        NULL,
        NULL
    );
    test((pnet == NULL) && (pnet_get_error() == pnet_error_no_arcs_were_given), "Test for only places, no arcs");
    pnet_delete(pnet);

    // #############################################################################
    // Test for autocorrection of values 
    pnet = pnet_new(
        pnet_arcs_map_new(2,3,
            -1, 0,
             0, 0,
             0, 0
        ),
        pnet_arcs_map_new(2,3,
             0, 0,
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
        ),
        cb,
        NULL
    );

    pnet_matrix_t *inhibit_arcs_map = pnet_matrix_new(2,3, 1,1, 0,1, 0,0);
    pnet_matrix_t *reset_arcs_map = pnet_matrix_new(2,3, 0,0, 1,1, 0,1);
    pnet_matrix_t *places_init = pnet_matrix_new(3,1, 1,0,0);
    pnet_matrix_t *transitions_delay = pnet_matrix_new(2,1, 0,0);
    pnet_matrix_t *inputs_map = pnet_matrix_new(2,2, 0,0,0,0);
    pnet_matrix_t *outputs_map = pnet_matrix_new(3,3, 1,0,0, 0,1,0, 0,0,1);

    test(
        (pnet != NULL) && 
        (pnet_get_error() == pnet_info_ok) &&
        pnet_matrix_cmp_eq(pnet->inhibit_arcs_map, inhibit_arcs_map) &&
        pnet_matrix_cmp_eq(pnet->reset_arcs_map, reset_arcs_map) &&
        pnet_matrix_cmp_eq(pnet->places_init, places_init) &&
        pnet_matrix_cmp_eq(pnet->transitions_delay, transitions_delay) &&
        pnet_matrix_cmp_eq(pnet->inputs_map, inputs_map) &&
        pnet_matrix_cmp_eq(pnet->outputs_map, outputs_map),
        "Test for autocorrection of values "
    );
    
    pnet_delete(pnet);
    pnet_matrix_delete(inhibit_arcs_map);
    pnet_matrix_delete(reset_arcs_map);
    pnet_matrix_delete(places_init);
    pnet_matrix_delete(transitions_delay);
    pnet_matrix_delete(inputs_map);
    pnet_matrix_delete(outputs_map);

    // #############################################################################
    // test for no weighted or inhibit arcs
    pnet = pnet_new(
        NULL,
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
        ),
        NULL,
        NULL
    );

    if(pnet != NULL){
        pnet_sense(pnet);
    }

    test(
        (pnet != NULL) && 
        (pnet_get_error() == pnet_info_no_neg_arcs_nor_inhibit_arcs_provided_no_transition_will_be_sensibilized), 
        "Test for no weighted or inhibit arcs"
    );
    pnet_delete(pnet);

    // #############################################################################
    // Test for multiple inputs on a single transition
    pnet = pnet_new(
        NULL,
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
        ),
        cb,
        NULL
    );

    test(
        (pnet == NULL) && 
        (pnet_get_error() == pnet_error_inputs_there_are_more_than_one_input_per_transition), 
        "Test for multiple inputs on a single transition"
    );
    pnet_delete(pnet);

    // #############################################################################
    // Test sense 1
    pnet = pnet_new(
        pnet_arcs_map_new(2,3,
            -1, 0,
             0, 0,
             0, 0
        ),
        pnet_arcs_map_new(2,3,
             0, 0,
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
        ),
        cb,
        NULL
    );

    if(pnet != NULL){
        pnet_sense(pnet);
    }

    pnet_matrix_t *sense = pnet_matrix_new(2, 1,  1,0);

    test(
        (pnet != NULL) && 
        (pnet_get_error() == pnet_info_ok) &&
        (pnet_matrix_cmp_eq(pnet->sensitive_transitions, sense)),
        "Test sense 1"
    );

    pnet_delete(pnet);
    pnet_matrix_delete(sense);

    // #############################################################################
    // Test fire without weighted or reset arcs
    pnet = pnet_new(
        NULL,
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
        ),
        NULL,
        NULL
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
    
    pnet_delete(pnet);

    // #############################################################################
    // Test fire with inputs to null input map
    pnet = pnet_new(
        pnet_arcs_map_new(2,3,
            -1, 0,
             0, 0,
             0, 0
        ),
        pnet_arcs_map_new(2,3,
             0, 0,
             1, 0,
             0, 1
        ),
        NULL,
        NULL,
        pnet_places_init_new(3,
            1, 0, 0
        ),
        NULL,
        NULL,
        pnet_outputs_map_new(3,3,
            1,0,0,
            0,1,0,
            0,0,1
        ),
        NULL,
        NULL
    );

    pnet_fire(pnet, pnet_inputs_new(2,0,0));

    test(
        (pnet != NULL) && 
        (pnet_get_error() == pnet_info_inputs_were_passed_but_no_input_map_was_set_when_the_petri_net_was_created),
        "Test fire with inputs to null input map"
    );
    
    pnet_delete(pnet);

    // #############################################################################
    // Test fire with null inputs to non null input map
    pnet = pnet_new(
        pnet_arcs_map_new(2,3,
            -1, 0,
             0, 0,
             0, 0
        ),
        pnet_arcs_map_new(2,3,
             0, 0,
             1, 0,
             0, 1
        ),
        NULL,
        NULL,
        pnet_places_init_new(3,
            1, 0, 0
        ),
        NULL,
        pnet_inputs_map_new(2,2,
            pnet_event_none, pnet_event_pos_edge,
            pnet_event_none, pnet_event_none
        ),
        pnet_outputs_map_new(3,3,
            1,0,0,
            0,1,0,
            0,0,1
        ),
        NULL,
        NULL
    );

    pnet_fire(pnet, NULL);

    test(
        (pnet != NULL) && 
        (pnet_get_error() == pnet_info_ok),
        "Test fire with null inputs to non null input map"
    );
    
    pnet_delete(pnet);

    // #############################################################################
    // Test firing logic
    pnet = pnet_new(
        pnet_arcs_map_new(2,3,
            -1, 0,
             0, 0,
             0, 0
        ),
        pnet_arcs_map_new(2,3,
             0, 0,
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
        NULL,
        pnet_inputs_map_new(2,2,
            pnet_event_none, pnet_event_pos_edge,
            pnet_event_none, pnet_event_none
        ),
        pnet_outputs_map_new(3,3,
            1,0,0,
            0,1,0,
            0,0,1
        ),
        cb,
        NULL
    );

    // Test firing of a single transition with input event of type none
    pnet_fire(pnet, pnet_inputs_new(2, 0,0));
    // pnet_matrix_print(pnet->places,"places");
    pnet_matrix_t *places = pnet_matrix_new(3,1, 0,1,0);

    test(
        (pnet != NULL) && 
        (pnet_get_error() == pnet_info_ok) &&
        (pnet_matrix_cmp_eq(pnet->places, places)),
        "Test firing of a single transition with input event of type none"
    );

    pnet_matrix_delete(places);

    // Test missfire
    pnet_fire(pnet, pnet_inputs_new(2, 0,0));
    places = pnet_matrix_new(3,1, 0,1,0);

    test(
        (pnet != NULL) && 
        (pnet_get_error() == pnet_info_ok) &&
        (pnet_matrix_cmp_eq(pnet->places, places)),
        "Test missfire"
    );
    
    pnet_matrix_delete(places);

    // Test transition with weighted, inhibit and reset arc and input event
    pnet_fire(pnet, pnet_inputs_new(2, 1,0));
    places = pnet_matrix_new(3,1, 0,0,1);
    pnet_matrix_t *transitions = pnet_matrix_new(2,1, 0,1);

    test(
        (pnet != NULL) && 
        (pnet_get_error() == pnet_info_ok) &&
        (pnet_matrix_cmp_eq(pnet->sensitive_transitions, transitions)) &&
        (pnet_matrix_cmp_eq(pnet->places, places)),
        "Test transition with weighted, inhibit and reset arc and input event"
    );
    
    pnet_matrix_delete(places);
    pnet_matrix_delete(transitions);
    pnet_delete(pnet);

    // #############################################################################
    // Test weighted and reset arc
    pnet = pnet_new(
        pnet_arcs_map_new(1,1,
            -1
        ),
        NULL,
        NULL,
        pnet_arcs_map_new(1, 1,
            1
        ),
        pnet_places_init_new(1,
            5
        ),
        NULL,
        NULL,
        NULL,
        NULL,
        NULL
    );

    if(pnet != NULL) 
        pnet_fire(pnet, NULL);

    places = pnet_matrix_new(1,1, 0);

    test(
        (pnet != NULL) && 
        (pnet_get_error() == pnet_info_ok) &&
        (pnet_matrix_cmp_eq(pnet->places, places)),
        "Test weighted and reset arc"
    );

    pnet_matrix_delete(places);
    pnet_delete(pnet);

    // #############################################################################
    // Test for mutual fire
    pnet = pnet_new(
        pnet_arcs_map_new(3,2,
            -1, -1, -1,
             0,  0,  0
        ),
        pnet_arcs_map_new(3,2,
             0,  0,  0,
             1,  1,  1
        ),
        NULL,
        NULL,
        pnet_places_init_new(2,
            1, 0
        ),
        NULL,
        NULL,
        NULL,
        NULL,
        NULL
    );

    places = pnet_matrix_new(2,1, 0, 1);
    transitions = pnet_matrix_new(3,1, 0,0,0);

    pnet_fire(pnet, NULL);
    pnet_sense(pnet);

    test(
        (pnet != NULL) && 
        (pnet_get_error() == pnet_info_ok) &&
        pnet_matrix_cmp_eq(pnet->sensitive_transitions, transitions), 
        "Test for transitions sensibilization for no input_map and null input for pnet_fire"
    );
    
    test(
        (pnet != NULL) && 
        (pnet_get_error() == pnet_info_ok) &&
        pnet_matrix_cmp_eq(pnet->places, places), 
        "Test for mutual fire"
    );

    pnet_delete(pnet);
    pnet_matrix_delete(places);
    pnet_matrix_delete(transitions);

    // #############################################################################
    // Test for multiple mutual transitions and a bidirectional arc
    pnet = pnet_new(
        pnet_arcs_map_new(3,5,
            -1,  0,  0,
            -1, -1, -1,
             0,  0,  0,
             0,  0,  0,
             0,  0,  0
        ),
        pnet_arcs_map_new(3,5,
             0,  0,  0,
             1,  0,  0,
             1,  0,  1,
             0,  0,  0,
             0,  1,  0
        ),
        NULL,
        NULL,
        pnet_places_init_new(5,
            1, 1, 0, 0, 0
        ),
        NULL,
        NULL,
        pnet_outputs_map_new(3,5,
            0,0,0,
            0,0,0,
            1,0,0,
            0,1,0,
            0,0,1 
        ),
        NULL,
        NULL
    );

    places = pnet_matrix_new(5,1, 0,0,1,0,1);
    pnet_matrix_t *outputs = pnet_matrix_new(3,1, 0,0,0);

    // Test for output state at start
    test(
        (pnet != NULL) && 
        (pnet_get_error() == pnet_info_ok) &&
        pnet_matrix_cmp_eq(pnet->outputs, outputs), 
        "Test for output state at start"
    );

    pnet_fire(pnet, NULL);
    pnet_fire(pnet, NULL);
    
    test(
        (pnet != NULL) && 
        (pnet_get_error() == pnet_info_ok) &&
        pnet_matrix_cmp_eq(pnet->places, places), 
        "Test for multiple mutual transitions and a bidirectional arc"
    );

    pnet_matrix_delete(outputs);
    outputs = pnet_matrix_new(3,1, 1,0,1);

    // Test for output state at the end
    test(
        (pnet != NULL) && 
        (pnet_get_error() == pnet_info_ok) &&
        pnet_matrix_cmp_eq(pnet->outputs, outputs), 
        "Test for output state at the end"
    );

    pnet_delete(pnet);
    pnet_matrix_delete(places);
    pnet_matrix_delete(outputs);

    // #############################################################################
    // Test timed petri net without passing callback as argument

    pnet = pnet_new(
        pnet_arcs_map_new(1,2,
            -1,
             0
        ),
        pnet_arcs_map_new(1,2,
             0,
             1
        ),
        NULL,
        NULL,
        pnet_places_init_new(2,
            1, 0
        ),
        pnet_transitions_delay_new(1,
            500000
        ),
        NULL,
        NULL,
        NULL,
        NULL
    );

    test(
        (pnet != NULL) && 
        (pnet_get_error() == pnet_info_no_callback_function_was_passed_while_using_timed_transitions_watch_out), 
        "Test timed petri net without passing callback as argument"
    );

    pnet_delete(pnet);

    // #############################################################################
    // Test timed transition

    pnet = pnet_new(
        pnet_arcs_map_new(1,2,
            -1,
             0
        ),
        pnet_arcs_map_new(1,2,
             0,
             1
        ),
        NULL,
        NULL,
        pnet_places_init_new(2,
            1, 0
        ),
        pnet_transitions_delay_new(1,
            TIME_TEST_DELAY_MS
        ),
        NULL,
        NULL,
        cb,
        NULL
    );

    clock_t now, start;
    bool precision = true;
    int last_elapsed = 0;
    cb_flag = false;

    // approximately three second of tests
    for(size_t i = 0; i < TIME_TEST_LOOP_MAX; i++){
        // start clock
        start = clock();
        
        // fire
        pnet_fire(pnet, NULL);

        // wait
        while(!cb_flag){
            now = clock();
        }

        // elapsed
        int elapsed_time = CLOCK_TO_MS(now - start);
        last_elapsed = elapsed_time;

        // test
        if(abs(elapsed_time - TIME_TEST_DELAY_MS) >= TIME_PRECISION_MS){
            precision = false;
            break;
        }
    }

    char *msg = calloc(500+1, sizeof(char));
    snprintf(msg, 500, "Test of timed transition with delay: [%d ms], precision: [+-%d ms] and iterations: [%d]. Last elapsed time: [%d ms]", TIME_TEST_DELAY_MS, TIME_PRECISION_MS, TIME_TEST_LOOP_MAX, last_elapsed);
    
    test(precision, msg);

    free(msg);
    pnet_delete(pnet);
    
    // #############################################################################
    // Test multi timed petri

    pnet = pnet_new(
        pnet_arcs_map_new(3, 2,
            -1,-1,-1,
            0,0,0
        ),
        pnet_arcs_map_new(3, 2,
            0,0,0,
            1,2,3
        ),
        NULL,
        NULL,
        pnet_places_init_new(2, 1, 0),
        pnet_transitions_delay_new(3, 300, 200, 100),
        NULL,
        NULL,
        cb,
        NULL
    );

    cb_flag = false;
    places = pnet_matrix_new(2, 1, 0, 3);

    // tree fires till desired state, fourth one for error detection
    if(pnet != NULL){
        pnet_fire(pnet, NULL);
        cb_flag = false;
        pnet_fire(pnet, NULL);
        cb_flag = false;
        pnet_fire(pnet, NULL);
        cb_flag = false;
        while(!cb_flag);
    }

    test(
        (pnet != NULL) && 
        (pnet_get_error() == pnet_info_ok) &&
        pnet_matrix_cmp_eq(pnet->places, places), 
        "Test multi timed petri"
    );
    
    pnet_delete(pnet);
    pnet_matrix_delete(places);

    // #############################################################################
    // Test sample petri net 1
    pnet = pnet_new(
        pnet_arcs_map_new(3,4,
            -1, 0, 0,
             0,-1, 0,
             0, 0,-1,
             0, 0, 0
        ),
        pnet_arcs_map_new(3,4,
             0, 0, 1,
             1, 0, 0,
             0, 1, 0,
             0, 0, 0
        ),
        pnet_arcs_map_new(3,4,
             0, 1, 0,
             0, 0, 0,
             0, 0, 0,
             0, 0, 0
        ),
        pnet_arcs_map_new(3,4,
             0, 0, 0,
             0, 0, 0,
             0, 0, 0,
             0, 1, 0
        ),
        pnet_places_init_new(4,
            1,0,0,1
        ),
        NULL,
        NULL,
        NULL,
        NULL,
        NULL
    );

    places = pnet_matrix_new(4,1, 0,1,0,0);

    // tree fires till desired state, fourth one for error detection
    if(pnet != NULL){
        pnet_fire(pnet, NULL);
        pnet_fire(pnet, NULL);
        pnet_fire(pnet, NULL);
        pnet_fire(pnet, NULL);
    }

    test(
        (pnet != NULL) && 
        (pnet_get_error() == pnet_info_ok) &&
        pnet_matrix_cmp_eq(pnet->places, places), 
        "Test sample petri net 1"
    );

    pnet_delete(pnet);
    pnet_matrix_delete(places);

    // #############################################################################
    // Test sample petri net 2
    pnet_matrix_t *m = pnet_matrix_new_zero(1,3);
    m->m[0][0] = -1; 
    m->m[1][0] =  0; 
    m->m[2][0] = -2; 
    
    pnet = m_pnet_new(
        m,
        pnet_matrix_new(1,3,
             0, 
             1,
             0 
        ),
        NULL,
        NULL,
        pnet_matrix_new(3, 1,
            1,0,32
        ),
        NULL,
        NULL,
        NULL,
        NULL,
        NULL
    );

    if(pnet != NULL){
        pnet_fire(pnet, NULL);
    }

    test(
        (pnet != NULL) && 
        (pnet_get_error() == pnet_info_ok) &&
        (pnet->places->m[0][0] == 0) && 
        (pnet->places->m[0][1] == 1) && 
        (pnet->places->m[0][2] == 30), 
        "Test sample petri net 2"
    );

    pnet_delete(pnet);

    // #############################################################################
    // Test matrix serialization

    pnet_matrix_t *mserial = pnet_matrix_new(4, 4,
        0, 0, 0, 1,
        64, 0, 0, 1,
        0, 0, 0, 0,
        0, 0, INT32_MAX, 16
    );

    size_t bytes;
    void *serialized_matrix = pnet_matrix_serialize(mserial, &bytes);

    test(serialized_matrix != NULL && pnet_get_error() == pnet_info_ok, "Test serialization of matrix");

    uint32_t testserial[] = {
        4, 4,
        0x80000000, 3, 1,
        0x80000001, 0, 64, 3, 1,
        0x80000003, 2, INT32_MAX, 3, 16
    };

    test(bytes == (sizeof(testserial)), "Test matrix serialization bytes written is equal to size of expected");
    test(!memcmp(serialized_matrix, testserial, sizeof(testserial)), "Test matrix serialization bytes is equal to expected");

    pnet_matrix_t *deserialized_matrix = pnet_matrix_deserialize(serialized_matrix, bytes);

    test(deserialized_matrix != NULL && pnet_get_error() == pnet_info_ok, "Test deserialization of matrix");
    test(pnet_matrix_cmp_eq(mserial, deserialized_matrix), "Compare if matrix == deserialize(serialize(matrix))");

    free(serialized_matrix);
    pnet_matrix_delete(mserial);
    pnet_matrix_delete(deserialized_matrix);

    // #############################################################################
    // Test matrix serialization edge cases

    pnet_matrix_t *matrix_single = pnet_matrix_new(1, 1, 42);
    pnet_matrix_t *matrix_max_doable = pnet_matrix_new_zero(0x000000FF, 0x000000FF);
    // pnet_matrix_t *matrix_max_doable = pnet_matrix_new_zero(0x00003DC3, 0x00003DC3);    // 1 Gb of memory for integers, a 158111 x 158111 matrix

    test(
        matrix_max_doable != NULL &&
        pnet_get_error() == pnet_info_ok,
        "Test creating big matrices"
    );

    size_t matrix_serial_edge_tmp_size;
    void *matrix_serial_edge_tmp;
    pnet_matrix_t *matrix_serial_edge_tmp_matrix;

    // single
    matrix_serial_edge_tmp = pnet_matrix_serialize(matrix_single, &matrix_serial_edge_tmp_size);
    matrix_serial_edge_tmp_matrix = pnet_matrix_deserialize(matrix_serial_edge_tmp, matrix_serial_edge_tmp_size);

    test(
        pnet_get_error() == pnet_info_ok &&
        matrix_serial_edge_tmp != NULL &&
        matrix_serial_edge_tmp_matrix != NULL &&
        pnet_matrix_cmp_eq(matrix_single, matrix_serial_edge_tmp_matrix),
        "Test 1x1 deserialization(serialization)"
    );

    pnet_matrix_delete(matrix_serial_edge_tmp_matrix);
    free(matrix_serial_edge_tmp);

    // doable
    matrix_serial_edge_tmp = pnet_matrix_serialize(matrix_max_doable, &matrix_serial_edge_tmp_size);
    matrix_serial_edge_tmp_matrix = pnet_matrix_deserialize(matrix_serial_edge_tmp, matrix_serial_edge_tmp_size);

    test(
        pnet_get_error() == pnet_info_ok &&
        matrix_serial_edge_tmp != NULL &&
        matrix_serial_edge_tmp_matrix != NULL &&
        pnet_matrix_cmp_eq(matrix_max_doable, matrix_serial_edge_tmp_matrix),
        "Test max doable size deserialization(serialization)"
    );

    pnet_matrix_delete(matrix_serial_edge_tmp_matrix);
    free(matrix_serial_edge_tmp);

    pnet_matrix_delete(matrix_single);
    pnet_matrix_delete(matrix_max_doable);

    // #############################################################################
    // Test pnet file

    pnet = pnet_new(
        pnet_arcs_map_new(3,4,
            -1, 0, 0,
             0,-1, 0,
             0, 0,-1,
             0, 0, 0
        ),
        pnet_arcs_map_new(3,4,
             0, 0, 1,
             1, 0, 0,
             0, 1, 0,
             0, 0, 0
        ),
        pnet_arcs_map_new(3,4,
             0, 1, 0,
             0, 0, 0,
             0, 0, 0,
             0, 0, 0
        ),
        pnet_arcs_map_new(3,4,
             0, 0, 0,
             0, 0, 0,
             0, 0, 0,
             0, 1, 0
        ),
        pnet_places_init_new(4,
            1,0,0,1
        ),
        NULL,
        NULL,
        NULL,
        NULL,
        NULL
    );

    if(pnet != NULL){
        pnet_fire(pnet, NULL);
    }

    pnet_save(pnet, "file/testfile-sample1.pnet");

    pnet_t *pnet_loaded = pnet_load("file/testfile-sample1.pnet", NULL, NULL); 

    test(
        pnet_loaded != NULL &&
        pnet_get_error() == pnet_info_ok &&
        pnet->num_places == pnet_loaded->num_places &&
        pnet_matrix_cmp_eq(pnet->places, pnet_loaded->places),
        "Test pnet desrialization(serialization)"
    );

    pnet_delete(pnet);
    pnet_delete(pnet_loaded);

    // #############################################################################
    // test weg tpw04 
    
    pnet = pnet_new(
        pnet_arcs_map_new(4,3,
            -1,-1, 0, 0,
             0, 0,-2, 0,
             0, 0, 0,-3
        ),
        pnet_arcs_map_new(4,3,
             0, 0, 1, 1,
             2, 0, 0, 0,
             0, 3, 0, 0
        ),
        pnet_arcs_map_new(4,3,
             0, 0, 0, 0,
             0, 0, 0, 1,
             0, 0, 1, 0
        ),
        pnet_arcs_map_new(4,3,
             0, 0, 0, 0,
             0, 0, 0, 0,
             0, 0, 0, 0
        ),
        pnet_places_init_new(3,
            1,0,0
        ),
        pnet_transitions_delay_new(4,
            2000, 0, 0, 0
        ),
        pnet_inputs_map_new(4, 4,
            pnet_event_pos_edge, 0, 0, 0,
            0, pnet_event_pos_edge, 0, 0,
            0, 0, pnet_event_pos_edge, 0,
            0, 0, 0, pnet_event_pos_edge
        ),
        pnet_outputs_map_new(2, 3,
            0,0,
            2,0,
            0,3
        ),
        NULL,
        NULL
    );

    test(pnet != NULL, "Sample pnet for compilation");

    char *il = pnet_compile_il_weg_tpw04(pnet, 0, 0, 30, 200, 0, 100, 0);

    FILE *outfile = fopen("file/compile_il_weg_tpw04.txt", "w");
    if(outfile == NULL){
        printf("Error: %s\n", strerror(errno));
        return 0; 
    }
    fwrite(il, strlen(il), 1, outfile);
    fflush(outfile);
    fclose(outfile);

    pnet_delete(pnet);
    free(il);












    // #############################################################################
    test_summary();

    // test for rush conditions with the timed transitions
    return 0;
}

void cb(pnet_t *pnet, size_t transition, void *data){
    cb_flag = true;
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
        printf("[TEST: %3lu / %d] [%s] [%s:%-4d] [PASSED]: %s\n", test_counter, total, bar, file, line, text);
    }
    else{
        printf("[TEST: %3lu / %d] [%s] [%s:%-4d] [FAILED]: [PNET_ERR: %s] %s\n", test_counter, total, bar, file, line, pnet_get_error_msg(), text);
        fail_counter++;
    }

    test_counter++;
}

void test_summary(void){
    printf("[SUMMARY]: %lu succeeded, %lu failed\n", test_counter - 1 - fail_counter, fail_counter);
}
