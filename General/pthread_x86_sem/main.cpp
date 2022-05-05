#include <iostream>
#include<pthread.h>
#include<stdlib.h>
#include<stdio.h>
#include<semaphore.h>
#include<sys/time.h>
#define NUM_THREADS 7
const int n = 2000;
float A[n][n];
using namespace std;
typedef struct{
    int t_id;
}threadParam_t;
sem_t sem_main;
sem_t sem_workerstart[NUM_THREADS];
sem_t sem_workerend[NUM_THREADS];
//³õÊ¼»¯¾ØÕóA
void ReSet(){
    for(int i = 0;i < n; i++){
        for(int j = 0;j < i; j++)
            A[i][j] = 0;
        A[i][i] = 1.0;
        for(int j = i + 1;j < n; j++)
            A[i][j] = rand() % 100;
    }
    for(int i = 0;i < n; i++){
        int k1 = rand() % n;
        int k2 = rand() % n;
        for(int j = 0;j < n; j++){
            A[i][j] += A[0][j];
            A[k1][j] += A[k2][j];
        }
    }

}
void *threadFunc(void *param){
    threadParam_t *p = (threadParam_t*)param;
    int t_id = p->t_id;
    for(int k = 0;k < n; k++){
        sem_wait(&sem_workerstart[t_id]);
        for(int i = k + 1 + t_id; i < n; i += NUM_THREADS){
           for(int j = k + 1;j < n; j++){
               A[i][j] = A[i][j] - A[i][k] * A[k][j];
           }
        A[i][k] = 0.0;
    }
    sem_post(&sem_main);
    sem_wait(&sem_workerend[t_id]);
    }
    pthread_exit(NULL);
}
int main()
{
    //³õÊ¼»¯
    ReSet();
    struct timeval head;
    struct timeval tail;
    sem_init(&sem_main,0,0);
    for(int i = 0;i < NUM_THREADS; i++){
        sem_init(&sem_workerstart[i],0,0);
        sem_init(&sem_workerend[i],0,0);
    }
    pthread_t handles[NUM_THREADS];
    threadParam_t param[NUM_THREADS];
    gettimeofday(&head,NULL);
    for(int t_id = 0;t_id < NUM_THREADS; t_id++){
        param[t_id].t_id = t_id;
        pthread_create(&handles[t_id],NULL,threadFunc,(void*)&param[t_id]);
    }
    for(int k = 0;k <n; k++){
        for(int j = k + 1;j < n; j++){
            A[k][j] = A[k][j] / A[k][k];
        }
        A[k][k] = 1.0;
        for(int t_id = 0;t_id <NUM_THREADS; t_id++){
            sem_post(&sem_workerstart[t_id]);
        }
        for(int t_id = 0;t_id < NUM_THREADS; t_id++){
            sem_wait(&sem_main);
        }
        for(int t_id = 0;t_id < NUM_THREADS; t_id++){
            sem_post(&sem_workerend[t_id]);
        }
    }
    for(int t_id = 0;t_id <NUM_THREADS; t_id++){
        pthread_join(handles[t_id],NULL);
    }
    gettimeofday(&tail,NULL);
    cout<<"N: "<<n<<" Time: "<<(tail.tv_sec-head.tv_sec)*1000.0+(tail.tv_usec-head.tv_usec)/1000.0<<"ms";
    sem_destroy(&sem_main);
    for(int i = 0;i < NUM_THREADS; i++){
        sem_destroy(&sem_workerstart[i]);
        sem_destroy(&sem_workerend[i]);
    }

    return 0;
}
