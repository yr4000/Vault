/*
 * main.c
 *
 *  Created on: Apr 30, 2017
 *      Author: Yair Hadas
 */
#include "Vault.h"
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
#include <ctype.h>

/*
void writeAToFile(int path, int size){
	int i;
	int f = open(path,O_CREAT | O_RDWR | O_TRUNC, 0755);
	if(f<0){
		printf( "Error opening file: %s\n", strerror( errno ) );
		//return errno;
	}
	for(i=0;i<size+1;i=i+1){
		if(write(f,"A",1) != 1)
			{
				printf("Error: something went wrong with the writing a vault - terminating the program.\n");
				//return -1;
			}
	}
	if(close(f)<0){
		printf( "Error closing file: %s\n", strerror( errno ) );
		//return -1;
		}
}
*/

int main(int argc, char* argv[]){
	//test:
	Vault v;
	int i;
	int buffer_size = 26;
	char buffer[buffer_size+1];

	init("V.vlt","200B");
	printf("Created vault successfully\n");
	for(i=0; i<2; i++){
		AddRecord("V.vlt","A.txt");
		AddRecord("V.vlt","B.txt");
		AddRecord("V.vlt","C.txt");
	}
	PrintList("V.vlt");
	RemoveOrFetchRecord("V.vlt","B.txt","fetch");
	RemoveOrFetchRecord("V.vlt","B.txt","rm");
	AddRecord("V.vlt","D.txt");
	int vf = open("V.vlt", O_RDONLY);
	v = readVault(vf);
	if(!v){return -1;}
	lseek(vf,FULL_VAULT_SIZE+1,SEEK_SET);
	buffer[26] = '\0';
	read(vf,buffer,buffer_size);
	printf("Buffer contains: %s\n",buffer);
	read(vf,buffer,buffer_size);
	printf("Buffer contains: %s\n",buffer);
	read(vf,buffer,buffer_size);
	printf("Buffer contains: %s\n",buffer);
	VaultStatus("V.vlt");



}
