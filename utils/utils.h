#ifndef UTILS_H
#define UTILS_H

#include <sys/types.h>

pid_t fork_and_wait(char *path);
void cont_process(pid_t pid);

#endif