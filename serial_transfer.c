#include<stdio.h>      /*��׼�����������*/
#include<stdlib.h>     /*��׼�����ⶨ��*/
#include<unistd.h>     /*Unix ��׼��������*/
#include<sys/types.h>   
#include<sys/stat.h>     
#include<fcntl.h>      /*�ļ����ƶ���*/
#include<termios.h>    /*PPSIX �ն˿��ƶ���*/
#include<errno.h>      /*����Ŷ���*/
#include<string.h>  
#include <sys/stat.h>
#include <sys/time.h>  

//�궨��  
#define FALSE  -1  
#define TRUE   0  

/******************************************************************* 
����    UART0_Open
���� �򿪴��ڲ����ش����豸�ļ�����
����
fd �ļ�������
port ���ں�(ttyS0, ttyS1, ttyS2)
��ȷ����Ϊ1�����󷵻�Ϊ0
*******************************************************************/  
int UART0_Open(int fd, char* port)
{
	fd = open( port, O_RDWR|O_NOCTTY|O_NDELAY);
	if (FALSE == fd)
	{
		perror("Can't Open Serial Port");
		return(FALSE);
	}
	//�ָ�����Ϊ����״̬
	if(fcntl(fd, F_SETFL, 0) < 0)
	{
		printf("fcntl failed!\n");
		return(FALSE);
	}
	else
	{
		printf( "fcntl = %d\n", fcntl(fd, F_SETFL, 0) );  
	}
	
	//�����Ƿ�Ϊ�ն��豸
	if(0 == isatty(STDIN_FILENO))
	{
		printf("standard input is not a terminal device\n");
		return(FALSE);
	}
	else
	{
		printf("isatty success!\n");
	}
	printf("fd->open = %d\n", fd);
	return fd;
}

/******************************************************************* 
����    UART0_Close
���� �رմ��ڲ����ش����豸�ļ�����
����
fd �ļ�������
port ���ں�(ttyS0, ttyS1, ttyS2)
*******************************************************************/
void UART0_Close(int fd)
{
	close(fd);
}

