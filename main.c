/*
* Author: Robbe Decapmaker <debber@dcpm.be>
*/

#include "config.h"
#include <bits/pthreadtypes.h>
#include <string.h>
#include <stdbool.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include "connmgr.h"
#include "sbuffer.h"
#include "sensor_db.h"
#include "datamgr.h"

/*
 * Function defines
 */
bool log_file_exists(char *filename);
int create_log_process();
int write_to_log_process(char *msg);
int end_log_process();

/*
 * Global variables
 */
// Pipe related variables
char write_message[SIZE];
char read_message[SIZE];
pid_t pid;
int fd[2];

// thread related variables
pthread_mutex_t mutex_main;
pthread_attr_t attr_main;

/*
* Main function
*/
int main(int argc, char *argv[])
{
  // First we check the arguments from the CLI
  if (argc != 3) {
    printf("Wrong number of CLI arguments\nUsage:\n\tCLI arg 1: TCP port\n\tCLI arg 2: Max number of clients\n\tExample: './sensor_gateway 5678 3'\n");
    exit(-1);
  }

  // Get the arguments
  int tcp_port = atoi(argv[1]);
  int clients_nr = atoi(argv[2]);

  // Basic input validation
  if (tcp_port < 1024) {
    printf("Use a valid TCP port in the non-root range\nUsage:\n\tCLI arg 1: TCP port\n\tCLI arg 2: Max number of clients\n\tExample: './sensor_gateway 5678 3'\n");
    exit(-1);
  }
  if (clients_nr <= 0) {
    printf("Provide a valid amount of client connections\nUsage:\n\tCLI arg 1: TCP port\n\tCLI arg 2: Max number of clients\n\tExample: './sensor_gateway 5678 3'\n");
    exit(-1);
  }

  // Check if room_sensor.map exists
  if (!log_file_exists("room_sensor.map")) {
    printf("room_sensor.map does not exist. Please add it.\n");
    exit(-1);
  }

  // Create the log process 
  if(create_log_process() != 0){
    printf("Unable to start log process, exiting.");
    exit(-1);
  }
  write_to_log_process("Started the logger");

  // make an sbuffer instance
  sbuffer_t *shared_buffer = NULL;
  if(sbuffer_init(&shared_buffer) != 0){
    write_to_log_process("Failed to create the buffer");
    end_log_process();
    exit(-1);
  }


  // Init the parameters for the connmgr 
  connmgr_param_t *parameters_connmgr = (connmgr_param_t*) malloc(sizeof(connmgr_param_t));
  parameters_connmgr->max_con = clients_nr;
  parameters_connmgr->listen_port = tcp_port;
  parameters_connmgr->shared_buffer = shared_buffer;

  // Init the parameters for the sensor_db
  sensor_db_param_t *parameters_sensor_db = (sensor_db_param_t*) malloc(sizeof(sensor_db_param_t));
  parameters_sensor_db->shared_buffer = shared_buffer;

  // Init the parameters for the datamgr
  datamgr_param_t *parameters_datamgr = (datamgr_param_t*) malloc(sizeof(datamgr_param_t));
  parameters_datamgr->shared_buffer = shared_buffer;

  // start the connection manager
  pthread_attr_init(&attr_main);
  pthread_t tid_connmgr; 
  pthread_create(&tid_connmgr, &attr_main, connmgr_init, parameters_connmgr);

  // start the connection manager
  pthread_t tid_sensor_db; 
  pthread_create(&tid_sensor_db, &attr_main, sensor_db_runner, parameters_sensor_db);

  // start the connection manager
  pthread_t tid_datamgr; 
  pthread_create(&tid_datamgr, &attr_main, datamgr_init, parameters_datamgr);

  // Wait for all threads to finish
  pthread_join(tid_connmgr, NULL);
  pthread_join(tid_sensor_db, NULL);
  pthread_join(tid_datamgr, NULL);

  // Free the shared buffer
  sbuffer_free(&shared_buffer);

  // Free alloced structs
  free(parameters_connmgr);
  free(parameters_sensor_db);
  free(parameters_datamgr);

  // End the log process
  write_to_log_process("Ending log process");
  end_log_process();
  return 0;
}

// Function for checking if log file exists
bool log_file_exists(char *filename){
  FILE *check = fopen(filename, "r");
  bool exists = false;

  if(check != NULL){
    exists = true;
    fclose(check);
  }
  return exists;
}

int create_log_process(){
  char *filename = "gateway.log";
  // Make the file if it does not exist
  if(!log_file_exists(filename)){
    FILE *make = fopen(filename, "w");
    fclose(make);
  }

  // create the pipe
  if (pipe(fd) == -1){
    printf("Pipe failed\n");
    return -1;
  }

  // fork the child
  pid = fork();

  if(pid < 0){
    printf("Fork failed\n");
    return -1;
  }
  
  if(pid == 0){ // Child process
    // init the sequence number
    int seq_num = 0;
    // Close writing end of pipe
    close(fd[WRITE_END]); 

    ssize_t rc = 0;
    while(1){
      // Check for errors with the pipe
      if(rc < 0){ 
        close(fd[READ_END]);
        exit(0);
      }

      // Clear the read buffer 
      memset(read_message, 0, SIZE);

      // Read from pipe
      while((rc = read(fd[READ_END], read_message, sizeof(read_message))) > 0){
        // Make sure we are only getting one log message at once
        char *local_cache = strtok(read_message, DELIMITER);
        char time_buffer[26];

        while (local_cache != NULL){
          // Check if we get a kill signal
          if(strcmp(local_cache, "END_PROC") == 0){
            close(fd[READ_END]);
            exit(0);
          }

          //get current time
          time_t log_time = time(NULL);
          strftime(time_buffer, 26, "%a %b %d %H:%M:%S %Y", localtime(&log_time));

          // open the log file
          FILE *log = fopen(filename, "a");

          // Write into log file
          fprintf(log,"%i - %s - %s\n", seq_num, time_buffer, local_cache);
          fclose(log);

          // increment seq and go to next log 
          seq_num++;
          local_cache = strtok(NULL, DELIMITER); 
        }
      }
    }
    close(fd[READ_END]); 
  }
  else{ // parent process
    close(fd[READ_END]);
  }
  return 0;
}

int write_to_log_process(char *msg){
  // Lock the mutex so that we have no race conditions
  pthread_mutex_lock(&mutex_main);

  // Clear the write buffer
  memset(write_message, 0, SIZE);

  // Copy the message into the buffer
  strncpy(write_message, msg, SIZE);

  // Concat the delimiter to the message
  strcat(write_message, DELIMITER); 

  // Pass the log through the pipe
  write(fd[WRITE_END], write_message, SIZE);

  // Unlock the mutex so that other processes can write as well
  pthread_mutex_unlock(&mutex_main);
  return 0;
}

int end_log_process(){
  // Write the magic kill command to the logging process
  write_to_log_process("END_PROC");

  // Close the pipe write end
  close(fd[WRITE_END]);

  // Wait untill the child proc has finished writing all logs
  wait(NULL);
  return 0;
}
