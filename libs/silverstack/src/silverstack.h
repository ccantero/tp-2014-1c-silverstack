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
#include <parser/metadata_program.h>

#define KEYS_AMOUNT 10
#define MAXDATASIZE 1024
#define MSG_CON_PRG 0x01
#define MSG_CON_PRG_OK 0x02
#define MSG_CON_PRG_FAIL 0x03
#define MSG_CON_PRG_TXT 0x04
#define MSG_CON_PRG_TXT_OK 0x05
#define CODE_SEGMENT 0x20
#define STACK_SEGMENT 0x21
#define HANDSHAKE 100
#define CPU 101
#define INSTRUCCIONREQUEST 102
#define QUANTUMFINISH 103

typedef struct _hdr {
	char desc_id[16];
	unsigned char pay_desc;
	int pay_len;
} thdr;

#define SIZE_HDR sizeof(thdr)

typedef struct _io {
	char* name;
	int retardo;
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

typedef struct _pcb {
	char* unique_id; /* Identificador Único */
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
