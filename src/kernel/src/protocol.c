/*
 * protocol.c
 *
 *  Created on: 26/04/2014
 *  Author: SilverStack
*/

/* Header File */

#include "protocol.h"

/*
 * Function: GetInfoConfFile
 * Purpose: Parse Configuration File
 * Created on: 18/04/2014
 * Author: SilverStack
*/

void GetInfoConfFile(char* PATH_CONFIG)
{
	t_config* config;
	char** HIO;
	char** ID_HIO;
	char** ID_Sem;
	char** Value_Sem;
	char** Globales;
	char** AUX;
	int i = 0, j = 0,retardo_io_value, valor_semaforo;
	t_io* io_node;
	pthread_t th;

	config = config_create(PATH_CONFIG);
	strcpy(myip,config_get_string_value(config, "IP"));
	strcpy(umv_ip,config_get_string_value(config, "UMV_IP"));
	ID_HIO = config_get_array_value(config, "ID_HIO");
	HIO = config_get_array_value(config, "HIO");
	ID_Sem = config_get_array_value(config, "SEMAFOROS");
	Value_Sem = config_get_array_value(config, "VALOR_SEMAFORO");
	multiprogramacion=config_get_int_value(config, "MULTIPROGRAMACION");
	port_cpu=config_get_int_value(config, "PUERTO_CPU");
	port_program=config_get_int_value(config, "PUERTO_PROG");
	port_umv=config_get_int_value(config, "PUERTO_UMV");
	quantum=config_get_int_value(config, "QUANTUM");
	retardo=config_get_int_value(config, "RETARDO");
	Globales = config_get_array_value(config, "GLOBALES");

	AUX = ID_HIO;

	while (*AUX != NULL) /* Cuenta cantidad de I/O */
	{
		AUX++;
		i++;
	}

	AUX = HIO;

	while (*AUX != NULL) /* Cuenta cantidad de valores en I/O */
	{
		AUX++;
		j++;
	}

	if(i != j)
	{
		log_error(logger, "No coinciden la cantidad de identificadores I/O con la cantidad de retardos");
		log_error(logger, "Extraccion incorrecta del archivo de configuracion");
		exit(1);
	}

	while(*ID_HIO != NULL && *HIO != NULL)
	{
		retardo_io_value = atoi(*HIO);
		pthread_create(&th,NULL,(void*)retardo_io,*ID_HIO);
		io_node = io_create(*ID_HIO, retardo_io_value);
		io_node->th_io = &th;
		sem_wait(&free_io_queue);
		list_add(list_io, io_node);
		sem_post(&free_io_queue);
		HIO++;
		ID_HIO++;
	}

	AUX = ID_Sem;

	i = 0;
	j = 0;

	while (*AUX != NULL) /* Cuenta cantidad de semaforos */
	{
		AUX++;
		i++;
	}

	AUX = Value_Sem;

	while (*AUX != NULL) /* Cuenta cantidad de valores de semaforos */
	{
		AUX++;
		j++;
	}

	if(i != j)
	{
		log_error(logger, "No coinciden la cantidad de identificadores semaforos con la cantidad de valores");
		log_error(logger, "Extraccion incorrecta del archivo de configuracion");
		exit(1);
	}

	while(*ID_Sem != NULL && *Value_Sem != NULL)
	{
		valor_semaforo = atoi(*Value_Sem);
		list_add(list_semaphores, semaphore_create(*ID_Sem, valor_semaforo));

		ID_Sem++;
		Value_Sem++;
	}

	while(*Globales != NULL)
	{
		list_add(list_globales, global_create(*Globales));
		Globales++;
	}

	log_info(logger, "Extraccion correcta del archivo de configuracion");
	config_destroy(config);
}

/*
 * Function: global_create
 * Purpose: Creates a global variables
 * Created on: 02/05/2014
 * Author: SilverStack
*/

t_global* global_create(char *global_name)
{
	t_global* new_global;
	if( (new_global = (t_global*) malloc(sizeof(t_global))) == NULL )
	{
	    log_error(logger,"Error al reservar memoria para nuevo nodo en global_create");
	    return new_global;
	}

	if((new_global->identifier = (char*) malloc(sizeof(char) * (strlen(global_name) + 1))) == NULL)
	{
		log_error(logger,"Error al reservar memoria para el identificador de nodo en global_create");
		return new_global;
	}
	strcpy(new_global->identifier,global_name);
	return new_global;
}

/*
 * Function: global_update
 * Purpose: Update a global variable's value
 * Created on: 02/05/2014
 * Author: SilverStack
*/

int global_update_value(char* global_name, int value)
{
	int flag_mod = 0;
	int nuevo_valor = value;
	char* global_id;

	if( (global_id = (char*) malloc(sizeof(char) * (strlen(global_name) + 1))) == NULL )
	{
		log_error(logger,"Error al reservar memoria para nuevo nodo en global_create");
	}

	strcpy(global_id,global_name);

	void _get_global(t_global *s)
	{
		if(strcmp(global_id,s->identifier) == 0)
		{
			s->value = nuevo_valor;
			flag_mod = 1;
		}
	}

	list_iterate(list_globales, (void*) _get_global);

	if(flag_mod == 0)
	{
		log_error(logger, "Variable compartida %s no encontrado", global_id);
		return 0;
	}

	free(global_id);
	return 1;
}

/*
 * Function: global_get_value
 * Purpose: Get a global variable's value
 * Created on: 02/05/2014
 * Author: SilverStack
*/

int global_get_value(char* global_name)
{
	int flag_mod = 0;
	int valor_retorno;
	char* global_id;

	if( (global_id = (char*) malloc(sizeof(char) * (strlen(global_name) + 1))) == NULL )
	{
		log_error(logger,"Error al reservar memoria para nuevo nodo en global_create");
	}

	strcpy(global_id,global_name);

	void _get_global(t_global *s)
	{
		if(strcmp(global_id,s->identifier) == 0)
		{
			valor_retorno = s->value;
			flag_mod = 1;
		}
	}

	list_iterate(list_globales, (void*) _get_global);

	if(flag_mod == 0)
	{
		log_error(logger, "Variable compartida %s no encontrado", global_id);
	}

	free(global_id);
	return valor_retorno;
}

/*
 * Function: io_create
 * Purpose: Creates an io node
 * Created on: 17/04/2014
 * Author: SilverStack
*/

