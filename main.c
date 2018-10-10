#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/shm.h>

void childClosed(int sig);
int currentProcesses = 0;

int main (int argc, char *argv[]) {
    signal(SIGCHLD, childClosed);
    int c;
    pid_t createdProcesses[100];
    const int TOTALCHILDREN = 100;
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

    int msgShmId = shmget(IPC_PRIVATE, sizeof(int)*4, IPC_CREAT | 0666);
    if (msgShmId < 0) {
        printf("shmget error in parrent\n");
        exit(1);
    }

    int* msgShmPtr = (int *) shmat(msgShmId, NULL, 0);
    if ((long) msgShmPtr == -1) {
        printf("shmat error in parrent\n");
        shmctl(msgShmId, IPC_RMID, NULL);
        exit(1);
    }

    int i;
    pid_t newForkPid;
    for(i = 0; i < TOTALCHILDREN; i++){
        while (currentProcesses >= maxProcesses ){sleep(1);}
        currentProcesses++;
        newForkPid = fork();
        if (newForkPid == 0){
            char msgShmIdString[25];
            sprintf(msgShmIdString, "%d", msgShmId);
            execlp("./worker","./worker", msgShmIdString, NULL);
		    fprintf(stderr,"%s failed to exec worker!\n",argv[0]);
            exit(1);
        }
        createdProcesses[i] = newForkPid;
    }

    while(1==1){
        printf("Sleeping.\n");
        sleep(5);
    }
}


void childClosed(int sig){
    currentProcesses--;
    printf("Child Closed\n");
}