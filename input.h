#ifndef _INPUT_H_
#define _INPUT_H_

#include <pthread.h>
#include "list.h"

// start input thread
void Input_init(List* messages, pthread_cond_t* msgAvailCondVar, pthread_cond_t* buffAvailCondVar, pthread_mutex_t* listMutex);

// stop input thread and clean up
void Input_shutdown();

#endif