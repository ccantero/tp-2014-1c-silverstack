/*
 * cpu.h
 *
 *  Created on: 23/04/2014
 *      Author: utnso
 */

#ifndef CPU_H_
#define CPU_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>
#include <commons/log.h>
#include <commons/config.h>
#include <parser/parser.h>
#include <errno.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>

t_log *logger;
t_config *config;

int port_kernel;
int port_umv;
int sockPrin;
char myip[16];
char kernelip[16];
char umvip[16];

int sockfd;
int numBytes;
char buf[256];
struct sockaddr_in their_addr;

void GetInfoConfFile(t_config *config, char *path);
void ConectarAKernel(void);

#endif /* CPU_H_ */
