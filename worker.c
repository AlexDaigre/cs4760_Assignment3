#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <semaphore.h> 
#include <fcntl.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/shm.h>

void closeProgramSignal(int sig);
void closeProgram();

int msgShmId;
int* msgShmPtr;

int main (int argc, char *argv[]) {
    signal(SIGINT, closeProgramSignal);
    printf("Child %d started\n", getpid());
    msgShmId = atoi(argv[1]);

    msgShmPtr = (int *) shmat(msgShmId, NULL, 0);
    if ((int) msgShmPtr == -1) {
        printf("shmat error in child\n");
        exit(1);
    }

    #define SNAME "/mysem"
    sem_t *sem = sem_open(SNAME, 0);
    // #define SNAME "/mysem"
    // sem_t* sem = sem_open(SNAME, O_CREAT, 0644, 3);

    int terminationSeconds = msgShmPtr[0];
    int terminationNanoSeconds = msgShmPtr[1] + ((rand() % 10000));

    while (terminationNanoSeconds >= 1000000000){
        terminationNanoSeconds -= 1000000000;
        terminationSeconds++;
    }

    sem_wait(sem);
        printf("  Child(%d) is in critical section.\n", getpid());
        while (((terminationSeconds >= msgShmPtr[0]) || ((terminationSeconds == msgShmPtr[0]) && (terminationNanoSeconds >= msgShmPtr[1])))){
            sem_post(sem);
            sem_wait(sem);
        }
        // printf("C: Child %d has terminated at system time %d:%d with termination time of %d:%d\n", getpid(), msgShmPtr[0], msgShmPtr[1], terminationSeconds, terminationNanoSeconds);
        while ((msgShmPtr[2] > 0) && (msgShmPtr[3] > 0)){
            // sem_post(sem);
            // sem_wait(sem);
        }
        msgShmPtr[2] = terminationSeconds;
        msgShmPtr[3] = terminationNanoSeconds;
    sem_post(sem);   

    printf("Child %d exiting\n", getpid());
    closeProgram();
}

void closeProgramSignal(int sig){
    closeProgram();
}

void closeProgram(){
    int i;
    shmdt(msgShmPtr);
    exit(0);
}