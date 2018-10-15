#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <semaphore.h> 
#include <fcntl.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/shm.h>

// sem_t mutex;

int main (int argc, char *argv[]) {
    printf("Child %d started\n", getpid());
    int msgShmId = atoi(argv[1]);

    int* msgShmPtr = (int *) shmat(msgShmId, NULL, 0);
    if ((int) msgShmPtr == -1) {
        printf("shmat error in child\n");
        exit(1);
    }

    // #define SNAME "/mysem"
    // sem_t *sem = sem_open(SNAME, 0);
    #define SNAME "/mysem"
    sem_t* sem = sem_open(SNAME, O_CREAT, 0644, 3);

    int terminationSeconds = msgShmPtr[0];
    int terminationNanoSeconds = msgShmPtr[1] + ((rand() % 10000) * 100);

    while (terminationNanoSeconds >= 1000000000){
        terminationNanoSeconds -= 1000000000;
        terminationSeconds++;
    }

    sem_wait(sem);
        printf("  Child(%d) is in critical section.\n", getpid());
        while (!(terminationSeconds >= msgShmPtr[0] || (terminationSeconds == msgShmPtr[0] && terminationNanoSeconds >= msgShmPtr[1]))){
            sem_post(sem);
            sem_wait(sem);
        }
        while ((msgShmPtr[2] > 0) && (msgShmPtr[3] > 0)){
            sem_post(sem);
            sem_wait(sem);
        }
        msgShmPtr[2] = terminationSeconds;
        msgShmPtr[3] = terminationNanoSeconds;
    sem_post(sem);   

    printf("Child %d exiting\n", getpid());
    shmdt(msgShmPtr);
    exit(0);
}