#ifndef _PRINT_H_
#define _PRINT_H_

#include <pthread.h>
#include "list.h"

// start print thread
void Print_init(char* remoteMachineName, List* messages, pthread_cond_t* msgAvailCondVar, pthread_cond_t* buffAvailCondVar, pthread_mutex_t* listMutex);

// stop print thread and clean up
void Print_shutdown();

#endif