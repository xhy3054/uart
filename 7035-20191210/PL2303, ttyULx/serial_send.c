#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <errno.h>
#include <sys/time.h>
#include <time.h>

#define BUFFER_SIZE 16
#define FALSE -1
#define TRUE 1

int speed_arr[] = { B115200, B38400, B19200, B9600, B4800, B2400, B1200, B300,
	    B38400, B19200, B9600, B4800, B2400, B1200, B300, };
int name_arr[] = {115200, 38400, 19200, 9600, 4800, 2400, 1200, 300,
	    38400, 19200, 9600, 4800, 2400, 1200, 300, };

void set_speed(int fd, int speed);
int set_Parity(int fd, int databits, int stopbits, int parity);

int main(int argc, char *argv[])
{
	char *interface = NULL, *filename = NULL;

	if (argc < 2)
	{
		interface = "/dev/ttyUSB0";
		filename = "test.txt";
	}
	else
	{
		interface = argv[1];
		filename = argv[2];
	}

	int fd = open(interface, O_RDWR|O_NOCTTY|O_NDELAY);
	if(fd == -1)
	{
	    printf("%s Open   Error!\n", interface);
	    return -1;
	}
	printf("open %s successfully !", interface);
	
	set_speed(fd, 115200);
	if (set_Parity(fd, 8, 1, 'N') == FALSE)
	{
		printf("Set Parity Error\n");
		exit(1);
	}

	int fp = open(filename, O_RDWR);
	if ( fp == -1 )
	{
		printf("open %s failured\n", filename);
		return -1;
	}
	int nread = 1;
	char buffer[BUFFER_SIZE] = {'\0'};
	int sum = 0;

	while(nread > 0)
	{
		int nwrite = 0;
		int tmp = 0, pos = 0;
		nread = read(fp, buffer, BUFFER_SIZE);	//一次要写进去的字节数
		//printf("read %d bytes \n", nread);
		int size_write = nread;		//size_write是要本次要写入的总字节数
		while (size_write > 0)
		{
			tmp = (nwrite > tmp)? nwrite : tmp;
			pos = tmp ;
		    nwrite = write(fd, buffer + pos, size_write);	//nwrite一次写进去的字节数		
			if (nwrite == -1)
			{
				sleep (2);
				continue;
			}
			sum = sum + nwrite;
			//printf("write %d left %d \n", sum, size_write);
			size_write = size_write - nwrite;	//剩下要写进去的字节数				
		}
	}
	printf("sussfully  write %s\n", filename);
	close(fp);
	close(fd);
	return 0; 
}

void set_speed(int fd, int speed)
{
	int   i;
	int   status;
	struct termios   Opt;
	tcgetattr(fd, &Opt);
	for ( i= 0;  i < sizeof(speed_arr) / sizeof(int);  i++)
	{
		if  (speed == name_arr[i])
		{
			tcflush(fd, TCIOFLUSH);
			cfsetispeed(&Opt, speed_arr[i]);
			cfsetospeed(&Opt, speed_arr[i]);
			status = tcsetattr(fd, TCSANOW, &Opt);
			if (status != 0)
			{
				perror("tcsetattr fd1");
				return;
			}
		}
		tcflush(fd,TCIOFLUSH);
	}
}
 
int set_Parity(int fd,int databits,int stopbits,int parity)
{
	struct termios options;
	if (tcgetattr( fd, &options) != 0)
	{
		perror("SetupSerial 1");
		return(FALSE);
	}
	options.c_cflag &= ~CSIZE;
	switch (databits) /*设置数据位数*/
	{
		case 7:
			options.c_cflag |= CS7;
			break;
		case 8:
			options.c_cflag |= CS8;
			break;
		default:
			fprintf(stderr,"Unsupported data size\n");
			return (FALSE);
	}
	switch (parity)
	{
		case 'n':
		case 'N':
			options.c_cflag &= ~PARENB;   /* Clear parity enable */
			options.c_iflag &= ~INPCK;     /* Enable parity checking */
			break;
		case 'o':
		case 'O':
			options.c_cflag |= (PARODD | PARENB);  /* 设置为奇效验*/
			options.c_iflag |= INPCK;             /* Disnable parity checking */
			break;
		case 'e':
		case 'E':
			options.c_cflag |= PARENB;     /* Enable parity */
			options.c_cflag &= ~PARODD;   /* 转换为偶效验*/
			options.c_iflag |= INPCK;       /* Disnable parity checking */
			break;
		case 'S':
		case 's':  /*as no parity*/
			options.c_cflag &= ~PARENB;
			options.c_cflag &= ~CSTOPB;
			break;
		default:
			fprintf(stderr,"Unsupported parity\n");
			return (FALSE);
	}
	/* 设置停止位*/
	switch (stopbits)
	{
		case 1:
			options.c_cflag &= ~CSTOPB;
			break;
		case 2:
			options.c_cflag |= CSTOPB;
			break;
		default:
			fprintf(stderr,"Unsupported stop bits\n");
			return (FALSE);
	}
	/* Set input parity option */
	if (parity != 'n')
		options.c_iflag |= INPCK;
	options.c_cc[VTIME] = 150; // 15 seconds
	options.c_cc[VMIN] = 0;

	tcflush(fd,TCIFLUSH); /* Update the options and do it NOW */
	if (tcsetattr(fd,TCSANOW,&options) != 0)
	{
		perror("SetupSerial 3");
		return (FALSE);
	}
	return (TRUE);
}

