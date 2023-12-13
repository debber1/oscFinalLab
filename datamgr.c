/*
* Author: Robbe Decapmaker <debber@dcpm.be>
*/

#include "datamgr.h"

sbuffer_t *shared_buffer_manager;

void *datamgr_init(void* param){
  datamgr_param_t *parameters = (datamgr_param_t*)param;
  shared_buffer_manager = parameters->shared_buffer;

  return 0;
}
