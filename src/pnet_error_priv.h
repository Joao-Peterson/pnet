#ifndef _PNET_ERROR_PRIV_HEADER_
#define _PNET_ERROR_PRIV_HEADER_

#include "pnet_error.h"

/**
 * @brief max string length for error messeges
 */
#define ERROR_MSG_MAX_LEN 500

/**
 * @brief set error code. !Avoid using
 * @param code: pnet_error_t enum code
 */
void pnet_set_error(pnet_error_t code);

/**
 * @brief set custom error message. !Avoid using
 * @param format: string format
 * @param ...: arguments
 */
void pnet_set_error_msg(char* format, ...);

#endif