t_io* io_create(char *io_name, int io_retardo)
{
	t_io* new_io;

	if( (new_io = (t_io*) malloc(sizeof(t_io))) == NULL )
	{
		log_error(logger,"Error al reservar memoria para nuevo nodo en io_create");
		return new_io;
	}

	if((new_io->name = (char*) malloc(sizeof(char) * (strlen(io_name) + 1))) == NULL)
	{
		log_error(logger,"Error al reservar memoria para el identificador de nodo en io_create");
		return new_io;
	}

	strcpy(new_io->name,io_name);
	new_io->retardo = io_retardo;
	new_io->io_queue = queue_create();
	sem_init(&(new_io->io_sem), 0, 0); //Empieza en cero porque tiene que bloquearse hasta que un nodo quede bloqueado
	return new_io;
}

/*
 * Function: io_destroy
 * Purpose: Destroy an io node
 * Created on: 18/04/2014
 * Author: SilverStack
*/

void io_destroy(t_io *self)
{
	free(self->name);
	queue_destroy(self->io_queue);
	free(self);
}

/*
 * Function: create_nodo_queue_semaphore
 * Purpose: Creates nodo to store process id of blocked process by the semaphore
 * Created on: 13/05/2014
 * Author: SilverStack
*/

t_nodo_queue_semaphore* create_nodo_queue_semaphore(int process_id)
{
	t_nodo_queue_semaphore* new_nodo_queue_semaphore;

	if( (new_nodo_queue_semaphore = (t_nodo_queue_semaphore*) malloc(sizeof(t_nodo_queue_semaphore))) == NULL )
	{
		log_error(logger,"Error al reservar memoria para nuevo nodo en create_nodo_queue_semaphore");
		return new_nodo_queue_semaphore;
	}

	new_nodo_queue_semaphore->process_id = process_id;

	return new_nodo_queue_semaphore;
}

/*
 * Function: semaphore_create
 * Purpose: Creates an semaphore
 * Created on: 26/04/2014
 * Author: SilverStack
*/

t_semaphore* semaphore_create(char* sem_name, int value)
{
	t_semaphore* new_sem;

	if( (new_sem = (t_semaphore*) malloc(sizeof(t_semaphore))) == NULL )
	{
		log_error(logger,"Error al reservar memoria para nuevo nodo en semaphore_create");
		return new_sem;
	}

	if((new_sem->identifier = (char*) malloc(sizeof(char) * (strlen(sem_name) + 1))) == NULL)
	{
		log_error(logger,"Error al reservar memoria para el identificador de nodo en semaphore_create");
		return new_sem;
	}

 	strcpy(new_sem->identifier,sem_name);
	new_sem->value = value;
	new_sem->queue = queue_create();
	return new_sem;
}

/*
 * Function: semaphore_destroy
 * Purpose: Destroy an semaphore
 * Created on: 26/04/2014
 * Author: SilverStack
*/

void semaphore_destroy(t_semaphore *self)
{
	//queue_clean_and_destroy_elements(self->queue, (void*) persona_destroy);
	free(self->identifier);
	free(self);
}

/*
 * Function: semphore_wait
 * Purpose: wait / down / P
 * Created on: 26/04/2014
 * Author: SilverStack
*/

int semaphore_wait(char* sem_name, int process_id)
{
	int flag_found = 0;
	int flag_blocked = 0;
	char* semaphore_id;
	int pid = process_id;

	if((semaphore_id = (char*) malloc (sizeof(char) * (strlen(sem_name) + 1))) == NULL)
	{
		log_error(logger,"Error al reservar memoria para el identificador de nodo en semaphore_wait");
		return -1;
	}

	strcpy(semaphore_id,sem_name);

	void _get_semaphore(t_semaphore *s)
	{
		if(strcmp(semaphore_id,s->identifier) == 0)
		{
			s->value = s->value - 1;
			if(s->value < 0)
			{
				queue_push(s->queue, create_nodo_queue_semaphore(pid));
				flag_blocked = 1;
			}
			flag_found = 1;
		}
	}

	list_iterate(list_semaphores, (void*) _get_semaphore);

	if(flag_found == 0)
	{
		log_error(logger, "Semaforo %s no encontrado", sem_name);
		return -1;
	}

	free(semaphore_id);

	if(flag_blocked == 1)
		return 1;

	return 0;
}

/*
 * Function: semphore_signal
 * Purpose: signal / up / v
 * Created on: 01/05/2014
 * Author: SilverStack
*/

int semaphore_signal(char* sem_name)
{
	int flag_found = 0;
	char* semaphore_id;
	t_nodo_queue_semaphore* nodo;

	if((semaphore_id = (char*) malloc (sizeof(char) * (strlen(sem_name) + 1))) == NULL)
	{
		log_error(logger,"Error al reservar memoria para el identificador de nodo en semaphore_wait");
		return -1;
	}

	strcpy(semaphore_id,sem_name);

	void _get_semaphore(t_semaphore *s)
	{
		if(strcmp(semaphore_id,s->identifier) == 0)
		{
			s->value = s->value + 1;
			if(s->value >= 0 && queue_size(s->queue) > 0)
			{
				nodo = queue_pop(s->queue);
				pcb_move(nodo->process_id,list_pcb_blocked,list_pcb_ready);
			}

			flag_found = 1;
		}
	}

	list_iterate(list_semaphores, (void*) _get_semaphore);

	if(flag_found == 0)
	{
		log_error(logger, "Semaforo %s no encontrado", sem_name);
		return -1;
	}

	free(semaphore_id);

	return 0;
}

/*
 * Function: servidor_Programa
 * Purpose: Start Listen Socket Program
 * Created on: 11/05/2014
 * Author: SilverStack
*/

int servidor_Programa(void)
{
	int sock_program;
	struct sockaddr_in my_addr;

	if( (sock_program=socket(AF_INET,SOCK_STREAM,0))==-1)
	{
		log_error(logger, "Error en funcion socket en servidor_Programa");
		return -1;
	}

	my_addr.sin_port=htons(port_program);
	my_addr.sin_family=AF_INET;
	my_addr.sin_addr.s_addr=inet_addr(myip);
	memset(&(my_addr.sin_zero),0,8);

	if (bind(sock_program,(struct sockaddr *)&my_addr,sizeof(struct sockaddr))==-1)
	{
		log_error(logger, "Error en funcion bind en servidor_Programa");
		return -1;
	}

	if (listen(sock_program,backlog)==-1)
	{
		log_error(logger, "Error en funcion listen en servidor_Programa");
		return -1;
	}

	return sock_program;
}

/*
 * Function: servidor_CPU
 * Purpose: Start Listen Socket Program
 * Created on: 11/05/2014
 * Author: SilverStack
*/

