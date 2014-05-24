/*
 * hilokernel.c
 *
 *  Created on: 24/05/2014
 *      Author: utnso
 */

#include "protocol.h"

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
	list_add(list_programas, crearPrograma(pid));
	sem_post(&mutex_program_list);
	recibirMensaje(socketKernel, &m);
	int tamanioPedido = m.datosNumericos;

	t_info_segmento segm;
	pedidoDeMemoria(pid, tamanioPedido, &segm);
	char* codigo = malloc(tamanioPedido);
	guardarEnSegmento(pid, segm.id, codigo);
	free(codigo);

	recibirMensaje(socketKernel, &m);

	recibirMensaje(socketKernel, &m);

	return 0;
}

int crearPrograma(int pid) {
	return 0;
}

int pedidoDeMemoria(int pid, int tamanioPedido, t_info_segmento* segm) {
	t_mensaje m;
	int dir;
	if((dir = buscarMemoriaDisponible(tamanioPedido))) {
		segm = crearSegmento(pid, dir, tamanioPedido);
		return 1;
	}

	sockets_send(socketKernel, LOW_MEMORY, 0, "");
	return 0;
}
