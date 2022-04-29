#include <iostream>
#include<pthread.h>
#include<stdlib.h>
#include<stdio.h>
#include<sys/time.h>
#define NUM_THREADS 8
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
            A[i][j] = rand();
    }
    for(int k = 0;k < n; k++)
        for(int i = k + 1;i < n; i++)
            for(int j = 0;j < n; j++){
                A[i][j] += A[k][j];
            }

}
//线程数据结构定义
typedef struct{
    int t_id;//线程id
}threadParam_t;
//barrier定义
pthread_barrier_t barrier_Division;
pthread_barrier_t barrier_Elimination;
//线程函数定义
void *threadFunc(void *param){
    threadParam_t *p = (threadParam_t*)param;
    int t_id = p->t_id;

    for(int k = 0;k < n;++k){
        if(t_id == 0){
            for(int j = k + 1;j < n;++j){
                A[k][j] = A[k][j] / A[k][k];
            }
            A[k][k] = 1.0;
        }
        //第一个同步点
        pthread_barrier_wait(&barrier_Division);

        for(int i = k + 1 + t_id;i < n;i += NUM_THREADS){
            //消去
            for(int j = k + 1;j < n;++j){
                A[i][j] = A[i][j] - A[i][k] * A[k][j];
            }
            A[i][k] = 0.0;
        }
        //第二个同步点
        pthread_barrier_wait(&barrier_Elimination);
    }
    pthread_exit(NULL);

}
int main()
{
    //初始化
    ReSet();
    struct timeval head;
    struct timeval tail;
    //初始化barrier
    pthread_barrier_init(&barrier_Division,NULL,NUM_THREADS);
    pthread_barrier_init(&barrier_Elimination,NULL,NUM_THREADS);
    //创建线程
    pthread_t handles[NUM_THREADS];//创建对应的handle
    threadParam_t param[NUM_THREADS];//创建对应的线程数据结构
    gettimeofday(&head,NULL);
    for(int t_id = 0;t_id < NUM_THREADS;++t_id){
        param[t_id].t_id = t_id;
        pthread_create(&handles[t_id],NULL,threadFunc,(void*)&param[t_id]);
    }

    for(int t_id = 0;t_id < NUM_THREADS;++t_id){
        pthread_join(handles[t_id],NULL);
    }
    gettimeofday(&tail,NULL);
    cout<<"N: "<<n<<" Time: "<<(tail.tv_sec-head.tv_sec)*1000.0+(tail.tv_usec-head.tv_usec)/1000.0<<"ms";
    //销毁所有的barrier
    pthread_barrier_destroy(&barrier_Division);
    pthread_barrier_destroy(&barrier_Elimination);

    return 0;
}
