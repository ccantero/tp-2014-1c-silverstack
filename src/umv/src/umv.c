/*
 * umv.c
 *
 *  Created on: 13/04/2014
 *  Author: SilverStack
 *  Compiler Include: Path to commons lib
 *  Linker Command: gcc -lpthread -pthread
 *  Linker Library: -commons
*/

#include "protocol.h"

int main(void) {
		pthread_t th1, new_th;

		logger = log_create("Log.txt", "UMV",false, LOG_LEVEL_INFO);

		GetInfoConfFile();

		memoria = malloc(space);

		list_programas = list_create();
		sem_init(&mutex_program_list, 0, 1);

		pthread_create(&th1,NULL,(void*)consola,NULL);

		int clientsock, serverSocket = sockets_createServer(myip,port, MAX_CONEXIONES);

		dic_cpus = dictionary_create();

		struct sockaddr_in client;
		int sin_size = sizeof(struct sockaddr_in);

		while ((clientsock = accept(serverSocket, (struct sockaddr *) &client, (socklen_t *) &sin_size)))
		{
			log_info(logger, "Conexion aceptada.");

			if(atenderConexionNueva(clientsock) < 1)
			    	break;
		}

		pause();
		return 1;
}
