/*
 * Vault.c
 *
 *  Created on: Apr 14, 2017
 *      Author: Yair Hadas
 */

#include "Vault.h"
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
#include <errno.h>
#include <time.h>
#include <sys/time.h>

#define RIGHT_BORDER "<<<<<<<<"
#define LEFT_BORDER ">>>>>>>>"
#define ZERO_BORDER "00000000"
#define BORDER_SIZE 8

/*
 * Write a new vault to a file
 */
int init(char* file_path, char* size){
	//open a new file
	//int f = open(file_path,O_CREAT | O_RDWR | O_TRUNC, 0755);
	int f = open(file_path,O_CREAT | O_RDWR | O_APPEND | O_TRUNC , 0755);
	if(f<0){
		printf( "Error opening file: %s\n", strerror( errno ) );
		return errno;
	}
	Vault temp = CreateVault(size);
	if(!temp){
		printf("Error: could not create vault\n");
		return -1;
	}
	if(writeVaultToFile(f,temp)<0){return -1;}

	if(close(f)<0){
		printf( "Error closing file: %s\n", strerror( errno ) );
		return -1;
	}
	destroyVault(temp);
	return 1;
}

/*
 * Creates a new empty vault
 */
Vault CreateVault(char* sizeAsString){
	Vault temp = calloc(VAULT_SIZE,1);
	//int i;
	ssize_t size = string_to_size(sizeAsString);
	struct timeval tv;
	if(size < 0){
		return NULL;
	}

	gettimeofday(&tv, NULL);
	temp->vault_size = size;
	temp->free_space = size;
	temp->creation_stump = tv.tv_sec; //TODO maybe needs to be more accurate
	temp->last_modified = tv.tv_sec;
	temp->files_amount = 0;
	temp->deleted_files = 0;
	temp->files = calloc(100,FR_SIZE);
	//for(i=0;i<100;i=i+1){

	//}
	//temp->recycle_bin = (FR*) calloc(100,sizeof(*(temp->recycle_bin)));
	temp->eof = FULL_VAULT_SIZE+1;
	//memset(temp->files, 0, sizeof(temp->files)); //use to clean the files array.

	return temp;
}

void destroyVault(Vault v){
	int i;
	/*
	for(i=0;i<v->files_amount;i=i+1){
		free(v->files[i]);
	}
	*/
	free(v->files);
	free(v);
}

/*
 * Prints all the files existing in the vault
 */
int PrintList(char* vault_path){
	int i;
	int file_counter = 0;
	Vault v;
	FR r;
	//open vault
	int vf = open(vault_path, O_RDWR);
	if(vf<0){
		printf( "Error opening file: %s\n", strerror( errno ) );
		return errno;
	}
	v = readVault(vf);
	for(i=0; i<100; i++){
		r = v->files[i];
		if(r->is_deleted>0){
			continue;
		}
		printf("%s	%dB		%o		%s",
				r->file_name, r->file_size, r->file_protection, asctime(localtime(&(r->insertion_time))));
		file_counter++;
		if(file_counter==v->files_amount) break;
	}
	//close vault
	if(close(vf)<0){
		printf( "Error closing file: %s\n", strerror( errno ) );
	}
	destroyVault(v);
	return 1;

}

/*
 * Adds a new Record to the vault
 */
