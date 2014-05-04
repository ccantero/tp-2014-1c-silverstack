/*
 * kernel.c
 *
 *  Created on: 26/04/2014
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
	int b = 0; // To Avoid Warning
	pthread_t th_plp;
	pthread_t th_pcp;

	logger = log_create("Log.txt", "Program",false, LOG_LEVEL_INFO);
	queue_io = queue_create();
	list_segment = list_create();
	list_semaphores = list_create();
	list_pcb_new = list_create();
	list_pcb_ready = list_create();
	list_globales = list_create();
	list_cpu = list_create();

	process_Id = 10000;
	cantidad_cpu = 0;

	GetInfoConfFile();
	//sock_umv = conectar_umv();
	//if(sock_umv == -1)
	//	return -1;

	pthread_create(&th_plp,NULL,(void*)servidor_plp,(void*)a);
	//pthread_create(&th_pcp,NULL,(void*)servidor_pcp,(void*)b);
	pause();
	log_destroy(logger);
	return 1;
}


