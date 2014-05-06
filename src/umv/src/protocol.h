/*
 * protocol.h
 *
 *  Created on: 15/04/2014
 *  Author: SilverStack
*/

#ifndef PROTOCOL_H_
#define PROTOCOL_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include <commons/log.h>
#include <commons/config.h>
#include <commons/collections/queue.h>
#include <commons/collections/list.h>
#include <src/silverstack.h>

#define MAX_CONEXIONES 7
#define PATH_CONFIG "../conf"

typedef struct {
	int id;
	int dirLogica;
	int dirFisica;
	int tamanio;
} t_info_segmento;

typedef struct {
	char* programa;
	t_info_segmento* segmentos;
} t_info_programa;

t_log* logger;
t_list* list_programas;
t_dictionary* dic_cpus;

int sockPrin,space, socketKernel;
char hostip[16],algoritmo[3];
char* memoria, *myip, *port;

void consola (void* param);
void GetInfoConfFile(void);
int aceptarConexionNueva(int newfd, fd_set *lista);
int enviarHandshake(int sockfd);
int sockets_createServer(char *addr, char *port, int backlog);
int sockets_send(int sockfd, t_mensaje *m, char *mensaje);
int sockets_accept(int sockfd);
int atenderPedido(int sockfd);

#endif /* PROTOCOL_H_ */
