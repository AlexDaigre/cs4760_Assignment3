#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <semaphore.h> 
#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/shm.h>

sem_t mutex; 

int main (int argc, char *argv[]) {
    int msgShmId = atoi(argv[1]);

    int* msgShmPtr = (int *) shmat(msgShmId, NULL, 0);
    if ((int) msgShmPtr == -1) {
        printf("shmat error in child\n");
        exit(1);
    }

    printf("Child %d started\n", getpid());
    exit(0);
}