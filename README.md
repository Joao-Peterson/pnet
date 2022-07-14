# pnet - a petri net library for C/C++

Easily make petri nets in C/C++. This library can create high level timed petri nets, with negated arcs and reset arcs, input events and outputs.

Is intended for embedding!

Created by João Peterson Scheffer - 2022. Version 1.0-0.

Licensed under the MIT License. Please reefer to the LICENSE file in the project root for license information.

# TOC

- [pnet - a petri net library for C/C++](#pnet---a-petri-net-library-for-cc)
- [TOC](#toc)
- [Usage](#usage)
  - [Arguments](#arguments)
    - [Weighted arcs](#weighted-arcs)
    - [Inhibit arcs](#inhibit-arcs)
    - [Reset arcs](#reset-arcs)
    - [Input events](#input-events)
    - [Outputs](#outputs)
  - [Error handling](#error-handling)
- [Compile and install](#compile-and-install)
- [Implementation details](#implementation-details)
- [TODO](#todo)

# Usage

To start include the header file:

```c
#include "pnet.h"
```

The you can create a new petri net:

```c
pnet_t *pnet = pnet_new(
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
        pnet_event_none, pnet_event_none
    ),
    pnet_outputs_map_new(3,3,
        1,0,0,
        0,1,0,
        0,0,1
    )
); 
```

Note how you can pass weighted arcs, inhibit arcs, reset arcs, the initial tokens for the places, delay for transitions, inputs events and outputs in order, however, only the `places_init` and **at least one** type of arc are required, so very simple declarations can be made, like this:

```c    
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
    NULL
);
```

To execute your petri net just call `pnet_fire`:

```c
pnet_fire(pnet, pnet_inputs_new(2, 1,0))
```

This will execute one transition at a time, so the execution is made in stepped manner.

## Arguments

### Weighted arcs

Weighted arcs define the amount of token that are consumed and given by some transition. Represented in matrix form by two matrices, it's shape should be like:

```c
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

```

The first matrix are the positive weights, the second are the negative weights. Where 2 is the amount of transitions, and 3 the amount of places, notice how the columns represent the transitions and rows the places. In the example we are telling that for the first transition, 1 token will be consumed from the first place, and 1 token will be given to the second place, and for the second transition, a token will be given to the third place.

When there are no negative weights, a transition can fire at any time, so negative weights act as conditions/restrictions for a transition to fire.

### Inhibit arcs

Inhibit arcs define that a transition shall occur when there are no token in the specified place. Represented in matrix form, values can be only 1 or 0:

```c
pnet_arcs_map_new(2,3,
    0, 1,
    0, 0,
    0, 0
),
```

In the example we are saying that for the second transition to fire, no tokens can be present in the first place. This type of arc doesn't move any tokens like the reset or weighted arcs, it solely represents a condition, like the negative weights in the weighted arcs.


### Reset arcs

Reset arcs express the act of setting the number of tokens in a place to 0 if the transition specified is fired. Represented in matrix form, values can be only 1 or 0:

```c
pnet_arcs_map_new(2,3,
    0, 0,
    0, 1,
    0, 0
),
```

In the example we are saying that the second transition will reset the tokens in the second place when fired. This type of arc expresses change, like the weighted arcs, but no condition/restriction.

### Input events

Inputs can be passed to `pnet_fire` and based on the events set by the `inputs_map` argument in `pnet_new` can dictate the triggering of transitions. 

`inputs_map` is given in matrix form, only the values of the enumerator `pnet_event_t` are valid.

```c
pnet_inputs_map_new(2,2,
    pnet_event_none, pnet_event_pos_edge,
    pnet_event_none, pnet_event_none
),
```

The columns are the transitions and the rows are the inputs. 

Only one input event can be assigned to a single transition.

`pnet_event_none` and 0 are the same.

The events are as follow:

```c
pnet_event_none         // no event, always trigger if arc conditions are met
pnet_event_pos_edge     // when input changes from 0 to 1 between fires
pnet_event_neg_edge     // when input changes from 1 to 0 between fires
pnet_event_any_edge     // when input changes value between fires
```

Firing can be called with or without inputs:

```c
pnet_fire(pnet, pnet_inputs_new(2, 1,0))
```

```c
pnet_fire(pnet, NULL)
```

### Outputs

Outputs are given in matrix form, values can be only 1 and 0.

```c
pnet_outputs_map_new(3,3,
    1,0,0,
    0,1,0,
    0,0,1
)
```

The columns are the outputs and the rows are the places. A output is only 1 when there >0 tokens inside the respective place.

The state of the outputs can be accessed reading the `outputs` member of the `pnet_t` struct.

```c
pnet->outputs
```

## Error handling

Errors are bound to occur when defining the petri net, we can check for then by comparing the pointer return value from the calls and by using the `pnet_get_error` and `pnet_get_error_msg` calls.

```c
pnet_t *pnet = pnet_new();

if(pnet == NULL || pnet_get_error() != pnet_info_ok){
    // error handling
}

// or 

if(pnet != NULL && pnet_get_error() == pnet_info_ok){
    // do petri net stuff 
}
```

# Compile and install

Compilation is done by executing:

```
$ make release
```

Installation is done by executing:

```
$ make install
```

The location of installation can be configured in the [Makefile](Makefile) by altering the following variables:

```
INSTALL_BIN_DIR
INSTALL_LIB_DIR
INSTALL_INC_DIR
```

# Implementation details

This implementation uses matrix representation and custom independent algorithms by the author for sensing and firing the petri net.

# TODO

- Prioritized petri net, add priority to transitions
- Don't make matrix.h visible to end user
- Make special calls for reading the output, or make up another type of abstraction that don't involves matrix_int_t 
- Better abstraction for embedding purposes
- Analysis tools
- PNML support
- Timed calls
- Valgrind test
- Doxygen doc page
- Badges on readme