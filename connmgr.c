/*
 * Author: Robbe Decapmaker <debber@dcpm.be>
 */

#include "config.h"
#include "connmgr.h"

int connmgr_init(int MAX_CONN, int PORT) {
  tcpsock_t *server;
  int conn_counter = 0;

  tcpsock_t *client[MAX_CONN];
  pthread_t tid[MAX_CONN]; /* the thread identifier */
  pthread_attr_t attr; /* set of thread attributes */
  /* set the default attributes of the thread */
  pthread_attr_init(&attr);
    printf("Test server is started\n");
    if (tcp_passive_open(&server, PORT) != TCP_NO_ERROR) exit(EXIT_FAILURE);
    do {
      if (tcp_wait_for_connection(server, &client[conn_counter]) != TCP_NO_ERROR) exit(EXIT_FAILURE);
      /* create the thread */
      pthread_create(&tid[conn_counter], &attr, handle_client, client[conn_counter]);
      /* wait for the thread to exit */
      printf("Incoming client connection\n");
      conn_counter++;
    } while (conn_counter < MAX_CONN);
    do{
      conn_counter--;
      pthread_join(tid[conn_counter], NULL);
    } while (conn_counter > 0);
    if (tcp_close(&server) != TCP_NO_ERROR) exit(EXIT_FAILURE);
    printf("Test server is shutting down\n");
    return 0;
}


void *handle_client(void *param){
  tcpsock_t *client = (tcpsock_t *)param;
  int bytes, result;
  sensor_data_t data;
  do {
    // read sensor ID
    bytes = sizeof(data.id);
    result = tcp_receive(client, (void *) &data.id, &bytes);
    // read temperature
    bytes = sizeof(data.value);
    result = tcp_receive(client, (void *) &data.value, &bytes);
    // read timestamp
    bytes = sizeof(data.ts);
    result = tcp_receive(client, (void *) &data.ts, &bytes);
    if ((result == TCP_NO_ERROR) && bytes) {
      printf("sensor id = %" PRIu16 " - temperature = %g - timestamp = %ld\n", data.id, data.value,
        (long int) data.ts);
    }
  } while (result == TCP_NO_ERROR);
  if (result == TCP_CONNECTION_CLOSED)
    printf("Peer has closed connection\n");
  else
    printf("Error occured on connection to peer\n");
  tcp_close(&client);
  return 0;
}
