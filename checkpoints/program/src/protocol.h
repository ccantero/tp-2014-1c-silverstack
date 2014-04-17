/*
 * protocol.h
 *
 *  Created on: 13/04/2014
 *      Author: SilverStack
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

#define KEYS_AMOUNT 4
#define PATH_CONFIG "conf"

t_log* logger;
t_config* config;

int port,sockPrin;
char myip[16],hostip[16];

void consola (void* param);
void GetInfoConfFile(void);

#endif /* PROTOCOL_H_ */
