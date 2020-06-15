#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/wait.h>
#include <string.h>
#include <stdio.h>

#include "../scheduler.h"
#include "../utils/linkedlist.h"

typedef struct SJF_SCHEDULE {
    LinkedList list;
} SJF_SCHEDULE;

SJF_SCHEDULE internal_schedule_struct = {
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

int check_should_schedule() {
    int wstatus;
    int ret = waitpid(scheduler_struct.current_pid, &wstatus, WNOHANG);
    if (ret == -1)
        return 1;
    if (ret != 0 && WIFEXITED(wstatus))
        return 1;
    return scheduler_struct.status != SCHEDULER_STATUS_RUNNING;
}

void initialize_scheduler() {
    // signal(SIGCHLD, schedule);

    // Sorting for SJF
    SORT_PROCESS(scheduler_struct, (int (*)(const void *, const void *))cmp_process_burst_time);
}