/*
 * program.c
 *
 *  Created on: 13/04/2014
 *  Author: SilverStack
 *  Compiler Include: Path to commons lib
 *  Linker Command: gcc -lpthread -pthread
 *  Linker Library: -commons
*/

/* Header File */

#include "protocol.h"

int main(void)
{
	int a=0; /* Just to avoid Warning Message */
	pthread_t th1;

	logger = log_create("Log.txt", "Program",false, LOG_LEVEL_INFO);

	GetInfoConfFile();
	pthread_create(&th1,NULL,(void*)consola,(void*)a);
	pause();
	return 1;
}
