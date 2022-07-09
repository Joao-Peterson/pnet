#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "pnet.h"

int main(int argc, char **argv){

    // test pnet_new for the validate function
    // test for simultaneous trasition firing. Do some research on how to implement correctly
    // test for possible negative tokens
    // test for all pnet errors and info

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