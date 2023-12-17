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

void *sensor_db_runner(void *param);
FILE * open_db(char * filename, bool append);
int insert_sensor(FILE * f, sensor_id_t id, sensor_value_t value, sensor_ts_t ts);
int close_db(FILE * f);


#endif /* _SENSOR_DB_H_ */
