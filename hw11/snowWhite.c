#define _GNU_SOURCE
#include <stdio.h>
#include <pthread.h>
#include <stdatomic.h>
#include <signal.h>
#include <unistd.h>
#include <sched.h>
#include <assert.h>
#include <stdlib.h>
#include <time.h>
#include <limits.h>

long nOutput;
long iteration;

//3MB
char mem_area[1024*1024*3];

void sig_handler(int sig_no) {
    printf("iteration = %ld\n", iteration);
    exit(0);
}

long ts2ns(struct timespec ts) {
    return ts.tv_sec*1E9+ts.tv_nsec;
}

int main(int argc, char** argv) {
    cpu_set_t set;
    struct timespec ts1, ts2;

    long cpuburst=LONG_MAX;
    int coreNum=0;
    printf("usage: ./periodicTask yield_interval OR ./periodicTask\n");
    if (argc == 2) {
        sscanf(argv[1], "%ld", &cpuburst);
    } else {
        //printf("yield interval = %ld nanoseconds\n", cpuburst);
    }
    printf("yield interval = %ld nanoseconds\n", cpuburst);

    CPU_ZERO(&set);
    CPU_SET(coreNum, &set);
    char buf[1024*4];
    int ret=0;
    ret=sched_setaffinity(getpid(), sizeof(set), &set);
    assert(ret==0);
    printf("runing on core%d\n", coreNum);


    FILE* outputFile=fopen("/dev/null", "r+");
    signal(SIGALRM, sig_handler);
    alarm(10);

    while(1) {
        clock_gettime(CLOCK_REALTIME, &ts1);
        while(1) {
            iteration++;
            mem_area[rand()%(1024*1024*3)] += 1;
            clock_gettime(CLOCK_REALTIME, &ts2);
            if (ts2ns(ts2)-ts2ns(ts1) > cpuburst) {
                //printf("yield\n");
                //sleep(0);
                break;
            }
        }
    }
    long ret_val;
    for (int i=0; i< 1024*1024*3; i++)
        ret_val=+mem_area[i];
    printf("%ld\n", ret_val);
	return 0;
}
