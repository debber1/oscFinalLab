/**
 * \author Robbe Decapmaker <debber@dcpm.be>
 */

#ifndef _CONFIG_H_
#define _CONFIG_H_

#include <stdint.h>
#include "lib/dplist.h"
#include <time.h>

// Pipe related defines
#define SIZE 4096
#define READ_END 0
#define WRITE_END 1
#define DELIMITER "$"

// Connmgr related defines
#ifndef TIMEOUT
#define TIMEOUT 5
#endif // !TIMEOUT

// Datamgr retlated defines
#ifndef SET_MAX_TEMP
#define SET_MAX_TEMP 20
#endif // !SET_MAX_TEMP
#ifndef SET_MIN_TEMP
#define SET_MIN_TEMP 10
#endif // !SET_MIN_TEMP
#define RUN_AVG_LENGTH 5
#define ABS_MIN -274 // this is below absolute 0, thus not fysically possible


typedef uint16_t sensor_id_t;
typedef double sensor_value_t;
typedef time_t sensor_ts_t;         // UTC timestamp as returned by time() - notice that the size of time_t is different on 32/64 bit machine

typedef uint16_t room_id_t;

typedef struct {
    sensor_id_t id;
    sensor_value_t value;
    sensor_ts_t ts;
} sensor_data_t;

// global function to write to the log process
int write_to_log_process(char *msg);

// struct for parameters given to connmgr
typedef struct {
  int max_con;
  int listen_port;
  void* shared_buffer;
} connmgr_param_t;

// struct for parameters given to sensor_db
typedef struct {
  void* shared_buffer;
} sensor_db_param_t;

// struct for parameters given to datamgr
typedef struct {
  void* shared_buffer;
} datamgr_param_t;

/**
 * Structure to hold mapping of room to sensor
 */
typedef struct {
  sensor_id_t sensorId;
  room_id_t roomId;
  dplist_t *readings;
  time_t lastModified;
} sensor_map_t;

#endif /* _CONFIG_H_ */
