#include <iostream>
#include<pthread.h>
#include<stdlib.h>
#include<stdio.h>
#include<sys/time.h>
#include<immintrin.h>
#include<windows.h>
#include<stdlib.h>
#define NUM_THREADS 7
using namespace std;
const int n = 2000;
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
}threadParam_t;
//barrier定义
pthread_barrier_t barrier_Division;
pthread_barrier_t barrier_Elimination;
//线程函数定义
void *threadFunc(void *param){
    threadParam_t *p = (threadParam_t*)param;
    int t_id = p->t_id;

    //__m128 t1, t2, t3;
    for(int k = 0;k < n;++k){
        int j = k + t_id + 1;
        for(;j < n; j += NUM_THREADS){
            A[k][j] = A[k][j] / A[k][k];
        }

        //第一个同步点
        pthread_barrier_wait(&barrier_Division);

        for(int i = k + 1 + t_id;i < n;i += NUM_THREADS){
            //消去
            for(int j = k + 1;j < n;++j){
                A[i][j] = A[i][j] - A[i][k] * A[k][j];
            }
            /*float temp[4] = { A[i][k],A[i][k],A[i][k],A[i][k] };
            t1 = _mm_loadu_ps(temp);
            int j = k + 1;
            for (; j + 4 <= n; j += 4)
            {
                t2 = _mm_loadu_ps(&A[k][j]);
                t3 = _mm_loadu_ps(&A[i][j]);
                t2 = _mm_mul_ps(t2, t1);
                t3 = _mm_sub_ps(t3, t2);
                _mm_storeu_ps(&A[i][j], t3);
            }
            for (; j < n; j++)
            {
                A[i][j] = A[i][j] - A[i][k] * A[k][j];
            }*/
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
    //初始化barrier
    pthread_barrier_init(&barrier_Division,NULL,NUM_THREADS);
    pthread_barrier_init(&barrier_Elimination,NULL,NUM_THREADS);
    //创建线程
    pthread_t handles[NUM_THREADS];//创建对应的handle
    threadParam_t param[NUM_THREADS];//创建对应的线程数据结构
   long long mhead,mtail;
   QueryPerformanceCounter((LARGE_INTEGER*)&mhead);

    for(int t_id = 0;t_id < NUM_THREADS;++t_id){
        param[t_id].t_id = t_id;
        pthread_create(&handles[t_id],NULL,threadFunc,(void*)&param[t_id]);
    }
QueryPerformanceCounter((LARGE_INTEGER*)&mtail);
    for(int t_id = 0;t_id < NUM_THREADS;++t_id){
        pthread_join(handles[t_id],NULL);
    }
    //QueryPerformanceCounter((LARGE_INTEGER*)&mtail);
    cout<<(mtail - mhead)/10;
    //销毁所有的barrier
    pthread_barrier_destroy(&barrier_Division);
    pthread_barrier_destroy(&barrier_Elimination);

    return 0;
}
