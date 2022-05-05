
void* threadFunc(void* param)
{
    threadParam_t* p = (threadParam_t*)param;
    int t_id = p->t_id;
    uint32x4_t va_Pas =  vmovq_n_u32(0);
    uint32x4_t va_Act =  vmovq_n_u32(0);

    do
    {
        //不升格地处理被消元行------------------------------------------------------
        //---------------------------begin-------------------------------------
        int i;
        for (i = lieNum - 1; i - 8 >= -1; i -= 8)
        {
            //每轮处理8个消元子，范围：首项在 i-7 到 i
            for (int j = t_id; j < pasNum; j+= NUM_THREADS)
            {
                //看4535个被消元行有没有首项在此范围内的
                while (Pas[j][Num] <= i && Pas[j][Num] >= i - 7)
                {
                    int index = Pas[j][Num];

                    if (Act[index][Num] == 1)//消元子不为空
                    {
                        //Pas[j][]和Act[（Pas[j][18]）][]做异或
                        //*******************SIMD优化部分***********************
                        //********
                        int k;
                        for (k = 0; k+4 <= Num; k+=4)
                        {
                            //Pas[j][k] = Pas[j][k] ^ Act[index][k];
                            va_Pas =  vld1q_u32(& (Pas[j][k]));
                            va_Act =  vld1q_u32(& (Act[index][k]));

                            va_Pas = veorq_u32(va_Pas,va_Act);
                            vst1q_u32( &(Pas[j][k]) , va_Pas );
                        }

                        for( ; k<Num; k++ )
                        {
                            Pas[j][k] = Pas[j][k] ^ Act[index][k];
                        }
                        //*******
                        //********************SIMD优化部分***********************

                        //更新Pas[j][18]存的首项值
                        //做完异或之后继续找这个数的首项，存到Pas[j][18]，若还在范围里会继续while循环
                        //找异或之后Pas[j][ ]的首项
                        int num = 0, S_num = 0;
                        for (num = 0; num < Num; num++)
                        {
                            if (Pas[j][num] != 0)
                            {
                                unsigned int temp = Pas[j][num];
                                while (temp != 0)
                                {
                                    temp = temp >> 1;
                                    S_num++;
                                }
                                S_num += num * 32;
                                break;
                            }
                        }
                        Pas[j][Num] = S_num - 1;
                    }
                    else//消元子为空
                    {
                        break;
                    }
                }
            }
        }

        for (i = i + 8; i >= 0; i--)
        {
            //每轮处理1个消元子，范围：首项等于i
            for (int j = t_id; j < pasNum; j += NUM_THREADS)
            {
                //看53个被消元行有没有首项等于i的
                while (Pas[j][Num] == i)
                {
                    if (Act[i][Num] == 1)//消元子不为空
                    {
                        //Pas[j][]和Act[i][]做异或
                        //*******************SIMD优化部分***********************
                        //********
                        int k;
                        for (k = 0; k+4 <= Num; k+=4)
                        {
                            //Pas[j][k] = Pas[j][k] ^ Act[index][k];
                            va_Pas =  vld1q_u32(& (Pas[j][k]));
                            va_Act =  vld1q_u32(& (Act[i][k]));

                            va_Pas = veorq_u32(va_Pas,va_Act);
                            vst1q_u32( &(Pas[j][k]) , va_Pas );
                        }

                        for( ; k<Num; k++ )
                        {
                            Pas[j][k] = Pas[j][k] ^ Act[i][k];
                        }
                        //*******
                        //********************SIMD优化部分***********************


                        //更新Pas[j][18]存的首项值
                        //做完异或之后继续找这个数的首项，存到Pas[j][18]，若还在范围里会继续while循环
                        //找异或之后Pas[j][ ]的首项
                        int num = 0, S_num = 0;
                        for (num = 0; num < Num; num++)
                        {
                            if (Pas[j][num] != 0)
                            {
                                unsigned int temp = Pas[j][num];
                                while (temp != 0)
                                {
                                    temp = temp >> 1;
                                    S_num++;
                                }
                                S_num += num * 32;
                                break;
                            }
                        }
                        Pas[j][Num] = S_num - 1;

                    }
                    else//消元子为空
                    {
                        break;
                    }
                }
            }
        }

        //----------------------------------end--------------------------------
        //不升格地处理被消元行--------------------------------------------------------




        if (t_id == 0)
        {
            for (int i = 0; i < NUM_THREADS - 1; ++i)
                sem_wait(&sem_leader); // 等待其它 worker 完成处理被消元行

        }
        else
        {
            //其他线程完成“处理”任务后，通知1线程已完成；然后进入睡眠，等待1线程完成升格，再进入下一轮
            sem_post(&sem_leader);// 通知 leader, 已完成处理被消元行
            sem_wait(&sem_Next[t_id - 1]); // 等待通知，进入下一轮
        }



        //其中一个线程做对消元子的升格
        if (t_id == 0)
        {

            //升格消元子，然后判断是否结束
            sign = false;
            for (int i = 0; i < pasNum; i++)
            {
                //找到第i个被消元行的首项
                int temp = Pas[i][Num];
                if (temp == -1)
                {
                    //说明他已经被升格为消元子了
                    continue;
                }

                //看这个首项对应的消元子是不是为空，若为空，则补齐
                if (Act[temp][Num] == 0)
                {
                    //补齐消元子
                    for (int k = 0; k < Num; k++)
                        Act[temp][k] = Pas[i][k];
                    //将被消元行升格
                    Pas[i][Num] = -1;
                    //标志bool设为true，说明此轮还需继续
                    sign = true;
                }
            }

        }

        //t_id完成了升格，通知其他线程可以进入下一轮
        if (t_id == 0)
        {
            for (int i = 0; i < NUM_THREADS - 1; ++i)
                sem_post(&sem_Next[i]); // 通知其它 worker 进入下一轮
        }


    } while (sign == true);


    pthread_exit(NULL);
}