int AddRecord(char* vault_path, char* file_to_write){
	Vault v;
	int i,j,block_counter;
	ssize_t bytesWrote,bytesToWrite,deleteBin;
	FR binRunner;
	ssize_t *size;
	char temp[30];
	//off_t *offset;	//TODO prepare for troubles...
	int buffer_size = 3;

	//creates new file record to f
	FR r = create_file_record(file_to_write);	//creates empty file-record.
	if(!r){
		printf("Error - couldn't create new file record\n");
		return -1;
	}
	//open file
	int vf = open(vault_path, O_RDWR);
	if(vf<0){
		printf( "Error opening file: %s\n", strerror( errno ) );
		return errno;
	}
	//read vault to v
	v = readVault(vf);
	if(!v){return -1;}

	//Is there enough space in the vault for the file?
	if(r->file_size > v->free_space){
		printf("Error - not enough space in the Vault for a file in this size.\n");
		return -1;
	}
	//simple writing file to eof of vault
	//add record to list
	r->block_offset_1 = v->eof;
	r->block_size_1 = r->file_size;

	//update vault.
	v->files[v->files_amount] = r;
	v->files_amount = v->files_amount+1;
	v->free_space = v->free_space-r->file_size;
	v->eof = v->eof + r->file_size +2*BORDER_SIZE;
	lseek(vf,0,SEEK_SET);
	//write updated vault to file
	if(writeVaultToFile(vf,v)<0){ return -1;}

	//jump to the end of the file
	lseek(vf,r->block_offset_1,SEEK_SET); //TODO: later modify to any offset
	//write file content to EOF
	//write right border
	if(writeStringToFile(vf,RIGHT_BORDER,BORDER_SIZE)<0){ return -1; }
	//read file to buffer
	if(writeFileToVault(vf,file_to_write,r->file_size,buffer_size)<0){return -1;}
	//write left border
	if(writeStringToFile(vf,LEFT_BORDER,BORDER_SIZE)<0){ return -1;}

	//close everything
	if(close(vf)<0){
		printf( "Error closing file: %s\n", strerror( errno ) );
	}
	destroyVault(v);

	/*
	//now we need to write the record to file:
	bytesWrote = 0;
	block_counter = 0;
	deleteBin = 0;
	binRunner = *(v->recycle_bin);	//TODO: check this works
	while(binRunner){	//for each file
		for(j=0; j<3; j = j+1){	//for each block
			size = &(binRunner->blocks[j]->block_size);	//TODO: check it changes the actual content
			offset = &(binRunner->blocks[j]->block_offset);
			//if size>0 then there is available space to write
			if(*size>0){
				//in order to frag to no more then 3 blocks
				if(bytesWrote > *size && j==3){
					continue;
				}
				else{
					r->blocks[block_counter]->block_offset = *offset;
					//is what's left is smaller then the size of the block, write what's left.
					if(r->file_size-bytesWrote < *size){
						bytesToWrite = r->file_size-bytesWrote;
					}
					else{
						bytesToWrite = *size;
					}
					r->blocks[block_counter]->block_size = bytesToWrite;
					*size = *size - bytesToWrite;
					*offset = 0;
					block_counter = block_counter+1;
					bytesWrote = bytesWrote + bytesToWrite;
				}
			}
			if(*size==0) {
				deleteBin = deleteBin + 1;
			}
		}
		//delete bin if it is empty.
		if(deleteBin == 3){
			//connect the two surrounding him
			binRunner->prev->next = binRunner->next;
			binRunner->next->prev = binRunner->prev;
			//take a step back
			binRunner = binRunner->prev;
			//get rid of him;
			binRunner->next->next = NULL;
			binRunner->next->prev = NULL;
		}
		binRunner = binRunner->next;
	}
	//TODO: if we didn't succeed to place the file but we know there is enough place for him - defrag
	//TODO: what if there is no place at the end of the file??
	int fw = open(file_to_write,O_RDONLY);

		defrag();
		//restrat the record
		//write in the end
	}
	//write everything you got
	//if you still have something to write - do it at the end of the file.
	else{
		for(i=0;i<block_counter;i = i+1){
			if(write(r->blocks[i]->block_offset,,output_b_p) < output_b_p)
			{
				dkjnvkxdjvnx
				printf("Error: something went wrong with the writing - terminating the program.\n");
				return -1;
			}
		}
	}
	*/

	return 1;
}

/*
 * Removes record form the vault
 */
int RemoveOrFetchRecord(char* vault_path, char* file_name,char* order){
	int i,j;
	Vault v;
	FR r;
	char* zero_record = calloc(1,FR_SIZE);
	//TODO: try to open v
	int vf = open(vault_path,O_RDWR);
	if (vf<0){
		printf("Error in removing record - could not open vault file\n");
		return -1;
	}
	v = readVault(vf);
	if (!v){
		printf("Error in removing record - could not load vault from file\n");
		return -1;
	}
	for(i=0;i<v->files_amount;i=i+1){
		r = v->files[i];
		if(strcmp(r->file_name,file_name)==0 && r->is_deleted<0 ){
			for(j=0;j<3;j++){
				switch(j){
				case 0:
					deleteBlockOrCreateFile(vf,r->block_offset_1,r->block_size_1,file_name,order);
					break;
				case 1:
					deleteBlockOrCreateFile(vf,r->block_offset_2,r->block_size_2,file_name,order);
					break;
				case 2:
					deleteBlockOrCreateFile(vf,r->block_offset_3,r->block_size_3,file_name,order);
					break;
				}
			}
			if(strcmp(order,"rm")==0){
				r->is_deleted = 1;
				v->files_amount--;
				v->free_space = v->free_space+r->file_size;
				/*
				lseek(vf,VAULT_SIZE+FR_SIZE*i+1,SEEK_SET);
				}
				*/
			}
		}
	}
	lseek(vf,0,SEEK_SET);
	if(writeVaultToFile(vf,v)<0){ return -1;}
	//close everything
	if(close(vf)<0){
		printf( "Error closing file: %s\n", strerror( errno ) );
	}
	destroyVault(v);
	return 1;
}

int deleteBlockOrCreateFile(int vf, off_t offset, ssize_t size,char* file_name,char* order){
	if(offset<=0){ return 1;}
	if(strcmp(order,"rm")==0){
		deleteSingleBlock(vf,offset,size);
	}
	if(strcmp(order,"fetch")==0){
		FetchRecord(vf,file_name,offset,size);
	}

	return 1;
}

