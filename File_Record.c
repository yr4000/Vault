/*
 * File_Record.c
 *
 *  Created on: Apr 14, 2017
 *      Author: Yair Hadas
 */

#include "File_Record.h"
#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <dirent.h>
#include <time.h>
#include <sys/time.h>


FR create_file_record(char* file){
	FR fr = (FR)malloc(FR_SIZE);
	struct stat fileStat;
	struct timeval tv;
	if(stat(file,&fileStat) < 0){
		return NULL;
	}

	strcpy(fr->file_name,file);
	gettimeofday(&tv, NULL);
	fr->file_size = fileStat.st_size;
	fr->file_protection = fileStat.st_mode;
	fr->insertion_time = tv.tv_sec;
	fr->block_offset_1 = 0;
	fr->block_size_1 = 0;
	fr->block_offset_2 = 0;
	fr->block_size_2 = 0;
	fr->block_offset_3 = 0;
	fr->block_size_3 = 0;

	return fr;

}

void destroyFR(FR fr){

}

int writeFR(int vault_file,FR fr){
	int i;
	if(write(vault_file,fr,FR_SIZE) != FR_SIZE)
	{
		printf("Error: something went wrong with the writing of a file record - terminating the program.\n");
		return -1;
	}
	/*
	for(i=0; i<3; i=i+1){
		if(write(vault_file,fr->blocks[i],BLOCK_SIZE) != BLOCK_SIZE)
			{
				printf("Error: something went wrong with the writing of a block - terminating the program.\n");
				return -1;
			}
	}
	*/
	return 1;
}

/*
 * pre: the pointer of vault file is in the right place
 * reads the file record and its three blocks
 */
FR readFR(int vault_file){
	FR fr = malloc(FR_SIZE);
	int i;
	if(read(vault_file,fr,FR_SIZE) != FR_SIZE){
		printf("Error - could not read file record\n");
		return NULL;
	}
	/*
	for(i=0;i<3;i=i+1){
		fr->blocks[i] = malloc(BLOCK_SIZE);
		if(read(vault_file,fr->blocks[i],BLOCK_SIZE) != BLOCK_SIZE){
			printf("Error - could not read file record\n");
			return NULL;
		}
	}
	 */
	return fr;
}

int moveFilePointer(int file,int bytes_no){
	if(lseek(file,sizeof(struct blocks),SEEK_CUR)<0){
		printf("Error - could not complete lseek \n");
		return -1;
	}
	return 1;
}

ssize_t string_to_size(char* size){
	//get request size
	ssize_t req_size;
	int data_amount_len =strlen(size);
	//printf("data_amount_len: %d\n",data_amount_len);
	char unit = size[data_amount_len-1];
	//printf("unit: %c\n",unit);
	char s_amount[data_amount_len-1];
	strncpy(s_amount,size,data_amount_len-1);
	double amount = atof(s_amount);
	if (amount == 0)
	{
		printf("the data amount argument is wrong or equals to zero - can't continue the process.\n");
		return -1;
	}
	//printf("amount: %d\n",amount);
	//printf("unit-M = %d\n", unit-'M');
	if(unit == 'B' || unit == 'b' ){
		req_size = amount;
	}
	else if(unit == 'K' || unit == 'k'){
		req_size = amount*1024;
	}
	else if(unit == 'M' || unit == 'm'){
		req_size = amount*1024*1024;
	}
	else if(unit == 'G' || unit == 'g'){
		req_size = amount*1024*1024*1024;
	}
	else{
		printf("The data amount unit is illegal.\n");
		return -1;
	}

	return req_size;
}
