/*
 * protocol.c
 *
 *  Created on: 15/04/2014
 *  Author: SilverStack
*/

/* Header File */

#include "protocol.h"

void consola (void* param)
{

	char comando[100];
	char *c;
	int flag_comandoOK;

	for(;;)
	{
		//system("clear");
		flag_comandoOK = 0;

		log_info(logger, "Se lanzo el hilo de consola");
    	printf("-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-\n");
    	printf("         Bienvenido a la consola de UMV           \n");
    	printf("-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-\n");

    	printf("Ingrese el comando: \n");
    	printf(">");

    	scanf("%s",comando);
    	c=strtok(comando,"\n");

   		if (flag_comandoOK == 0 && strncmp(c,"operacion",strlen("operacion"))==0 )
   		{
   			printf("Llamar a funcion operacion\n");
   			flag_comandoOK = 1;
   		}

   		if (flag_comandoOK == 0 && strncmp(c,"retardo",strlen("retardo"))==0)
   		{
   			printf("Llamar a funcion retardo\n");
   			flag_comandoOK = 1;
   		}

   		if (flag_comandoOK == 0 && strncmp(c,"algoritmo",strlen("algoritmo"))==0)
   		{
   			printf("Llamar a funcion algoritmo\n");
   		   	flag_comandoOK = 1;
   		}

   		if (flag_comandoOK == 0 && strncmp(c,"compactacion",strlen("compactacion"))==0)
   		{
   			printf("Llamar a funcion compactacion\n");
   		   	flag_comandoOK = 1;
   		}

   		if (flag_comandoOK == 0 && strncmp(c,"dump",strlen("dump"))==0)
   		{
   			printf("Llamar a funcion dump\n");
   		   	flag_comandoOK = 1;
   		}

   		if(flag_comandoOK == 0)
   		{
   			printf("Por favor verifique la sintaxis de los comandos utilizados\n");
   			printf("Los unicos comandos habilitados son: \n");
   			printf("\toperacion\n");
   			printf("\tretardo\n");
   			printf("\talgoritmo\n");
   			printf("\tcompactacion\n");
   			printf("\tdump\n");
   		}
   	}
	return;
}

void GetInfoConfFile(void)
{
	t_config* config;

	config = config_create(PATH_CONFIG);
	if (config_has_property(config, "IP")) {
			myip = (char*) malloc(strlen(config_get_string_value(config, "IP")) + 1);
			strcpy(myip, config_get_string_value(config, "IP"));
	}
	strcpy(algoritmo,config_get_string_value(config, "ALGORITMO"));
	port=config_get_string_value(config, "PORT");
	space=config_get_int_value(config, "DISK_SPACE");
	return;
}

int aceptarConexionNueva(int newfd, fd_set *lista) {
	t_mensaje m;
	int nbytes;

	if ((nbytes = recv(newfd, &m, sizeof(m), MSG_WAITALL)) <= 0) {
		if (nbytes == 0) {//conexion cerrada
			printf("Select: socket %d desconectado.\n", newfd);
		}
		else {
			log_error(logger, "receive");
		}
		close(newfd);
		FD_CLR(newfd, lista);
	}
	else
	{
		switch (m.id_proceso) {
		case KERNEL:
			nbytes = enviarHandshake(newfd);
			if (nbytes == -1)
				log_error(logger, "Envio handshake a Kernel.");
			socketKernel = newfd;
			FD_SET(newfd, lista);
			break;
		case CPU:
			nbytes = enviarHandshake(newfd);
			if (nbytes == -1)
				log_error(logger, "Envio handshake a CPU.");

			FD_SET(newfd, lista);
			break;
		default:
			log_error(logger, "No se identifica proceso en handshake.");
			break;
		}
	}
	return newfd;
}

int enviarHandshake(int sockfd) {
	t_mensaje m;
	m.id_proceso = UMV;
	m.datosNumericos = 0;
	m.tipo = HANDSHAKEOK;
	strcpy(m.mensaje, "hola");

	return send(sockfd, &m, sizeof(t_mensaje), 0);
}

int sockets_listen(int sockfd, int backlog) {
	return listen(sockfd, backlog);
}

int sockets_bind(int sockfd, char *addr, char *port) {
	struct sockaddr_in my_addr;

	my_addr.sin_family = AF_INET;
	my_addr.sin_port = htons(atoi(port));
	my_addr.sin_addr.s_addr = inet_addr(addr);
	memset(&(my_addr.sin_zero), '\0', 8);

	return bind(sockfd, (struct sockaddr *) &my_addr, sizeof(struct sockaddr));
}

int sockets_getSocket(void) {
	int yes = 1;
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);

	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
		close(sockfd);
		return -1;
	}

	return sockfd;
}

int sockets_createServer(char *addr, char *port, int backlog) {
	int sockfd = sockets_getSocket();

	if (sockets_bind(sockfd, addr, port) == -1) {
		close(sockfd);
		return -1;
	}

	if (sockets_listen(sockfd, backlog) == -1) {
		close(sockfd);
		return -1;
	}

	return sockfd;
}

int sockets_accept(int sockfd) {
	struct sockaddr_in their_addr;
	int sin_size = sizeof(struct sockaddr_in);

	return accept(sockfd, (struct sockaddr *) &their_addr,
			(socklen_t *) &sin_size);
}

int sockets_send(int sockfd, t_mensaje *mensaje, char *data) {
	strcpy(mensaje->mensaje, "Hola kernel.");
	return send(sockfd, &mensaje, sizeof(t_mensaje), 0);
}

int atenderPedido(int sockfd) {
	return 0;
}
