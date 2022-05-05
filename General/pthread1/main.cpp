#include <iostream>
#include<pthread.h>
#include<stdlib.h>
#include<stdio.h>
#include<sys/time.h>
#define NUM_THREADS 7
using namespace std;
const int n = 500;
float A[n][n];

//初始化矩阵A
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
//线程数据结构定义
typedef struct{
    int t_id;//线程id
    int k;
}threadParam_t;
void *threadFunc(void *param){
    threadParam_t *p = (threadParam_t*)param;
    int k = p->k;
    int t_id = p->t_id;
    int i = k + t_id + 1;
    for(;i < n; i += NUM_THREADS){
        for(int j = k + 1;j < n; j++){
            A[i][j] = A[i][j] - A[i][k] * A[k][j];
        }
            A[i][k] = 0;
        }
    pthread_exit(NULL);
}
int main()
{
    ReSet();
    struct timeval head;
    struct timeval tail;
    gettimeofday(&head,NULL);
    for(int k = 0;k < n; k++){
        for(int j = k + 1;j < n; j++){
            A[k][j] = A[k][j] / A[k][k];
        }
        A[k][k] = 1.0;
        //int worker_count = n - 1 - k;
        pthread_t* handles = new pthread_t[NUM_THREADS];
        threadParam_t* param = new threadParam_t[NUM_THREADS];
        for(int t_id = 0;t_id < NUM_THREADS; t_id++){
            param[t_id].k = k;
            param[t_id].t_id = t_id;
        }

        for(int t_id = 0;t_id < NUM_THREADS; t_id++){
            pthread_create(&handles[t_id],NULL,threadFunc,(void*)&param[t_id]);
        }
        for(int t_id = 0;t_id < NUM_THREADS; t_id++){
            pthread_join(handles[t_id],NULL);
        }

    }
    gettimeofday(&tail,NULL);
    cout<<"N: "<<n<<" Time: "<<(tail.tv_sec-head.tv_sec)*1000.0+(tail.tv_usec-head.tv_usec)/1000.0<<"ms";
    return 0;
}
