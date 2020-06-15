#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <time.h>
#include <sys/types.h>
#include <stdlib.h>
#include "utils/linkedlist.h"

#define SCHEDULER_STATUS_RUNNING            0
#define SCHEDULER_STATUS_SHOULD_SCHEDULE    1

#define SMOOTHING_FACTOR                    0.5

typedef struct RUNNING_TIME {
    unsigned long start;
    unsigned long end; 
} RUNNING_TIME;

typedef struct PROCESS { 
    pid_t pid;
    unsigned long predicted_burst_time;
    unsigned long burst_time;
    char *program;
    LinkedList timelist;
} PROCESS;

typedef struct SCHEDULER {
    unsigned int status;
    unsigned int isalarmed;
    pid_t current_pid;
    int process_cnt;
    PROCESS *process;
} SCHEDULER;

void initialize_scheduler();
void schedule(int signo);
void push_schedule_entry(int pid);
pid_t next_process();
int check_should_schedule();

extern SCHEDULER scheduler_struct;

#define SORT_PROCESS(scheduler, cmp)     qsort(scheduler.process, scheduler.process_cnt, sizeof(PROCESS), cmp)

#endif