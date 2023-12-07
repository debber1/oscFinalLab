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
char write_message[SIZE];
char read_message[SIZE];
pid_t pid;
int fd[2];

int main(int argc, char *argv[])
{

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
  // Clear the write buffer
  memset(write_message, 0, SIZE);

  // Copy the message into the buffer
  strncpy(write_message, msg, SIZE);

  // Concat the delimiter to the message
  strcat(write_message, DELIMITER); 

  // Pass the log through the pipe
  write(fd[WRITE_END], write_message, SIZE);
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
