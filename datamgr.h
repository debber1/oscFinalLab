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

/*
 * Defining callback functions for a sensor data dplist
 */
void* sensor_data_copy(void * element);
void sensor_data_free(void ** element);
int sensor_data_compare(void * x, void * y);

void *datamgr_init(void* param);

/**
 * Insert all mapping data from file into dplist
 * \param list the list to add the data to 
 * \param fp_file the file containing all relevant mapping data
 * \return the populated list
 */
dplist_t *insert_mappings(dplist_t *list, FILE *fp_file);

#endif // !DATAMGR_H_