/******************************************************************* 
���� UART0_Set
���� ���ô�������λ��ֹͣλ��Ч��λ
��ڲ���
fd �����ļ�������
speed �����ٶ�
flow_ctrl ����������
databits ����λ   ȡֵΪ 7 ����8
stopbitsֹͣλ   ȡֵΪ 1 ����2
parity Ч������ ȡֵΪN, E, O, S
��ȷ����Ϊ1�����󷵻�Ϊ0
*******************************************************************/  
int UART0_Set(int fd, int speed, int flow_ctrl, int databits, int stopbits, int parity)
{
	int i;
	int status;
	int speed_arr[] = { B115200, B19200, B9600, B4800, B2400, B1200, B300};
	int name_arr[] = { 115200, 19200,  9600,  4800,  2400,  1200,  300};
    
	struct termios options;
    
	/*tcgetattr(fd,&options)�õ���fdָ��������ز������������Ǳ�����options,�ú���
	�����Բ��������Ƿ���ȷ���ô����Ƿ���õȡ������óɹ�����������ֵΪ0��������ʧ�ܣ�
	��������ֵΪ1. */
	if( tcgetattr( fd, &options) != 0 )
	{
		perror("SetupSerial 1");
		return(FALSE);
	}
    
    //���ô������벨���ʺ����������  
	for ( i= 0;  i < sizeof(speed_arr) / sizeof(int);  i++)
	{
		if  (speed == name_arr[i])
		{
			cfsetispeed(&options, speed_arr[i]);
			cfsetospeed(&options, speed_arr[i]);
		}
	}
     
    //�޸Ŀ���ģʽ����֤���򲻻�ռ�ô���  
    options.c_cflag |= CLOCAL;  
    //�޸Ŀ���ģʽ��ʹ���ܹ��Ӵ����ж�ȡ��������  
    options.c_cflag |= CREAD;
    
    //��������������
    switch(flow_ctrl)
    {
		case 0:		//��ʹ��������
			options.c_cflag &= ~CRTSCTS;
			break;
        
		case 1:		//ʹ��Ӳ��������
			options.c_cflag |= CRTSCTS;
			break;
		case 2:		//ʹ�����������
			options.c_cflag |= IXON | IXOFF | IXANY;
			break;
    }
    //��������λ
    //����������־λ
    options.c_cflag &= ~CSIZE;
    switch (databits)
    {
		case 5:
			options.c_cflag |= CS5;
			break;
		case 6:
			options.c_cflag |= CS6;
			break;
		case 7:
			options.c_cflag |= CS7;
			break;
		case 8:
			options.c_cflag |= CS8;
			break;
		default:
			fprintf(stderr, "Unsupported data size\n");
			return (FALSE);
    }
    //����У��λ
    switch (parity)
    {
		case 'n':
		case 'N': //����żУ��λ
			options.c_cflag &= ~PARENB;
			options.c_iflag &= ~INPCK;
			break;
		case 'o':
		case 'O'://����Ϊ��У��
			options.c_cflag |= (PARODD | PARENB);
			options.c_iflag |= INPCK;
			break;
		case 'e':
		case 'E'://����ΪżУ��
			options.c_cflag |= PARENB;
			options.c_cflag &= ~PARODD;
			options.c_iflag |= INPCK;
			break;
		case 's':
		case 'S': //����Ϊ�ո�
			options.c_cflag &= ~PARENB;
			options.c_cflag &= ~CSTOPB;
			break;
        default:
			fprintf(stderr,"Unsupported parity\n");
			return (FALSE);
    }
    // ����ֹͣλ
    switch (stopbits)
    {
		case 1:
			options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
			options.c_oflag &= ~OPOST;
			break;

		case 2:
			options.c_cflag |= CSTOPB;
			break;
			
		default:
			fprintf(stderr, "Unsupported stop bits\n");
			return (FALSE);
    }
    
	//�޸����ģʽ��ԭʼ�������  
	options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
	options.c_oflag &= ~OPOST;
    
    //���õȴ�ʱ�����С�����ַ�
    options.c_cc[VTIME] = 1;	/* ��ȡһ���ַ��ȴ�1*(1/10)s */
    options.c_cc[VMIN] = 0;	/* ��ȡ�ַ������ٸ���Ϊ1 */
    /*
    VTIME	VMIN
    0		>0		һֱ���������յ�VMIN������ʱread����
    >0		0		��ͨ��ʱ
    >0		>0		�����յ���һ���ֽ�ʱ��ʼ���㳬ʱ��
	�����ʱʱ��δ���������Ѿ��ﵽVMIN��read�������ء�
	�����ʱʱ�䵽�˾ͷ��ص�ǰ�����ĸ�����
    */
    
    //�����������������������ݵ��ǲ��ٶ�ȡ
    tcflush(fd, TCIFLUSH);
    
    //�������� (���޸ĺ��termios�������õ������У�
    if (tcsetattr(fd, TCSANOW, &options) != 0)
	{
		perror("com set error!\n");
		return (FALSE);
	}
    return (TRUE);
}

/******************************************************************* 
���ƣ�   UART0_Init()
���ܣ����ڳ�ʼ��
������
fd �ļ�������
speed �����ٶ�
flow_ctrl ����������
databits ����λ   ȡֵΪ 7 ����8
stopbits ֹͣλ   ȡֵΪ 1 ����2
parity Ч������ ȡֵΪN, E, O, S 
��ȷ����Ϊ1�����󷵻�Ϊ0
*******************************************************************/  
int UART0_Init(int fd, int speed, int flow_ctrl, int databits, int stopbits, int parity)
{
    int err;
    //���ô�������֡��ʽ
    if ( UART0_Set(fd, 115200, 0, 8, 1, 'N') == FALSE )
	{
		return FALSE;
	}
    else
	{
		return  TRUE;
	}
}

