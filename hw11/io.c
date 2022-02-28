#define _GNU_SOURCE
#include <stdio.h>
#include <pthread.h>
#include <stdatomic.h>
#include <signal.h>
#include <unistd.h>
#include <sched.h>
#include <assert.h>
#include <stdlib.h>

long nOutput;

void sig_handler(int sig_no) {
    printf("output = %f MB/sec\n", ((double)nOutput*4096)/(1024*1024)/10);
    exit(0);
}

int main(int argc, char** argv) {
    cpu_set_t set;

    int coreNum;
    printf("usage: ./io core_to_bind OR ./io\n");
    if (argc == 2) {
        sscanf(argv[1], "%d", &coreNum);
    } else {
        printf("the default value of coreNum is 0\n");
    }

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
        fwrite(buf, 1024*4, 1, outputFile);
        nOutput++;
        fseek(outputFile, 0, SEEK_SET);
    }
	return 0;
}
