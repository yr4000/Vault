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
#define BORDER_SIZE 8

/*
 * Write a new vault to a file
 */
int init(char* file_path, char* size){
	//open a new file
	int f = open(file_path,O_CREAT | O_RDWR | O_TRUNC, 0755);
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
	//temp->recycle_bin = (FR*) calloc(100,sizeof(*(temp->recycle_bin)));
	temp->eof = FULL_VAULT_SIZE;
	//memset(temp->files, 0, sizeof(temp->files)); //use to clean the files array.

	return temp;
}

void destroyVault(Vault v){
	int i;
	for(i=0;i<v->files_amount;i=i+1){
		free(v->files[i]);
	}
	free(v->files);
	free(v);
}

//TODO: complete?
/*
 * Loads existing vault from file
 */
Vault LoadVault(char *file_path){
	Vault v;
	int f = open(file_path,O_CREAT | O_RDWR | O_TRUNC, 0755);
	if(f<0){
		printf( "Error opening file: %s\n", strerror( errno ) );
		return NULL;
	}
	if(read(f,&v,sizeof(v))<sizeof(v)){
		printf("Failed reading Vault from file\n");
		return NULL;
	}
	if(close(f)<0){
		printf( "Error closing file: %s\n", strerror( errno ) );
		//return NULL;
	}

	return v;
}

/*
 * Prints all the files existing in the vault
 */
void PrintList(char* file_path){
	/*
	int i;
	Vault v;
	FR e;
	char entryStats[356];
	//TODO: try to open v
	for(i=0; i<v->files_amount; i=i+1){
		//runs over the entry relevant variables and add them to string
		e = v->files[i];
		entryStats = e->file_name+"\t"+size_to_string(e->file_size)+"\t"
				+protection_to_string(e->file_protection)+"\t"+e->insertion_time+"\n";
		printf(entryStats);
		//TODO: empty string if necessary
	}
	*/
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
	off_t *offset;	//TODO prepare for troubles...
	int buffer_size = 5;
	char buffer[buffer_size];

	//creates new file record to f
	FR r = create_file_record(file_to_write);	//creates empty file-record.
	if(!r){
		printf("Error - couldn't create new file record\n");
		return -1;
	}
	//open file
	int vf = open(vault_path, O_RDWR | O_TRUNC);
	if(vf<0){
		printf( "Error opening file: %s\n", strerror( errno ) );
		return errno;
	}
	//read vault to v
	v = (Vault)malloc(VAULT_SIZE);
		if(read(vf,v,VAULT_SIZE) != VAULT_SIZE){
			printf("Error: could not read vault\n");
			return NULL;
		}

		if(read(vf,v->files,FR_SIZE*100) != FR_SIZE*100){
			printf("Error: could not read vault\n");
			return NULL;
		}
	//v = readVault(vf);
	//if(!v){return -1;}

	//Is there enough space in the vault for the file?
	if(r->file_size < v->free_space){
		printf("Error - not enough space in the Vault for a file in this size.\n");
		return -1;
	}
	//simple writing file to eof of vault
	int bytes_read = 0;
	//add record to list
	r->block_offset_1 = v->eof;
	r->block_size_1 = r->file_size;

	lseek(vault_path,v->eof,SEEK_SET); //TODO: later modify to any offset

	v->files[v->files_amount] = r;
	v->files_amount = v->files_amount+1;
	v->eof = v->eof + FR_SIZE +2*BORDER_SIZE;
	//jump to the end of the file
	//write right border
	if(writeStringToFile(vf,RIGHT_BORDER,BORDER_SIZE)<0){ return -1; }
	//read file to buffer
	while(r->file_size > bytes_read){
		read(file_to_write,buffer,buffer_size);
		//write file content with buffer
		if(writeVaultToFile(vf,buffer)<0){ return -1;}
	}
	//write left border
	if(writeStringToFile(vf,LEFT_BORDER,BORDER_SIZE)<0){ return -1;}
	//close everything


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
void RemoveRecord(char* file_path, char* file){
	int i;
	Vault v;
	FR r;
	//TODO: try to open v
	for(i=0;i<v->files_amount;i=i+1){
		r = v->files[i];
		if(r->file_name == file){
			//TODO: remove that file
			break;
		}
	}
}

/*
 * Fetches record from the vault
 */
void GetRecord(char* file_path, char* file){
	int i;
	Vault v;
	FR r;
	//TODO: try to open v
	for(i=0;i<v->files_amount;i=i+1){
		r = v->files[i];
		if(r->file_name == file){
			//TODO: fetch that file
			break;
		}
	}
}

void DefragVault(char* file_path){

}

void VaultStatus(char* file_path){

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
	if(write(vault_file,&(v->files),FR_SIZE*100) != FR_SIZE*100)
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

	if(read(vault_file,v->files,FR_SIZE*100) != FR_SIZE*100){
		printf("Error: could not read vault\n");
		return NULL;
	}

	return v;
}