int servidor_CPU(void)
{
	int sock_cpu;
	struct sockaddr_in my_addr;

	if( (sock_cpu=socket(AF_INET,SOCK_STREAM,0))==-1)
	{
		log_error(logger, "Error en funcion socket en servidor_CPU");
		return -1;
	}

	my_addr.sin_port=htons(port_cpu);
	my_addr.sin_family=AF_INET;
	my_addr.sin_addr.s_addr=inet_addr(myip);
	memset(&(my_addr.sin_zero),0,8);

	if (bind(sock_cpu,(struct sockaddr *)&my_addr,sizeof(struct sockaddr))==-1)
	{
		log_error(logger, "Error en funcion bind en servidor_CPU");
		return -1 ;
	}

	if (listen(sock_cpu,backlog)==-1)
	{
		log_error(logger, "Error en funcion listen en servidor_CPU");
		return -1;
	}

	return sock_cpu;
}

/*
 * Function: escuchar_Nuevo_Programa
 * Purpose: Check for new Program connections
 * Created on: 18/04/2014
 * Author: SilverStack
*/

int escuchar_Nuevo_Programa(int sock_program)
{
	socklen_t sin_size;
	struct sockaddr_in my_addr;
	struct sockaddr_in their_addr;
	t_mensaje mensaje;
	int size_mensaje = sizeof(t_mensaje);
	char * buffer;

	int new_socket;
	int numbytes;

	if((buffer = (char*) malloc (sizeof(char) * MAXDATASIZE)) == NULL)
	{
		log_error(logger,"Error al reservar memoria para el identificador de nodo en semaphore_wait");
		return -1;
	}

	my_addr.sin_port=htons(port_program); /* No creo que sea necesario para el sizeof */
	my_addr.sin_family=AF_INET; /* No creo que sea necesario para el sizeof */
	my_addr.sin_addr.s_addr=inet_addr(myip); /* No creo que sea necesario para el sizeof */
	memset(&(my_addr.sin_zero),0,8); /* No creo que sea necesario para el sizeof */

	sin_size=sizeof(struct sockaddr_in);

	log_info(logger, "Escuchar_Nuevo_Programa");

	if((new_socket=accept(sock_program,(struct sockaddr *)&their_addr,	&sin_size))==-1)
	{
		log_error(logger, "Error en funcion accept en escuchar_Nuevo_Programa");
		return -1;
	}

	memset(buffer,'\0',MAXDATASIZE);

	if((numbytes=read(new_socket,buffer,size_mensaje))<=0)
	{
		log_error(logger, "Error en el read en escuchar_Nuevo_Programa");
		close(new_socket);
		return -1;
	}

	memcpy(&mensaje,buffer,size_mensaje);
	if(mensaje.tipo != HANDSHAKE && mensaje.id_proceso != PROGRAMA)
	{
		log_error(logger, "No se pudo crear nueva conexion. Error en el handshake");
		return -1;
	}

	mensaje.tipo = HANDSHAKE_OK;
	memset(buffer,'\0',MAXDATASIZE);
	memcpy(buffer,&mensaje,size_mensaje);

	if((numbytes=write(new_socket,buffer,size_mensaje))<=0)
	{
		log_error(logger, "Error en el write en escuchar_Nuevo_Programa");
		close(new_socket);
		return -1;
	}

	log_info(logger, "Nueva conexion lograda con programa");
	//return new_socket;
	memset(buffer,'\0',MAXDATASIZE);

	if((numbytes=read(new_socket,buffer,SIZE_MSG))<=0)
	{
		log_error(logger, "Error en el read en escuchar_Nuevo_Programa");
		return -1;
	}

	memcpy(&mensaje,buffer,size_mensaje);

	if(mensaje.tipo != SENDFILE && mensaje.id_proceso != PROGRAMA)
	{
		log_error(logger, "Error en el descriptor. escuchar_Nuevo_Programa");
		return -1;
	}

	/* Para tratamiento del envio de archivos o comandos*/
	memset(buffer,'\0',MAXDATASIZE);
	if(mensaje.datosNumericos > MAXDATASIZE)
	{
		log_error(logger, "Archivo muy grande %d", mensaje.datosNumericos);
	}

	if((numbytes=read(new_socket,buffer,mensaje.datosNumericos))<=0)
	{
		log_error(logger, "Error en el read en escuchar_Nuevo_Programa");
		return -1;
	}

	log_info(logger, "Se recibieron %d bytes desde programa", mensaje.datosNumericos);
	log_info(logger, "File \n%s", buffer);
	//create_pcb(buffer,numbytes, new_socket);

	return new_socket;
}

/*
 * Function: escuchar_Programa
 * Purpose: Listen for program
 * Created on: 01/05/2014
 * Author: SilverStack
*/

int escuchar_Programa(int sock_program, char* buffer)
{
	//thdr hdr;
	t_mensaje mensaje;
	int numbytes;
	int size_mensaje = sizeof(t_mensaje);

	log_info(logger, "Recepcion de datos desde Programa");
	memset(buffer,'\0',MAXDATASIZE);
	if((numbytes=read(sock_program,buffer,size_mensaje))<=0)
	{
		log_error(logger, "Error en el read en escuchar_Programa");
		return -1;
	}

	memcpy(&mensaje,buffer,size_mensaje);

	if(mensaje.tipo==SENDFILE && mensaje.id_proceso ==PROGRAMA)
	{
		/* Para tratamiento del envio de archivos o comandos*/
		memset(buffer,'\0',MAXDATASIZE);

		if(mensaje.datosNumericos > MAXDATASIZE)
		{
			log_error(logger, "Archivo muy grande %d", mensaje.datosNumericos);
		}

		if((numbytes=read(sock_program,buffer,mensaje.datosNumericos))<=0)
		{
			log_error(logger, "Error en el read en escuchar_Programa");
			return -1;
		}

		//log_info(logger, "Se recibieron %d bytes desde programa", hdr.pay_len);
		log_info(logger, "File \n%s", buffer);
		create_pcb(buffer,numbytes, sock_program);

		return 0;
	}
	else
	{
		log_error(logger, "Error en el descriptor. escuchar_Programa");
		return -1;
	}
}

/*
 * Function: create_pcb
 * Purpose: create pcb node
 * Created on: 18/04/2014
 * Author: SilverStack
*/

void create_pcb(char* buffer, int tamanio_buffer, int sock_program)
{
	t_medatada_program* metadata;
	t_pcb* new_pcb = (t_pcb*) malloc(sizeof(t_pcb));

	if((new_pcb = (t_pcb*) malloc (sizeof(t_pcb))) == NULL)
	{
		log_error(logger,"Error al reservar memoria para el pcb de nodo en create_pcb");
		return;
	}

	metadata = metadatada_desde_literal(buffer);

	new_pcb->unique_id = process_Id;
	new_pcb->code_segment = send_umv_code_segment(buffer, ++process_Id);
	new_pcb->stack_segment = receive_umv_stack();
	new_pcb->stack_pointer = new_pcb->stack_segment;
	new_pcb->instruction_index = send_umv_instructions(	metadata->instrucciones_size,
														metadata->instrucciones_serializado);

	new_pcb->etiquetas_index = send_umv_etiquetas(	metadata->etiquetas_size,
													metadata->etiquetas);

	new_pcb->program_counter = metadata->instruccion_inicio;
	new_pcb->context_actual = 0;
	new_pcb->peso = 5 * metadata->cantidad_de_etiquetas +
					3 * metadata->cantidad_de_funciones +
					metadata->instrucciones_size;

	list_add(list_pcb_new, new_pcb);
	list_add(list_process,process_create(new_pcb->unique_id, sock_program));
	sem_post(&sem_plp);
	metadata_destruir(metadata);
}

