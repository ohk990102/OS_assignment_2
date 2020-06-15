#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/wait.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

#include "../scheduler.h"
#include "../utils/linkedlist.h"

#define RR_TIME_QUANTUM     100000   // 100 ms

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

int cont_process(pid_t pid) {
    kill(pid, SIGCONT);
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
    if (((float) ((unsigned long)(clock() - cu)) / CLOCKS_PER_SEC) >= (float)0.18) {
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

void initialize_scheduler() {
    cu = clock();
    // signal(SIGCHLD, schedule);
}