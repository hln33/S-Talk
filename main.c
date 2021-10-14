#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <ctype.h>

#include "receiver.h"
#include "input.h"
#include "print.h"
#include "transmit.h"
#include "list.h"
#include "shutdownManager.h"


int main(int argc, char* argv[]) {
    // check if arguments were provided
    if (argc < 4) {
        printf("Please Enter Arguments\n");
        return 1;
    } 
    int myPort = atoi(argv[1]);
    char* remoteMachineName = argv[2];
    int remotePort = atoi(argv[3]);

    // check if port numbers are valid
    if (myPort <= 1024 || myPort > 65535) {
        printf("First Port Argument Invalid");
        return 1;
    }
    if (remotePort <= 1024 || remotePort > 65535) {
        printf("Second Port Argument Invalid");
        return 1;
    }

    printf("Starting...\n");

    // start incoming threads
    List* incomingMessages = List_create();
    pthread_cond_t InMsgAvailCondVar = PTHREAD_COND_INITIALIZER;
    pthread_cond_t InBuffAvailCondVar = PTHREAD_COND_INITIALIZER;
    pthread_mutex_t InlistMutex = PTHREAD_MUTEX_INITIALIZER;
    Receiver_init(myPort, incomingMessages, &InMsgAvailCondVar, &InBuffAvailCondVar, &InlistMutex);
    Print_init(remoteMachineName, incomingMessages, &InMsgAvailCondVar, &InBuffAvailCondVar ,&InlistMutex);

    // start outgoing threads
    List* outgoingMessages = List_create();
    pthread_cond_t OutMsgAvailCondVar = PTHREAD_COND_INITIALIZER;
    pthread_cond_t OutBuffAvailCondVar = PTHREAD_COND_INITIALIZER;
    pthread_mutex_t OutlistMutex = PTHREAD_MUTEX_INITIALIZER;
    Input_init(outgoingMessages, &OutMsgAvailCondVar, &OutBuffAvailCondVar, &OutlistMutex);
    Transmit_init(remoteMachineName, remotePort, outgoingMessages, &OutMsgAvailCondVar, &OutBuffAvailCondVar, &OutlistMutex);

    // wait for shutdown signal from a thread
    ShutdownManager_waitForShutdown();

    return 0;
}