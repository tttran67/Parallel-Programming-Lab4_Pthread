
void* threadFunc(void* param)
{
    threadParam_t* p = (threadParam_t*)param;
    int t_id = p->t_id;
    uint32x4_t va_Pas =  vmovq_n_u32(0);
    uint32x4_t va_Act =  vmovq_n_u32(0);

    do
    {
        //������ش�����Ԫ��------------------------------------------------------
        //---------------------------begin-------------------------------------
        int i;
        for (i = lieNum - 1; i - 8 >= -1; i -= 8)
        {
            //ÿ�ִ���8����Ԫ�ӣ���Χ�������� i-7 �� i
            for (int j = t_id; j < pasNum; j+= NUM_THREADS)
            {
                //��4535������Ԫ����û�������ڴ˷�Χ�ڵ�
                while (Pas[j][Num] <= i && Pas[j][Num] >= i - 7)
                {
                    int index = Pas[j][Num];

                    if (Act[index][Num] == 1)//��Ԫ�Ӳ�Ϊ��
                    {
                        //Pas[j][]��Act[��Pas[j][18]��][]�����
                        //*******************SIMD�Ż�����***********************
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
                        //********************SIMD�Ż�����***********************

                        //����Pas[j][18]�������ֵ
                        //�������֮������������������浽Pas[j][18]�������ڷ�Χ������whileѭ��
                        //�����֮��Pas[j][ ]������
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
                    else//��Ԫ��Ϊ��
                    {
                        break;
                    }
                }
            }
        }

        for (i = i + 8; i >= 0; i--)
        {
            //ÿ�ִ���1����Ԫ�ӣ���Χ���������i
            for (int j = t_id; j < pasNum; j += NUM_THREADS)
            {
                //��53������Ԫ����û���������i��
                while (Pas[j][Num] == i)
                {
                    if (Act[i][Num] == 1)//��Ԫ�Ӳ�Ϊ��
                    {
                        //Pas[j][]��Act[i][]�����
                        //*******************SIMD�Ż�����***********************
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
                        //********************SIMD�Ż�����***********************


                        //����Pas[j][18]�������ֵ
                        //�������֮������������������浽Pas[j][18]�������ڷ�Χ������whileѭ��
                        //�����֮��Pas[j][ ]������
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
                    else//��Ԫ��Ϊ��
                    {
                        break;
                    }
                }
            }
        }

        //----------------------------------end--------------------------------
        //������ش�����Ԫ��--------------------------------------------------------




        if (t_id == 0)
        {
            for (int i = 0; i < NUM_THREADS - 1; ++i)
                sem_wait(&sem_leader); // �ȴ����� worker ��ɴ�����Ԫ��

        }
        else
        {
            //�����߳���ɡ����������֪ͨ1�߳�����ɣ�Ȼ�����˯�ߣ��ȴ�1�߳���������ٽ�����һ��
            sem_post(&sem_leader);// ֪ͨ leader, ����ɴ�����Ԫ��
            sem_wait(&sem_Next[t_id - 1]); // �ȴ�֪ͨ��������һ��
        }



        //����һ���߳�������Ԫ�ӵ�����
        if (t_id == 0)
        {

            //������Ԫ�ӣ�Ȼ���ж��Ƿ����
            sign = false;
            for (int i = 0; i < pasNum; i++)
            {
                //�ҵ���i������Ԫ�е�����
                int temp = Pas[i][Num];
                if (temp == -1)
                {
                    //˵�����Ѿ�������Ϊ��Ԫ����
                    continue;
                }

                //����������Ӧ����Ԫ���ǲ���Ϊ�գ���Ϊ�գ�����
                if (Act[temp][Num] == 0)
                {
                    //������Ԫ��
                    for (int k = 0; k < Num; k++)
                        Act[temp][k] = Pas[i][k];
                    //������Ԫ������
                    Pas[i][Num] = -1;
                    //��־bool��Ϊtrue��˵�����ֻ������
                    sign = true;
                }
            }

        }

        //t_id���������֪ͨ�����߳̿��Խ�����һ��
        if (t_id == 0)
        {
            for (int i = 0; i < NUM_THREADS - 1; ++i)
                sem_post(&sem_Next[i]); // ֪ͨ���� worker ������һ��
        }


    } while (sign == true);


    pthread_exit(NULL);
}





