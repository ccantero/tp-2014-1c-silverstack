/*
 * protocol.h
 *
 *  Created on: 15/04/2014
 *  Author: SilverStack
*/

#ifndef PROTOCOL_H_
#define PROTOCOL_H_

#include <stdio.h> // printf
#include <stdlib.h>  // atoi
#include <string.h> //strcpy, strtok, strcmp
#include <sys/types.h> // read
#include <unistd.h> // pause
#include <pthread.h> //
#include <commons/log.h> // log_create, log_info, log_error
#include <commons/config.h> //

#define KEYS_AMOUNT 7
#define PATH_CONFIG "conf"

t_log* logger;


int port_kernel,port_cpu,sockPrin,space;
char myip[16],hostip[16],algoritmo[3];

void consola (void* param);
void GetInfoConfFile(void);

#endif /* PROTOCOL_H_ */