int send_umv_code_segment(char* buffer, int pid)
{
	t_mensaje mensaje;
	int numbytes;
	char* buffer_msg;

	if((buffer_msg = (char*) malloc (sizeof(char) * ( strlen(buffer) + 1 ) )) == NULL)
	{
		log_error(logger,"Error al reservar memoria para el pcb de nodo en send_umv_code_segment");
		return -1;
	}

	mensaje.id_proceso = KERNEL;
	mensaje.tipo = NEW_PROGRAM_REQUEST;
	mensaje.datosNumericos = pid;

	memset(buffer_msg,'\0',MAXDATASIZE);
	memcpy(buffer_msg,&mensaje,SIZE_MSG);

	log_info(logger,"Envio el PID");
	if((numbytes=write(sock_umv,buffer_msg,SIZE_MSG))<=0)
	{
		log_error(logger, "Error en el write del codigo");
		close(sock_umv);
		free(buffer_msg);
		return -1;
	}

	mensaje.id_proceso = KERNEL;
	mensaje.tipo = TAMANIO_REQUEST;
	mensaje.datosNumericos = strlen(buffer);

	memset(buffer_msg,'\0',MAXDATASIZE);
	memcpy(buffer_msg,&mensaje,SIZE_MSG);

	log_info(logger,"Pregunto si hay espacio disponible");
	if((numbytes=write(sock_umv,buffer_msg,SIZE_MSG))<=0)
	{
		log_error(logger, "Error en el write del codigo");
		close(sock_umv);
		free(buffer_msg);
		return -1;
	}

	memset(buffer_msg,'\0',MAXDATASIZE);

	log_info(logger,"Respuesta acerca del espacio disponible");
	if((numbytes=read(sock_umv,buffer_msg,SIZE_MSG))<=0)
	{
		log_error(logger, "Error en el write del codigo");
		close(sock_umv);
		free(buffer_msg);
		return -1;
	}

	memcpy(&mensaje,buffer_msg,SIZE_MSG);

	if(mensaje.tipo == LOW_MEMORY)
	{
		log_error(logger,"Memoria Insuficiente");
		free(buffer_msg);
		return -1; /* Todo MAL*/
	}

	if(mensaje.tipo != TAMANIOOK)
	{
		log_error(logger,"Error en descriptor");
		free(buffer_msg);
		return -1; /* Todo MAL*/
	}

	log_info(logger,"Envio Codigo ANSISOP");
	if((numbytes=write(sock_umv,buffer,strlen(buffer))<=0))
	{
		log_error(logger, "Error en el write del codigo");
		close(sock_umv);
		free(buffer_msg);
		return -1;
	}

	memset(buffer_msg,'\0',MAXDATASIZE);

	log_info(logger,"Espero respuesta");
	if((numbytes=read(sock_umv,buffer_msg,SIZE_MSG))<=0)
	{
		log_error(logger, "Error en el write del codigo");
		close(sock_umv);
		free(buffer_msg);
		return -1;
	}
	memcpy(&mensaje,buffer_msg,SIZE_MSG);

	if(mensaje.tipo == NEW_PROGRAMOK)
	{
		free(buffer_msg);
		return mensaje.datosNumericos; /* ALL GOOD*/
	}

	free(buffer_msg);
	return -1;
}

/*
 * Function: receive_umv_stack
 * Purpose: Get from UMV Socket the Stack Pointer
 * Created on: 10/05/2014
 * Author: SilverStack
*/

int receive_umv_stack(void)
{
	t_mensaje mensaje;
	char* buffer_msg;
	int numbytes;

	if((buffer_msg = (char*) malloc (sizeof(char) * MAXDATASIZE )) == NULL)
	{
		log_error(logger,"Error al reservar memoria para el pcb de nodo en send_umv_code_segment");
		return -1;
	}

	mensaje.id_proceso = KERNEL;
	mensaje.tipo = STACKREQUEST;

	memset(buffer_msg,'\0',MAXDATASIZE);
	memcpy(buffer_msg,&mensaje,SIZE_MSG);

	log_error(logger, "Envio solicitud para el STACK");

	if((numbytes=write(sock_umv,buffer_msg,SIZE_MSG))<=0)
	{
		log_error(logger, "Error en el write del codigo");
		close(sock_umv);
		free(buffer_msg);
		return -1;
	}

	memset(buffer_msg,'\0',MAXDATASIZE);

	log_error(logger, "Recibo respuesta");
	if((numbytes=read(sock_umv,buffer_msg,SIZE_MSG))<=0)
	{
		log_error(logger, "Error en el write del codigo");
		close(sock_umv);
		free(buffer_msg);
		return -1;
	}

	memcpy(&mensaje,buffer_msg,SIZE_MSG);

	if(mensaje.tipo == LOW_MEMORY)
	{
		log_error(logger,"Memoria Insuficiente");
		free(buffer_msg);
		return -1; /* ALL WRONG*/
	}

	if(mensaje.tipo == STACKOK)
	{
		free(buffer_msg);
		return mensaje.datosNumericos; /* ALL GOOD */
	}

	free(buffer_msg);
	return -1;
}

