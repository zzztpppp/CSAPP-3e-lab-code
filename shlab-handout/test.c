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
        printf("argv[ %d]: %s", )
    }
}

int main(int argc, char *argv[]){
    int in = atoi(argv[1]);
    snooze(in);
    return 0;
}