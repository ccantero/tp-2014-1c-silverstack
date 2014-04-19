/*
 * protocol.h
 *
 *  Created on: 17/04/2014
 *      Author: SylverStack
*/

#ifndef PROTOCOL_H_
#define PROTOCOL_H_

#include <stdio.h> // printf
#include <stdlib.h>  // atoi
#include <string.h> //strcpy, strtok, strcmp
#include <sys/types.h> // read
#include <unistd.h> // pause
#include <pthread.h> //
#include <sys/socket.h> // socket, connect
#include <arpa/inet.h> // struct sockaddr_in>
#include <commons/log.h> // log_create, log_info, log_error
#include <commons/config.h> // config_get_int_value
#include <commons/collections/queue.h> // queue_create

typedef struct _hdr
{
	char desc_id[16];
	unsigned char pay_desc;
	int pay_len;
}thdr;

typedef struct _io {
	char* name;
	int retardo;
}t_io;

#define KEYS_AMOUNT 10
#define PATH_CONFIG "conf"

#define MAXDATASIZE 1024
#define SIZE_HDR sizeof(thdr)
#define MSG_CON_PRG 0x01
#define MSG_CON_PRG_OK 0x02
#define MSG_CON_PRG_FAIL 0x03
#define MSG_CON_PRG_TXT 0x04
#define backlog 10

t_log *logger;
t_queue *queue_io;

int port_cpu,port_program,sockPrin,multiprogramacion,quantum,retardo;
char myip[16];

void GetInfoConfFile(void);
t_io* io_create(char *io_name, int io_retardo);
void io_destroy(t_io*);
void servidor_plp(void);

#endif /* PROTOCOL_H_ */
