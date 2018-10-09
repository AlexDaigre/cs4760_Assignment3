#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/shm.h>

int main (int argc, char *argv[]) {
    int c;
    const int TOTALCHILDREN = 100;
    int maxNumberOfChildren = 5;
    int maxRunTime = 2;
    char* logFile = "logFile.txt";

    while ((c = getopt (argc, argv, "hs:l:t:")) != -1){
        switch (c){
            case 'h':
                printf("Help\n");
                exit(0);
                break;
            case 's':
                maxNumberOfChildren = atoi(optarg);
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

    printf("Number of children: %d\n", maxNumberOfChildren);
    printf("Log file name: %s\n", logFile);
    printf("Max run time: %d\n", maxRunTime);

    int i;
    pid_t newForkPid;
    for(i = 0; i < TOTALCHILDREN; i++){
        newForkPid = fork();
        if (newForkPid == 0){
            execlp("./worker","./worker", NULL);
		    fprintf(stderr,"%s failed to exec worker!\n",argv[0]);
            exit(1);
        }
    }

    while(1==1){
        printf("Sleeping.\n");
        sleep(5);
    }
}