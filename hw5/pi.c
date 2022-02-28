#define _GNU_SOURCE
#include <stdio.h>
#include <pthread.h>
#include <stdatomic.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <strings.h>
#include <sys/syscall.h> 
#include <assert.h>
#include <string.h>
#include <math.h>
#include <limits.h>
//#define loopCount 100000000

int numCPU=-1;
char* exename;
int max=2147483647;
double width,size,pi;
double deloop;

/*long gettid() {
    return (long int)syscall(__NR_gettid);
}*/

//注意，我使用了「volatile」
volatile double score[100],scoreup[100];

void thread(void *givenName) {
    int id = (intptr_t)givenName;
    //deloop=(double)1000000000/numCPU+1;
    deloop=(double)7*( 2147483640/(numCPU+6) +1);
    
    width=1/(double)numCPU;
    size=width/deloop;
        
    for(long i=0;i<deloop-1;i++){
    	double x=(double) width*id + i*size;
    	score[id]+=size*sqrt(1-x*x);
    	x=(double) width*id + (i+1)*size;
    	scoreup[id]+=size*sqrt(1-x*x);
    }
}

int main(int argc, char **argv) {
	exename=argv[0];
    numCPU = sysconf( _SC_NPROCESSORS_ONLN );
    pthread_t* tid = (pthread_t*)malloc(sizeof(pthread_t) * numCPU);
    int x=atoi(argv[1]);
    
    for (long i=0; i< numCPU; i++)
        pthread_create(&tid[i], NULL, (void *) thread, (void*)i);

    for (int i=0; i< numCPU; i++)
	    pthread_join(tid[i], NULL);

    double totallow=0,upp=0;
    for (int i=0; i< numCPU; i++){
        totallow += score[i];
        upp+=scoreup[i];
    }
    printf("\n%.10lf\n",(totallow+upp)*2);    
}
