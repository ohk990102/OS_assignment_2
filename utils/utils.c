#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>

#include "utils.h"

/**
 * fork and wait for SIGCONT
 * 
 * @returns pid of child process. 
 **/

#define MAX_CMD_SIZE    100
#define MAX_ARG_COUNT   10

void __attribute__((weak)) cont_process(pid_t pid) {
    kill(pid, SIGCONT);
}

pid_t fork_and_wait(char *path) {

    // https://stackoverflow.com/questions/24796266/tokenizing-a-string-to-pass-as-char-into-execve
    // Tokenization for execve
    char cmd_buf[MAX_CMD_SIZE] = { 0 };
    strncpy(cmd_buf, path, sizeof(cmd_buf));
    char *strtok_state = NULL;
    char *filename = strtok_r(cmd_buf, " ", &strtok_state);
    printf("filename = %s\n", filename);

    char *args[MAX_ARG_COUNT] = { NULL };

    size_t current_arg_idx;
    args[0] = filename;
    for (current_arg_idx = 1; current_arg_idx < MAX_ARG_COUNT; ++current_arg_idx) {
        /* Note that the first argument to strtok_r() is NULL.
         * That means resume from a point saved in the strtok_state. */
        char *current_arg = strtok_r(NULL, " ", &strtok_state);
        if (current_arg == NULL) {
            break;
        }

        args[current_arg_idx] = current_arg;
        printf("args[%d] = %s\n", current_arg_idx, args[current_arg_idx]);
    }

    pid_t pid = fork();

    if (pid < 0) {
        perror("fork_and_wait: ");
        exit(-1);
    }
    else if (pid == 0) {
        kill(getpid(), SIGSTOP);
        execvp(filename, args);
        printf("[!] Cannot execute %s\n", path);
        exit(-1);
    }
    return pid;
}