int send_umv_instructions(int instrucciones_size, t_intructions* instrucciones_serializado)
{
	t_mensaje mensaje;
	char* buffer_msg;
	int numbytes;

	if((buffer_msg = (char*) malloc (sizeof(char) * MAXDATASIZE )) == NULL)
	{
		log_error(logger,"Error al reservar memoria para el pcb de nodo en send_umv_code_segment");
		return -1;
	}

	mensaje.id_proceso = KERNEL;
	mensaje.tipo = INSTRUCTIONREQUEST;
	mensaje.datosNumericos = instrucciones_size;

	memset(buffer_msg,'\0',MAXDATASIZE);
	memcpy(buffer_msg,&mensaje,SIZE_MSG);

	log_error(logger, "Envio tamanio de instrucciones");

	if((numbytes=write(sock_umv,buffer_msg,SIZE_MSG))<=0)
	{
		log_error(logger, "Error en el write del codigo");
		close(sock_umv);
		free(buffer_msg);
		return -1;
	}

	memset(buffer_msg,'\0',MAXDATASIZE);

	log_info(logger, "Recibo respuesta");
	if((numbytes=read(sock_umv,buffer_msg,SIZE_MSG))<=0)
	{
		log_error(logger, "Error en el write del codigo");
		close(sock_umv);
		free(buffer_msg);
		return -1;
	}

	memcpy(&mensaje,buffer_msg,SIZE_MSG);

	if(mensaje.tipo == LOW_MEMORY)
	{
		log_error(logger,"Memoria Insuficiente");
		free(buffer_msg);
		return -1; /* ALL WRONG*/
	}

	if(mensaje.tipo != INSTRUCTIONREQUESTOK)
	{
		log_error(logger,"Todo MAL");
		free(buffer_msg);
		return -1; /* ALL WRONG*/
	}

	memset(buffer_msg,'\0',MAXDATASIZE);
	memcpy(buffer_msg,instrucciones_serializado,instrucciones_size);

	log_error(logger, "Envio indice de instrucciones");

	if((numbytes=write(sock_umv,buffer_msg,instrucciones_size))<=0)
	{
		log_error(logger, "Error en el write del codigo");
		close(sock_umv);
		return -1;
	}

	memset(buffer_msg,'\0',MAXDATASIZE);

	log_info(logger, "Recibo respuesta");
	if((numbytes=read(sock_umv,buffer_msg,SIZE_MSG))<=0)
	{
		log_error(logger, "Error en el write del codigo");
		close(sock_umv);
		return -1;
	}

	memcpy(&mensaje,buffer_msg,SIZE_MSG);

	if(mensaje.tipo == INSTRUCTIONREQUESTOK)
	{
		free(buffer_msg);
		return mensaje.datosNumericos; /* ALL OK*/
	}

	free(buffer_msg);
	return -1;
}

int send_umv_etiquetas(int etiquetas_size, char* etiquetas)
{
	t_mensaje mensaje;
	char* buffer_msg;
	int numbytes;

	if((buffer_msg = (char*) malloc (sizeof(char) * MAXDATASIZE )) == NULL)
	{
		log_error(logger,"Error al reservar memoria para el pcb de nodo en send_umv_code_segment");
		return -1;
	}

	mensaje.id_proceso = KERNEL;
	mensaje.tipo = ETIQUETASREQUEST;
	mensaje.datosNumericos = etiquetas_size;

	memset(buffer_msg,'\0',MAXDATASIZE);
	memcpy(buffer_msg,&mensaje,SIZE_MSG);

	log_info(logger, "Envio tamanio de instrucciones");

	if((numbytes=write(sock_umv,buffer_msg,SIZE_MSG))<=0)
	{
		log_error(logger, "Error en el write del codigo");
		close(sock_umv);
		free(buffer_msg);
		return -1;
	}

	memset(buffer_msg,'\0',MAXDATASIZE);

	log_info(logger, "Recibo respuesta");
	if((numbytes=read(sock_umv,buffer_msg,SIZE_MSG))<=0)
	{
		log_error(logger, "Error en el write del codigo");
		close(sock_umv);
		free(buffer_msg);
		return -1;
	}

	memcpy(&mensaje,buffer_msg,SIZE_MSG);

	if(mensaje.tipo == LOW_MEMORY)
	{
		log_error(logger,"Memoria Insuficiente");
		free(buffer_msg);
		return -1; /* ALL WRONG*/
	}

	if(mensaje.tipo != ETIQUETASREQUESTOK)
	{
		log_error(logger,"Descriptor Incorrecto");
		free(buffer_msg);
		return -1; /* ALL WRONG*/
	}

	memset(buffer_msg,'\0',MAXDATASIZE);
	memcpy(buffer_msg,etiquetas,etiquetas_size);

	log_info(logger, "Envio indice de etiquetas");

	if((numbytes=write(sock_umv,buffer_msg,etiquetas_size))<=0)
	{
		log_error(logger, "Error en el write del codigo");
		close(sock_umv);
		free(buffer_msg);
		return -1;
	}

	memset(buffer_msg,'\0',MAXDATASIZE);

	log_info(logger, "Recibo respuesta");
	if((numbytes=read(sock_umv,buffer_msg,SIZE_MSG))<=0)
	{
		log_error(logger, "Error en el write del codigo");
		close(sock_umv);
		free(buffer_msg);
		return -1;
	}

	memcpy(&mensaje,buffer_msg,SIZE_MSG);

	if(mensaje.tipo == ETIQUETASREQUESTOK)
	{
		free(buffer_msg);
		return mensaje.datosNumericos; /* ALL OK*/
	}

	free(buffer_msg);
	return -1;
}

/*
 * Function: destroy_pcb
 * Purpose: Destroy pcb node
 * Created on: 01/05/2014
 * Author: SilverStack
*/

void destroy_pcb(t_pcb* self)
{
	free(self->instruction_index.index);
	free(self->etiquetas_index.etiquetas);
	free(self);
}

/*
 * Function: sort_pcb_plp
 * Purpose: Destroy pcb node
 * Created on: 01/05/2014
 * Author: SilverStack
*/

void sort_plp()
{
	bool _menor_peso(t_pcb *one, t_pcb *two) {
		return one->peso < two->peso;
	}

	list_sort(list_pcb_new, (void*) _menor_peso);
}

/*
 * Function: segment_create
 * Purpose: Create segment node
 * Created on: 25/04/2014
 * Author: SilverStack
*/

t_nodo_segment* segment_create(int start, int offset)
{
	t_nodo_segment* new = (t_nodo_segment*) malloc( sizeof(t_nodo_segment));
	new->start = start;
	new->offset = offset;
	return new;
}

/*
 * Function: segment_destroy
 * Purpose: Destroy segment node
 * Created on: 25/04/2014
 * Author: SilverStack
*/

void segment_destroy(t_nodo_segment *self)
{
	free(self);
}

/*
 * Function: conectar_umv
 * Purpose: Perform the sock connection with UMV
 * Created on: 02/05/2014
 * Author: SilverStack
*/

