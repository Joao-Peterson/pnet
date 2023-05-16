![](https://img.shields.io/github/license/Joao-Peterson/pnet) ![](https://img.shields.io/badge/Version-1.0--0-brightgreen) ![](https://img.shields.io/github/last-commit/Joao-Peterson/pnet)

# pnet - a petri net library for C/C++

Easily make petri nets in C/C++. This library can create high level timed petri nets, with negated arcs and reset arcs, input events and outputs.

Is intended for embedding!

Created by João Peterson Scheffer - 2022. Version 1.0-0.

Licensed under the MIT License. Please reefer to the LICENSE file in the project root for license information.

Documentation available on [**github pages**](https://joao-peterson.github.io/pnet/html/index.html).

# TOC

- [pnet - a petri net library for C/C++](#pnet---a-petri-net-library-for-cc)
- [TOC](#toc)
- [Usage](#usage)
  - [Arguments](#arguments)
    - [Weighted arcs](#weighted-arcs)
    - [Inhibit arcs](#inhibit-arcs)
    - [Reset arcs](#reset-arcs)
    - [Input events](#input-events)
    - [Delay](#delay)
    - [Outputs](#outputs)
    - [Callback](#callback)
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
    ),
    NULL,
    NULL
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
    NULL,
    NULL,
    NULL
);
```

To execute your petri net just call `pnet_fire`:

```c
pnet_fire(pnet, pnet_inputs_new(2, 1,0))
```

This will execute **one and only transition** at a time, so the execution is made in stepped manner.

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

### Delay

You can add delay to transitions by mapping the value in milliseconds to every transition, a 0 represents a instant transition. Given in matrix form, one row and the columns are the transitions.

```c
// 500 ms delay on transition 0
pnet_transitions_delay_new(2,
    500, 0                                                      
),
```

Note that when using instant transitions, after the `pnet_fire()` call, the tokens would have moved already, but when using a delay you can only expect the net state after the define time, so to react accordingly you have to provide a callback, see section [Callback](#callback). When a callback is given it will be called after a delayed transition is fired.

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

### Callback

A callback of type `pnet_callback_t` must be provided as argument when using timed transitions. **It will** be called after the execution of the delay for a given transition and that transition is still sensible. **It is also called** when a instant transition was fired.

It's form is as follows:

```c
void cb(pnet_t *pnet, void *data){
    your_data_struct *my_data = (your_data_struct*)data;
    
    // you code here
}
```

You can access the pnet directly through the callback as well a user data passed in the `data` parameter on the `pnet_new()` and `m_pnet_new()` calls.

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

Positive and negative arcs are separated instead of using a single matrix for weighted arcs, that allows for arcs to be defined from a place to that same place, which is intended design/functionality.

# TODO

- Prioritized petri net, add priority to transitions
- Make special calls for reading the output, or make up another type of abstraction that don't involves matrix_int_t 
- Callback for output change
- Analysis tools
- Analysis to highlight mutual firing transitions
- PNML support
- Better abstraction for embedding purposes
- Timed implementation for embedded systems, custom timers
- Custom memory allocator option
- Compile macro for minimal build, used for excluding some logic checking and abstraction so the library could be compiled for minimal space keeping in mind embedded systems