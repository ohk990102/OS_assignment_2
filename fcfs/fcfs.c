#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/wait.h>
#include <string.h>
#include <stdio.h>

#include "../scheduler.h"
#include "../utils/linkedlist.h"

typedef struct FCFS_SCHEDULE {
    LinkedList list;
} FCFS_SCHEDULE;

FCFS_SCHEDULE internal_schedule_struct = {
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
    LinkedListNode *node = peek_head(&internal_schedule_struct.list);
    pid_t next_pid = -1;
    if (node != NULL) {
        next_pid = *((pid_t *)node->data);
        pop_head(&internal_schedule_struct.list);
        free(node->data);
        delete_node(node);
    }
    return next_pid;
}

int check_should_schedule() {
    return scheduler_struct.status != SCHEDULER_STATUS_RUNNING;
}

void initialize_scheduler() {
    signal(SIGCHLD, schedule);
}