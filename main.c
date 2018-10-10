#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <semaphore.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/shm.h>

void childClosed(int sig);
void closeProgram(int sig);
int msgShmId;
int* msgShmPtr;
sem_t* sem;
int currentProcesses = 0;
const int TOTALCHILDREN = 100;
pid_t createdProcesses[TOTALCHILDREN] = {0};

int main (int argc, char *argv[]) {
    signal(SIGCHLD, childClosed);
    signal(SIGINT, closeProgram);
    int c;
    int maxProcesses = 5;
    int maxRunTime = 2;
    char* logFile = "logFile.txt";

    while ((c = getopt (argc, argv, "hs:l:t:")) != -1){
        switch (c){
            case 'h':
                printf("Help\n");
                exit(0);
                break;
            case 's':
                maxProcesses = atoi(optarg);
                break;
            case 'l':
                logFile = optarg;
                break;
            case 't':
                maxRunTime = atoi(optarg);
                break;
            default:
                printf("there was an error with arguments");
                exit(1);
                break;
        }
    }

    printf("Number of children: %d\n", maxProcesses);
    printf("Log file name: %s\n", logFile);
    printf("Max run time: %d\n", maxRunTime);

    msgShmId = shmget(IPC_PRIVATE, sizeof(int)*4, IPC_CREAT | 0666);
    if (msgShmId < 0) {
        printf("shmget error in parrent\n");
        exit(1);
    }

    msgShmPtr = (int *) shmat(msgShmId, NULL, 0);
    if ((long) msgShmPtr == -1) {
        printf("shmat error in parrent\n");
        shmctl(msgShmId, IPC_RMID, NULL);
        exit(1);
    }

    msgShmPtr[0] = 0;
    msgShmPtr[1] = 0;

    #define SNAME "/mysem"
    sem = sem_open(SNAME, O_CREAT, 0644, 3);

    int i;
    pid_t newForkPid;
    for(i = 0; i < TOTALCHILDREN; i++){
        while (currentProcesses >= maxProcesses ){sleep(1);}
        currentProcesses++;
        newForkPid = fork();
        if (newForkPid == 0){
            char msgShmIdString[20];
            sprintf(msgShmIdString, "%d", msgShmId);
            execlp("./worker","./worker", msgShmIdString, NULL);
		    fprintf(stderr,"%s failed to exec worker!\n",argv[0]);
            exit(1);
        }
        createdProcesses[i] = newForkPid;
    }

    int closedChildren;
    while(closedChildren < 100 && msgShmPtr[0] <= 2){
        if ((msgShmPtr[2] > 0) || (msgShmPtr[3] > 0)){
            pid_t childEnded = wait(NULL);
            closedChildren++;
            msgShmPtr[2] = 0;
            msgShmPtr[3] = 0;
            printf("Child %d has terminated at %d:%d with message %d:%d\n", childEnded, msgShmPtr[0], msgShmPtr[1], msgShmPtr[2], msgShmPtr[3]);
        }
        msgShmPtr[1]++;
        if (msgShmPtr[1] >= 1000000000){
            msgShmPtr[1] -= 1000000000;
            msgShmPtr[0]++;
        }
    }
}

void childClosed(int sig){
    currentProcesses--;
    printf("Child Closed\n");
}

void closeProgram(int sig){
    int i;
    for(i = 0; i < TOTALCHILDREN; i++){
        if(i > 0){
            kill(createdProcesses[i], SIGINT);
        }
    }
    shmctl(msgShmId, IPC_RMID, NULL);
    shmdt(msgShmPtr);
    sem_unlink(sem);
    exit(0);
}