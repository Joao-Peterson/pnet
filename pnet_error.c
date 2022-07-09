#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdbool.h>
#include "pnet_error.h"
#include "pnet_error_priv.h"

// put enum name to string
#define PNET_DEF_ERR(enum) #enum

// global error code
static pnet_error_t error = pnet_ok;

// global error message
char* error_msg = NULL;
bool error_msg_free = false;

// static array error message list
static const char* error_msg_list[] = {
    PNET_DEF_ERR(pnet_ok),
    PNET_DEF_ERR(pnet_error_invalid_number_of_places_or_transitions)
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
