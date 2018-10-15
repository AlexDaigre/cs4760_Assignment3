#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/shm.h>
#include <sys/time.h>

void childClosed(int sig);
void closeProgramSignal(int sig);
void interrupt(int sig, siginfo_t* info, void* context);
int setTimer(double sec);
void closeProgram();
int setInterrupt();

int msgShmId;
int* msgShmPtr;
sem_t* sem;
int currentProcesses = 0;
pid_t createdProcesses[100] = {-5};

int main (int argc, char *argv[]) {
    signal(SIGCHLD, childClosed);
    signal(SIGINT, closeProgramSignal);
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
    msgShmPtr[2] = -1;
    msgShmPtr[3] = -1;

    #define SNAME "/mysem"
    sem = sem_open(SNAME, O_CREAT, 0644, 100);
    if (sem == SEM_FAILED) {
        perror("Failed to open semphore for empty");
        closeProgram();
    }

    // int i;
    // pid_t newForkPid;
    // for(i = 0; i < 100; i++){
    //     while (currentProcesses >= maxProcesses ){sleep(1);}
    //     currentProcesses++;
    //     newForkPid = fork();
    //     if (newForkPid == 0){
    //         char msgShmIdString[20];
    //         sprintf(msgShmIdString, "%d", msgShmId);
    //         execlp("./worker","./worker", msgShmIdString, NULL);
	// 	    fprintf(stderr,"%s failed to exec worker!\n",argv[0]);
    //         exit(1);
    //     }
    //     createdProcesses[i] = newForkPid;
    // }

    if (setInterrupt() == -1){
        printf("Failed to set up SIGPROF handler.\n");
    }

    if (setTimer(maxRunTime) == -1){
        printf("Failed to set up SIGPROF handler.\n");
    }

    int totalCreatedProcesses = 0;
    pid_t newForkPid;
    int closedChildren = 0;
    while(1==1/*(closedChildren < 100)*/ /*&& (msgShmPtr[0] <= 2)*/){    
        if ((currentProcesses <= maxProcesses) && (totalCreatedProcesses < 100)){
            currentProcesses++;
            newForkPid = fork();
            if (newForkPid == 0){
                char msgShmIdString[20];
                sprintf(msgShmIdString, "%d", msgShmId);
                execlp("./worker","./worker", msgShmIdString, NULL);
                fprintf(stderr,"%s failed to exec worker!\n",argv[0]);
                exit(1);
            }
            createdProcesses[totalCreatedProcesses] = newForkPid;
            totalCreatedProcesses++;
        }
        if ((msgShmPtr[2] >= 0) && (msgShmPtr[3] >= 0)){
            pid_t childEnded = wait(NULL);     
            printf("P: Child %d has terminated at system time %d:%d with termination time of %d:%d\n", childEnded, msgShmPtr[0], msgShmPtr[1], msgShmPtr[2], msgShmPtr[3]);
            closedChildren++;
            msgShmPtr[2] = -1;
            msgShmPtr[3] = -1;
        }
        msgShmPtr[1]++;
        if (msgShmPtr[1] >= 1000000000){
            msgShmPtr[1] -= 1000000000;
            msgShmPtr[0]++;
        }
    }
    closeProgram();
}

void childClosed(int sig){
    currentProcesses--;
    printf("Child Closed\n");
}

void closeProgramSignal(int sig){
    closeProgram();
}

int setInterrupt(){
    struct sigaction act;
    act.sa_sigaction = interrupt;
    act.sa_flags = 0;
    return ((sigemptyset(&act.sa_mask) == -1) || (sigaction(SIGALRM, &act, NULL) == -1));
}

void interrupt(int signo, siginfo_t* info, void* context){
    closeProgram();
}

int setTimer(double sec){
    struct itimerval value;
    value.it_interval.tv_sec = sec;
    value.it_interval.tv_usec = 0;
    value.it_value = value.it_interval;
    return (setitimer(ITIMER_PROF, &value,  NULL));
}

void closeProgram(){
    int i;
    for(i = 0; i < 100; i++){
        if(i > 0){
            if(createdProcesses[i] != -5){
                kill(createdProcesses[i], SIGINT);
            }
        }
    }
    shmctl(msgShmId, IPC_RMID, NULL);
    shmdt(msgShmPtr);
    sem_unlink(SNAME);
    exit(0);
}