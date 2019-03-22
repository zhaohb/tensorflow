#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#include "fpga_mag.h"


#define DMA_BAR_LEN (512 * 1024)
#define DMA_MEM_LEN (512 * 1024)
#define ALTERA_DMA_DESCRIPTOR_NUM 128
#define BUFFER_LENGTH 40


#define ALTERA_IOCTL_BASE 'm'
#define GET_MEM_ADDR _IOR(ALTERA_IOCTL_BASE, 0, unsigned long)
#define GET_BAR_ADDR _IOR(ALTERA_IOCTL_BASE, 1, unsigned long)

#define ALTERA_CMD_START_DMA    1
#define ALTERA_CMD_POLL_SIZE    2
#define ALTERA_DMA_READ_ALL     3
#define ALTERA_DMA_WRITE_ALL    4
#define ALTERA_EXIT             5

#define BUF_SIZE                128
#define POLL_SIZE               (512 * 1024)
#define BUF_PTR_SIZE            (POLL_SIZE/BUF_SIZE)
#define DISPLAY_LINE_NUM        20

typedef unsigned int            u32;
typedef unsigned char           u8;
typedef unsigned short          u16;
typedef unsigned long long      u64;


volatile unsigned char *mmap_mem;
volatile unsigned char *mmap_bar;
ssize_t file_mem;
ssize_t file_bar; 

int init_device(void)
{

	file_mem = open ("/dev/altera_mem", O_RDWR);

	if(file_mem < 0)
	{
		printf("open file_mem error\n");
		return -1;
	}

	mmap_mem = (unsigned char *)mmap(NULL, POLL_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, file_mem, 0);


	if(mmap_mem == MAP_FAILED)
	{
		printf("mmap mem error\n");
		return -1; 
	}


	file_bar = open ("/dev/altera_bar", O_RDWR);

	if(file_mem < 0)
	{
		printf("open file_mem error\n");
		return -1;
	}

	mmap_bar = (unsigned char *)mmap(NULL, DMA_BAR_LEN, PROT_READ|PROT_WRITE, MAP_SHARED, file_bar, 0);

	if(mmap_bar == MAP_FAILED)
	{
		printf("mmap bar error\n");
		return -1; 
	}

	return 0;
}


int close_device(void)
{

	munmap((void*)mmap_bar, DMA_BAR_LEN);
	munmap((void*)mmap_mem, POLL_SIZE);
	close(file_bar);
	close(file_mem);

	return 0;
}

int calc_vector_add(u8 a, u8 b)
{
	u8 c;


	*((unsigned char*) mmap_bar + 0x144) = a;
	*((unsigned char*) mmap_bar + 0x148) = b;

	usleep(100);
	c = *((unsigned char*) mmap_bar + 0x14c);

	return c;

}

