#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <string.h>

#include "utils/utils.h"
#include "scheduler.h"

#define MAX_PROGRAM_COUNT   100  

#define FOR_ITER_LIST(it, list, size) for(typeof((list)) it = (list); it < &list[(size)]; it++)
#define GET_ELAPSED_TIME(time)  (unsigned long)((time) - start)

SCHEDULER scheduler_struct = {
    .current_pid = -1,
    .status = 0,
    .process = 0,
    .process_cnt = 0,
};

void initialize(int cnt) {
    setvbuf(stdout, 0, 2, 0);
    setvbuf(stdin, 0, 1, 0);
    scheduler_struct.process_cnt = cnt;
    scheduler_struct.process = (PROCESS *) malloc(sizeof(PROCESS) * scheduler_struct.process_cnt);
    bzero(scheduler_struct.process, sizeof(PROCESS) * scheduler_struct.process_cnt);
    FOR_ITER_LIST(it, scheduler_struct.process, scheduler_struct.process_cnt) {
        it->pid = -1;
    }
}

pid_t make_and_wait_process(char *path) {
    pid_t pid = fork_and_wait(path);

    FOR_ITER_LIST(it, scheduler_struct.process, scheduler_struct.process_cnt) {
        if (it->pid == -1) {
            it->pid = pid;
            it->program = path;
            break;
        }
    }
    return pid;
}

unsigned int parse_config(char **program_list, char *path) {
    FILE *fp = fopen(path, "r");
    if (fp == NULL) {
        fprintf(stderr, "Config file not found. (%s)\n", path);
        exit(-2);
    }
    char program_count_str[100];
    fgets(program_count_str, 100, fp);

    unsigned int program_count = atoi(program_count_str);
    if (program_count > MAX_PROGRAM_COUNT) {
        fprintf(stderr, "Too much program. \n");
        exit(-3);
    }

    if (program_count == 0) {
        fprintf(stderr, "Need at least one program.\n");
        exit(-4);
    }

    *program_list = (char *) malloc(100 * program_count);
    unsigned int i = 0;
    for (; i < program_count; i++) {
        if (fgets(&(*program_list)[i*100], 100, fp) == NULL)
            break;
        // Strip trailing newline
        char *pos = NULL;
        if ((pos = strchr(&(*program_list)[i*100], '\n')) != NULL)
            *pos = '\0';
    }
    fclose(fp);
    return i;
}

void parse_log(char *path) {
    FILE *fp = fopen(path, "r");
    if (fp == NULL) {
        fprintf(stderr, "Log file not found. (%s)\n", path);
        exit(-2);
    }

    while(1) {
        char log[200];
        if (fgets(log, 200, fp) == NULL)
            break;
        char *pos = NULL;
        if ((pos = strchr(log, '\n')) != NULL)
            *pos = '\0';

        char *program_name = strtok(log, ",");
        char *burst_time_str = strtok(NULL, ",");
        if (burst_time_str == NULL)
            continue;
        unsigned long burst_time = atoll(burst_time_str);
        
        FOR_ITER_LIST(it, scheduler_struct.process, scheduler_struct.process_cnt) {
            if (strcmp(it->program, program_name) == 0) {
                it->predicted_burst_time = (unsigned long) (burst_time * SMOOTHING_FACTOR + it->predicted_burst_time * (1 - SMOOTHING_FACTOR));
                break;
            }
        }
    }
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s [Config] [Log]\n", argv[0]);
        exit(-1);
    }

    char *program_list;
    unsigned int program_count = parse_config(&program_list, argv[1]);

    initialize(program_count);
    for(unsigned int i = 0; i < program_count; i++) {
        pid_t pid = make_and_wait_process(&program_list[i*100]);
        
    }
    parse_log(argv[2]);
    initialize_scheduler();

    for(unsigned int i = 0; i < program_count; i++) {
        push_schedule_entry(scheduler_struct.process[i].pid);
    }

    // Wait for all process go to SIGSTOP

    puts("Loading...");
    sleep(1);

    const clock_t start = clock();
    clock_t cu = -1;

    while(1) {
        clock_t end = clock();
        if (scheduler_struct.current_pid != -1) {
            unsigned long burst_time = GET_ELAPSED_TIME(end) - GET_ELAPSED_TIME(cu);
            FOR_ITER_LIST(it, scheduler_struct.process, scheduler_struct.process_cnt) {
                if (it->pid == scheduler_struct.current_pid) {
                    it->burst_time += burst_time;
                    RUNNING_TIME *running_time = (RUNNING_TIME *) malloc(sizeof(RUNNING_TIME));
                    running_time->start = GET_ELAPSED_TIME(cu);
                    running_time->end = GET_ELAPSED_TIME(end);
                    LinkedListNode *node = make_node(running_time);
                    append_tail(&it->timelist, node);
                    break;
                }
            }
        }
        pid_t next_pid = next_process();
        if (next_pid == -1)
            break;
        printf("[+] Running %d\n", next_pid);
        scheduler_struct.current_pid = next_pid;
        scheduler_struct.status = SCHEDULER_STATUS_RUNNING;
        cu = clock();
        cont_process(next_pid);

        while (!check_should_schedule()) {}
    }
    
    puts("[+] Complete all process");

    unsigned long long all_waiting_time = 0;

    FILE *fp = fopen(argv[2], "a");
    if (fp == NULL) {
        fprintf(stderr, "Unexpected error. \n");
        exit(-5);
    }

    FOR_ITER_LIST(it, scheduler_struct.process, scheduler_struct.process_cnt) {
        printf("[+] %s: %d\n", it->program, it->pid);
        fprintf(fp, "%s,%lu\n", it->program, it->burst_time);
        if (it->timelist.head == NULL)
            puts("[!] This process has never been started. maybe error");
        else {
            unsigned long prev_end_time = 0;
            unsigned long waiting_time = 0;
            FOR_ITER_LINKEDLIST(link_it, &it->timelist) {
                RUNNING_TIME *data = (RUNNING_TIME *)link_it->data;
                waiting_time += data->start - prev_end_time;
                prev_end_time = data->end;
            }
            printf("waiting_time: %f\n", (float) waiting_time / CLOCKS_PER_SEC );
            all_waiting_time += waiting_time;
        }
        printf("burst_time: %f\n", (float) it->burst_time / CLOCKS_PER_SEC);
        puts("running_time: ");
        
        FOR_ITER_LINKEDLIST(link_it, &it->timelist) {
            RUNNING_TIME *data = (RUNNING_TIME *)link_it->data;
            printf("\tstart: %f end: %f\n", (float) data->start / CLOCKS_PER_SEC, (float) data->end / CLOCKS_PER_SEC);
        }
        puts("");
    }
    if (scheduler_struct.process_cnt != 0)
        printf("avg_waiting_time: %f\n", (float) all_waiting_time / CLOCKS_PER_SEC / scheduler_struct.process_cnt );
    fclose(fp);
    return 0;
}