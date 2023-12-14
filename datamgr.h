/*
* Author: Robbe Decapmaker <debber@dcpm.be>
*/

#ifndef DATAMGR_H_
#define DATAMGR_H_
#include "config.h"
#include "sbuffer.h"
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include "lib/dplist.h"

/*
 * Defining callback functions for a sensor map dplist
 */
void* sensor_map_copy(void * element);
void sensor_map_free(void ** element);
int sensor_map_compare(void * x, void * y);

void *datamgr_init(void* param);
#endif // !DATAMGR_H_
