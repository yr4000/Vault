/*
 * File_Record.h
 *
 *  Created on: Apr 14, 2017
 *      Author: Yair Hadas
 */

#ifndef FILE_RECORD_H_
#define FILE_RECORD_H_

#include <sys/types.h>

//#define FULL_FR_SIZE (sizeof(struct File_Record)+3*sizeof(struct blocks))
#define FR_SIZE sizeof(struct File_Record)
#define BLOCK_SIZE sizeof(struct blocks)

typedef struct File_Record *FR;
typedef struct blocks *Blocks;
//TODO: maybe the block offset should be handled in a different DS
typedef struct File_Record {
	char file_name[256];
	int is_deleted;
	ssize_t file_size;
	mode_t file_protection;
	time_t insertion_time;
	off_t block_offset_1;
	ssize_t block_size_1;
	off_t block_offset_2;
	ssize_t block_size_2;
	off_t block_offset_3;
	ssize_t block_size_3;
};

typedef struct blocks {
	off_t block_offset;
	ssize_t block_size;
};

/* gets a string in the form "xxxunit" and returns a number where unit = {"b","k","m","g"}
 * Allocate a vault repository file. Initialize Catalog section.
 * Fail if the requested file size isn’t big enough.
 * Example: $./vault my_repository.vlt init 2M
 */
ssize_t string_to_size(char* size);

FR create_file_record(char* file);

int writeFR(int vault_file,FR fr);

int moveFilePointer(int file,int bytes_no);

#endif /* FILE_RECORD_H_ */