/*
 * removes a single block from the array.
 */
int deleteSingleBlock(int vf, off_t offset, ssize_t size){
	//delete right border
	lseek(vf,offset,SEEK_SET);
	if(writeStringToFile(vf,ZERO_BORDER,BORDER_SIZE)<0){return -1;}
	//delete left border
	lseek(vf,size,SEEK_CUR);
	if(writeStringToFile(vf,ZERO_BORDER,BORDER_SIZE)<0){return -1;}

	return 1;
}

/*
 * Fetches record from the vault
 */
int FetchRecord(int vf, char* file_name,off_t offset,ssize_t size){
	//temporary:
	char new_file_name[260];
	int name_len = 0;
	while(file_name[name_len] != '\0'){
		name_len++;
	}
	strncpy(new_file_name,file_name,name_len-4);
	strcat(new_file_name,"-fetched.txt");

	//real code:
	int bytes_wrote = 0;
	int buffer_size = 5;
	char* buffer[buffer_size];
	//open the file
	int f = open(new_file_name,O_CREAT | O_RDWR | O_APPEND , 0755);
	if(f<0){
		printf("Error in fetch file - could not open the file\n");
				return -1;
	}
	//set vault to the location of the file
	lseek(vf,offset+BORDER_SIZE,SEEK_SET);

	while(size>bytes_wrote){
		//read to the buffer
		int k = read(vf,buffer,buffer_size);
		//write from the buffer
		if(write(f,buffer,k)<k){
			printf("Error in fetch file - could not write to the file\n");
			return -1;
		}
		//clean the buffer
		memset(buffer, 0, buffer_size);
		bytes_wrote = bytes_wrote + k;
	}
	if(close(f)<0){
		printf( "Error closing file: %s\n", strerror( errno ) );
	}

	return 1;
}

void DefragVault(char* file_path){

}

int VaultStatus(char* vault_path){
	Vault v;
	int i;
	int file_counter = 0;
	int total_files_size = 0;
	int vf = open(vault_path,O_RDONLY);
	if(vf<0){return -1;}
	v = readVault(vf);
	//get total size of files
	for(i=0;i<100;i++){
		if(v->files[i]->is_deleted>0){
			continue;
		}
		total_files_size = total_files_size + v->files[i]->file_size;
		file_counter++;
		if(file_counter == v->files_amount) break;
	}
	//get fragmentation ratio

	//close vault
	if(close(vf)<0){
		printf( "Error closing file: %s\n", strerror( errno ) );
	}
	destroyVault(v);

	//print statistics
	printf("Number of files:		%d\n",v->files_amount);
	printf("Total size:			%dB\n",total_files_size);
	printf("Fragmentation ratio:		we may never know....\n");

	return 1;
}


int writeStringToFile(int file,char* buffer,int buffer_size){
	if(write(file,buffer,buffer_size) != buffer_size)
	{
		printf("Error: something went wrong with the writing - terminating the program.\n");
		return -1;
	}
	return 1;
}

//source: http://moodle.tau.ac.il/mod/forum/discuss.php?d=49960
//TODO: should it be responsible to open the file?
int writeVaultToFile(int vault_file,Vault v){
	if(write(vault_file,v,VAULT_SIZE) != VAULT_SIZE)
	{
		printf("Error: something went wrong with the writing a vault - terminating the program.\n");
		return -1;
	}
	//write files
	if(write(vault_file,v->files,FR_SIZE*100) != FR_SIZE*100)
		{
		printf("Error: something went wrong with the vault's files.\n");
		return -1;
		}


	return 1;
}


Vault readVault(int vault_file){
	Vault v = (Vault)malloc(VAULT_SIZE);
	if(read(vault_file,v,VAULT_SIZE) != VAULT_SIZE){
		printf("Error: could not read vault\n");
		return NULL;
	}

	v->files = (FR*)calloc(100,FR_SIZE);
	if(read(vault_file,v->files,FR_SIZE*100) != FR_SIZE*100){
		printf("Error: could not read vault\n");
		return NULL;
	}

	return v;
}

int writeFileToVault(int vault_file,char* file_path,int file_size, int buffer_size){
	char buffer[buffer_size];
	int bytes_read = 0;
	//open file
	int f = open(file_path,O_RDONLY);
	if(f<0){
		printf( "Error opening file: %s\n", strerror( errno ) );
		return errno;
	}

	//write file content using buffer
	while(file_size > bytes_read){
		int k = read(f,buffer,buffer_size);
		if(write(vault_file,buffer,k) != k)
		{
			printf("Error: something went wrong with the vault's files.\n");
			return -1;
		}
		bytes_read = bytes_read+k;
	}
	if(close(f)<0){
		printf( "Error closing file: %s\n", strerror( errno ) );
	}
	return 1;
}


