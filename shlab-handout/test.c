#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include "csapp.h"


unsigned int snooze(unsigned int secs){
    unsigned int rc = sleep(secs);
    printf("Sleep for %d of %d secs. \n", secs-rc, secs);
    return rc;
}


void sigint_handler(int sig){
    
    return;
}

int main(int argc, char *argv[]){

    // Install new signal handler
    if (signal(SIGINT, sigint_handler) == SIG_ERR){
        unix_error("signal_error");
    }

    snooze(atoi(argv[1]));

    return 0;
}