int conectar_umv(void)
{
	unsigned char buffer[MAXDATASIZE];
	int numbytes,sockfd;
	struct sockaddr_in their_addr;
	t_mensaje mensaje;

	their_addr.sin_family=AF_INET;
	their_addr.sin_port=htons(port_umv);
	their_addr.sin_addr.s_addr=inet_addr(umv_ip);
	memset(&(their_addr.sin_zero),0,8);

	if((sockfd=socket(AF_INET,SOCK_STREAM,0))==-1)
	{
		log_error(logger, "Error al abrir el socket UMV");
		close(sockfd);
		return 1;
	}

	if(connect(sockfd,(struct sockaddr *)&their_addr,sizeof(struct sockaddr))==-1)
	{
		log_error(logger, "Error en el connect con el socket UMV");
		close(sockfd);
		return 1;
	}

	mensaje.tipo = HANDSHAKE;
	mensaje.id_proceso = KERNEL;
	mensaje.datosNumericos = 0;
	strcpy(mensaje.mensaje,"Hola UMV!");

	memcpy(buffer,&mensaje,SIZE_MSG);

	if((numbytes=write(sockfd,buffer,SIZE_MSG))<=0)
	{
		log_error(logger, "Error en el write en el socket UMV");
		return -1;
	}

	memset(buffer,'\0',MAXDATASIZE);

	if((numbytes=read(sockfd,buffer,SIZE_MSG))<=0)
	{
		log_error(logger, "Error en el read en el socket UMV");
		close(sockfd);
		return -1;
	}

	memcpy(&mensaje,buffer,SIZE_MSG);

	if(mensaje.tipo==HANDSHAKE_OK)
	{
		log_info(logger, "Conexion Lograda con la UMV");
		return sockfd;
	}
	else
	{
		log_error(logger, "No recibi Handshake OK");
	}

	return -1;
}

/*
 * Function: planificador_sjn
 * Purpose: Sort New Queue using SJN. Choose PCB and Moves to Ready Queue
 * Created on: 02/05/2014
 * Author: SilverStack
*/

void planificador_sjn(void)
{
	int cantidad_procesos_sistema;
	t_pcb *element;
	// TODO: Agregar Semaforo de CPU Disponible
	for(;;)
	{
		sem_wait(&sem_plp);
		sem_wait(&sem_cpu_list); // Tiene que haber un CPU conectado minimo
		sort_plp();
		cantidad_procesos_sistema = list_size(list_pcb_ready) +
								list_size(list_pcb_blocked) +
								list_size(list_pcb_execute);

		if(list_size(list_pcb_new) > 0)
		{
			if(cantidad_procesos_sistema <= multiprogramacion)
			{
				element = list_remove(list_pcb_new, 0);
				list_add(list_pcb_ready, element);

				log_info(logger, "PCB -> %d moved from New Queue to Ready Queue", element->unique_id);
				sem_post(&sem_plp);
			}
		}
		sem_post(&sem_cpu_list); // Tiene que haber un CPU conectado minimo
	} // for(;;)
}


/*
 * Function: is_Connected_CPU
 * Purpose: Finds if the CPU is already connected
 * Created on: 09/05/2014
 * Author: SilverStack
*/

int is_Connected_CPU(int socket)
{
	int flag_found = 0;
	int actual_socket = socket;

	void _get_cpu(t_nodo_cpu *cpu)
	{
		if(cpu->socket == actual_socket && flag_found == 0)
		{
			flag_found = 1;
		}
	}

	if(list_size(list_cpu) == 0)
		return 0;

	list_iterate(list_cpu, (void*) _get_cpu);

	if(flag_found == 0)
		return 0;

	return 1;
}

/*
 * Function: escuchar_Nuevo_cpu
 * Purpose: Accept new cpu
 * Created on: 03/05/2014
 * Author: SilverStack
*/

int escuchar_Nuevo_cpu(int sock_cpu,char* buffer)
{
	socklen_t sin_size;
	struct sockaddr_in my_addr;
	struct sockaddr_in their_addr;
	t_mensaje mensaje;
	int size_msg = sizeof(t_mensaje);

	int new_socket;
	int numbytes;

	my_addr.sin_port=htons(port_cpu); /* No creo que sea necesario para el sizeof */
	my_addr.sin_family=AF_INET; /* No creo que sea necesario para el sizeof */
	my_addr.sin_addr.s_addr=inet_addr(myip); /* No creo que sea necesario para el sizeof */
	memset(&(my_addr.sin_zero),0,8); /* No creo que sea necesario para el sizeof */

	sin_size=sizeof(struct sockaddr_in);

	log_info(logger, "Escucho una nueva CPU");

	if((new_socket=accept(sock_cpu,(struct sockaddr *)&their_addr,	&sin_size))==-1)
	{
		log_error(logger, "Error en funcion accept en escuchar_Nuevo_CPU");
		return -1;
	}

	memset(buffer,'\0',MAXDATASIZE);

	if((numbytes=read(new_socket,buffer,size_msg))<=0)
	{
		log_error(logger, "Error en el read en escuchar_Nuevo_CPU");
		close(new_socket);
		return -1;
	}

	memcpy(&mensaje,buffer,size_msg);

	//if(mensaje.tipo==HANDSHAKE && mensaje.id_proceso ==CPU)
	if(mensaje.tipo==HANDSHAKE && mensaje.id_proceso ==CPU)
	{
		/* Es un nuevo cpu que quiere conectarse */

		memset(buffer,'\0',MAXDATASIZE);
		mensaje.tipo=HANDSHAKE_OK;
		memcpy(buffer,&mensaje,size_msg);
		log_info(logger,"Mensaje recibido = %s", mensaje.mensaje);

		if((numbytes=write(new_socket,buffer,size_msg))<=0)
		{
			log_error(logger, "Error en el write en escuchar_Nuevo_CPU");
			close(new_socket);
			return -1;
		}

		log_info(logger, "Nueva conexion lograda con cpu");
		return new_socket;
	}
	else
	{
		log_error(logger, "No se pudo crear nueva conexion. Error en el handshake");
		return -1;
	}

}

/*
 * Function: escuchar_cpu
 * Purpose: Works with cpu
 * Created on: 03/05/2014
 * Author: SilverStack
*/

int escuchar_cpu(int sock_cpu, char* buffer)
{
	t_mensaje mensaje;
	int numbytes;
	int size_mensaje = sizeof(t_mensaje);

	log_info(logger, "Recepcion de datos desde CPU");
	memset(buffer,'\0',MAXDATASIZE);
	if((numbytes=read(sock_cpu,buffer,size_mensaje))<=0)
	{
		log_error(logger, "Error en el read en escuchar_Programa");
		return -1;
	}

	memcpy(&mensaje,buffer,size_mensaje);
	log_info(logger,"Mensaje recibido en escuchar_cpu = %s", mensaje.mensaje);

	switch(mensaje.tipo)
	{
		//case QUANTUMFINISH: finalizo_Quantum(sock_cpu); break;
		/*case IMPRIMIR: imprimir(); break;
		case IMPRIMIRTEXTO: imprimirTexto(); break;
		case ASIGNACION: asignar_valor_VariableCompartida(); break;
		case VARCOMREQUEST: obtener_valor_VariableCompartida(); break;
		case ENTRADASALIDA: io(); break;
		case SIGNALSEM: { semaphore_signal(); isBlocked(); break };
		case WAITSEM: { semaphore_wait(); isBlocked(); break };*/
	};

	/*if(mensaje.tipo==QUANTUMFINISH && mensaje.id_proceso ==CPU)
	{
		cpu_update(sock_cpu);
		process_update(sock_cpu);
		return 0;
	}
	else
	{
		log_error(logger, "Error en el descriptor. escuchar_cpu");
		return -1;
	}
	*/
	return -1;
}

