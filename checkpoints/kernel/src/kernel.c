/*
 * kernel.c
 *
 *  Created on: 17/04/2014
 *  Author: SilverStack
 *	Compiler Include: Path to commons lib
 *  Linker Command: gcc -lpthread -pthread
 *  Linker Library: -commons
 */

/* Header File */

#include "protocol.h"

int main(void)
{
	int a = 0; // To Avoid Warning
	pthread_t th1;

	logger = log_create("Log.txt", "Program",false, LOG_LEVEL_INFO);
	queue_io = queue_create();

	GetInfoConfFile();
	pthread_create(&th1,NULL,(void*)servidor_plp,(void*)a);
	return 1;
}
