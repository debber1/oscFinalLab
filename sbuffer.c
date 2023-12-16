/**
 * \author Robbe Decapmaker <debber@dcpm.be>
 */

#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include "sbuffer.h"

/**
 * basic node for the buffer, these nodes are linked together to create the buffer
 */
typedef struct sbuffer_node {
    struct sbuffer_node *next;  /**< a pointer to the next node*/
    bool is_processed;          /**< a boolean to keep track of the processing state of the node*/
    sensor_data_t data;         /**< a structure containing the data */
} sbuffer_node_t;

/**
 * a structure to keep track of the buffer
 */
struct sbuffer {
    sbuffer_node_t *head;       /**< a pointer to the first node in the buffer */
    sbuffer_node_t *tail;       /**< a pointer to the last node in the buffer */
};

pthread_mutex_t mutex;
pthread_cond_t condvar;
int condition = 0;

int sbuffer_init(sbuffer_t **buffer) {
    *buffer = malloc(sizeof(sbuffer_t));
    if (*buffer == NULL) return SBUFFER_FAILURE;
    (*buffer)->head = NULL;
    (*buffer)->tail = NULL;
    return SBUFFER_SUCCESS;
}

int sbuffer_free(sbuffer_t **buffer) {
    sbuffer_node_t *dummy;
    if ((buffer == NULL) || (*buffer == NULL)) {
        return SBUFFER_FAILURE;
    }
    while ((*buffer)->head) {
        dummy = (*buffer)->head;
        (*buffer)->head = (*buffer)->head->next;
        free(dummy);
    }
    free(*buffer);
    *buffer = NULL;
    return SBUFFER_SUCCESS;
}

int sbuffer_remove(sbuffer_t *buffer, sensor_data_t *data) {
  pthread_mutex_lock(&mutex);
  sbuffer_node_t *dummy;
  if (buffer == NULL){
    pthread_mutex_unlock(&mutex);
    return SBUFFER_FAILURE;
  }

  // wait for new data
  if (buffer->head == NULL){
    condition = 0;
    while (condition == 0){
      pthread_cond_wait(&condvar, &mutex);
    }
  } 

  // Check if we are at the end of the stream
  if(buffer->head->data.id == 0 && buffer->head->is_processed == false){
    pthread_mutex_unlock(&mutex);
    // Tell other threads to wake up and read the end of stream
    condition = 1;
    pthread_cond_signal(&condvar);
    return SBUFFER_NO_DATA;
  }

  // wait for valid data
  if (buffer->head->is_processed == false){
    condition = 1;
    pthread_cond_signal(&condvar);
    while (condition == 1){
      pthread_cond_wait(&condvar, &mutex);
    }
  } 

  // Check if we are at the end of the stream
  if(buffer->head->data.id == 0){
    pthread_mutex_unlock(&mutex);
    // Tell other threads to wake up and read the end of stream
    condition = 2;
    pthread_cond_signal(&condvar);
    return SBUFFER_NO_DATA;
  }


  *data = buffer->head->data;
  dummy = buffer->head;
  if (buffer->head == buffer->tail) // buffer has only one node
  {
      buffer->head = buffer->tail = NULL;
  } else  // buffer has many nodes empty
  {
      buffer->head = buffer->head->next;
  }
  free(dummy);
  pthread_mutex_unlock(&mutex);
  return SBUFFER_SUCCESS;
}

int sbuffer_peek(sbuffer_t *buffer, sensor_data_t *data) {
  pthread_mutex_lock(&mutex);
  sbuffer_node_t *dummy;
  if (buffer == NULL){
    pthread_mutex_unlock(&mutex);
    return SBUFFER_FAILURE;
  }

  // wait for new data
  if (buffer->head == NULL){
    condition = 0;
    while (condition == 0){
      pthread_cond_wait(&condvar, &mutex);
    }
  } 

  // Check if we are at the end of the stream
  if(buffer->head->data.id == 0){
    pthread_mutex_unlock(&mutex);
    // Tell other threads to wake up and read the end of stream
    condition = 2;
    pthread_cond_signal(&condvar);
    return SBUFFER_NO_DATA;
  }

  // wait for invalid data
  if (buffer->head->is_processed == true){
    condition = 2;
    while (condition == 2){
      pthread_cond_wait(&condvar, &mutex);
    }
  } 

  // Check if we are at the end of the stream
  if(buffer->head->data.id == 0){
    pthread_mutex_unlock(&mutex);
    // Tell other threads to wake up and read the end of stream
    condition = 2;
    pthread_cond_signal(&condvar);
    return SBUFFER_NO_DATA;
  }

  *data = buffer->head->data;
  dummy = buffer->head;
  dummy->is_processed = true;
  condition = 2;
  pthread_cond_signal(&condvar);
  pthread_mutex_unlock(&mutex);
  return SBUFFER_SUCCESS;
}

int sbuffer_insert(sbuffer_t *buffer, sensor_data_t *data) {
    sbuffer_node_t *dummy;
    if (buffer == NULL) return SBUFFER_FAILURE;
    dummy = malloc(sizeof(sbuffer_node_t));
    if (dummy == NULL) return SBUFFER_FAILURE;
    dummy->data = *data;
    dummy->is_processed = false;
    dummy->next = NULL;
    pthread_mutex_lock(&mutex);
    if (buffer->tail == NULL) // buffer empty (buffer->head should also be NULL
    {
      buffer->head = buffer->tail = dummy;
      condition = 1;
    } else // buffer not empty
    {
      buffer->tail->next = dummy;
      buffer->tail = buffer->tail->next;
    }
    if(data->id == 0){
      pthread_cond_signal(&condvar);
    }
    pthread_cond_broadcast(&condvar);
    pthread_mutex_unlock(&mutex);
    return SBUFFER_SUCCESS;
}
