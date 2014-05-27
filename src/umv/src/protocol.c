/*
 * protocol.c
 *
 *  Created on: 15/04/2014
 *  Author: SilverStack
*/

/* Header File */

#include "globales.h"
#include "protocol.h"

void consola (void* param)
{
	initscr();
	echo();
	scrollok(stdscr, TRUE);
	char comando[100];
	char comando2[100];
	int flag_comandoOK;
	int flag_comandoOK2;
	int nuevo_valor;
	int valor_numerico;
	int valor_numerico2;
	int valor_numerico3;
	int proc_id = -1;
	int respuesta;
	int dir_fisica;
	log_info(logger, "Se lanzo el hilo de consola");
	printw("-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-\n");
	printw("         Bienvenido a la consola de UMV           \n");
	printw("-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-\n");
	refresh();
	// Bucle principal esperando peticiones del usuario
	for(;;)
	{
		flag_comandoOK = 0;
		flag_comandoOK2 = 0;
		fflush(stdin);
    	printw("Ingrese el comando: \n");
    	printw(">");
    	refresh();
    	getstr(comando);
   		if (flag_comandoOK == 0 && strcmp(comando, "operacion") == 0)
   		{
   			printw("Operaciones disponibles: \n");
   			printw("\tcrear segmento\n");
   			printw("\tsolicitar memoria\n");
   			printw("\tescribir memoria\n");
   			printw("\tdestruir segmentos\n");
   			printw("Ingrese la operacion: \n");
   			printw(">");
   			refresh();
   			getstr(comando2);
   			if (flag_comandoOK2 == 0 && strcmp(comando2, "crear segmento") == 0)
   			{
   				printw("Ingrese proceso: \n");
   				printw(">");
   				refresh();
   				scanw("%d", &proc_id);
   				respuesta = verificar_proc_id(proc_id);
   				if (respuesta == 0)
   				{
   					printw("El proceso ingresado no existe en memoria.\n");
   					printw("Ingrese un proceso que exista en memoria para crear un segmento.\n");
   					refresh();
   				}
   				else
   				{
   					printw("Ingrese tamanio del segmento: \n");
   					printw(">");
   					refresh();
   					scanw("%d", &valor_numerico);
   					pthread_mutex_lock(&semCompactacion);
   					respuesta = crear_segmento(proc_id, valor_numerico);
   					pthread_mutex_unlock(&semCompactacion);
   					switch(respuesta)
   					{
   					case -1:
   						printw("No hay memoria disponible para ese tamanio de segmento.\n");
   						printw("El segmento no fue creado.\n");
   						refresh();
   						break;
   					default:
   						log_info(logger, "Segmento de proceso %d creado.", proc_id);
   						printw("El segmento fue creado.\n");
   						dir_fisica = transformar_direccion_en_fisica(respuesta, proc_id);
   						printw("Posicion en memoria del segmento: %d\n", dir_fisica);
   						refresh();
   						break;
   					}
   				}
   				flag_comandoOK2 = 1;
   			}
   			if (flag_comandoOK2 == 0 && strcmp(comando2, "solicitar memoria") == 0)
   			{
   				// TODO
   				flag_comandoOK2 = 1;
   			}
   			if (flag_comandoOK2 == 0 && strcmp(comando2, "escribir memoria") == 0)
   			{
   				// TODO
   				flag_comandoOK2 = 1;
   			}
   			if (flag_comandoOK2 == 0 && strcmp(comando2, "destruir segmentos") == 0)
   			{
   				printw("Ingrese proceso: \n");
   				printw(">");
   				refresh();
   				scanw("%d", &proc_id);
   				pthread_mutex_lock(&semCompactacion);
   				respuesta = destruir_segmentos(proc_id);
   				pthread_mutex_unlock(&semCompactacion);
   				if (respuesta == 1)
   				{
   					log_info(logger, "Segmentos de proceso %d destruidos satisfactoriamente.", proc_id);
   					printw("Los segmentos del programa fueron destruidos satisfactoriamente.\n");
   					refresh();
   				}
   				else
   				{
   					printw("El programa ingresado no es valido.\n");
   					refresh();
   				}
   				flag_comandoOK2 = 1;
   			}
   			flag_comandoOK = 1;
   		}
   		if (flag_comandoOK == 0 && strcmp(comando, "retardo") == 0)
   		{
   			log_info(logger, "Se cambio el valor de retardo por consola.");
   			printw("Nuevo valor de retardo: ");
   			refresh();
   			scanw("%d", &nuevo_valor);
   			printw("Valor anterior de retardo: %d\n", retardo);
   			cambiar_retardo(nuevo_valor);
   			printw("Valor actual de retardo: %d\n", retardo);
   			refresh();
   			flag_comandoOK = 1;
   		}
   		if (flag_comandoOK == 0 && strcmp(comando, "algoritmo") == 0)
   		{
   			log_info(logger, "Se cambio el algoritmo por consola.");
   			printw("Se cambio el algoritmo.\n");
   			printw("Algoritmo anterior: %s\n", algoritmo);
   			cambiar_algoritmo();
   			printw("Algoritmo actual: %s\n", algoritmo);
   			refresh();
   		   	flag_comandoOK = 1;
   		}
   		if (flag_comandoOK == 0 && strcmp(comando, "compactacion") == 0)
   		{
   			log_info(logger, "Se pidio compactar memoria por consola.");
   			compactar_memoria();
   		   	flag_comandoOK = 1;
   		}
   		if (flag_comandoOK == 0 && strcmp(comando, "dump") == 0)
   		{
   			pthread_mutex_lock(&semCompactacion);
   			dump_memoria();
   			pthread_mutex_unlock(&semCompactacion);
   		   	flag_comandoOK = 1;
   		}
   		if (flag_comandoOK == 0 && strcmp(comando, "help") == 0)
   		{
   			printw("Los unicos comandos habilitados son: \n");
   			printw("\toperacion\n");
   			printw("\tretardo\n");
   			printw("\talgoritmo\n");
   			printw("\tcompactacion\n");
   			printw("\tdump\n");
   			refresh();
   			flag_comandoOK = 1;
   		}
   		if(flag_comandoOK == 0)
   		{
   			printw("Por favor verifique la sintaxis de los comandos utilizados.\n");
   			printw("Los unicos comandos habilitados son: \n");
   			printw("\toperacion\n");
   			printw("\tretardo\n");
   			printw("\talgoritmo\n");
   			printw("\tcompactacion\n");
   			printw("\tdump\n");
   			refresh();
   		}
   	}
}

