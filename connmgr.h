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

/*
 * This function inits the connection manager
 * \param: MAX_CONN this represents the maximum amount of connections allowed
 * \param: PORT this is the port on which the connmgr should listen for incomming connections
 * \return: status code which gives information about the execution state
 */
int connmgr_init(int MAX_CONN, int PORT);
void *handle_client(void *param);
#endif /* _CONNNMGR_H_ */
