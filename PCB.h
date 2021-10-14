#ifndef _PCB_H_
#define _PCB_H_

enum ProcessState {
    PROCESS_RUNNING,
    PROCESS_READY,
    PROCESS_BLOCKED
};

typedef struct PCB_s PCB;
struct PCB_s {
    int PID;
    int priority;
    ProcessState state;
};

#endif _PCB_H_