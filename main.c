#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include  <sys/types.h>
#include <sys/shm.h>

void abortExecution(int status);

int main (int argc, char *argv[]) {
    int numberOfChildren = 0;
    int maxProcesses = __INT32_MAX__;
    int currentProcesses = 0;
    int c;

    while ((c = getopt (argc, argv, "hn:s:")) != -1){
        switch (c){
            case 'h':
                printf("directions");
                abortExecution(0);
                break;
            case 'n':
                numberOfChildren = atoi(optarg);
                break;
            case 's':
                maxProcesses = atoi(optarg);
            case '?':
            case ':':
            default:
                printf("there was an error with arguments");
                abortExecution(1);
                break;
        }
    }

    int clockShmId = shmget(IPC_PRIVATE, 2*sizeof(int), IPC_CREAT | 0666);
    if (clockShmId < 0) {
        printf("shmget error in parrent\n");
        abortExecution(1);
    }

    int* clockShmPtr = (int *) shmat(clockShmId, NULL, 0);
    if ((int) clockShmPtr == -1) {
        printf("shmat error in parrent\n");
        abortExecution(1);
    }
    
    int numberOfRepetitions = numberOfChildren * 1000000;

    int i;
    for(i = 0; i < numberOfChildren; i++){
        if (fork() == 0){
            char numberOfRepetitionsString[12];
            sprintf(numberOfRepetitionsString, "%d", numberOfRepetitions);
            char clockShmIdString[12];
            sprintf(clockShmIdString, "%d", clockShmId);

            execlp("./worker","./worker", numberOfRepetitionsString, clockShmIdString, NULL);
		    fprintf(stderr,"%s failed to exec worker!\n",argv[0]);
		    exit(-1);
            abortExecution(0);
        }
    }

    printf ("numberOfChildren = %d, maxProcesses = %d\n", numberOfChildren, maxProcesses);
    abortExecution(0);
}

void abortExecution(int status){
    exit(status);
}