void GetInfoConfFile(void)
{
	t_config* config;

	config = config_create(PATH_CONFIG);
	if (config_has_property(config, "IP")) {
			myip = (char*) malloc(strlen(config_get_string_value(config, "IP")) + 1);
			strcpy(myip, config_get_string_value(config, "IP"));
	}
	strcpy(algoritmo,config_get_string_value(config, "ALGORITMO"));
	port=config_get_string_value(config, "PORT");
	space=config_get_int_value(config, "DISK_SPACE");
	retardo = config_get_int_value(config, "RETARDO");
	return;
}

int atenderConexionNueva(int newfd) {
	t_mensaje m;
	int nbytes;

	recibirMensaje(newfd, &m);

	switch (m.tipo) {
	case KERNEL:
		nbytes = enviarHandshake(newfd);
		if (nbytes < 1)
			log_error(logger, "Envio handshake a Kernel.");
		else
			lanzarHiloKernel(newfd);
		break;
	case CPU:
		nbytes = enviarHandshake(newfd);
		if (nbytes < 1)
			log_error(logger, "Envio handshake a CPU.");
		else
			lanzarHiloCPU(newfd);
		break;
	default:
		return 0;
		break;
	}

	return nbytes;
}

int lanzarHiloKernel(int newfd) {
	pthread_t new_th;
	if(pthread_create(&new_th, NULL, hilokernel, (void*) &newfd) < 0)
				{
					log_error(logger, "No se pudo crear el hilo kernel.");
					return 0;
				}
	return 1;
}

