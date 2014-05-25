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
#include <commons/config.h>
#include <commons/log.h>
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
	int pid;
	t_list *segmentos;
} t_info_programa;

t_log* logger;
t_list* list_programas;
t_dictionary* dic_cpus;

sem_t mutex_program_list;

pthread_mutex_t semRetardo = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t semAlgoritmo = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t semCompactacion = PTHREAD_MUTEX_INITIALIZER;

int sockPrin,space, socketKernel;
char hostip[16],algoritmo[16];
char* memoria, *myip, *port;
int proceso_activo;
int retardo;

void consola (void* param);
void GetInfoConfFile(void);
int atenderConexionNueva(int newfd);
int enviarHandshake(int sockfd);
int sockets_createServer(char *addr, char *port, int backlog);
int sockets_send(int sockfd, t_mensaje *m, char *mensaje);
int sockets_accept(int sockfd);
int atenderPedido(int sockfd);
void compactar_memoria();
int obtener_cant_segmentos();
void cambiar_retardo(int valor);
void cambiar_algoritmo();
int findLastSegmentIn(int address, int limit, t_info_segmento* segm);
int getSegmentByBase(int address, t_info_segmento* segm);
int getProgramBySegmentId(int segmId, t_info_programa* prog);
int guardarEnSegmento(int pid, int segmId, char* codigo);
t_info_segmento* crearSegmento(int pid, int dirFisica, int tamanioPedido);
int getFirstFitMemory(int memSize);
int getWorstFitMemory(int memSize);
int buscarMemoriaDisponible(int tamanio);
int recibirMensaje(int newfd, t_mensaje* m);
int enviarMensajeKernel(int tipo, int datosNumericos);
int lanzarHiloCPU(int newfd);
int lanzarHiloKernel(int newfd);
int guardarEnMemoria(t_info_segmento* segm,char* buffer);
void *hilokernel(void *socket_desc);
void *hilocpu(void *socket_desc);
#endif /* PROTOCOL_H_ */
