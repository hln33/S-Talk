#ifndef _SHUTDOWN_MANAGER_H_
#define _SHUTDOWN_MANAGER_H_

// wait for shutdown signal
void ShutdownManager_waitForShutdown();

// signal shutdown of all threads
void ShutdownManager_triggerShutdown();

// return true if shutdown signalled,false otherwise
bool ShutdownManager_isShuttingDown();

// free memory allocated to message
void ShutdownManager_freeMessage(char* message);

#endif