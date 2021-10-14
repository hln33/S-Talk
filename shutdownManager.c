#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <pthread.h>

#include "receiver.h"
#include "print.h"
#include "input.h"
#include "transmit.h"

static bool s_shuttingDown = false;
static pthread_mutex_t freeMutex = PTHREAD_MUTEX_INITIALIZER;

void ShutdownManager_waitForShutdown() {
    // busy wait
    while(s_shuttingDown == false);

    printf("shutting down\n");

    // shutdown threads
    Receiver_shutdown();
    Print_shutdown();
    Input_shutdown();
    Transmit_shutdown();

    printf("Shut Down.\n");
}

void ShutdownManager_triggerShutdown() {
    s_shuttingDown = true;
}

bool ShutdownManager_isShuttingDown() {
    return s_shuttingDown;
}

void ShutdownManager_freeMessage(char* message) {
    pthread_mutex_lock(&freeMutex);
    {
        if (message != NULL) {
            free(message);
            message = NULL;
        }
    }
    pthread_mutex_unlock(&freeMutex);
}