/*******************************************************************
���� UART0_Recv
����    ���մ�������
����
fd �ļ�������
rcv_buf ���մ��������ݴ���rcv_buf��������
data_len һ֡���ݵĳ���
��ȷ����Ϊ1�����󷵻�Ϊ0
*******************************************************************/  
int UART0_Recv(int fd, char *rcv_buf,int data_len)  
{
	int len, fs_sel;
    fd_set fs_read;
    
    struct timeval time;
    
    FD_ZERO(&fs_read);
    FD_SET(fd, &fs_read);
    
    time.tv_sec = 10;
    time.tv_usec = 0;
    
    //ʹ��selectʵ�ִ��ڵĶ�·ͨ��
    fs_sel = select(fd + 1, &fs_read, NULL, NULL, &time);
    printf("fs_sel = %d\n", fs_sel);
    if(fs_sel)
	{
		len = read(fd, rcv_buf, data_len);
		printf("(version1.2) len = %d fs_sel = %d\n", len, fs_sel);
		return len;
	}
    else
	{
		printf("fault!\n");
		return FALSE;
	}
}

int readDataFun (int fd, char *pp, int *iLen)
{
    char ch[55];
    int i;
    
    memset (ch, 0, sizeof (ch));
    if ((*iLen = read (fd, ch, 55)) > 0)
    {
        memcpy(pp, ch, *iLen);
        return *iLen;
    }
    return 0;
}

unsigned char cal_sum(unsigned char data[], unsigned char len)
{
	int i;
	unsigned char sum = 0;
	for(i = 0; i++; i < len)
	{
		sum = sum + data[i];
	}
	return sum;
}

int file_size2(char* filename)
{
    struct stat statbuf;
    stat(filename,&statbuf);
    int size=statbuf.st_size;
 
    return size;
}

int send_file(int fd,FILE *filep,int size)
{
	float rat;
	unsigned char st[55] = {0};
	unsigned char r_buf[4];
	memset (r_buf, 0, sizeof (r_buf));
	unsigned char bbuf[4];
	unsigned char stt = 0xAA;
	unsigned char sp = 0x55;
	unsigned char ok_f = 0xA5;
	unsigned char resend_f = 0x5A;
	bbuf[2] = ok_f;
	bbuf[3] = resend_f;
	unsigned char count = 1;
	unsigned char length;
	unsigned char c_sum;
	int iLen;
	unsigned char rsize;
	unsigned char retv;

	unsigned char max_buffer_size = 50;
	unsigned char sbuf[50];
	
	int size_send = 0;
	
	int numm = 1;
	
	int jj = 0;		//���ջش����ݴ������
	int timee = 0;		
	memset (sbuf, 0, sizeof (sbuf));
	while(!feof(filep))
	{
		rsize = fread(sbuf, 1, max_buffer_size, filep);
		if(rsize > 0)
		{
			memset (st, 0, sizeof (st));
			st[0] = stt;
			st[1] = stt;
			st[2] = count;
			st[3] = rsize;
			
			memcpy(&st[4], sbuf, sizeof(sbuf));
			
			c_sum = cal_sum(sbuf, rsize);
			
			st[4 + sizeof(sbuf)] = c_sum;
			 
			retv = write(fd, st, rsize + 5);	//������
			int aa = retv;
			
			//printf("%s \n", sbuf);
			 
			//printf("the number of charater sent is %d\n", retv);
			//printf("is %d  \t st[2] is %d\n", count, st[2]);
			int num = 0;
			while(1)	//�ȴ����ջش�����
			{
				if( (num = read(fd, r_buf, 4)) > 0 )	//�ջش�����
				{
					//printf("%s \n", r_buf);
					//printf("%d \n", r_buf[2]);
					
					if(r_buf[2] == bbuf[2] || r_buf[3] == ok_f)
					{
						count++;
						if(count == 10)
							count=1;
					
						size_send += rsize;
						rat = (float)size_send/size*100;
						//printf("has send : %.4f %%\n", rat);
						break;
					}
					else if( r_buf[2] == bbuf[3] || r_buf[3] == resend_f )   
					{
						//printf("start resend \n");
						int i = 1;
						int flag = 0;
						while(i < 10)
						{
							retv=write(fd,st,rsize+5);  //������	
							int aa=retv;
							
							//printf("resend %d time", i);
							while( (num = read(fd, r_buf, 4)) > 0 )
							{
								if( r_buf[2] == bbuf[2] || r_buf[3] == bbuf[2] )
								{
									flag = 1;
									count++;
									if(count == 10)
										count=1;
									size_send += rsize;
									rat = (float)size_send/size*100;
									//printf("has send : %.4f %%\n", rat); 
									break;
								}
								else
								{
									flag = 0;
									break;
								}
							}
							
							if(flag == 1)
							{
								break;
							}
							i++;
						}
						if(i == 10)
						{
							//printf("resend  timeout!\n ");
							return 0;
						}
						break;
					}
					else
					{
						printf("error: Send back status fault!\n");
						jj++;
						if(jj == 11)
						{
							printf("error:Can't receive valid send back data, please check link!\n");
							return 0;
						}
					}
				}
			}
		}
	}
	printf("---send-finished!---\nfilesize��%.1f  KB\n",(float)size/1000);

}