/*
 * Function: cpu_create
 * Purpose: Adds cpu to CPU List
 * Created on: 03/05/2014
 * Author: SilverStack
*/

t_nodo_cpu* cpu_create(int socket)
{
	t_nodo_cpu* new_cpu = (t_nodo_cpu*) malloc(sizeof(t_nodo_cpu));
	new_cpu->socket = socket;
	if(cantidad_cpu <= multiprogramacion)
		new_cpu->status = CPU_AVAILABLE;
	else
		new_cpu->status = CPU_IDLE;
	return new_cpu;
}

/*
 * Function: cpu_remove
 * Purpose: removes a cpu that is not connected any more.
 * Created on: 03/05/2014
 * Author: SilverStack
*/

void cpu_remove(int socket)
{
	int flag_found = 0;
	int index = 0;
	int indice_buscado = 0;
	int socket_cpu = socket;

	void _get_index(t_nodo_cpu *s)
	{
		if(s->socket == socket_cpu)
		{
			indice_buscado = index;
			flag_found = 1;
		}
		index++;
	}

	list_iterate(list_cpu, (void*) _get_index);

	if(flag_found == 0)
		log_error(logger, "CPU Socket %d no encontrado", socket);

	t_nodo_cpu *cpu = list_remove(list_cpu, indice_buscado);
	free(cpu);
}

/*
 * Function: cpu_update
 * Purpose: update cpu's status
 * Created on: 04/05/2014
 * Author: SilverStack
*/

void cpu_update(int socket)
{
	int flag_found = 0;
	int socket_cpu = socket;

	void _change_status(t_nodo_cpu *s)
	{
		if(s->socket == socket_cpu)
		{
			s->status = CPU_AVAILABLE;
			flag_found = 1;
		}
	}

	list_iterate(list_cpu, (void*) _change_status);

	if(flag_found == 0)
	{
		log_error(logger, "CPU Socket %d no encontrado", socket);
		return;
	}

}

/*
 * Function: process_create
 * Purpose: create process node
 * Created on: 04/05/2014
 * Author: SilverStack
*/

t_process* process_create(unsigned int pid, int sock_program)
{
	t_process* new_process;

	if ((new_process = (t_process*) malloc(sizeof(t_process)))== NULL)
	{
	    log_error(logger, "No se pudo pedir memoria para el nuevo proceso");
	    return NULL;
	}

	new_process->status = PROCESS_NEW;
	new_process->pid = pid;
	new_process->program_socket = sock_program;
	return new_process;
}

/*
 * Function: process_update
 * Purpose: update process node, and moves the PCB to correct queue
 * Created on: 14/05/2014
 * Author: SilverStack
*/

void process_update(int pid, unsigned char previous_status, unsigned char next_status)
{
	t_list* from;
	t_list* to;
	unsigned char status;
	int process_id = pid;
	int flag_found = 0;

	switch(previous_status)
	{
		case PROCESS_NEW: from = list_pcb_new; break;
		case PROCESS_READY: from = list_pcb_ready; break;
		case PROCESS_EXECUTE: from = list_pcb_execute; break;
		case PROCESS_BLOCKED: from = list_pcb_blocked; break;
	}

	switch(next_status)
	{
		case PROCESS_NEW: to = list_pcb_new; status = PROCESS_NEW; break;
		case PROCESS_READY: to = list_pcb_ready; status = PROCESS_READY; break;
		case PROCESS_EXECUTE: to = list_pcb_execute; status = PROCESS_EXECUTE; break;
		case PROCESS_BLOCKED: to = list_pcb_blocked; status = PROCESS_BLOCKED; break;
	}

	void _change_status(t_process *s)
	{
		if(s->pid == process_id)
		{
			s->status = status;
			flag_found = 1;
		}
	}

	list_iterate(list_process, (void*) _change_status);

	if(flag_found == 0)
		log_error(logger, "CPU Socket %d no encontrado", socket);

	if(flag_found == 1)
	{
		pcb_move(process_id,from, to);
	}
}

/*
 * Function: pcb_move
 * Purpose: update pcb queue
 * Created on: 04/05/2014
 * Author: SilverStack
*/

void pcb_move(unsigned int pid,t_list* from, t_list* to)
{
	int flag_found = 0;
	int process_id = pid;
	int index = 0;
	int indice_buscado = 0;

	void _get_node(t_pcb *s)
	{
		if(s->unique_id == process_id)
		{
			indice_buscado = index;
			flag_found = 1;
		}
		index++;
	}

	list_iterate(from, (void*) _get_node);

	if(flag_found == 0)
		log_error(logger, "PID %d no encontrado en queue execute", pid);

	t_pcb *pcb = list_remove(from, indice_buscado);
	list_add(to,pcb);
}

/*
 * Function: io_wait
 * Purpose: Moved PCB From Execute to Blocked
 * Created on: 06/05/2014
 * Author: SilverStack
*/

void io_wait(unsigned int pid, char* io_name, int amount)
{
	int flag_found = 0;
	int process_id = pid;
	int retardo = amount;
	char* io_id = (char*) malloc(sizeof(char) * (strlen(io_name) + 1));
	t_io* io_node;

	strcpy(io_id,io_name);

	//pcb_move(process_id,list_pcb_execute, list_pcb_blocked);

	void _get_io_node(t_io *s)
	{
		if(strcmp(s->name, io_id) == 0)
		{
			io_node = s;
			queue_push(s->io_queue, io_queue_create(process_id,s->retardo * retardo));
			log_info(logger,"Se Agrega a la io_queue %s el proceso %d con retardo %d", s->name, process_id, s->retardo * retardo);
			flag_found = 1;
		}
	}

	sem_wait(&free_io_queue); // Bloqueo el mutex de lista IO
	list_iterate(list_io, (void*) _get_io_node);
	sem_post(&(io_node->io_sem)); // Libero el semaforo de la queue IO que corresponde
	sem_post(&free_io_queue); // Libero el mutex de lista IO

	if(flag_found == 0)
		log_error(logger, "No se encontro elemento de IO %s", io_id);


	free(io_id);
}

/*
 * Function: io_queue_create
 * Purpose: Add a Process to IO_Queue
 * Created on: 06/05/2014
 * Author: SilverStack
*/

t_io_queue_nodo* io_queue_create(unsigned int process_id, int retardo)
{
	t_io_queue_nodo *new = malloc( sizeof(t_io_queue_nodo) );
	new->pcb = process_id;
	new->retardo = retardo;
	return new;
}

