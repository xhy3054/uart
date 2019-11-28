#include<stdio.h>      /*标准输入输出定义*/
#include<stdlib.h>     /*标准函数库定义*/
#include<unistd.h>     /*Unix 标准函数定义*/
#include<sys/types.h>   
#include<sys/stat.h>     
#include<fcntl.h>      /*文件控制定义*/
#include<termios.h>    /*PPSIX 终端控制定义*/
#include<errno.h>      /*错误号定义*/
#include<string.h>  
#include <sys/stat.h>
#include <sys/time.h>  

//宏定义  
#define FALSE  -1  
#define TRUE   0  

/******************************************************************* 
名称    UART0_Open
功能 打开串口并返回串口设备文件描述
参数
fd 文件描述符
port 串口号(ttyS0, ttyS1, ttyS2)
正确返回为1，错误返回为0
*******************************************************************/  
int UART0_Open(int fd, char* port)
{
	fd = open( port, O_RDWR|O_NOCTTY|O_NDELAY);
	if (FALSE == fd)
	{
		perror("Can't Open Serial Port");
		return(FALSE);
	}
	//恢复串口为阻塞状态
	if(fcntl(fd, F_SETFL, 0) < 0)
	{
		printf("fcntl failed!\n");
		return(FALSE);
	}
	else
	{
		printf( "fcntl = %d\n", fcntl(fd, F_SETFL, 0) );  
	}
	
	//测试是否为终端设备
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
名称    UART0_Close
功能 关闭串口并返回串口设备文件描述
参数
fd 文件描述符
port 串口号(ttyS0, ttyS1, ttyS2)
*******************************************************************/
void UART0_Close(int fd)
{
	close(fd);
}

/******************************************************************* 
名称 UART0_Set
功能 设置串口数据位，停止位和效验位
入口参数
fd 串口文件描述符
speed 串口速度
flow_ctrl 数据流控制
databits 数据位   取值为 7 或者8
stopbits停止位   取值为 1 或者2
parity 效验类型 取值为N, E, O, S
正确返回为1，错误返回为0
*******************************************************************/  
int UART0_Set(int fd, int speed, int flow_ctrl, int databits, int stopbits, int parity)
{
	int i;
	int status;
	int speed_arr[] = { B115200, B19200, B9600, B4800, B2400, B1200, B300};
	int name_arr[] = { 115200, 19200,  9600,  4800,  2400,  1200,  300};
    
	struct termios options;
    
	/*tcgetattr(fd,&options)得到与fd指向对象的相关参数，并将它们保存于options,该函数
	还可以测试配置是否正确，该串口是否可用等。若调用成功，函数返回值为0，若调用失败，
	函数返回值为1. */
	if( tcgetattr( fd, &options) != 0 )
	{
		perror("SetupSerial 1");
		return(FALSE);
	}
    
    //设置串口输入波特率和输出波特率  
	for ( i= 0;  i < sizeof(speed_arr) / sizeof(int);  i++)
	{
		if  (speed == name_arr[i])
		{
			cfsetispeed(&options, speed_arr[i]);
			cfsetospeed(&options, speed_arr[i]);
		}
	}
     
    //修改控制模式，保证程序不会占用串口  
    options.c_cflag |= CLOCAL;  
    //修改控制模式，使得能够从串口中读取输入数据  
    options.c_cflag |= CREAD;
    
    //设置数据流控制
    switch(flow_ctrl)
    {
		case 0:		//不使用流控制
			options.c_cflag &= ~CRTSCTS;
			break;
        
		case 1:		//使用硬件流控制
			options.c_cflag |= CRTSCTS;
			break;
		case 2:		//使用软件流控制
			options.c_cflag |= IXON | IXOFF | IXANY;
			break;
    }
    //设置数据位
    //屏蔽其他标志位
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
    //设置校验位
    switch (parity)
    {
		case 'n':
		case 'N': //无奇偶校验位
			options.c_cflag &= ~PARENB;
			options.c_iflag &= ~INPCK;
			break;
		case 'o':
		case 'O'://设置为奇校验
			options.c_cflag |= (PARODD | PARENB);
			options.c_iflag |= INPCK;
			break;
		case 'e':
		case 'E'://设置为偶校验
			options.c_cflag |= PARENB;
			options.c_cflag &= ~PARODD;
			options.c_iflag |= INPCK;
			break;
		case 's':
		case 'S': //设置为空格
			options.c_cflag &= ~PARENB;
			options.c_cflag &= ~CSTOPB;
			break;
        default:
			fprintf(stderr,"Unsupported parity\n");
			return (FALSE);
    }
    // 设置停止位
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
    
	//修改输出模式，原始数据输出  
	options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
	options.c_oflag &= ~OPOST;
    
    //设置等待时间和最小接收字符
    options.c_cc[VTIME] = 1;	/* 读取一个字符等待1*(1/10)s */
    options.c_cc[VMIN] = 0;	/* 读取字符的最少个数为1 */
    /*
    VTIME	VMIN
    0		>0		一直阻塞到接收到VMIN个数据时read返回
    >0		0		普通超时
    >0		>0		当接收到第一个字节时开始计算超时。
	如果超时时间未到但数据已经达到VMIN个read立即返回。
	如果超时时间到了就返回当前读到的个数。
    */
    
    //如果发生数据溢出，接收数据但是不再读取
    tcflush(fd, TCIFLUSH);
    
    //激活配置 (将修改后的termios数据设置到串口中）
    if (tcsetattr(fd, TCSANOW, &options) != 0)
	{
		perror("com set error!\n");
		return (FALSE);
	}
    return (TRUE);
}

/******************************************************************* 
名称：   UART0_Init()
功能：串口初始化
参数：
fd 文件描述符
speed 串口速度
flow_ctrl 数据流控制
databits 数据位   取值为 7 或者8
stopbits 停止位   取值为 1 或者2
parity 效验类型 取值为N, E, O, S 
正确返回为1，错误返回为0
*******************************************************************/  
int UART0_Init(int fd, int speed, int flow_ctrl, int databits, int stopbits, int parity)
{
    int err;
    //设置串口数据帧格式
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
名称 UART0_Recv
功能    接收串口数据
参数
fd 文件描述符
rcv_buf 接收串口中数据存入rcv_buf缓冲区中
data_len 一帧数据的长度
正确返回为1，错误返回为0
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
    
    //使用select实现串口的多路通信
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
	
	int jj = 0;		//接收回传数据错误次数
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
			 
			retv = write(fd, st, rsize + 5);	//发数据
			int aa = retv;
			
			//printf("%s \n", sbuf);
			 
			//printf("the number of charater sent is %d\n", retv);
			//printf("is %d  \t st[2] is %d\n", count, st[2]);
			int num = 0;
			while(1)	//等待接收回传数据
			{
				if( (num = read(fd, r_buf, 4)) > 0 )	//收回传数据
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
							retv=write(fd,st,rsize+5);  //发数据	
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
	printf("---send-finished!---\nfilesize：%.1f  KB\n",(float)size/1000);

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
   	 数据正常，继续发送下一帧信息
   	 长度错误和数据错误处理一样，要求重发此帧信息
   	 序号错误此包为上一包，发送下一帧信息
   	 */
   	
 	unsigned char count = 1;
 	
 	unsigned char r_count;
	unsigned char length;
	unsigned char solength;
	unsigned char c_sum; 
	unsigned char sum;
    
	int ii = 0;			//数据错误次数
	int iii = 0;		//回传失败次数
	int time = 0;		//回传后接受新数据等待
	int time_f = 0;
	int j = 0;			//回传丢失重发次数
	
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
			if(tv.tv_sec-timer > 1)	//等待1秒
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
			time_f = 1;		//已经收到过数据
			time = 0;
			gettimeofday(&tv, NULL);
			timer = tv.tv_sec;
			//printf("recieve data: %s \t \n", read_buf);
			if( read_buf[0] == stt && read_buf[1] == stt )
			{
				r_count = read_buf[2];
				//printf("r_count:%d  \t count:%d\n", r_count, count);
				if(r_count==count) // 序号正确
				{
					length = read_buf[3];
					memcpy(bbuf, &read_buf[4], length);
					sum = read_buf[3 + length + 1];
					
					c_sum = cal_sum(bbuf, length);
					if(length < max_buffer_size)	// 最后一帧
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
					if(c_sum == sum)	//数据正确
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
					else	//有效数据错误
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
				else	//序号错误
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
			
			else	//收到数据报头错误
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
