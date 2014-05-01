/*
 * program.c
 *
 *  Created on: 13/04/2014
 *  Author: SilverStack
 *  Compiler Include: Path to commons lib
 *  Linker Command: gcc -lpthread -pthread
 *  Linker Library: -commons
*/

#include "protocol.h"

int main(void) {
		int a=0; /* Just to avoid Warning Message */
		pthread_t th1;

		logger = log_create("Log.txt", "UMV",false, LOG_LEVEL_INFO);

		GetInfoConfFile();

		memoria = malloc(space);

		list_programas = list_create();

		pthread_create(&th1,NULL,(void*)consola,(void*)a);
		pause();
		return 1;
}