/*
 * Function: retardo_io
 * Purpose: Thread function. One per IO Device
 * Created on: 06/05/2014
 * Author: SilverStack
*/

void retardo_io(void *ptr)
{
	char* name_io;
	name_io = (char *) ptr;
	t_io* io_node;
	t_io_queue_nodo* io_queue_nodo;

	void _get_io_node(t_io *s)
	{
		if(strcmp(s->name, name_io) == 0)
		{
			//semaphore_local = s->io_sem;
			io_node = s;
		}
	}

	sem_wait(&free_io_queue);
	list_iterate(list_io, (void*) _get_io_node);
	sem_post(&free_io_queue);
	log_info(logger,"Se lanzo hilo de escucha para IO = %s",io_node->name);

	for(;;)
	{
		sem_wait(&(io_node->io_sem));	// Down Semaphore_Local
		sem_wait(&free_io_queue);		// Down Semaphore_Free_Io_Queue
			/* START CRITICAL REGION */
			log_info(logger,"[retardo_io] Start Critical Section IO = %s",name_io);
			io_queue_nodo = queue_pop(io_node->io_queue);
			log_info(logger,"[retardo_io] Finish Critical Section IO = %s",name_io);
			/* END CRITICAL REGION */
		sem_post(&free_io_queue); // Up Semaphore

		log_info(logger,"[retardo_io] Dormir = %d",io_queue_nodo->retardo);
		sleep(io_queue_nodo->retardo / 1000);
		//pcb_move(io_queue_nodo->pcb,list_pcb_blocked, list_pcb_ready);
		log_info(logger,"[retardo_io] Finalizo el retardo = %d",name_io);
		free(io_queue_nodo);
	}
}

/*
 * Function: planificador_rr
 * Purpose: Choose PCB from Ready Queue and Moves to Execute Queue
 * Created on: 06/05/2014
 * Author: SilverStack
*/

void planificador_rr(void)
{
	t_nodo_cpu* cpu;
	int flag_cpu_found = 0;
	t_pcb* pcb;

	void _get_cpu_element(t_nodo_cpu *c)
	{
		if(c->status == CPU_AVAILABLE && flag_cpu_found == 0)
		{
			cpu = c;
			flag_cpu_found = 1;
		}
	}

	for(;;)
	{
		sem_wait(&sem_pcp);
		sem_wait(&sem_cpu_list);

		found_cpus_available();

		flag_cpu_found = 0;

		sem_wait(&mutex_cpu_list);
		list_iterate(list_cpu, (void*) _get_cpu_element);
		sem_post(&mutex_cpu_list);

		if(flag_cpu_found == 1)
		{
			log_info(logger, "Enviar PCB a CPU en socket %d", cpu->socket);
			sem_wait(&sem_ready_queue);
			pcb = list_remove(list_pcb_ready, 0);
			sem_post(&sem_ready_queue);
			list_add(list_pcb_execute, pcb);
			ejecutar_proceso(pcb->unique_id, cpu->socket);
			// process->status = PROCESS_EXECUTE
			// process->current_cpu_socket = cpu->socket
			// process->quantum_available = quantum;
			// cpu->status = CPU_WORKING
		}
	} //for(;;)

	return;
}

/*
 * Function: found_cpus_available
 * Purpose: Seeks if there is a CPU Iddle that can be in use
 * Created on: 06/05/2014
 * Author: SilverStack
*/

void found_cpus_available(void)
{
	int max_value = 0;
	int cantidad_cpu_en_uso;
	int cantidad_cpu_iddle;

	bool _is_IDLE(t_nodo_cpu* cpu)
	{
		return cpu->status != CPU_IDLE;
	}

	void _set_cpu_status(t_nodo_cpu *cpu)
	{
		if(cpu->status == CPU_IDLE && max_value > 0)
		{
			cpu->status = CPU_AVAILABLE;
			max_value--;
		}
	}

	cantidad_cpu_iddle = list_size(list_filter(list_cpu, (void*) _is_IDLE));

	if(cantidad_cpu_iddle > 0)
	{
		cantidad_cpu_en_uso = cantidad_cpu - cantidad_cpu_iddle;
		if(cantidad_cpu_en_uso < multiprogramacion )
		{
			max_value = multiprogramacion - cantidad_cpu_en_uso;
			list_iterate(list_cpu, (void*) _set_cpu_status);
		}
	}

	return;
}

/*
 * Function: depurar
 * Purpose: Close all cpu sockets
 * Created on: 06/05/2014
 * Author: SilverStack
*/

void depurar(int signum)
{
	log_info(logger,"Depuracion");

	void _get_cpu_element(t_nodo_cpu *c)
	{
		close(c->socket);
	}

	void _get_process_element(t_process *p)
	{
		close(p->program_socket);
	}

	if(list_size(list_cpu) > 0)
		list_iterate(list_cpu, (void*) _get_cpu_element);

	if(list_size(list_process) > 0)
		list_iterate(list_process, (void*) _get_process_element);

	return;
}

/*
 * Function: buscar_Mayor
 * Purpose: Returns the Max value between 3 elements
 * Created on: 11/05/2014
 * Author: SilverStack
*/

int buscar_Mayor(int a, int b, int c)
{
	int mayor = 0;

	if(a > b)
		mayor = a;
	else
		mayor = b;

	if(mayor > c)
		return mayor;
	else
		return c;
}

/*
 * Function: escuchar_umv
 * Purpose: Returns the Max value between 3 elements
 * Created on: 11/05/2014
 * Author: SilverStack
*/

void escuchar_umv(void)
{
	// TODO: Desarrollar Funcion
	log_error(logger, "Funcion escuchar_umv() aun no desarrollada");
	return;
}

/*
 * Function: is_Connected_Program
 * Purpose: Finds if the Program is already connected
 * Created on: 11/05/2014
 * Author: SilverStack
*/

int is_Connected_Program(int sock_program)
{
	// TODO: Desarrollar Funcion
	log_error(logger, "Funcion is_Connected_Program() aun no desarrollada");
	return -1;
}

/*
 * Function: process_remove_by_socket
 * Purpose: Remove a Process from list_process
 * Created on: 11/05/2014
 * Author: SilverStack
*/

void process_remove_by_socket(int socket)
{
	// TODO: Desarrollar Funcion
	log_error(logger, "Funcion process_remove() aun no desarrollada");
	return;
}

/*
 * Function: process_remove_by_socket
 * Purpose: Remove a Process from list_process
 * Created on: 11/05/2014
 * Author: SilverStack
*/

void ejecutar_proceso(int unique_id, int cpu_socket)
{
	// TODO: Desarrollar Funcion
	log_error(logger, "Funcion process_remove() aun no desarrollada");
	return;
}

