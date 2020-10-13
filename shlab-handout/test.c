#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

unsigned int snooze(unsigned int secs){
    unsigned int rc = sleep(secs);
    printf("Sleep for %d of %d secs. \n", secs-rc, secs);
    return rc;
}

int myecho(int argc, char **argv, char **envp){
    
    // Print command line arguments
    for(int i=0; i <argc; i++){
        printf("argv[ %d]: %s \n", i, argv[i]);
    }

    // Print enviromental variables
    for (int j=0;;j++){
        if (envp[j] == NULL){
            return 0;
        }
        printf("envp[ %d]: %s \n", j, envp[j]);
    }
}

int main(int argc, char *argv[], char *envp[]){
    myecho(argc, argv, envp);
    return 0;
}