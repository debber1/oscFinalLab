/*
* Author: Robbe Decapmaker <debber@dcpm.be>
*/

#include "sensor_db.h"

pthread_mutex_t mutex_reader;
void *sensor_db_runner(void *param){

  // Getting the parameters
  sensor_db_param_t *parameters = (sensor_db_param_t*)param;
  sbuffer_t *shared_buffer = parameters->shared_buffer;
  char* file_name = "data.csv";

  FILE *db = open_db(file_name, true);

  sensor_data_t *data_ptr = malloc(sizeof(sensor_data_t));

  while(sbuffer_remove(shared_buffer, data_ptr) == 0){
    pthread_mutex_lock(&mutex_reader);
    insert_sensor(db, data_ptr->id, data_ptr->value, data_ptr->ts);
    pthread_mutex_unlock(&mutex_reader);
  }

  write_to_log_process("Closing sensor db");

  free(data_ptr);
  close_db(db);
  pthread_exit(0);
}

bool file_exists(char *filename){
  FILE *check = fopen(filename, "r");
  bool exists = false;

  if(check != NULL){
    exists = true;
    fclose(check);
  }
 return exists;
}

FILE * open_db(char * filename, bool append){
  // Check if the filename is good
  if(filename == NULL){
    write_to_log_process("DB file name is invalid");
    return NULL;
  }

  // Make the file if it does not exist
  if(!file_exists(filename)){
    write_to_log_process("DB file does not exist");
    FILE *make = fopen(filename, "w");
    fclose(make);
    write_to_log_process("DB file now exists");
  }

  // Clear the file if we don't want to append
  if(!append){
    write_to_log_process("No append configured, clearing current DB file");
    FILE *empty = fopen(filename, "w");
    fclose(empty);
    write_to_log_process("Current DB file cleared");
  }


  // The file exists and we have prepared it as the user wants it
  // Now we open the file in append mode to add all new data
  FILE *data = fopen(filename, "a");

  if(data != NULL){
    write_to_log_process("Data file opened.");
  }

  // Return the file
  return data;
}
int insert_sensor(FILE * f, sensor_id_t id, sensor_value_t value, sensor_ts_t ts){
  // Did we get a valid file, if not return failed
  if(f == NULL){
    write_to_log_process("No valid file provided for data insertion");
    return -1;
  }

  // We have a valid file, insert the data into it return fprintf's status code
  int status = fprintf(f, "%hu, %g, %ld\n", id, value, ts);

  if(status > 0){
    fflush(f);
    char buffer[300];
    snprintf(buffer, 300, "Data insertion from sensor %i succeeded.", id);
    write_to_log_process(buffer);
  } else {
    write_to_log_process("Data failed to insert.");
  }

  return status;
}

int close_db(FILE * f){
  // Did we get a valid file, if not return failed
  if(f == NULL){
    write_to_log_process("No valid file provided for DB closure");
    return -1;
  }

  // We have a valid file, now save it and keep its status code
  int status = fclose(f);

  if(status != 0){
    write_to_log_process("DB could not be closed succesfully");
    return status;
  }

  // We have succesfully closed the DB
  write_to_log_process("Data file closed.");

  return status;
}
