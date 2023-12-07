/**
 * \author Robbe Decapmaker <debber@dcpm.be>
 */

#ifndef _CONFIG_H_
#define _CONFIG_H_

#include <stdint.h>
#include <time.h>

// Pipe related defines
#define SIZE 4096
#define READ_END 0
#define WRITE_END 1
#define DELIMITER ")()("

// Connmgr related defines
#define MAX_CONC_CONN 3
#define LISTEN_PORT 5678
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


typedef uint16_t sensor_id_t;
typedef double sensor_value_t;
typedef time_t sensor_ts_t;         // UTC timestamp as returned by time() - notice that the size of time_t is different on 32/64 bit machine

typedef struct {
    sensor_id_t id;
    sensor_value_t value;
    sensor_ts_t ts;
} sensor_data_t;

// global function to write to the log process
int write_to_log_process(char *msg);

#endif /* _CONFIG_H_ */