int lanzarHiloCPU(int newfd) {
	pthread_t new_th;
	if(pthread_create(&new_th, NULL, hilocpu, (void*) &newfd) < 0)
				{
					log_error(logger, "No se pudo crear el hilo cpu.");
					return 0;
				}
	return 1;
}

int recibirMensaje(int newfd, t_mensaje* m) {
	int nbytes;
	if ((nbytes = recv(newfd, m, sizeof(t_mensaje), MSG_WAITALL)) <= 0) {
		if (nbytes == 0) {//conexion cerrada
			log_error(logger, "socket %d desconectado.\n", newfd);
		}
		else {
			log_error(logger, "receive");
		}
		close(newfd);
	}
	return nbytes;
}

int enviarMensajeKernel(int tipo, int datosNumericos) {
	t_mensaje m;
	m.datosNumericos = datosNumericos;
	m.id_proceso = UMV;
	m.tipo = tipo;
	return sockets_send(socketKernel, &m, "");
}

int enviarHandshake(int sockfd) {
	t_msg_handshake m;
	m.tipo = UMV;
	return send(sockfd, &m, sizeof(t_msg_handshake), 0);
}

int sockets_listen(int sockfd, int backlog) {
	return listen(sockfd, backlog);
}

int sockets_bind(int sockfd, char *addr, char *port) {
	struct sockaddr_in my_addr;

	my_addr.sin_family = AF_INET;
	my_addr.sin_port = htons(atoi(port));
	my_addr.sin_addr.s_addr = inet_addr(addr);
	memset(&(my_addr.sin_zero), '\0', 8);

	return bind(sockfd, (struct sockaddr *) &my_addr, sizeof(struct sockaddr));
}

int sockets_getSocket(void) {
	int yes = 1;
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);

	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
		close(sockfd);
		return -1;
	}

	return sockfd;
}

int sockets_createServer(char *addr, char *port, int backlog) {
	int sockfd = sockets_getSocket();

	if (sockets_bind(sockfd, addr, port) == -1) {
		close(sockfd);
		return -1;
	}

	if (sockets_listen(sockfd, backlog) == -1) {
		close(sockfd);
		return -1;
	}

	return sockfd;
}

int sockets_accept(int sockfd) {
	struct sockaddr_in their_addr;
	int sin_size = sizeof(struct sockaddr_in);

	return accept(sockfd, (struct sockaddr *) &their_addr,
			(socklen_t *) &sin_size);
}

int sockets_send(int sockfd, t_mensaje *mensaje, char *data) {
	strcpy(mensaje->mensaje, "Hola kernel.");
	return send(sockfd, &mensaje, sizeof(t_mensaje), 0);
}

int atenderPedido(int sockfd) {
	return 0;
}

void cambiar_algoritmo()
{
	pthread_mutex_lock(&semAlgoritmo);
	if (!strcmp(algoritmo, "WF") ||
		!strcmp(algoritmo, "Wf") ||
		!strcmp(algoritmo, "wf") ||
		!strcmp(algoritmo, "Worst-Fit") ||
		!strcmp(algoritmo, "Worst-fit") ||
		!strcmp(algoritmo, "worst-fit"))
	{
		strcpy(algoritmo, "First-Fit");
	}
	else
	{
		strcpy(algoritmo, "Worst-Fit");
	}
	pthread_mutex_unlock(&semAlgoritmo);
}

void cambiar_retardo(int valor)
{
	pthread_mutex_lock(&semRetardo);
	retardo = valor;
	pthread_mutex_unlock(&semRetardo);
}

