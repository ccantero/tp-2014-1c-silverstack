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
#include <signal.h> // SIGINT
#include <semaphore.h>  /* Semaphore */
#include <commons/log.h> // log_create, log_info, log_error
#include <commons/config.h> // config_get_int_value
#include <commons/collections/queue.h> // queue_create
#include <parser/metadata_program.h>
#include <src/silverstack.h>

/*Definidio en silverstack.h

typedef struct {
	int tipo;
	int id_proceso;
	int datosNumericos;
	char mensaje[16];
} t_mensaje;

typedef struct _hdr
{
	char desc_id[16];
	unsigned char pay_desc;
	int pay_len;
}thdr;

typedef struct _io {
	char* name;
	int retardo;
	t_queue *io_queue;
	sem_t io_sem;
	pthread_t* th_io;
}t_io;

typedef struct _t_semaphore {
	char* identifier;
	int	value;
	t_queue* queue;
} t_semaphore;

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

typedef struct _t_global {
	char* identifier;
	int	value;
} t_global;

typedef struct _t_nodo_cpu {
	int	socket;
	unsigned char status;
}t_nodo_cpu;

typedef struct _pcb {
	unsigned int unique_id; // Identificador Único
	t_segment code_segment; // Código Ansisop del programa
	t_segment stack_segment; // Segmento de stack
	int stack_pointer; // Puntero al inicio del contexto de ejecución actual
	t_instruction_index instruction_index; // Índice de código
	t_etiquetas_index etiquetas_index; // Índice de etiquetas
	int program_counter; // Número de la próxima instrucción
	int context_actual;
	int peso;
}t_pcb;



 Finaliza Sylverstack.h
*/

typedef struct _t_process {
	unsigned int pid;
	int program_socket;
	int current_cpu_socket;
	unsigned char status;
} t_process;

typedef struct _t_io_queue_nodo {
	unsigned int pcb;
	int retardo;
} t_io_queue_nodo;

typedef struct t_nodo_queue_semaphore {
	int process_id;
}t_nodo_queue_semaphore;

#define MAXDATASIZE 1024 // SylverStack
#define SIZE_MSG sizeof(t_mensaje)
#define MSG_CON_UMV 0x10
#define MSG_CON_UMV_OK 0x11
#define MSG_CON_UMV_FAIL 0x12
#define CODE_SEGMENT 0x20
#define STACK_SEGMENT 0x21
#define backlog 10
#define HANDSHAKE 100 // SylverStack
#define HANDSHAKE_OK 101 // SylverStack
#define CPU 200 // SylverStack
#define PROGRAMA 203 // Syl1verStack
#define SENDFILE 104 // SylverStack
#define KERNEL 202 // SylverStack
#define QUANTUMFINISH 301 // SylverStack
#define CPU_AVAILABLE 0x30 // CPU Node Status // Ready
#define CPU_IDLE 0x31 // CPU Node Status // Not working because multiprogramacion
#define CPU_WORKING 0x32 // CPU Node Status
#define PROCESS_NEW 0x40 // Process Node Status
#define PROCESS_READY 0x41 // Process Node Status
#define PROCESS_EXECUTE 0x42 // Process Node Status
#define PROCESS_BLOCKED 0x43 // Process Node Status

#define STACK_AMOUNT 400

t_log *logger;
t_list *list_io;
t_list *list_pcb_new;
t_list *list_pcb_ready;
t_list *list_pcb_execute;
t_list *list_pcb_blocked;
t_list *list_segment;
t_list *list_semaphores;
t_list *list_globales;
t_list *list_cpu;
t_list *list_process;

int port_cpu,port_program,port_umv,sockPrin,multiprogramacion,quantum,retardo,stack_tamanio;
int sock_umv, process_Id, cantidad_cpu;
char myip[16],umv_ip[16];
sem_t free_io_queue;

sem_t mutex_new_queue;
sem_t mutex_ready_queue;
sem_t mutex_execute_queue;
sem_t mutex_block_queue;

sem_t mutex_process_list;
sem_t sem_plp;
sem_t sem_pcp;
sem_t mutex_cpu_list;
sem_t sem_cpu_list;

void GetInfoConfFile(char* PATH_CONFIG);
int conectar_umv(void);
t_global* global_create(char *global_name);
int global_update_value(char* global_name, int value);
int global_get_value(char* global_name);
t_io* io_create(char *io_name, int io_retardo);
t_semaphore* semaphore_create(char* sem_name, int value);
int semaphore_wait(char* sem_name, int process_id);
int semaphore_signal(char* sem_name);
int escuchar_Nuevo_Programa(int sock_program);
int escuchar_Programa(int sock_program, char* buffer);
void create_pcb(char* buffer, int tamanio_buffer, int sock_program);
int create_segment(int process_id, int tamanio);
int change_process(int process_id);
int send_bytes(int base, int offset, int tamanio);

void sort_plp(void);
void planificador_sjn(void);
t_nodo_segment* segment_create(int start, int offset);
void segment_destroy(t_nodo_segment *self);
int is_Connected_CPU(int socket);
int escuchar_Nuevo_cpu(int sock_cpu);
int escuchar_cpu(int sock_cpu);
t_nodo_cpu* cpu_create(int socket);
void cpu_remove(int socket);
void cpu_update(int socket);
t_process* process_create(unsigned int pid, int socket);
void pcb_move(unsigned int pid,t_list* from, t_list* to);
void io_wait(unsigned int pid, char* io_name, int amount);
t_io_queue_nodo* io_queue_create(unsigned int process_id, int retardo);
void retardo_io(void *ptr);
void found_cpus_available(void);
void depurar(int signum);
int send_segment_umv(int pid, char* buffer, int tamanio);
int servidor_Programa(void);
int servidor_CPU(void);
int buscar_Mayor(int a, int b, int c);
void escuchar_umv(void);
int is_Connected_Program(int sock_program);
void process_remove_by_socket(int socket);
void planificador_rr(void);
void ejecutar_proceso(int unique_id, int cpu_socket);
int send_umv_stack(int process_id);

int get_Segment_Start(int offset);			// A revisar si va o no va
void io_destroy(t_io*); 					// A revisar si va o no va
void semaphore_destroy(t_semaphore *self);	// A revisar si va o no va
void destroy_pcb(t_pcb* self);				// A revisar si va o no va

void servidor_pcp(void); 					// Para eliminar
void servidor_plp(void); 					// Para eliminar

#endif /* PROTOCOL_H_ */
