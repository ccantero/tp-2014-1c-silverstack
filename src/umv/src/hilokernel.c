/*
 * hilokernel.c
 *
 *  Created on: 24/05/2014
 *      Author: utnso
 */

#include "protocol.h"

int ReservarNuevoPrograma(int pid);
t_info_programa* crearPrograma(int pid);
int nuevoSegmento(int pid, int tipo, int tipoOk);
int pedidoDeMemoria(int pid, int tamanioPedido, t_info_segmento* segm);
int solicitudSegmento(int pid, int tamanioPedido);

void *hilokernel(void *socket_desc)
{
    //Get the socket descriptor
    socketKernel = *(int*)socket_desc;

    t_mensaje m;
    int nbytes;

	while (recibirMensaje(socketKernel, &m) > 1)
	{
		switch (m.tipo) {
		case NEW_PROGRAM_REQUEST:
			ReservarNuevoPrograma(m.datosNumericos);
			break;
		default:
			log_error(logger, "No se identifica mensaje del Kernel");
			break;
		}
	}

	return 0;
}

int ReservarNuevoPrograma(int pid) {
	sem_wait(&mutex_program_list);
	t_info_programa* prog = crearPrograma(pid);
	sem_post(&mutex_program_list);

	nuevoSegmento(pid, TAMANIO_REQUEST, TAMANIOOK);
	nuevoSegmento(pid, ETIQUETASREQUEST, ETIQUETASREQUESTOK);
	nuevoSegmento(pid, INSTRUCTIONREQUEST, INSTRUCTIONREQUESTOK);
	nuevoSegmento(pid, STACKREQUEST, STACKOK);

	return 1;
}

t_info_programa* crearPrograma(int pid) {
	t_info_programa* prog = malloc(sizeof(t_info_programa));
	prog->pid = pid;
	prog->segmentos = list_create();
	list_add(list_programas, prog);
	return prog;
}

int nuevoSegmento(int pid, int tipo, int tipoOk) {
	t_mensaje m;
	recibirMensaje(socketKernel, &m);
	if(m.tipo == tipo) {
		if(solicitudSegmento(pid, m.datosNumericos))
			enviarMensajeKernel(tipoOk, 0);
		else {
			log_error(logger, "No se pudo crear segmento: %d.", tipo);
			return 0;
		}
	}
	else {
		log_error(logger, "Mensaje inesperado: %d (%d)", m.tipo, tipo);
		return 0;
	}
	return 1;
}

int pedidoDeMemoria(int pid, int tamanioPedido, t_info_segmento* segm) {
	int dir;
	if((dir = buscarMemoriaDisponible(tamanioPedido))) {
		segm = crearSegmento(pid, dir, tamanioPedido);
		return 1;
	}
	enviarMensajeKernel(LOW_MEMORY, 0);
	return 0;
}

int solicitudSegmento(int pid, int tamanioPedido) {
	t_info_segmento* segm = malloc(sizeof(t_info_segmento));
	if(!pedidoDeMemoria(pid, tamanioPedido, segm))
		return 0;
	char* buffer = malloc(tamanioPedido);
	recv(socketKernel, buffer, tamanioPedido, 0);
	if(!guardarEnSegmento(pid, segm->id, buffer))
		return 0;
	free(buffer);
	return 1;
}
