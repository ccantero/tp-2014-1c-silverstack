/*
 * silverstack.h
 *
 *  Created on: 28/04/2014
 *      Author: utnso
 */

#ifndef SILVERSTACK_H_
#define SILVERSTACK_H_

#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>  /* Semaphore */
#include <parser/metadata_program.h>
#include <commons/collections/queue.h>

#define MAXDATASIZE 1024
//#define MSG_CON_PRG 0x01
//#define MSG_CON_PRG_OK 0x02
//#define MSG_CON_PRG_FAIL 0x03
//#define MSG_CON_PRG_TXT 0x04
//#define MSG_CON_PRG_TXT_OK 0x05
#define CODE_SEGMENT 0x20
#define STACK_SEGMENT 0x21
#define HANDSHAKE 100
#define HANDSHAKEOK 101
#define CPU 200
#define UMV 201
#define KERNEL 202
#define PROGRAMA 203
#define INSTRUCCIONREQUEST 300
#define QUANTUMFINISH 301
#define MEMORIAREQUEST 302
#define VARIABLEREQUEST 303
#define POSICIONREQUEST 304
#define REQUESTOK 305
#define ASIGNACION 306
#define IMPRIMIR 307
#define IMPRIMIRTEXTO 308
#define VARCOMREQUEST 309
#define ENTRADASALIDA 310
#define SIGNALSEM 311
#define WAITSEM 312
#define NEW_PROGRAM_REQUEST 401
#define NEW_PROGRAMOK 402
#define STACK_SIZE 403
#define LOW_MEMORY 404
#define TAMANIO_REQUEST 405
#define TAMANIOOK 406
#define STACKREQUEST 407
#define STACKOK 408
#define INSTRUCTIONREQUEST 409
#define INSTRUCTIONREQUESTOK 410
#define ETIQUETASREQUEST 409
#define ETIQUETASREQUESTOK 410

typedef struct {
	int tipo;
	int id_proceso;
	int datosNumericos;
	char mensaje[16];
} t_mensaje;

typedef struct _hdr {
	char desc_id[16];
	unsigned char pay_desc;
	int pay_len;
} thdr;

#define SIZE_HDR sizeof(thdr)

typedef struct _io {
	char* name;
	int retardo;
	t_queue *io_queue;
	sem_t io_sem;
	pthread_t* th_io;
} t_io;

typedef struct _t_semaphore {
	char* identifier;
	int	value;
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
} t_nodo_segment;

typedef struct _t_global {
	char* identifier;
	int	value;
} t_global;

typedef struct _t_nodo_cpu {
	int	socket;
	unsigned char status;
}t_nodo_cpu;

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
} t_pcb;


#endif /* SILVERSTACK_H_ */
