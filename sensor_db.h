/**
 * \author Bert Lagaisse
 */

#ifndef _SENSOR_DB_H_
#define _SENSOR_DB_H_

#include <stdio.h>
#include <stdlib.h>
#include "config.h"
#include "sbuffer.h"
#include <pthread.h>
#include <unistd.h>
#include <stdbool.h>

/**
 * When initialized by a pthread, this starts writing the sared buffer to disk
 * \param: param This is a pointer to a sensor_db_param_t struct
 * \return: void pointer
 */
void *sensor_db_runner(void *param);

/**
 * Opens a file according to the settings given in the parameters
 * \param filename The filename of the csv file to write
 * \param append start a new log file or append to the old one
 */
FILE * open_db(char * filename, bool append);

/**
 * Takes sensor data and instantly writes it to disk
 * \param f File descriptor for the file to write to
 * \param id The id of the sensor
 * \param value The temperature value of the sensor
 * \param ts The timestamp of the reading
 * \return The status code of the write
 */
int insert_sensor(FILE * f, sensor_id_t id, sensor_value_t value, sensor_ts_t ts);

/**
 * Closes the file used for writing to disk
 * \param f The file to close
 * \return The status of the closing
 */
int close_db(FILE * f);

#endif /* _SENSOR_DB_H_ */
