#include <iostream>
#include<sys/time.h>
#include<windows.h>
#include<stdlib.h>
using namespace std;
const int maxN = 2000;
float A[maxN][maxN];
void ReSet(){
    for(int i = 0;i < maxN; i++){
        for(int j = 0;j < i; j++)
            A[i][j] = 0;
        A[i][i] = 1.0;
        for(int j = i + 1;j < maxN; j++)
            A[i][j] = rand() % 100;
    }
    for(int i = 0;i < maxN; i++){
        int k1 = rand() % maxN;
        int k2 = rand() % maxN;
        for(int j = 0;j < maxN; j++){
            A[i][j] += A[0][j];
            A[k1][j] += A[k2][j];
        }
    }

}
void LU(){
    for(int k = 0;k < maxN; k++){
        for(int j = k + 1;j < maxN; j++){
            A[k][j] = A[k][j] * 1.0 / A[k][k];
        }
        A[k][k] = 1.0;
        for(int i = k + 1;i < maxN; i++){
            for(int j = k + 1;j < maxN; j++){
                A[i][j] = A[i][j] - A[i][k] * A[k][j];
            }
            A[i][k] = 0;
        }
    }
    return;
}
void Print(){
    for(int i = 0;i < maxN; i++){
        for(int j = 0;j < maxN; j++)
            cout<<A[i][j]<<" ";
        cout<<endl;
    }


}
int main()
{
    struct timeval head;
    struct timeval tail;

    ReSet();
    gettimeofday(&head,NULL);
    LU();
    gettimeofday(&tail,NULL);
    cout<<"N: "<<maxN<<" Time: "<<(tail.tv_sec-head.tv_sec)*1000.0+(tail.tv_usec-head.tv_usec)/1000.0<<"ms";


    return 0;
}
