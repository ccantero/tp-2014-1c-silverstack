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

		int fdMax = 0, sockfd, newfd, rv = 0;
		int serverSocket = sockets_createServer(myip,port, MAX_CONEXIONES);

		fd_set readfds;
		fd_set master;
		dic_cpus = dictionary_create();
		FD_ZERO(&readfds);
		FD_ZERO(&master);
		FD_SET(serverSocket, &master);
		fdMax = serverSocket;

		while (1) {
			readfds = master;
			rv = select(fdMax + 1, &readfds, NULL, NULL, NULL );
			if (rv < 0) {
				log_error(logger, "select");
			} else if (rv == 0) {
				log_error(logger, "select time out connection");
			} else {
				for (sockfd = 0; sockfd <= fdMax; sockfd++) {
					if (FD_ISSET(sockfd, &readfds)) {
						if (sockfd == serverSocket) {
							if ((newfd = sockets_accept(serverSocket)) == -1)
								log_error(logger, "accept");
							else {
								newfd = aceptarConexionNueva(newfd, &master);
								if (newfd > fdMax)
									fdMax = newfd;
							}
						} else {
							if (atenderPedido(sockfd) == 0) {
								FD_CLR(sockfd, &master);
								close(sockfd);
							}
						}
					}
				}
			}
		}

		pause();
		return 1;
}
