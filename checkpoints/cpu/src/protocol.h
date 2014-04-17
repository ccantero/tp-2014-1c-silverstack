/*
 * protocol.h
 *
 *  Created on: 16/04/2014
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

#define KEYS_AMOUNT 5
#define PATH_CONFIG "conf"

t_log* logger;


int port_kernel,port_umv,sockPrin;
char myip[16],kernelip[16],umvip[16];

void GetInfoConfFile(void);

#endif /* PROTOCOL_H_ */
