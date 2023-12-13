/*
* Author: Robbe Decapmaker <debber@dcpm.be>
*/

#include "config.h"
#include <string.h>
#include <stdbool.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include "connmgr.h"
#include "sbuffer.h"

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

/*
* Main function
*/
int main(int argc, char *argv[])
{
  // Create the log process 
  if(create_log_process() != 0){
    printf("Unable to start log process, exiting.");
    exit(-1);
  }
  write_to_log_process("Started the logger");

  // make an sbuffer instance
  sbuffer_t *shared_buffer;
  if(sbuffer_init(&shared_buffer) != 0){
    write_to_log_process("Failed to create the buffer");
    end_log_process();
    exit(-1);
  }


  // Init the parameters for the connmgr 
  connmgr_param_t *parameters_connmgr = (connmgr_param_t*) malloc(sizeof(connmgr_param_t));
  parameters_connmgr->max_con = MAX_CONC_CONN;
  parameters_connmgr->listen_port = LISTEN_PORT;
  parameters_connmgr->shared_buffer = shared_buffer;


  // start the connection manager
  connmgr_init(parameters_connmgr);

  // Free the shared buffer
  sbuffer_free(&shared_buffer);

  // Free alloced structs
  free(parameters_connmgr);

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