void compactar_memoria()
{
	pthread_mutex_lock(&semCompactacion);
	uintptr_t primer_direccion = (uintptr_t) memoria + space;
	int primer_programa, i, j;
	int nueva_direccion = 0;
	int arranque = 0;
	t_info_programa *prog;
	t_info_segmento *segm;
	int cant_segmentos = obtener_cant_segmentos();
	int cont;
	if (cant_segmentos != 0)
	{
		// TODO Reflejar cambios en "memoria"
		printw("Compactando...\n");
		refresh();
		for (cont = 0; cont < cant_segmentos; cont++)
		{
			// Busco la menor direccion en mi lista de segmentos utilizados
			for (i = 0; i < list_size(list_programas); i++)
			{
				prog = list_get(list_programas, i);
				for (j = 0; j < list_size(prog->segmentos); j++)
				{
					segm = list_get(prog->segmentos, j);
					if (segm->dirFisica < ((int)primer_direccion) && segm->dirFisica >= arranque)
					{
						primer_direccion = segm->dirFisica;
						primer_programa = prog->pid;
					}
					else
					{
						// TODO
					}
				}
			}
			for (i = 0; i < list_size(list_programas); i++)
			{
				prog = list_get(list_programas, i);
				if (prog->pid == primer_programa)
				{
					break;
				}
			}
			for (j = 0; j < list_size(prog->segmentos); j++)
			{
				segm = list_get(prog->segmentos, j);
				if (segm->dirFisica == ((int)primer_direccion))
				{
					break;
				}
			}
			segm->dirFisica = nueva_direccion;
			nueva_direccion = segm->tamanio + 1;
			primer_direccion = space;
			arranque = nueva_direccion;
		}
	}
	else
	{
		printw("Todavia no hay segmentos a compactar.\n");
		refresh();
	}
	pthread_mutex_unlock(&semCompactacion);
}

int obtener_cant_segmentos()
{
	int cant = 0;
	int i, j;
	t_info_programa *prog;
	t_info_segmento *segm;
	for (i = 0; i < list_size(list_programas); i++)
	{
		prog = list_get(list_programas, i);
		cant += list_size(prog->segmentos);
	}
	return cant;
}

int guardarEnMemoria(t_info_segmento* segm,char* buffer) {
	int lenBuffer = string_length(buffer);
	if(segm->tamanio < lenBuffer) {
		log_error(logger, "Segmentation fault.");
		return 0;
	}
	memcpy(memoria + segm->dirFisica, buffer, lenBuffer);
	return 1;
}

int buscarMemoriaDisponible(int tamanio)
{
	int dir;
	if(algoritmo == "First-Fit")
		dir = getFirstFitMemory(tamanio);
	else if(algoritmo == "Worst-Fit")
		dir = getWorstFitMemory(tamanio);
	else
		dir = 0;

	return dir;
}

int getFirstFitMemory(int memSize)
{
	t_info_segmento* lastSegmentAddress = malloc(sizeof(t_info_segmento));
	uintptr_t currentAddress = (uintptr_t) memoria;

	while((currentAddress + memSize) <= ((uintptr_t) memoria + space)) {

		//TODO: Verificar parametro getFist
		// 		Ahora lo puse en 0
		if(findSegmentIn(currentAddress, memSize, 0, lastSegmentAddress))
			currentAddress = lastSegmentAddress->dirFisica + lastSegmentAddress->tamanio + 1;
		else
			break;
	}
	free(lastSegmentAddress);

	if ((currentAddress + memSize) >= ((uintptr_t) memoria + space))
		return 0;

	return currentAddress;
}

int getWorstFitMemory(int memSize)
{
	return 0;
}

t_info_segmento* crearSegmento(int pid, int dirFisica, int tamanioPedido) {
	t_info_segmento* s = malloc(sizeof(t_info_segmento));
	s->dirFisica = dirFisica;
	s->dirLogica = generarDireccionLogica(pid);
	s->tamanio = tamanioPedido;
	t_info_programa prog;
	if(getProgramBy(pid, &prog)) {
		list_add(prog.segmentos, &prog);
		return s;
	}

	return NULL;
}

int guardarEnSegmento(int pid, int segmId, char* buffer) {
	t_info_programa prog;
	t_info_segmento segm;
	if(getProgramById(pid, &prog)) {
		if(getSegmentById(segmId, &prog, &segm)) {
			if(guardarEnMemoria(&segm, buffer))
				return 1;
		}
	}
	return 0;
}

