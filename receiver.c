#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <netdb.h>
#include <unistd.h>
#include <pthread.h>
#include <stdbool.h>

#include "receiver.h"
#include "shutdownManager.h"

static pthread_t threadPID;
static pthread_cond_t* s_pBuffAvail;
static pthread_cond_t* s_pMsgAvail;
static pthread_mutex_t* s_pInlistMutex;
static bool s_running;

static int myPort;
static int mySocketDescriptor;

static char* s_messageRx = NULL;

void* recieveThread(void* messages) {
    // address
    struct sockaddr_in sin;
    memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = htonl(INADDR_ANY);
    sin.sin_port = htons(myPort);

    // create socket for UDP
    mySocketDescriptor = socket(PF_INET, SOCK_DGRAM, 0);
    if (mySocketDescriptor == -1) {
        printf("Error creating socket descriptor in reciever.c\n");
        ShutdownManager_triggerShutdown();
    }

    // bind socket to the port that we specify
    int x = bind(mySocketDescriptor, (struct sockaddr*)&sin, sizeof(sin));
    if (x == -1) {
        printf("Error binding socket in reciever.c\n");
        ShutdownManager_triggerShutdown();
    }

    while(s_running) {
        char* terminator = "!\n";

        // get the data (blocking)
        struct sockaddr_in sinRemote;
        unsigned int sin_len = sizeof(sinRemote);

        // recieve message (print is responsible for freeing)
        s_messageRx = malloc(1024);
        int bytesRx = recvfrom(mySocketDescriptor, s_messageRx, 1024, 0,
                               (struct sockaddr*)&sinRemote, &sin_len);
        if(bytesRx == -1) {
            perror("Error in reciever.c: ");
            ShutdownManager_triggerShutdown();
        }
        if (ShutdownManager_isShuttingDown()) {
            ShutdownManager_freeMessage(s_messageRx);
            close(mySocketDescriptor);
            return NULL;
        }
        
        // make it null terminated (Strings end with NULL)
        int terminateIdx = (bytesRx < 1024) ? bytesRx : 1024 - 1;
        s_messageRx[terminateIdx] = 0;

        // synchronize
        pthread_mutex_lock(s_pInlistMutex);
        {
            // proceed when we have available space
            while(List_count(messages) >= LIST_MAX_NUM_NODES) {
                pthread_cond_wait(s_pBuffAvail, s_pInlistMutex);
            }
            List_prepend(messages, s_messageRx);

            // signal that we added a message
            pthread_cond_signal(s_pMsgAvail);
        }
        pthread_mutex_unlock(s_pInlistMutex);

        // if message recieved was to terminate then return from thread
        if (strcmp(s_messageRx, terminator) == 0) {
            printf("closing\n");
            close(mySocketDescriptor);
            return NULL;
        }
    }
    return NULL;
}

void Receiver_init(int port, List* messages, pthread_cond_t* msgAvailCondVar, pthread_cond_t* buffAvailCondVar, pthread_mutex_t* listMutex) {
    s_pMsgAvail = msgAvailCondVar;
    s_pBuffAvail = buffAvailCondVar;
    s_pInlistMutex = listMutex;
    myPort = port;

    s_running = true;
    int x = pthread_create(&threadPID, NULL, recieveThread, messages);
    if (x != 0) {
        perror("Error in reciever.c: ");
    }
}

void Receiver_shutdown() {
    pthread_cond_signal(s_pMsgAvail);

    // stop thread
    s_running = false;
    shutdown(mySocketDescriptor, SHUT_RDWR);
    //pthread_cancel(threadPID);
    pthread_join(threadPID, NULL);
}