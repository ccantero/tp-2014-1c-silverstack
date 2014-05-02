/*
 * protocol.h
 *
 *  Created on: 26/04/2014
 *      Author: utnso
 */

#ifndef PROTOCOL_H_
#define PROTOCOL_H_

#include <stdio.h> // printf
#include <stdlib.h>  // atoi
#include <string.h> //strcpy, strtok, strcmp
#include <sys/types.h> // read
#include <unistd.h> // pause
#include <pthread.h> //
#include <fcntl.h> // O_RDONLY
#include <sys/socket.h> // socket, connect
#include <arpa/inet.h> // struct sockaddr_in>
#include <time.h> // time()
#include <commons/log.h> // log_create, log_info, log_error
#include <commons/config.h> // config_get_int_value
#include <commons/collections/queue.h> // queue_create
#include <parser/metadata_program.h>

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

typedef struct _t_semaphore {
	char* identifier;
	int	value;
} t_semaphore;

typedef struct _t_global {
	char* identifier;
	int	value;
} t_global;

typedef struct _instruction_index {
	t_size size;
	t_intructions* index;
} t_instruction_index;

typedef struct _etiquetas_index {
	t_size size;
	char* etiquetas;
} t_etiquetas_index;

typedef struct _t_segment {
	unsigned char code_identifier;
	int start;
	size_t offset;
} t_segment;

typedef struct _t_nodo_segment {
	int start;
	size_t offset;
}t_nodo_segment;

typedef struct _pcb {
	unsigned int unique_id; /* Identificador Único */
	t_segment code_segment; /* Código Ansisop del programa */
	t_segment stack_segment; /* Segmento de stack */
	int stack_pointer; /* Puntero al inicio del contexto de ejecución actual */
	t_instruction_index instruction_index; /* Índice de código */
	t_etiquetas_index etiquetas_index; /* Índice de etiquetas */
	int program_counter; /* Número de la próxima instrucción */
	int context_actual;
	int peso;
}t_pcb;

#define KEYS_AMOUNT 14
#define PATH_CONFIG "conf"

#define MAXDATASIZE 1024
#define SIZE_HDR sizeof(thdr)
#define MSG_CON_PRG 0x01
#define MSG_CON_PRG_OK 0x02
#define MSG_CON_PRG_FAIL 0x03
#define MSG_CON_PRG_TXT 0x04
#define MSG_CON_PRG_TXT_OK 0x05
#define MSG_CON_UMV 0x10
#define MSG_CON_UMV_OK 0x11
#define MSG_CON_UMV_FAIL 0x12
#define CODE_SEGMENT 0x20
#define STACK_SEGMENT 0x21
#define backlog 10

t_log *logger;
t_queue *queue_io;
t_list *list_pcb_new;
t_list *list_pcb_ready;
t_list *list_segment;
t_list *list_semaphores;
t_list *list_globales;

int port_cpu,port_program,port_umv,sockPrin,multiprogramacion,quantum,retardo;
int sock_umv, process_Id;
char myip[16],umv_ip[16];

void GetInfoConfFile(void);
int conectar_umv(void);
t_global* global_create(char *global_name, int value);
int global_update_value(char* global_name, int value);
int global_get_value(char* global_name);
t_io* io_create(char *io_name, int io_retardo);
void io_destroy(t_io*);
t_semaphore* semaphore_create(char* sem_name, int value);
void semaphore_destroy(t_semaphore *self);
void semaphore_wait(char* sem_name);
void semaphore_signal(char* sem_name);
void servidor_plp(void);
int escuchar_Nuevo_Programa(int sock_program, char* buffer);
int escuchar_Programa(int sock_program, char* buffer);
void create_pcb(char* buffer, int tamanio_buffer);
void destroy_pcb(t_pcb* self);
void sort_plp(void);
void planificador_sjn(void);
t_nodo_segment* segment_create(int start, int offset);
void segment_destroy(t_nodo_segment *self);
int get_Segment_Start(int offset);

#endif /* PROTOCOL_H_ */
