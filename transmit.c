#include <pthread.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <netdb.h>
#include <unistd.h>

#include "transmit.h"
#include "shutdownManager.h"

static pthread_t threadPID;
static pthread_cond_t* s_pBuffAvail;
static pthread_cond_t* s_pMsgAvail;
static pthread_mutex_t* s_pOutlistMutex;
static bool s_running;

static char remotePort[12];
static int remoteSocketDescriptor;

static char* s_messageTx = NULL;
static const char* s_remoteMachineName;
static struct addrinfo* s_servinfo;


void* transmitThread(void* messages) {
    char* terminator = "!\n";

    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    //hints.ai_addr = INADDR_ANY;
    hints.ai_flags = AI_CANONNAME;

    // obtain address(es) matching host/port
    int status = getaddrinfo(s_remoteMachineName, remotePort, &hints, &s_servinfo);
    if(status != 0) {
        fprintf(stderr, "Error (getaddrinfo): %s", strerror(status));
        ShutdownManager_triggerShutdown();
        return NULL;
    }

    // create socket for UDP
    remoteSocketDescriptor = socket(s_servinfo->ai_family, s_servinfo->ai_socktype, s_servinfo->ai_protocol);
    if (remoteSocketDescriptor == -1) {
        printf("Error creating socket descriptor in transmit.c\n");
        ShutdownManager_triggerShutdown();
        return NULL;
    }

    while(s_running) {

        // synchronize
        pthread_mutex_lock(s_pOutlistMutex);
        {
            // wait until we have a message to transmit
            while(List_count(messages) < 1) {
                pthread_cond_wait(s_pMsgAvail, s_pOutlistMutex);

                // in case shutdown was triggered while we were waiting
                if (ShutdownManager_isShuttingDown()) {
                    return NULL;
                }
            }
            // get message from list
            s_messageTx = List_trim(messages);

            // signal that a buffer is available
            pthread_cond_signal(s_pBuffAvail);
        }
        pthread_mutex_unlock(s_pOutlistMutex);

        // transmit message       
        int bytesTx = sendto(remoteSocketDescriptor, s_messageTx, strlen(s_messageTx), 0,
                            (struct sockaddr *) s_servinfo->ai_addr, s_servinfo->ai_addrlen);
        if(bytesTx == -1) {
            perror("Error in transmit.c (sendto): ");
            ShutdownManager_triggerShutdown();
            return NULL;
        }

        // check if message was to terminate
        if(strcmp(s_messageTx, terminator) == 0) { 
            ShutdownManager_freeMessage(s_messageTx);
            ShutdownManager_triggerShutdown();
            return NULL;
        } 
        
        //free message
        ShutdownManager_freeMessage(s_messageTx);       
    }
    return NULL;
}

void Transmit_init(char* remoteMachineName, int port, List* messages, pthread_cond_t* msgAvailCondVar, pthread_cond_t* buffAvailCondVar, pthread_mutex_t* listMutex) {
    s_pMsgAvail = msgAvailCondVar;
    s_pBuffAvail = buffAvailCondVar;
    s_pOutlistMutex = listMutex;
    s_remoteMachineName = remoteMachineName;
    sprintf(remotePort, "%d", port);

    s_running = true;
    int x = pthread_create(&threadPID, NULL, transmitThread, messages);
    if (x != 0) {
        perror("Error in transmit.c: ");
    }
}

void Transmit_shutdown() {
    // signal shutdown of other threads as well
    ShutdownManager_triggerShutdown();

    // signal any threads stuck on this cond var so they may finish
    pthread_cond_signal(s_pBuffAvail);

    // stop thread
    s_running = false;
    pthread_join(threadPID, NULL);

    //cleanup
    if (s_servinfo != NULL) {
        freeaddrinfo(s_servinfo);
        s_servinfo = NULL;
    }
    //freeMessage(s_messageTx);
}