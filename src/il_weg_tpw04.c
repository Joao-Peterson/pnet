#include "pnet_il.h"
#include "str.h"

#define BUFFER_SIZE 200

// labels
#define ALWAYS 	8000
#define INITIAL_RE 	8001

// compiler
char *pnet_compile_il_weg_tpw04(pnet_t *pnet, int input_offset, int output_offset, int transition_offset, int place_offset, int timer_offset, int timer_min, int jump_offset){
	if(pnet == NULL) return NULL;
	string_t *buffer = string_new(0);	

	// LD M8001 MOV K1 D000
	for(size_t place = 0; place < pnet->num_places; place++){						// initial tokens
		if(pnet->places_init->m[0][place]){
			string_cat_fmt(buffer, "LD M%u\nMOV K%u D%u\n", BUFFER_SIZE, INITIAL_RE, 1, place + place_offset);
		}
	}

	// LDP X000 OUT M000
	for(size_t transition = 0; transition < pnet->num_transitions; transition++){	// transition trigger
		if(pnet->inputs_map == NULL){												// no input map detection
			string_cat_fmt(buffer, "LD M%u\nMPS\nOUT %u\n", BUFFER_SIZE, ALWAYS, transition + transition_offset);
			string_cat_fmt(buffer, "MPP\nCJ P%u\n", BUFFER_SIZE, jump_offset);
			continue;
		}
	
		for(size_t input = 0; input < pnet->num_inputs; input++){
			if(
				(input == pnet->num_inputs - 1) &&
				(pnet->inputs_map->m[input][transition] == 0)
			){																		// no input for this transition
				string_cat_fmt(buffer, "LD M%u\nMPS\nOUT %u\n", BUFFER_SIZE, ALWAYS, transition + transition_offset);
			}
			else if(
				pnet->transitions_delay != NULL && 
				pnet->transitions_delay->m[0][transition]
			){ 																		// for timed transitions
				switch(pnet->inputs_map->m[input][transition]){
					case pnet_event_pos_edge:
						string_cat_fmt(buffer, "LDP X%u\nSET T%u\nK%u\nLD T%u\n", BUFFER_SIZE, input + input_offset, transition + timer_offset, pnet->transitions_delay->m[0][transition] / timer_min, transition + timer_offset);
						string_cat_fmt(buffer, "MPS\nOUT M%u\nMRD\nRST T%u\n", BUFFER_SIZE, transition + transition_offset, transition + timer_offset);
						break;

					case pnet_event_neg_edge:
						string_cat_fmt(buffer, "LDF X%u\nSET T%u\nK%u\nLD T%u\n", BUFFER_SIZE, input + input_offset, transition + timer_offset, pnet->transitions_delay->m[0][transition] / timer_min, transition + timer_offset);
						string_cat_fmt(buffer, "MPS\nOUT M%u\nMRD\nRST T%u\n", BUFFER_SIZE, transition + transition_offset, transition + timer_offset);
						break;

					case pnet_event_any_edge:
						string_cat_fmt(buffer, "LDP X%u\nORP X%u\nSET T%u\nK%u\nLD T%u\n", BUFFER_SIZE, input + input_offset, input + input_offset, transition + timer_offset, pnet->transitions_delay->m[0][transition] / timer_min, transition + timer_offset);
						string_cat_fmt(buffer, "MPS\nOUT M%u\nMRD\nRST T%u\n", BUFFER_SIZE, transition + transition_offset, transition + timer_offset);
						break;
				}
			}
			else if(pnet->inputs_map->m[input][transition]){						// regular transitions
				switch(pnet->inputs_map->m[input][transition]){
					case pnet_event_pos_edge:
						string_cat_fmt(buffer, "LDP X%u\nMPS\nOUT M%u\n", BUFFER_SIZE, input + input_offset, transition + transition_offset);
						break;

					case pnet_event_neg_edge:
						string_cat_fmt(buffer, "LDF X%u\nMPS\nOUT M%u\n", BUFFER_SIZE, input + input_offset, transition + transition_offset);
						break;

					case pnet_event_any_edge:
						string_cat_fmt(buffer, "LDP X%u\nORP X%u\nMPS\nOUT M%u\n", BUFFER_SIZE, input + input_offset, input + input_offset, transition + transition_offset);
						break;
				}
			}
			else{
				continue;
			}

			string_cat_fmt(buffer, "MPP\nCJ P%u\n", BUFFER_SIZE, jump_offset);
			break;
		}
	}

	string_cat_fmt(buffer, "P%u\n", BUFFER_SIZE, jump_offset);						// jump after input detection

	// LD (M00 | M8000) AND>= D000 + place_offset K000 AND>= ... MPS ADD D000 K000 D000 MRD ... MPP SUB D000 K000 D000    
	for(size_t transition = 0; transition < pnet->num_transitions; transition++){	// move tokens

		string_cat_fmt(buffer, "LD M%u\n", BUFFER_SIZE, transition + transition_offset);

		for(size_t place = 0; place < pnet->num_places; place++){					// check conditions
			if(
				pnet->inhibit_arcs_map != NULL &&
				pnet->inhibit_arcs_map->m[place][transition]
			){						 												// check inhibit arcs
				string_cat_fmt(buffer, "AND= D%u K0\n", BUFFER_SIZE, place + place_offset);
			}

			if(
				pnet->neg_arcs_map != NULL &&
				pnet->neg_arcs_map->m[place][transition]
			){																		// check neg arcs
				string_cat_fmt(buffer, "AND>= D%u K%u\n", BUFFER_SIZE, place + place_offset, -pnet->neg_arcs_map->m[place][transition]);
			}
		}

		if(
			pnet->neg_arcs_map != NULL ||
			pnet->pos_arcs_map != NULL ||
			pnet->reset_arcs_map != NULL
		){
			string_cat_raw(buffer, "MPS\n");										// save comparison value

			for(size_t place = 0; place < pnet->num_places; place++){				// move tokens
				if(
					pnet->neg_arcs_map != NULL && 
					pnet->neg_arcs_map->m[place][transition]
				)																	// sub
					string_cat_fmt(buffer, "SUB D%u K%u D%u\nMRD\n", BUFFER_SIZE, place + place_offset, -pnet->neg_arcs_map->m[place][transition], place + place_offset);

				if(
					pnet->pos_arcs_map != NULL && 
					pnet->pos_arcs_map->m[place][transition]
				)																	// add
					string_cat_fmt(buffer, "ADD D%u K%u D%u\nMRD\n", BUFFER_SIZE, place + place_offset, pnet->pos_arcs_map->m[place][transition], place + place_offset);

				if(
					pnet->reset_arcs_map != NULL && 
					pnet->reset_arcs_map->m[place][transition]
				)																	// reset arcs
					string_cat_fmt(buffer, "MOV K%u D%u\nMRD\n", BUFFER_SIZE, 0, place + place_offset);
			}

			string_trim_end(buffer, 4);
			string_cat_raw(buffer, "MPP\n");
		}
	}

	if(pnet->outputs_map != NULL){
		for(size_t place = 0; place < pnet->num_places; place++){					// outputs
			for(size_t output = 0; output < pnet->num_outputs; output++){
				if(pnet->outputs_map->m[place][output])
					string_cat_fmt(buffer, "LD>= D%u K%u\nOUT Y%u\n", BUFFER_SIZE, place + place_offset, pnet->outputs_map->m[place][output], output + output_offset);
			}
		}
	}

	return string_unwrap(buffer);
}