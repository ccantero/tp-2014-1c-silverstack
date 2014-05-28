/*
 * hilokernel.c
 *
 *  Created on: 24/05/2014
 *      Author: utnso
 */

#include "protocol.h"

int reservarNuevoPrograma(int pid);
t_info_programa* crearPrograma(int pid);
int nuevoSegmento();
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
		case CREARSEGMENTO:
			nuevoSegmento();
			break;
		case DESTRUIRSEGMENTOS:
			break;
		default:
			log_error(logger, "No se identifica mensaje del Kernel");
			break;
		}
	}

	return 0;
}

int reservarNuevoPrograma(int pid) {
	//TODO: Esto no anda
	int _existe_programa(t_info_programa* p) {
		return p->pid == pid;
	}

	log_info(logger,"list_any_satisfy");

	if(list_any_satisfy(list_programas, (void *) _existe_programa) > 0) {
		log_info(logger,"list_any_satisfy. Entre al if");
		sem_wait(&mutex_program_list);
		t_info_programa* prog = crearPrograma(pid);
		sem_post(&mutex_program_list);
	}
	return 1;
}

t_info_programa* crearPrograma(int pid) {
	t_info_programa* prog = malloc(sizeof(t_info_programa));
	prog->pid = pid;
	prog->segmentos = list_create();
	list_add(list_programas, prog);
	return prog;
}

int nuevoSegmento() {
	t_msg_crear_segmento* m = malloc(sizeof(t_msg_crear_segmento));

	reservarNuevoPrograma(m->id_programa);

	if(recv(socketKernel, m, sizeof(t_msg_crear_segmento), MSG_WAITALL)) {
		if(!solicitudSegmento(m->id_programa, m->tamanio)) {
			free(m);
			log_error(logger, "No se pudo crear segmento.");
			return 0;
		}
	}
	else {
		log_error(logger, "Mensaje inesperado.");
		return 0;
	}

	send(socketKernel, &m, sizeof(t_msg_crear_segmento), 0);
	free(m);
	return 1;
}

int pedidoDeMemoria(int pid, int tamanioPedido, t_info_segmento* segm) {
	int dir = buscarMemoriaDisponible(tamanioPedido);

	if(dir >= 0) {
		//TODO: Descomentar comportamiento de los semaforos.
		//pthread_mutex_lock(&semCompactacion);
		segm = crearSegmento(pid, dir, tamanioPedido);
		//pthread_mutex_unlock(&semCompactacion);
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
	//TODO: Agrego mensaje de de CREARSEGMENTO
	/*

	t_mensaje mensaje;
	mensaje.id_proceso = UMV;
	mensaje.tipo = CREARSEGMENTO;
	log_info(logger, "Mando t_mensaje %d", mensaje.tipo);
	send(socketKernel, &mensaje, sizeof(t_mensaje), 0);
	*/
	// TODO: NO se guarda el buffer todavia. Es solo CREARSEGMENTO
	//		 Hacer comportamiento para envio_bytes

	recv(socketKernel, buffer, tamanioPedido, 0);
	if(!guardarEnSegmento(pid, segm->id, buffer))
		return 0;
	free(buffer);
	return 1;
}
