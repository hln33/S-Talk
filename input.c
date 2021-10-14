#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <netdb.h>
#include <unistd.h>

#include "input.h"
#include "shutdownManager.h"

static pthread_t threadPID;
static pthread_cond_t* s_pBuffAvail;
static pthread_cond_t* s_pMsgAvail;
static pthread_mutex_t* s_pOutlistMutex;
static bool s_running;

static char* s_messageTx = NULL;


void* inputThread(void* messages) {

    while(s_running) {

        if (ShutdownManager_isShuttingDown()) {
            return NULL;
        } 

        fd_set rfds;
        struct timeval tv;

        FD_ZERO(&rfds);
        FD_SET(0, &rfds);

        tv.tv_sec = 7;
        tv.tv_usec = 0;

        // this will wait 7 seconds before proceeding OR
        // until input is detected
        select(1, &rfds, NULL, NULL, &tv);

        // transmit is responsible for freeing this
        s_messageTx = malloc(512);
        if (ShutdownManager_isShuttingDown()) {
            ShutdownManager_freeMessage(s_messageTx);
            return NULL;
        } else {
            fflush(stdin);
            fgets(s_messageTx, 512, stdin);
        }


        // synchronize
        pthread_mutex_lock(s_pOutlistMutex);
        {
            // wait until we have available space
            if (List_count(messages) >= LIST_MAX_NUM_NODES) {
                pthread_cond_wait(s_pBuffAvail, s_pOutlistMutex);
                
                // in case shutdown was triggered while we were waiting
                if (ShutdownManager_isShuttingDown()) {
                    return NULL;
                }
            }

            // add message to list
            List_prepend(messages, s_messageTx);

            // signal that we have added a message
            pthread_cond_signal(s_pMsgAvail);
        }
        pthread_mutex_unlock(s_pOutlistMutex);
    }
    return NULL;   
}

void Input_init(List* messages, pthread_cond_t* msgAvailCondVar, pthread_cond_t* buffAvailCondVar, pthread_mutex_t* listMutex) {
    s_pMsgAvail = msgAvailCondVar;
    s_pBuffAvail = buffAvailCondVar;
    s_pOutlistMutex = listMutex;

    s_running = true;
    int x = pthread_create(&threadPID, NULL, inputThread, messages);
    if (x != 0) {
        perror("Error in input.c: ");
    }
}

void Input_shutdown() {
    // signal threads that may be stuck waiting
    pthread_cond_signal(s_pMsgAvail);

    s_running = false;

    // stop thread
    //pthread_cancel(threadPID);
    pthread_join(threadPID, NULL);
}