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
#define MAX 1024
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
		filename = "hello.txt";
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

	int fp = open(filename, O_RDWR|O_CREAT, 0666);
	if ( fp == -1 )
	{
		printf("open %s failured\n", filename);
		return -1;
	}

	int time_f = 0;
	struct timeval tv, tv_1, tv_2;
	long timer = 0;
	long timerr = 0;
	gettimeofday(&tv, NULL);
	timerr = tv.tv_sec;

	int nread = 1, nwrite = 0;
	char buffer[BUFFER_SIZE] = {'\0'};
	while(1)
	{
		if(time_f == 0)
		{
			gettimeofday(&tv, NULL);
			if(tv.tv_sec - timerr > 60)
			{
				printf("No received data!\n");
				return 0;
			}
		}

		if(time_f == -1)
		{
			nread = read(fd, buffer, BUFFER_SIZE);
			if (nread < 0)
			{
				time_f = -1;
				gettimeofday(&tv_2, NULL);
				if( (tv_2.tv_sec - tv_1.tv_sec) > 3)
				{
					break;
				}
			}
			
			if (nread > 0)
			{
				time_f = 1;
				nwrite = write(fp, buffer, nread);
				//printf("nwrite %d\n", nwrite);
				if (nwrite <= 0)
				{
					printf("write into file finished\n");
					break;
				}
				continue;
			}
		}

		if(time_f == 1)
		{
			nread = read(fd, buffer, BUFFER_SIZE);
			if (nread > 0)
			{
				time_f = 1;
				nwrite = write(fp, buffer, nread);
				if (nwrite <= 0)
				{
					printf("write into file finished\n");
					break;
				}
				continue;
			}
			
			if (nread < 0)
			{
				time_f = -1;
				gettimeofday(&tv_1, NULL);
				continue;
			}
			
		}
		
		if( ( nread = read(fd, buffer, BUFFER_SIZE) ) > 0 )
		{
			time_f = 1;
			nwrite = write(fp, buffer, nread);
			//printf("nwrite %d\n", nwrite);
			if (nwrite <= 0)
			{
				printf("write into file finished\n");
				break;
			}
		}
		
	}
	printf("sussfully get %s\n", filename);
	
	close(fp);
	close(fd);
	
	return 0;
}

void set_speed(int fd, int speed)
{
	int i;
	int status;
	struct termios Opt;
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
		tcflush(fd, TCIOFLUSH);
	}
}

int set_Parity(int fd, int databits, int stopbits, int parity)
{
	struct termios options;
	if (tcgetattr(fd, &options) !=  0)
	{
		perror("SetupSerial 1");
		return(FALSE);
	}
	options.c_cflag &= ~CSIZE;
	switch (databits)
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
			options.c_cflag |= (PARODD | PARENB);
			options.c_iflag |= INPCK;
			break;
		case 'e':
		case 'E':
			options.c_cflag |= PARENB;
			options.c_cflag &= ~PARODD;
			options.c_iflag |= INPCK;
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

	/* Set input parity option */
	if (parity != 'n')
		options.c_iflag |= INPCK;
	options.c_cc[VTIME] = 150; // 15 seconds
	options.c_cc[VMIN] = 0;

	tcflush(fd,TCIFLUSH);	/* Update the options and do it NOW */
	if (tcsetattr(fd, TCSANOW, &options) != 0)
	{
		perror("SetupSerial 3");
		return (FALSE);
	}
	return (TRUE);
}
