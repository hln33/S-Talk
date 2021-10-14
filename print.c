#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "print.h"
#include "shutdownManager.h"

static pthread_t threadPID;
static pthread_cond_t* s_pBuffAvail;
static pthread_cond_t* s_pMsgAvail;
static pthread_mutex_t* s_pInlistMutex;
static bool s_running;

static char* s_messageRx = NULL;
static char* s_remoteMachineName;


void* printThread(void* messages) {

    char* remoteUser = "Remote User:";

    while(s_running) {
        // synchronize
        pthread_mutex_lock(s_pInlistMutex);
        {
            // wait for a message
            while(List_count(messages) < 1) {
                pthread_cond_wait(s_pMsgAvail, s_pInlistMutex);

                // in case shutdown was triggered while we were waiting
                if(ShutdownManager_isShuttingDown()) {
                    return NULL;
                }
            }

            // get message from list and display on screen
            s_messageRx = List_trim(messages);

            // signal that a buffer is now free
            pthread_cond_signal(s_pBuffAvail);
        }
        pthread_mutex_unlock(s_pInlistMutex);

        // print message to screen
        puts(remoteUser);
        puts(s_messageRx);

        // check if message was to terminate
        if(*s_messageRx == '!') {
            ShutdownManager_freeMessage(s_messageRx);
            ShutdownManager_triggerShutdown();
            return NULL;
        }

        //free message
        ShutdownManager_freeMessage(s_messageRx);
    }
    return NULL;
}

void Print_init(char* remoteMachineName, List* messages, pthread_cond_t* msgAvailCondVar, pthread_cond_t* buffAvailCondVar, pthread_mutex_t* listMutex) {
    s_pMsgAvail = msgAvailCondVar;
    s_pBuffAvail = buffAvailCondVar;
    s_pInlistMutex = listMutex;

    s_remoteMachineName = remoteMachineName;

    s_running = true;
    int x = pthread_create(&threadPID, NULL, printThread, messages);
    if (x != 0) {
        perror("Error in print.c: ");
    }
}

void Print_shutdown() {
    // signal shutdown of other threads as well
    ShutdownManager_triggerShutdown();

    // signal threads to finish so they can eventually join
    pthread_cond_signal(s_pBuffAvail);
        
    // stop thread
    s_running = false;
    pthread_join(threadPID, NULL);

    //cleanup
    //ShutdownManager_freeMessage(s_messageRx);
}