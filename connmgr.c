/*
 * Author: Robbe Decapmaker <debber@dcpm.be>
 */

#include "config.h"
#include "connmgr.h"
#include "lib/tcpsock.h"
#include "sbuffer.h"
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

sbuffer_t *shared_buffer;
void *connmgr_init(void *param) {
  // get parameters
  connmgr_param_t *parameters = (connmgr_param_t*)param;
  int MAX_CONN = parameters->max_con;
  int PORT = parameters->listen_port;
  shared_buffer = parameters->shared_buffer;

  tcpsock_t *server;
  int conn_counter = 0;
  write_to_log_process("Started the connection manager");

  tcpsock_t *client[MAX_CONN];
  pthread_t tid[MAX_CONN]; /* the thread identifier */
  pthread_attr_t attr; /* set of thread attributes */
  /* set the default attributes of the thread */
  pthread_attr_init(&attr);
  if (tcp_passive_open(&server, PORT) != TCP_NO_ERROR) exit(EXIT_FAILURE);
  do {
    if (tcp_wait_for_connection(server, &client[conn_counter]) != TCP_NO_ERROR) exit(EXIT_FAILURE);
    /* create the thread */
    pthread_create(&tid[conn_counter], &attr, handle_client, client[conn_counter]);
    conn_counter++;
  } while (conn_counter < MAX_CONN);
  do{
    conn_counter--;
    pthread_join(tid[conn_counter], NULL);
  } while (conn_counter > 0);
  
  sensor_data_t *dummy = malloc(sizeof(sensor_data_t));
  dummy->id = 0;
  sbuffer_insert(shared_buffer, dummy);
  free(dummy);

  write_to_log_process("Closed the connection manager");

  if (tcp_close(&server) != TCP_NO_ERROR) exit(EXIT_FAILURE);
  return 0;
}


void *handle_client(void *param){
  tcpsock_t *client = (tcpsock_t *)param;
  int bytes, result;
  bool first_connection = true;
  sensor_data_t data;
  do {
    // read sensor ID
    bytes = sizeof(data.id);
    result = tcp_receive_timeout(client, (void *) &data.id, &bytes, TIMEOUT);
    if(first_connection){
      char buffer[300];
      snprintf(buffer, 300, "Sensor node %i has opened a new connection", data.id);
      write_to_log_process(buffer);
      first_connection = false;
    }
    if(result == TCP_TIMEOUT_ERROR){
      char buffer[300];
      snprintf(buffer, 300, "Sensor node %i has timed out", data.id);
      write_to_log_process(buffer);
      tcp_close(&client);
      return 0;
    }
    if(result == TCP_CONNECTION_CLOSED){
      char buffer[300];
      snprintf(buffer, 300, "Sensor node %i has closed the connection", data.id);
      write_to_log_process(buffer);
      tcp_close(&client);
      return 0;
    }
    if(result != 0){
      char buffer[300];
      snprintf(buffer, 300, "Sensor node %i has had an error with the connection", data.id);
      write_to_log_process(buffer);
      tcp_close(&client);
      return 0;
    }
    // read temperature
    bytes = sizeof(data.value);
    result = tcp_receive(client, (void *) &data.value, &bytes);
    // read timestamp
    bytes = sizeof(data.ts);
    result = tcp_receive(client, (void *) &data.ts, &bytes);
    if ((result == TCP_NO_ERROR) && bytes) {
      sbuffer_insert(shared_buffer, &data);
      printf("sensor id = %" PRIu16 " - temperature = %g - timestamp = %ld\n", data.id, data.value,(long int) data.ts);
    }
  } while (result == TCP_NO_ERROR);
  tcp_close(&client);
  return 0;
}
