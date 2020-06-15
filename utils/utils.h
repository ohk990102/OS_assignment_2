#ifndef UTILS_H
#define UTILS_H

#include <sys/types.h>

#define FOR_ITER_LIST(it, list, size) for(typeof((list)) it = (list); it < &list[(size)]; it++)

pid_t fork_and_wait(char *path);
void cont_process(pid_t pid);

#endif