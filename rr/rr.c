#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/wait.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

#include "../scheduler.h"
#include "../utils/linkedlist.h"
#include "../utils/utils.h"

float rr_time_quantum = 0.05;

typedef struct RR_SCHEDULE {
    LinkedList list;
} RR_SCHEDULE;

RR_SCHEDULE internal_schedule_struct = {
    .list = {
        .head = NULL,
        .tail = NULL
    }
};

/**
 * signal handler that sets status of global scheduler struct
 * 
 * Note: be careful of preempt condition
 * 
 * @param signo signal number
 **/
void schedule(int signo) {
    if (signo != SIGCHLD) {
        return;
    }

    int wstatus;
    // Check if current process exited
    int ret = waitpid(scheduler_struct.current_pid, &wstatus, WNOHANG);
    if (ret == -1 || ret == 0)
        return;
    if (WIFEXITED(wstatus)) {
        // SET status to schedule next process
        // TODO: enforce atomic
        scheduler_struct.status = SCHEDULER_STATUS_SHOULD_SCHEDULE;
    }
}

void push_schedule_entry(pid_t pid) {
    pid_t *data = (pid_t *) malloc(sizeof(pid_t));
    *data = pid;
    LinkedListNode *node = make_node(data);
    append_tail(&internal_schedule_struct.list, node);
}

pid_t next_process() {
    while(1) {
        LinkedListNode *node = peek_head(&internal_schedule_struct.list);
        if (node == NULL)
            return -1;

        pid_t next_pid = *((pid_t *)node->data);
        pop_head(&internal_schedule_struct.list);
        
        int wstatus;
        int exit = waitpid(next_pid, &wstatus, WNOHANG);
        if (exit == -1 || (exit != 0 && WIFEXITED(wstatus))) {
            // Process has exited. 
            free(node->data);
            delete_node(node);
        }
        else {
            // Process has stopped. 
            node->next = NULL;
            append_tail(&internal_schedule_struct.list, node);
            
            return next_pid;
        }
    }
}

clock_t cu = 0;

int check_should_schedule() {
    if (((float) ((unsigned long)(clock() - cu)) / CLOCKS_PER_SEC) >= rr_time_quantum) {
        kill(scheduler_struct.current_pid, SIGSTOP);
        while(1) {
            int wstatus;
            // Check if current process exited
            int ret = waitpid(scheduler_struct.current_pid, &wstatus, WNOHANG | WUNTRACED);
            if (ret == -1)
                break;
            if (ret != 0 && (WIFEXITED(wstatus) || WIFSTOPPED(wstatus)))
                break;
        }
        cu = clock();
        return 1;
    }
    else {
        int wstatus;
        int ret = waitpid(scheduler_struct.current_pid, &wstatus, WNOHANG);
        if (ret == -1)
            return 1;
        if (ret != 0 && WIFEXITED(wstatus))
            return 1;
    }
    return 0;
}

int cmp_process_burst_time(const PROCESS *a, const PROCESS *b) {
    // Shortest job first
    if (a->predicted_burst_time < b->predicted_burst_time)
        return -1;
    else if (a->predicted_burst_time > b->predicted_burst_time)
        return 1;
    else if (a < b)
        return -1;
    else
        return 1;
}

void initialize_scheduler() {
    SORT_PROCESS(scheduler_struct, (int (*)(const void *, const void *))cmp_process_burst_time);
    
    int idx = (int) (scheduler_struct.process_cnt * 4 / 5);
    if (idx < scheduler_struct.process_cnt) {
        if (scheduler_struct.process[idx].predicted_burst_time != 0) {
            rr_time_quantum = (float) scheduler_struct.process[idx].predicted_burst_time / CLOCKS_PER_SEC;
        }
    }

    printf("[*] rr time quantum: %f\n", rr_time_quantum);
    cu = clock();
    // signal(SIGCHLD, schedule);
}