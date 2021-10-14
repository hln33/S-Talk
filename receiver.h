#ifndef _RECEIVER_H_
#define _RECEIVER_H_

#include <pthread.h>
#include "list.h"

// start recieve thread
void Receiver_init(int port, List* messages, pthread_cond_t* msgAvailCondVar, pthread_cond_t* buffAvailCondVar, pthread_mutex_t* listMutex);

// stop recieve thread and clean up
void Receiver_shutdown();

#endif