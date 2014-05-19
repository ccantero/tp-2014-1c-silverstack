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
	int nuevo_valor;
	log_info(logger, "Se lanzo el hilo de consola");
	// Bucle principal esperando peticiones del usuario
	for(;;)
	{
		flag_comandoOK = 0;
    	printf("-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-\n");
    	printf("         Bienvenido a la consola de UMV           \n");
    	printf("-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-\n");
    	printf("Ingrese el comando: \n");
    	printf(">");
    	scanf("%s",comando);
    	c = strtok(comando,"\n");
   		if (flag_comandoOK == 0 && strncmp(c,"operacion",strlen("operacion"))==0 )
   		{
   			printf("Llamar a funcion operacion\n");
   			flag_comandoOK = 1;
   		}
   		if (flag_comandoOK == 0 && strncmp(c,"retardo",strlen("retardo"))==0)
   		{
   			log_info(logger, "Se cambio el valor de retardo por consola.");
   			printf("Nuevo valor de retardo: ");
   			scanf("%d", &nuevo_valor);
   			printf("Valor anterior de retardo: %d\n", retardo);
   			cambiar_retardo(nuevo_valor);
   			printf("Valor actual de retardo: %d\n", retardo);
   			flag_comandoOK = 1;
   		}
   		if (flag_comandoOK == 0 && strncmp(c,"algoritmo",strlen("algoritmo"))==0)
   		{
   			log_info(logger, "Se cambio el algoritmo por consola.");
   			printf("Se cambio el algoritmo.\n");
   			printf("Algoritmo anterior: %s\n", algoritmo);
   			cambiar_algoritmo();
   			printf("Algoritmo actual: %s\n", algoritmo);
   		   	flag_comandoOK = 1;
   		}
   		if (flag_comandoOK == 0 && strncmp(c,"compactacion",strlen("compactacion"))==0)
   		{
   			log_info(logger, "Se pidio compactar memoria por consola.");
   			compactar_memoria();
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
	retardo = config_get_int_value(config, "RETARDO");
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

void cambiar_algoritmo()
{
	pthread_mutex_lock(&semAlgoritmo);
	if (!strcmp(algoritmo, "WF") ||
		!strcmp(algoritmo, "Wf") ||
		!strcmp(algoritmo, "wf") ||
		!strcmp(algoritmo, "Worst-Fit") ||
		!strcmp(algoritmo, "Worst-fit") ||
		!strcmp(algoritmo, "worst-fit"))
	{
		strcpy(algoritmo, "First-Fit");
	}
	else
	{
		strcpy(algoritmo, "Worst-Fit");
	}
	pthread_mutex_unlock(&semAlgoritmo);
}

void cambiar_retardo(int valor)
{
	pthread_mutex_lock(&semRetardo);
	retardo = valor;
	pthread_mutex_unlock(&semRetardo);
}

void compactar_memoria()
{
	pthread_mutex_lock(&semCompactacion);
	int primer_direccion = space;
	int primer_programa;
	int i;
	int j;
	int nueva_direccion = 0;
	int arranque = 0;
	t_info_programa *prog;
	t_info_segmento *segm;
	int cant_segmentos = obtener_cant_segmentos();
	int cont;
	if (cant_segmentos != 0)
	{
		// TODO Reflejar cambios en "memoria"
		printf("Compactando...\n");
		for (cont = 0; cont < cant_segmentos; cont++)
		{
			// Busco la menor direccion en mi lista de segmentos utilizados
			for (i = 0; i < list_size(list_programas); i++)
			{
				prog = list_get(list_programas, i);
				for (j = 0; j < list_size(prog->segmentos); j++)
				{
					segm = list_get(prog->segmentos, j);
					if (segm->dirFisica < primer_direccion && segm->dirFisica >= arranque)
					{
						primer_direccion = segm->dirFisica;
						primer_programa = segm->id;
					}
					else
					{
						// TODO
					}
				}
			}
			for (i = 0; i < list_size(list_programas); i++)
			{
				prog = list_get(list_programas, i);
				if (prog->programa == primer_programa)
				{
					break;
				}
			}
			for (j = 0; j < list_size(prog->segmentos); j++)
			{
				segm = list_get(prog->segmentos, j);
				if (segm->dirFisica == primer_direccion)
				{
					break;
				}
			}
			segm->dirFisica = nueva_direccion;
			nueva_direccion = segm->tamanio + 1;
			primer_direccion = space;
			arranque = nueva_direccion;
		}
	}
	else
	{
		printf("Todavia no hay segmentos a compactar.\n");
	}
	pthread_mutex_unlock(&semCompactacion);
}

int obtener_cant_segmentos()
{
	int cant = 0;
	int i, j;
	t_info_programa *prog;
	t_info_segmento *segm;
	for (i = 0; i < list_size(list_programas); i++)
	{
		prog = list_get(list_programas, i);
		cant += list_size(prog->segmentos);
	}
	return cant;
}