int receive(int fd, FILE *fout)
{
  	int num = 0;
   	unsigned char read_buf[55];
   	unsigned char bbuf[50];
   	unsigned char send_buf[4];
   	memset (read_buf, 0, sizeof (read_buf));
    int iLen;
   	int ncount = 0;
   	unsigned char stt = 0xAA;
   	unsigned char sp = 0x55;
   	unsigned char ok_f = 0xA5;
   	unsigned char resend_f = 0x5A;
   	unsigned char max_buffer_size = 50;
   	
   	/*
   	 ��������������������һ֡��Ϣ
   	 ���ȴ�������ݴ�����һ����Ҫ���ط���֡��Ϣ
   	 ��Ŵ���˰�Ϊ��һ����������һ֡��Ϣ
   	 */
   	
 	unsigned char count = 1;
 	
 	unsigned char r_count;
	unsigned char length;
	unsigned char solength;
	unsigned char c_sum; 
	unsigned char sum;
    
	int ii = 0;			//���ݴ������
	int iii = 0;		//�ش�ʧ�ܴ���
	int time = 0;		//�ش�����������ݵȴ�
	int time_f = 0;
	int j = 0;			//�ش���ʧ�ط�����
	
	struct timeval tv;
	long timer = 0;
	long timerr = 0;
	gettimeofday(&tv, NULL);
	timerr = tv.tv_sec;
	//printf("%ld \n", timerr);
	
	while(1)
	{
		if(time_f == 0)
		{
			gettimeofday(&tv, NULL);
			if(tv.tv_sec-timerr > 20)	//20S
			{
				printf("No received data!\n");
				return 0;
			}
		}
		if(time_f == 1)
		{
			gettimeofday(&tv, NULL);
			if(tv.tv_sec-timer > 1)	//�ȴ�1��
			{
				write(fd, send_buf, 4);
				//printf("Send back data %s \n", send_buf);
				j++;
				if(j == 10)
				{
					fclose(fout);
					printf("Send back failed, please check link!\n");
					return 0;
				}
			}
		}
		//printf("time is : %d \n ",time);
		if( (num = readDataFun(fd, read_buf, &iLen)) > 0 )
		{
			time_f = 1;		//�Ѿ��յ�������
			time = 0;
			gettimeofday(&tv, NULL);
			timer = tv.tv_sec;
			//printf("recieve data: %s \t \n", read_buf);
			if( read_buf[0] == stt && read_buf[1] == stt )
			{
				r_count = read_buf[2];
				//printf("r_count:%d  \t count:%d\n", r_count, count);
				if(r_count==count) // �����ȷ
				{
					length = read_buf[3];
					memcpy(bbuf, &read_buf[4], length);
					sum = read_buf[3 + length + 1];
					
					c_sum = cal_sum(bbuf, length);
					if(length < max_buffer_size)	// ���һ֡
					{
						count++;
						if(count == 10)
							count = 1;
							
						//printf("length : %d \n", num);
						//printf("data_len : %d \n", length);
					
						memcpy(bbuf, &read_buf[4], length);
						fwrite(bbuf, length, 1, fout);
						//printf("%s \n", bbuf);
						
						send_buf[0] = stt;
						send_buf[1] = stt;
						send_buf[2] = ok_f;
						send_buf[3] = ok_f;
						
						write(fd, send_buf, 4);
						gettimeofday(&tv, NULL);
						timer = tv.tv_sec;
						 
						fclose(fout);
						printf("---recieve-finished!---\n");
						gettimeofday(&tv, NULL);
						timerr = tv.tv_sec;
						//printf("%ld \n", timerr);
						//printf("%ld \n", tv.tv_sec - timerr);
						return 0;
						
					
					}
					if(c_sum == sum)	//������ȷ
					{
						count++;
						if(count==10)
							count=1;
					
						//printf("length : %d \n",num);
						//printf("data_len : %d \n",length);
					
						memcpy(bbuf, &read_buf[4], length);
						fwrite(bbuf, length, 1, fout); 
						//printf("%s \n",bbuf);
						
						send_buf[0] = stt;
						send_buf[1] = stt;
						send_buf[2] = ok_f;
						send_buf[3] = ok_f;
						unsigned char len_f;
						len_f = write(fd, send_buf, 4);
						gettimeofday(&tv, NULL);
						timer = tv.tv_sec;  
						//printf("Send back data: %s \t length :%d", send_buf, len_f);
						ii = 0;
					}
					else	//��Ч���ݴ���
					{
						send_buf[0] = stt;
						send_buf[1] = stt;
						send_buf[2] = resend_f;
						send_buf[3] = resend_f;
						unsigned char len_f;
						len_f = write(fd, send_buf, 4);
						gettimeofday(&tv, NULL);
						timer = tv.tv_sec;
						//printf("Send back data: %s \t length :%d", send_buf, len_f);
						printf("data error ! \n");
						
						ii++;
						if(ii == 10)
						{
							fclose(fout);
							printf("---recieve-data-error!---\n");
							return 0;
						}
					}
				}
				else	//��Ŵ���
				{
					send_buf[0] = stt;
					send_buf[1] = stt;
					send_buf[2] = resend_f;
					send_buf[3] = resend_f;
					write(fd, send_buf, 4);
					gettimeofday(&tv, NULL);
					timer = tv.tv_sec;  
					printf("error !  r_count:%d  \t count:%d\n", r_count, count);
					ii++;
					if(ii == 10)
					{
						fclose(fout);
						printf("---recieve-bn-error!---\n");
						return 0;
					}
				}
			}
			
			else	//�յ����ݱ�ͷ����
			{
				send_buf[0] = stt;
				send_buf[1] = stt;
				send_buf[2] = resend_f;
				send_buf[3] = resend_f;
				write(fd, send_buf, 4);
				gettimeofday(&tv, NULL);
				timer = tv.tv_sec;
				printf("not find 0xAA&0xAA  ! ask for resend \t \n");
				ii++;
				if(ii == 10)
				{
					fclose(fout);
					printf("---recieve-0xAA&0xAA-error!---\n");
					return 0;
				}
			}
		}
	}
}

int main(int argc, char **argv)
{
    int fd;
    int err;
    int len;
    int i;
    int size;
    
    if(argc != 4)
	{
		printf("Usage: %s /dev/ttyUSBn	0(send)/1(receive)	filename \n",argv[0]);
		return FALSE;
	}
    fd = UART0_Open(fd,argv[1]);
    do
	{
		err = UART0_Init(fd, 115200, 0, 8, 1, 'N');
		printf("Set Port Exactly!\n");
	}while(FALSE == err || FALSE == fd);
	
    if(0 == strcmp(argv[2], "0"))
	{
		FILE *filep;
		filep = fopen(argv[3], "r+b");
		
		size = file_size2(argv[3]);
		send_file(fd, filep, size);
		fclose(filep);
        UART0_Close(fd);
	}
    else
	{
		FILE *fout;
		fout = fopen(argv[3], "w");
		receive(fd, fout);
		UART0_Close(fd);
	}  
}  