int getProgramById(int pid, t_info_programa* prog) {
	int _existe_pid(t_info_programa *p) {
		return p->pid == pid;
	}

	if(list_any_satisfy(list_programas, (void *) _existe_pid)) {
		 prog = list_find(list_programas, (void *) _existe_pid);
		 return 1;
	}

	return 0;
}

int getSegmentById(int segmId, t_info_programa* prog, t_info_segmento* segm) {
	int _existe_segm(t_info_segmento *s) {
		return s->id == segmId;
	}

	if(list_any_satisfy(prog->segmentos, (void *) _existe_segm)) {
		 segm = list_find(prog->segmentos, (void *) _existe_segm);
		 return 1;
	}

	return 0;
}

int getSegmentByBase(int address, t_info_segmento* segm)
{
	int i;
	for (i = 0; i < list_size(list_programas); i++)
	{
		t_info_programa* prog = list_get(list_programas, i);
		int _es_su_base(t_info_segmento *segm) {
			return segm->dirFisica == address;
		}

		if(list_any_satisfy(prog->segmentos, (void *) _es_su_base)) { //TODO: hacer funcion que pasandole una dir sepa si es su base
			segm = list_find(prog->segmentos, (void *) _es_su_base);
			return 1;
		}
	}
	return 0;
}

int findSegmentIn(int address, int limit, int getFirst, t_info_segmento* segm)
{
	int i, j, maxBaseAddress = address;

	for (i = 0; i < list_size(list_programas); i++)
	{
		t_info_programa* prog = list_get(list_programas, i);
		int _existe_segmento(t_info_segmento *segm) {
			return segm->dirFisica >= address && segm->dirFisica <= (address + limit);
		}

		t_list* segmentosEnMemoria = list_filter(prog->segmentos, (void *) _existe_segmento);

		for (j = 0; j < list_size(segmentosEnMemoria); j++) {
			t_info_segmento* segmMax = list_get(segmentosEnMemoria, j);
			if (!getFirst && segmMax->dirFisica > maxBaseAddress)
				maxBaseAddress = segmMax->dirFisica;
			else if (segmMax->dirFisica < maxBaseAddress)
				maxBaseAddress = segmMax->dirFisica;
		}
	}

	if(!getSegmentByBase(maxBaseAddress, segm))
		return 0;

	return 1;
}

int generarDireccionLogica(int pid)
{
	return 0;
}

int getProgramBy(int pid, t_info_programa prog)
{
	return 0;
}

int verificar_proc_id(int pid)
{
	int i;
	t_info_programa *prog;
	for (i = 0; i < list_size(list_programas); i++)
	{
		prog = list_get(list_programas, i);
		if (prog->pid == pid)
		{
			return 1;
		}
	}
	return 0;
}

int crear_segmento(int idproc, int tamanio)
{
	int tamanio_lista = list_size(list_programas);
	t_info_programa *prog;
	t_info_segmento *seg;
	int i;
	if (tamanio_lista != 0)
	{
		// Busco al programa en mi lista de programas
		for (i = 0; i < tamanio_lista; i++)
		{
			prog = list_get(list_programas, i);
			if (prog->pid == idproc)
			{
				break;
			}
		}
		seg = (t_info_segmento *)malloc(sizeof(t_info_segmento));
		seg->id = idproc;
		seg->tamanio = tamanio;
		seg->dirFisica = asignar_direccion_en_memoria();
		seg->dirLogica = asignar_direccion_logica();
		list_add(prog->segmentos, seg);
		return seg->dirLogica;
	}
	else
	{
		if (hay_espacio_en_memoria(tamanio))
		{
			prog = (t_info_programa *)malloc(sizeof(t_info_programa));
			prog->pid = idproc;
			prog->segmentos = list_create();
			seg = (t_info_segmento *)malloc(sizeof(t_info_segmento));
			seg->id = idproc;
			seg->tamanio = tamanio;
			seg->dirFisica = asignar_direccion_en_memoria();
			seg->dirLogica = asignar_direccion_logica();
			list_add(prog->segmentos, seg);
			list_add(list_programas, prog);
			return seg->dirLogica;
		}
		else
		{
			return -1;
		}
	}
}

