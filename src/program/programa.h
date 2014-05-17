/*
 * programa.h
 *
 *  Created on: 30/04/2014
 *      Author: utnso
 */

#ifndef PROGRAMA_H_
#define PROGRAMA_H_


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <commons/log.h>
#include <commons/config.h>
#include <unistd.h>
#include <errno.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include <sys/stat.h>

typedef struct {
	int tipo;
	int id_proceso;
	int datosNumericos;
	char mensaje[16];
} t_mensaje;

#define HANDSHAKE 100
#define HANDSHAKEOK 101
#define PROGRAMA 203
#define SENDFILE 104

#define IMPRIMIR 108
#define IMPRIMIRTEXTO 109
#define SALIR 110

t_config *config;
t_log *logger;

#endif /* PROGRAMA_H_ */
