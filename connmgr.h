/*
 * Author: Robbe Decapmaker <debber@dcpm.be>
 */

#ifndef _CONNMGR_H_
#define _CONNMGR_H_

#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <pthread.h>
#include "config.h"
#include "lib/tcpsock.h"
#include "sbuffer.h"
#include <unistd.h>

typedef struct {
  tcpsock_t *client;
  sbuffer_t *shared_buffer;
} client_handler_param_t;

/*
 * This function inits the connection manager
 * \param: param This is a pointer to a connmgr_param_t struct
 * \return: void pointer
 */
void *connmgr_init(void *param);

/**
 * Handles a single client on a different thread
 * \param: param This is a pointer to a client_handler_param_t struct
 * \return: void pointer
 */
void *handle_client(void *param);
#endif /* _CONNNMGR_H_ */
