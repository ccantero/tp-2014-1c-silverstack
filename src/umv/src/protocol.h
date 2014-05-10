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
#include <unistd.h>
#include <pthread.h>
#include <commons/config.h>
#include <commons/log.h>
#include <commons/collections/queue.h>
#include <commons/collections/list.h>

#define KEYS_AMOUNT 7
#define PATH_CONFIG "conf"

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

int port_kernel,port_cpu,sockPrin,space;
char myip[16],hostip[16],algoritmo[3];
char* memoria;

void consola (void* param);
void GetInfoConfFile(void);

#endif /* PROTOCOL_H_ */
