/*
* Author: Robbe Decapmaker <debber@dcpm.be>
*/

#include "datamgr.h"
#include "config.h"
#include "lib/dplist.h"
#include <stdio.h>
#include <stdlib.h>

sbuffer_t *shared_buffer_manager;

void *datamgr_init(void* param){
  // Getting parameters
  datamgr_param_t *parameters = (datamgr_param_t*)param;
  shared_buffer_manager = parameters->shared_buffer;

  // Init the sensor mappings
  dplist_t *data;
  FILE *fp_sensor_map = fopen("room_sensor.map", "r");
  data = dpl_create(sensor_map_copy, sensor_map_free, sensor_map_compare);
  data = insert_mappings(data, fp_sensor_map);

  // get sensor data
  sensor_data_t *data_ptr = malloc(sizeof(sensor_data_t));
  while (sbuffer_peek(shared_buffer_manager, data_ptr) == 0) {
    data = insert_data_point(data, data_ptr);
    calculate_average_sensor(data, data_ptr->id);
  }
  
  free(data_ptr);
  fclose(fp_sensor_map);
  datamgr_free(data);
  return 0;
}

void calculate_average_sensor(dplist_t *list, sensor_id_t id){
  double average = ABS_MIN; 
  sensor_map_t *dummy_map;
  for (int i = 0; i < dpl_size(list); i++) {
    dummy_map = dpl_get_element_at_index(list, i);
    if(dummy_map->sensorId != id){
      continue;
    }
    dplist_t *temp = dummy_map->readings;
    int length = dpl_size(temp);
    if (length != RUN_AVG_LENGTH) {
      continue; 
    }
    average = 0;
    for (int j = 0; j < RUN_AVG_LENGTH; j++) {
      average += ((sensor_data_t*)dpl_get_element_at_index(temp, j))->value;
    }
    average /= RUN_AVG_LENGTH;
    printf("avg = %g", average);
    
    if(average > SET_MAX_TEMP){
      char buffer[300];
      snprintf(buffer, 300, "Sensor node %i reports it's too hot (avg temp = %g)", dummy_map->sensorId, average);
      write_to_log_process(buffer);
    }
    if(average < SET_MIN_TEMP){
      char buffer[300];
      snprintf(buffer, 300, "Sensor node %i reports it's too cold (avg temp = %g)", dummy_map->sensorId, average);
      write_to_log_process(buffer);
    }
    break;
  }
}

dplist_t *insert_data_point(dplist_t *list, sensor_data_t *dataPoint){
  sensor_map_t *dummy_map;
  for (int i = 0; i < dpl_size(list); i++) {
    dummy_map = dpl_get_element_at_index(list, i);
    if(dummy_map->sensorId != dataPoint->id){
      continue;
    }
    if(dpl_size(dummy_map->readings) == RUN_AVG_LENGTH){
      dummy_map->readings = dpl_remove_at_index(dummy_map->readings, 0, true);
    }
    dummy_map->readings = dpl_insert_at_index(dummy_map->readings, dataPoint, RUN_AVG_LENGTH - 1, true);
    time_t current_time = time(&current_time);
    dummy_map->lastModified = current_time;
    break;
  }
  return list;
}

dplist_t *insert_mappings(dplist_t *list, FILE *fp_file){
  // Check if the file exists
  if(fp_file == NULL){
    printf("Not a valid file");
    return list;
  }

  sensor_map_t dummy;
  uint16_t roomId, sensorId;
  while (fscanf(fp_file, "%hu %hu", &roomId, &sensorId) == 2) {

    dummy.sensorId = sensorId;
    dummy.roomId = roomId;
    dummy.readings = dpl_create(sensor_data_copy, sensor_data_free, sensor_data_compare);
    time_t starttime = time(&starttime);
    dummy.lastModified = starttime; 

    list = dpl_insert_at_index(list, &dummy, dpl_size(list), true);
  }
  rewind(fp_file); // cleanup of file read
  return list;
}


void datamgr_free(dplist_t *list){
  dpl_free(&list, true);
}

void * sensor_map_copy(void * element) {
  sensor_map_t* copy = malloc(sizeof (sensor_map_t));
  sensor_map_t *temp = (sensor_map_t*)element;
  copy->sensorId = temp->sensorId;
  copy->roomId = temp->roomId;
  copy->readings = temp->readings;
  copy->lastModified = temp->lastModified;
  return (void *) copy;
}

void sensor_map_free(void ** element) {
  sensor_map_t *dummy = (sensor_map_t*)*element;
  dpl_free(&dummy->readings, true);
  free(*element);
  *element = NULL;
}

int sensor_map_compare(void * x, void * y) {
  return ((((sensor_map_t*)x)->sensorId < ((sensor_map_t*)y)->sensorId) ? -1 : (((sensor_map_t*)x)->sensorId == ((sensor_map_t*)y)->sensorId) ? 0 : 1);
}

void * sensor_data_copy(void * element) {
  sensor_data_t* copy = malloc(sizeof (sensor_data_t));
  copy->id = ((sensor_data_t*)element)->id;
  copy->value = ((sensor_data_t*)element)->value;
  copy->ts = ((sensor_data_t*)element)->ts;
  return (void *) copy;
}

void sensor_data_free(void ** element) {
  free(*element);
  *element = NULL;
}

int sensor_data_compare(void * x, void * y) {
  return ((((sensor_data_t*)x)->value < ((sensor_data_t*)y)->value) ? -1 : (((sensor_data_t*)x)->value == ((sensor_data_t*)y)->value) ? 0 : 1);
}
