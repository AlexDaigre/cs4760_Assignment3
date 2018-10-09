#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/shm.h>

int main (int argc, char *argv[]) {
    printf("Child %d started\n", getpid());
    exit(0);
}