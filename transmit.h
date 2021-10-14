#ifndef _TRANSMIT_H_
#define _TRANSMIT_H_

#include <pthread.h>
#include "list.h"

// start transmit thread
void Transmit_init(char* remoteMachineName, int port, List* messages, pthread_cond_t* msgAvailCondVar, pthread_cond_t* buffAvailCondVar, pthread_mutex_t* listMutex);

// stop transmit thread and clean up
void Transmit_shutdown();

#endif