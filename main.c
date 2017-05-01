/*
 * main.c
 *
 *  Created on: Apr 30, 2017
 *      Author: Yair Hadas
 */
#include "Vault.h"
#include <stdio.h>

int main(int argc, char* argv[]){
	/*
	if(argc<4){
		printf("Invalid number of arguments\n");
		return -1;
	}
	*/
	Vault v;

	init("V.vlt","200B");
	printf("Created vault successfully\n");
	AddRecord("V.vlt","A.txt");
	printf("Wrote A to V\n");
	AddRecord("V.vlt","B.txt");
	printf("Wrote B to V\n");
	AddRecord("V.vlt","C.txt");
	printf("Wrote C to V\n");


}
