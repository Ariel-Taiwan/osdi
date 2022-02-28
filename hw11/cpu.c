#define _GNU_SOURCE
#include <stdio.h>
#include <pthread.h>
#include <stdatomic.h>
#include <signal.h>
#include <unistd.h>
#include <sched.h>
#include <assert.h>
#include <stdlib.h>
#include <sys/time.h>

volatile long iteration;

#define cacheSize 1024*1024
char mem[cacheSize];

void sig_terminal_handler(int sig_no) {
    printf("iteration = %.2e\n", (float)iteration);
    exit(0);
}

void sig_sleep_handler(int sig_no) {
    sleep(0);
}


long randAccess() {
    while(1) {
        iteration++;
        for (int i=0; i<cacheSize; i+=64) {
            mem[i] += 1;
            if (iteration%5000==0 )
            ;
                //sleep(0);
        }
    }
}

int main(int argc, char** argv) {
    cpu_set_t set;

    int coreNum=0;
    //printf("usage: ./cpu core_to_bind OR ./cpu\n");
    if (argc == 2) {
        sscanf(argv[1], "%d", &coreNum);
    }

    CPU_ZERO(&set);
    CPU_SET(coreNum, &set);
    int ret=0;
    ret=sched_setaffinity(getpid(), sizeof(set), &set);
    assert(ret==0);
    //printf("bind the process to core #%d\n", coreNum);

    struct itimerval pitv = {{0,0}, {10,0}};
    setitimer(ITIMER_PROF, &pitv, NULL);
    signal(SIGPROF, sig_terminal_handler);
    

    //struct itimerval vitv = {{0,10000000}, {0,0}};
    //setitimer(ITIMER_VIRTUAL, &vitv, NULL);
    //signal(SIGPROF, sig_sleep_handler);
    
    printf("start\n");
    randAccess();
	return 0;
}
