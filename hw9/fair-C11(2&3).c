#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <stdatomic.h>
#include <pthread.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#define likely(x)    __builtin_expect(!!(x), 1)
#define unlikely(x)  __builtin_expect(!!(x), 0)

static inline int my_spin_lock (atomic_int *lock) {
    int val=0;
    if (likely(atomic_exchange_explicit(lock, 1, memory_order_acq_rel) == 0))//acq_rel: 讀取加釋放訊號。假設spinlock會成功，回傳lock舊的值，若lock=0，有lock了，就回傳0結束；若lock=1，交換值，if不成立，執行下面do while
        return 0;
    do {
        do {    //讓大家隨機地等待一段時間(TODO Back-off)
            asm("pause");   //相當於GNU的atomic_spin_nop()，目的不希望CPU太熱(燙是因為不斷測試)
        } while (*lock != 0);   //(假設cache有實現memory coherence) 直接讀lock值，為0，跳出
        val = 0;    //讓下面atomic_compare_exchange成功(lock為0)
    } while (!atomic_compare_exchange_weak_explicit(lock, &val, 1, memory_order_acq_rel, memory_order_relaxed));//weak: 可能所有人都失敗。比較慢，所以需要17到19行，效能會變好。lock和val都等於0，跳出do while，lock被設定成1，再鎖上
    return 0;
}
static inline int my_spin_unlock(atomic_int *lock) {
    atomic_store_explicit(lock, 0, memory_order_release);   //release: 釋放訊號。把lock值設定成0
    return 0;
}

atomic_int a_lock;
atomic_long count_array[256];
int numCPU;

void sigHandler(int signo) {
    for (int i=0; i<numCPU; i++) {
        printf("%i, %ld\n", i, count_array[i]);
    }
    exit(0);
}

atomic_int in_cs=0;
atomic_int wait=1;

void thread(void *givenName) {
    int givenID = (intptr_t)givenName;
    srand((unsigned)time(NULL));
    unsigned int rand_seq;
    cpu_set_t set; CPU_ZERO(&set); CPU_SET(givenID, &set);  //第0個thread在第0個core上面，第1個thread在第1個core上面
    sched_setaffinity(gettid(), sizeof(set), &set);      //設定親和力，表示可以在其上執行程序的 CPU 核心集
    while(atomic_load_explicit(&wait, memory_order_acquire))    //等待，讓大家一起同時往下執行下面的while(1)
        ;
    while(1) {
        my_spin_lock(&a_lock);
        atomic_fetch_add(&in_cs, 1);
        atomic_fetch_add_explicit(&count_array[givenID], 1, memory_order_relaxed);  //統計上鎖次數，進入幾次就會加了多少
        if (in_cs != 1) {   //驗證進入CS的thread，只有一個
            printf("violation: mutual exclusion\n");
            exit(0);    //重大錯誤結束程式
        }
        atomic_fetch_add(&in_cs, -1);   //把cs減1
        my_spin_unlock(&a_lock);
        int delay_size = rand_r(&rand_seq)%73;
        for (int i=0; i<delay_size; i++)    //讓他sleep(最多72次)，怕會有隱含的同步效應
            ;
    }
}

int main(int argc, char **argv) {
    signal(SIGALRM, sigHandler);
    alarm(5);
    numCPU = sysconf( _SC_NPROCESSORS_ONLN );
    pthread_t* tid = (pthread_t*)malloc(sizeof(pthread_t) * numCPU);

    for (long i=0; i< numCPU; i++)
        pthread_create(&tid[i],NULL,(void *) thread, (void*)i);
    atomic_store(&wait,0);

    for (int i=0; i< numCPU; i++)
        pthread_join(tid[i],NULL);
}
