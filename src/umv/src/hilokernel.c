/*
 * hilokernel.c
 *
 *  Created on: 24/05/2014
 *      Author: utnso
 */

#include "protocol.h"

t_info_programa* crearPrograma(int pid);
int pedidoDeMemoria(int pid, int tamanioPedido, t_info_segmento* segm);

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
	t_mensaje m;

	sem_wait(&mutex_program_list);

	t_info_programa* prog = crearPrograma(pid);
	list_add(list_programas, prog);
	sem_post(&mutex_program_list);
	recibirMensaje(socketKernel, &m);
	int tamanioPedido = m.datosNumericos;

	t_info_segmento segm;
	pedidoDeMemoria(pid, tamanioPedido, &segm);
	char* codigo = malloc(tamanioPedido);
	recv(socketKernel, codigo, tamanioPedido, 0);
	guardarEnSegmento(pid, segm.id, codigo);
	free(codigo);

	recibirMensaje(socketKernel, &m);

	recibirMensaje(socketKernel, &m);

	return 0;
}

t_info_programa* crearPrograma(int pid) {
	t_info_programa* prog = malloc(sizeof(t_info_programa));
	prog->pid = pid;
	prog->segmentos = list_create();
	return prog;
}

int pedidoDeMemoria(int pid, int tamanioPedido, t_info_segmento* segm) {
	int dir;
	if((dir = buscarMemoriaDisponible(tamanioPedido))) {
		crearSegmento(pid, dir, tamanioPedido);
		return 1;
	}
	t_mensaje m;
	m.datosNumericos = 0;
	m.id_proceso = UMV;
	m.tipo = LOW_MEMORY;
	sockets_send(socketKernel, &m, "");
	return 0;
}