int destruir_segmentos(int idproc)
{
	int tamanio_lista = list_size(list_programas);
	int i;
	int indice;
	int encontre_programa = 0;
	t_info_programa *prog;
	// Busco al programa en mi lista de programas
	for (i = 0; i < tamanio_lista; i++)
	{
		prog = list_get(list_programas, i);
		if (prog->pid == idproc)
		{
			indice = i;
			encontre_programa = 1;
			break;
		}
	}
	if (encontre_programa == 1)
	{
		for (i = list_size(prog->segmentos) - 1; i >= 0; i--)
		{
			list_remove(prog->segmentos, i);
		}
		list_destroy(prog->segmentos);
		list_remove(list_programas, indice);
		return 1;
	}
	else
	{
		return 0;
	}
}

int transformar_direccion_en_fisica(int direccion, int pid)
{

	return 0;
}

int transformar_direccion_en_logica(int direccion, int pid)
{

	return 0;
}

int hay_espacio_en_memoria(int tam)
{
	int mem[space];
	int i;
	int j;
	int k;
	t_info_programa *prog;
	t_info_segmento *seg;
	int espacio_libre = 0;
	for (i = 0; i < space; i++)
	{
		mem[i] = 0;
	}
	for (i = 0; i < list_size(list_programas); i++)
	{
		prog = list_get(list_programas, i);
		for (j = 0; j < list_size(prog->segmentos); j++)
		{
			seg = list_get(prog->segmentos, j);
			for (k = 0; k < seg->tamanio; k++)
			{
				mem[k + seg->dirFisica] = 1;
			}
		}
	}
	for (i = 0; i < space; i++)
	{
		if (mem[i] == 0)
		{
			espacio_libre++;
			if (espacio_libre == tam)
			{
				return 1;
			}
		}
		else
		{
			espacio_libre = 0;
		}
	}
	return 0;
}

int asignar_direccion_logica()
{

	return 0;
}

int asignar_direccion_en_memoria()
{
	if (obtener_cant_segmentos() == 0)
	{
		return 0;
	}
	else
	{
		if (!strcmp(algoritmo, "WF") ||
			!strcmp(algoritmo, "Wf") ||
			!strcmp(algoritmo, "wf") ||
			!strcmp(algoritmo, "Worst-Fit") ||
			!strcmp(algoritmo, "Worst-fit") ||
			!strcmp(algoritmo, "worst-fit"))
		{
			// El algoritmo es worst-fit
		}
		else
		{
			// El algoritmo es first-fit
		}
	}
	return 0;
}

void dump_memoria()
{
	int mem_utilizada = 0;
	clear();
	refresh();
	int x = 0;
	int y = 0;
	move(y, x);
	printw("Id Proceso\n");
	x += 15;
	move(y, x);
	printw("Dir. Fisica Segmento\n");
	x += 25;
	move(y, x);
	printw("Tamanio Segmento\n");
	refresh();
	t_info_programa *prog;
	t_info_segmento *seg;
	int i;
	int j;
	x = 0;
	y = 1;
	for (i = 0; i < list_size(list_programas); i++)
	{
		prog = list_get(list_programas, i);
		for (j = 0; j < list_size(prog->segmentos); j++)
		{
			seg = list_get(prog->segmentos, j);
			move(y, x);
			printw("%d", seg->id);
			x += 15;
			move(y, x);
			printw("%d", seg->dirFisica);
			x += 25;
			move(y, x);
			printw("%d", seg->tamanio);
			y++;
			x = 0;
			refresh();
			mem_utilizada += seg->tamanio;
		}
	}
	y++;
	move(y, x);
	printw("Memoria total: %d\n", space);
	printw("Memoria utilizada: %d\n", mem_utilizada);
	refresh();
}
