#define _GNU_SOURCE
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <sched.h>
#include <sys/wait.h>

#define NUM_THREADS 100
#define NUM_TASKS 10000

static int cnt = 0;

void* worker(void* arg){
    int progress;
    for(int i = 0; i < NUM_TASKS; i++){
        progress = cnt++;
    }

    pthread_</4.1>(</4.2>);
}

int main(int argc, char* argv[]){
    pthread_t tids[NUM_THREADS];
    int status;
    int progress = 0;

    for(int i = 0; i < NUM_THREADS; i++){

        status = pthread_</1>(</2>);

        if(status !=0){
            printf("error");
            return -1;
        }
    }

    for(int i=0; i<NUM_THREADS; i++){
        pthread_</3.1>(</3.2>);

        printf("\r%d ", progress);

        fflush(stdout);
        usleep(10*1000); // 10ms
    }

    printf("\nexpected: %d\n", NUM_THREADS * NUM_TASKS);
    printf("result: %d\n", cnt);

    return 0;
}

/*
Expected output:

6267912 
expectd: 1000000
result: some number (could be 1000000)
*/