/*
* Author: Robbe Decapmaker <debber@dcpm.be>
*/

#include "datamgr.h"

sbuffer_t *shared_buffer_manager;
dplist_t *data;

void *datamgr_init(void* param){
  // Getting parameters
  datamgr_param_t *parameters = (datamgr_param_t*)param;
  shared_buffer_manager = parameters->shared_buffer;
  
  return 0;
}

void * sensor_map_copy(void * element) {
  sensor_map_t* copy = malloc(sizeof (sensor_map_t));
  copy->sensorId = ((sensor_map_t*)element)->sensorId;
  copy->roomId = ((sensor_map_t*)element)->roomId;
  return (void *) copy;
}

void sensor_map_free(void ** element) {
  free(*element);
  *element = NULL;
}

int sensor_map_compare(void * x, void * y) {
  return ((((sensor_map_t*)x)->sensorId < ((sensor_map_t*)y)->sensorId) ? -1 : (((sensor_map_t*)x)->sensorId == ((sensor_map_t*)y)->sensorId) ? 0 : 1);
}
