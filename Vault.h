/*
 * Vault.h
 *
 *  Created on: Apr 14, 2017
 *      Author: Yair Hadas
 */

#include "File_Record.h"
#include <time.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>

#ifndef VAULT_H_
#define VAULT_H_

#define FULL_VAULT_SIZE (sizeof(struct Vault) + 100*FR_SIZE)
#define VAULT_SIZE sizeof(struct Vault)

typedef struct Vault *Vault;

typedef struct Vault {
	ssize_t vault_size;
	ssize_t free_space;
	time_t creation_stump;
	time_t last_modified;
	off_t eof;	//this is the offset from the begging of the file to the end of it.
	short files_amount;
	short deleted_files;
	FR* files; //TODO is this correct?
	//FR* recycle_bin;
};

//typedef struct File_Allocation_Table *FAT;

/*
 * Receives a string with the size of the vault, when initialize it.
 */
int init(char* file_path, char* size);

/*
 * creates a Vault object
 */
Vault CreateVault(char* sizeAsString);

/*
 * destroys vault
 */
void destroyVault(Vault v);

/*
 * Loads a Vault from a file
 */
Vault LoadVault(char* file_path);

//Printout all FAT entries.
void PrintList(char* file_path);

/*
 * Take a file and try to insert it into the vault.
 * Fail if such file_name exists already in the repository.
 * Fail if no free space available.
 * Fail if there is free space but the content has to be fragmented into more than 3 blocks.
 */
int AddRecord(char* file_path, char* file);

/*
 * File deletion. Remove file form the vault.
 * Delete (zero) all the related delimiters in the vault file.
 * Fail if no such file_name in the vault.
 * Example: $./vault my_repository.vlt rm file_name
 */
void RemoveRecord(char* file_path, char* file);

/*
 * File fetch. Create a file with some_file_name in the current directory,
 * with the content and permissions stored in the vault.
 * Fail if no such some_file_name is in the vault.
 * Fail if no write permission for the current directory.
 * Example: $./vault my_repository.vlt fetch some_file_name
 */
void GetRecord(char* file_path, char* file);

/*
 * Vault repository defragmentation. Recognize gaps between data blocks.
 * Move data blocks so that no gaps remain. Adjust FAT accordingly.
 * Example: $./vault my_repository.vlt defrag
 */

void DefragVault(char* file_path);

/*
 * Vault repository status retrieve.
 * Print out the number of the files, sum of their sizes, and the Fragmentation Ratio.
 * This number is computed as the following:
 * Find the offset of the first start delimiter (‘<<<<<<<<’, the offset of the 1st ‘<’),
 * and the last closing delimiter (‘>>>>>>>>’, the offset of the last ‘>’).
 * Compute the distance between the two offsets – consumed length.
 * In the same manner, measure the length of every gap, i.e. the distance between every
 * sequential pair of closing and start delimiters. Sum these distances.
 * Fragmentation Ratio is the ratio of the sum of the gaps and the consumed length.
 * Example: $./vault my_repository.vlt status
 */
void VaultStatus(char* file_path);

/*
 *
 */
int writeStringToFile(int file,char* buffer,int buffer_size);

int writeVaultToFile(int vault_file,Vault v);

Vault readVault(int vault_file);

#endif /* VAULT_H_ */
