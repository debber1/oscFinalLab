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

/**
 * This method should be called to clean up the datamgr, and to free all used memory. 
 * After this, any call to datamgr_get_room_id, datamgr_get_avg, datamgr_get_last_modified or datamgr_get_total_sensors will not return a valid result
 * \param list the list to free data from
 */
void datamgr_free(dplist_t *list);

#endif // !DATAMGR_H_
