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
	stack_size=config_get_int_value(config, "STACK");
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
 * 			Returns 0 if fails. Otherwise it returns 1
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
		if(flag_mod == 0 && strcmp(global_id,s->identifier) == 0)
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

void semaphore_wait(int sock_cpu, char* sem_name)
{
	int flag_found = 0;
	int flag_blocked = 0;
	char* semaphore_id;
	t_mensaje mensaje;
	int size_msg = sizeof(t_mensaje);
	int numbytes;

	int pid = get_process_id_by_sock_cpu(sock_cpu);

	log_info(logger,"semaphore_wait(%s)",sem_name);

	if((semaphore_id = (char*) malloc (sizeof(char) * (strlen(sem_name) + 1))) == NULL)
	{
		log_error(logger,"Error al reservar memoria para el identificador de nodo en semaphore_wait");
		return;
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
		return;
	}

	mensaje.id_proceso = KERNEL;
	mensaje.datosNumericos = 0;

	if(flag_blocked == 1)
	{
		mensaje.tipo = BLOCK;
		process_set_status(pid,PROCESS_BLOCKED);
		log_info(logger, "Process %d BLOCK", pid);

	}
	else
		mensaje.tipo = WAITSEM;

	if((numbytes=write(sock_cpu,&mensaje,size_msg))<=0)
	{
		log_error(logger, "Fallo el envio de respuesta de sem_wait al cpu");
		close(sock_cpu);
		cpu_remove(sock_cpu);
		return;
	}

	free(semaphore_id);

	return;
}

/*
 * Function: semphore_signal
 * Purpose: signal / up / v
 * Created on: 01/05/2014
 * Author: SilverStack
*/

void semaphore_signal(int sock_cpu, char* sem_name)
{
	int flag_found = 0;
	char* semaphore_id;
	t_nodo_queue_semaphore* nodo;
	t_mensaje mensaje;
	int size_msg = sizeof(t_mensaje);
	int numbytes;

	log_info(logger,"semaphore_signal(%s)",sem_name);

	if((semaphore_id = (char*) malloc (sizeof(char) * (strlen(sem_name) + 1))) == NULL)
	{
		log_error(logger,"Error al reservar memoria para el identificador de nodo en semaphore_wait");
		return;
	}

	strcpy(semaphore_id,sem_name);

	void _get_semaphore(t_semaphore *s)
	{
		if(strcmp(semaphore_id,s->identifier) == 0)
		{
			s->value = s->value + 1;
			if(s->value >= 0 )
			{
				while(queue_size(s->queue) > 0)
				{
					nodo = queue_pop(s->queue);
					pthread_mutex_lock(&mutex_pedidos);
					queue_push(queue_rr,pedido_create(nodo->process_id,PROCESS_BLOCKED,PROCESS_READY));
					pthread_mutex_unlock(&mutex_pedidos);
				}
			}
			flag_found = 1;
		}
	}

	list_iterate(list_semaphores, (void*) _get_semaphore);

	if(flag_found == 0)
	{
		log_error(logger, "Semaforo %s no encontrado", sem_name);
		return;
	}

	mensaje.id_proceso = KERNEL;
	mensaje.datosNumericos = 0;
	mensaje.tipo = SIGNALSEM;

	if((numbytes=write(sock_cpu,&mensaje,size_msg))<=0)
	{
		log_error(logger, "Fallo el envio de respuesta de sem_wait al cpu");
		close(sock_cpu);
		cpu_remove(sock_cpu);
		return;
	}

	free(semaphore_id);

	return;
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
	char* buffer;

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
		log_error(logger, "-->Error en el read en escuchar_Nuevo_Programa");
		log_error(logger, "-->mensaje.datosNumericos = %d", mensaje.datosNumericos);
		return -1;
	}

	log_info(logger, "Se recibieron %d bytes desde programa", mensaje.datosNumericos);

	pcb_create(buffer,numbytes, new_socket);

	free(buffer);

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

		//log_info(logger, "File \n%s", buffer);
		//pcb_create(buffer,numbytes, sock_program);

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

void pcb_create(char* buffer, int tamanio_buffer, int sock_program)
{
	t_medatada_program* metadata;
	t_pcb* new_pcb;

	if((new_pcb = (t_pcb*) malloc (sizeof(t_pcb))) == NULL)
	{
		log_error(logger,"Error al reservar memoria para el pcb de nodo en create_pcb");
		return;
	}

	metadata = metadatada_desde_literal(buffer);

	new_pcb->unique_id = ++process_Id;
	new_pcb->code_segment = umv_send_segment(process_Id, buffer, tamanio_buffer);
	new_pcb->stack_segment = umv_send_segment(process_Id, "", stack_size);
	new_pcb->stack_pointer = new_pcb->stack_segment;
	new_pcb->instruction_index = umv_send_segment(process_Id, (char*) metadata->instrucciones_serializado, metadata->instrucciones_size * sizeof(int) * 2);
	new_pcb->instruction_size = metadata->instrucciones_size;
	new_pcb->size_etiquetas_index = metadata->etiquetas_size;
	if(metadata->etiquetas_size >0)
	{
		new_pcb->etiquetas_index = umv_send_segment(process_Id, (char*) metadata->etiquetas, metadata->etiquetas_size);
		log_info(logger,"etiquetas_size = %d", metadata->etiquetas_size);
	}
	else
	{
		new_pcb->etiquetas_index = -1;
	}
	new_pcb->program_counter = metadata->instruccion_inicio;
	new_pcb->context_actual = 0;
	new_pcb->peso = 5 * metadata->cantidad_de_etiquetas +
					3 * metadata->cantidad_de_funciones +
					metadata->instrucciones_size;

	sem_wait(&mutex_new_queue);
	list_add(list_pcb_new, new_pcb);
	sem_post(&mutex_new_queue);

	sem_wait(&mutex_process_list);
	list_add(list_process,process_create(new_pcb->unique_id, sock_program));
	sem_post(&mutex_process_list);

	sem_post(&sem_plp);
	metadata_destruir(metadata);

	log_info(logger,"PCB Creado satisfactoriamente");
}

/*
 * Function: umv_create_segment
 * Purpose: Create Segment
 * Created on: 17/05/2014
 * Author: SilverStack
*/

int umv_create_segment(int process_id, int tamanio)
{
	char* buffer_msg;
	int numbytes;
	t_msg_crear_segmento msg_crear_segmento;
	int size_msg_crear_segmento = sizeof(t_msg_crear_segmento);

	msg_crear_segmento.id_programa = process_id;
	msg_crear_segmento.tamanio = tamanio;

	if((buffer_msg = (char*) malloc (sizeof(char) * MAXDATASIZE )) == NULL)
	{
		log_error(logger,"Error al reservar memoria create_segment");
		return -1;
	}

	memset(buffer_msg,'\0',MAXDATASIZE);
	memcpy(buffer_msg,&msg_crear_segmento,size_msg_crear_segmento);

	if((numbytes=write(sock_umv,buffer_msg,size_msg_crear_segmento))<=0)
	{
		log_error(logger, "Error en el write msg_crear_segmento");
		close(sock_umv);
		free(buffer_msg);
		return -1;
	}

	free(buffer_msg);
	return 0;
}

/*
 * Function: umv_change_process
 * Purpose: Send msg to umv to change active process
 * Created on: 17/05/2014
 * Author: SilverStack
*/

int umv_change_process(int process_id)
{
	char* buffer_msg;
	int numbytes;
	t_msg_cambio_proceso_activo msg_cambio_proceso_activo;
	int size_msg_cambio_proceso_activo = sizeof(t_msg_cambio_proceso_activo);

	msg_cambio_proceso_activo.id_programa = process_id;

	if((buffer_msg = (char*) malloc (sizeof(char) * MAXDATASIZE )) == NULL)
	{
		log_error(logger,"Error al reservar memoria change_process");
		return -1;
	}

	memset(buffer_msg,'\0',MAXDATASIZE);
	memcpy(buffer_msg,&msg_cambio_proceso_activo,size_msg_cambio_proceso_activo);

	if((numbytes=write(sock_umv,buffer_msg,size_msg_cambio_proceso_activo))<=0)
	{
		log_error(logger, "Error en el write change_process");
		close(sock_umv);
		free(buffer_msg);
		return -1;
	}

	free(buffer_msg);
	return 0;
}

/*
 * Function: umv_send_bytes
 * Purpose: Send set of bytes to umv
 * Created on: 17/05/2014
 * Author: SilverStack
*/

int umv_send_bytes(int base, int offset, int tamanio)
{
	char* buffer_msg;
	int numbytes;
	t_msg_envio_bytes msg_envio_bytes;
	int size_msg_envio_bytes = sizeof(t_msg_envio_bytes);

	if((buffer_msg = (char*) malloc (sizeof(char) * MAXDATASIZE )) == NULL)
	{
		log_error(logger,"Error al reservar memoria send_bytes");
		return -1;
	}

	msg_envio_bytes.base = base;
	msg_envio_bytes.offset = offset;
	msg_envio_bytes.tamanio = tamanio;

	memset(buffer_msg,'\0',MAXDATASIZE);
	memcpy(buffer_msg,&msg_envio_bytes,size_msg_envio_bytes);

	if((numbytes=write(sock_umv,buffer_msg,size_msg_envio_bytes))<=0)
	{
		log_error(logger, "Error en el write send_bytes");
		close(sock_umv);
		free(buffer_msg);
		return -1;
	}

	free(buffer_msg);
	return 0;
}

/*
 * Function: umv_destroy_segment
 * Purpose: Send to UMV instrucction to destroy all related segments with process
 * Created on: 20/05/2014
 * Author: SilverStack
*/

void umv_destroy_segment(int process_id)
{
	char* buffer_msg;
	t_mensaje mensaje;
	int size_mensaje = sizeof(t_mensaje);
	int numbytes;
	t_msg_destruir_segmentos msg_destruir_segmentos;
	int size_msg_destruir_segmentos = sizeof(t_msg_destruir_segmentos);

	if((buffer_msg = (char*) malloc (sizeof(char) * MAXDATASIZE )) == NULL)
	{
		log_error(logger,"Error al reservar memoria destroy_segment");
		return ;
	}

	mensaje.id_proceso = KERNEL;
	mensaje.tipo = DESTRUIRSEGMENTOS;

	memset(buffer_msg,'\0',MAXDATASIZE);
	memcpy(buffer_msg,&mensaje,size_mensaje);

	if((numbytes=write(sock_umv,buffer_msg,size_mensaje))<=0)
	{
		log_error(logger, "Error en el write destroy_segment");
		close(sock_umv);
		free(buffer_msg);
		return;
	}

	msg_destruir_segmentos.id_programa = process_id;

	memset(buffer_msg,'\0',MAXDATASIZE);
	memcpy(buffer_msg,&msg_destruir_segmentos,size_msg_destruir_segmentos);

	if((numbytes=write(sock_umv,buffer_msg,size_msg_destruir_segmentos))<=0)
	{
		log_error(logger, "Error en el write destroy_segment");
		close(sock_umv);
		free(buffer_msg);
		return;
	}

	if((numbytes=read(sock_umv,buffer_msg,size_mensaje))<=0)
	{
		log_error(logger, "Error en el read destroy_segment");
		close(sock_umv);
		free(buffer_msg);
		return;
	}

	free(buffer_msg);
	return;
}

/*
 * Function: send_umv_data
 * Purpose: Send Segment To UMV
 * Created on: 17/05/2014
 * Author: SilverStack
*/

int umv_send_segment(int pid, char* buffer, int tamanio)
{
	t_mensaje mensaje;
	int numbytes;
	char* buffer_msg;
	int tamanio_code_segment = tamanio;
	int direccion_logica;

	if((buffer_msg = (char*) malloc (sizeof(char) * MAXDATASIZE )) == NULL)
	{
		log_error(logger,"Error al reservar memoria para el pcb de nodo en send_umv_code_segment");
		return -1;
	}

	mensaje.id_proceso = KERNEL;
	mensaje.tipo = CREARSEGMENTO;
	mensaje.datosNumericos = 0;

	memset(buffer_msg,'\0',MAXDATASIZE);
	memcpy(buffer_msg,&mensaje,SIZE_MSG);

	if((numbytes=write(sock_umv,buffer_msg,SIZE_MSG))<=0)
	{
		log_error(logger, "Error en el write del codigo");
		close(sock_umv);
		free(buffer_msg);
		return -1;
	}

	if(umv_create_segment(pid, tamanio_code_segment) != 0)
	{
		log_error(logger, "Error Inesperado create_segmento");
		free(buffer_msg);
		return -1;
	}

	memset(buffer_msg,'\0',MAXDATASIZE);

	if((numbytes=read(sock_umv,buffer_msg,SIZE_MSG))<=0)
	{
		log_error(logger, "Error en el read de sock_umv");
		close(sock_umv);
		free(buffer_msg);
		return -1;
	}

	memcpy(&mensaje,buffer_msg,SIZE_MSG);

	if(mensaje.tipo == LOW_MEMORY)
	{
		log_error(logger,"Memoria Insuficiente");
		free(buffer_msg);
		return -1; /* ALL is Wrong*/
	}

	direccion_logica = mensaje.datosNumericos;

	if(strlen(buffer) == 0)
		return direccion_logica;

	mensaje.id_proceso = KERNEL;
	mensaje.tipo = ENVIOBYTES;
	mensaje.datosNumericos = 0;

	memset(buffer_msg,'\0',MAXDATASIZE);
	memcpy(buffer_msg,&mensaje,SIZE_MSG);

	if((numbytes=write(sock_umv,buffer_msg,SIZE_MSG))<=0)
	{
		log_error(logger, "Error en el write del codigo");
		close(sock_umv);
		free(buffer_msg);
		return -1;
	}

	if(umv_change_process(pid) != 0)
	{
		log_error(logger, "Error Inesperado change_process");
		free(buffer_msg);
		return -1;
	}

	if(umv_send_bytes(direccion_logica, 0, tamanio_code_segment) != 0)
	{
		log_error(logger, "Error Inesperado change_process");
		free(buffer_msg);
		return -1;
	}

	if((numbytes=write(sock_umv,buffer,tamanio_code_segment)<=0))
	{
		log_error(logger, "Error en el write del codigo");
		close(sock_umv);
		free(buffer_msg);
		return -1;
	}

	memset(buffer_msg,'\0',MAXDATASIZE);

	if((numbytes=read(sock_umv,buffer_msg,SIZE_MSG))<=0)
	{
		log_error(logger, "Error en el write del codigo");
		close(sock_umv);
		free(buffer_msg);
		return -1;
	}

	memcpy(&mensaje,buffer_msg,SIZE_MSG);

	if(mensaje.tipo == ENVIOBYTES)
	{
		free(buffer_msg);
		return direccion_logica; /* ALL GOOD*/
	}

	log_info(logger,"ENVIOBYTES Fallo");

	free(buffer_msg);
	return -1;
}

/*
 * Function: send_umv_stack
 * Purpose: Get from UMV Socket the Stack Pointer
 * Created on: 10/05/2014
 * Author: SilverStack
*/

int send_umv_stack(int process_id)
{
	t_mensaje mensaje;
	int numbytes;
	char* buffer_msg;
	int tamanio_stack = 100;

	if((buffer_msg = (char*) malloc (sizeof(char) * MAXDATASIZE)) == NULL)
	{
		log_error(logger,"Error al reservar memoria para el pcb de nodo en send_umv_code_segment");
		return -1;
	}

	log_info(logger,"Envio el t_mensaje");

	mensaje.id_proceso = KERNEL;
	mensaje.tipo = CREARSEGMENTO;
	mensaje.datosNumericos = 0;

	memset(buffer_msg,'\0',MAXDATASIZE);
	memcpy(buffer_msg,&mensaje,SIZE_MSG);

	if((numbytes=write(sock_umv,buffer_msg,SIZE_MSG))<=0)
	{
		log_error(logger, "Error en el write del codigo");
		close(sock_umv);
		free(buffer_msg);
		return -1;
	}

	if(umv_create_segment(process_id, tamanio_stack) != 0)
	{
		log_error(logger, "Error Inesperado create_segmento");
		free(buffer_msg);
		return -1;
	}

	memset(buffer_msg,'\0',MAXDATASIZE);

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

/*
 * Function: destroy_pcb
 * Purpose: Destroy pcb node
 * Created on: 01/05/2014
 * Author: SilverStack
*/

void pcb_destroy(t_pcb* self)
{
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

	sem_wait(&mutex_new_queue);
	list_sort(list_pcb_new, (void*) _menor_peso);
	sem_post(&mutex_new_queue);
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
	char* buffer;
	int numbytes,sockfd;
	struct sockaddr_in their_addr;
	t_mensaje mensaje;
	t_msg_handshake msj_handshake;
	int size_msg_handshake = sizeof(t_msg_handshake);

	if((buffer = (char*) malloc (sizeof(char) * MAXDATASIZE)) == NULL)
	{
		log_error(logger,"Error al reservar memoria para el buffer en conectar_umv");
		return -1;
	}

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

	//mensaje.tipo = HANDSHAKE;
	//mensaje.id_proceso = KERNEL;
	//mensaje.datosNumericos = 0;
	mensaje.tipo = KERNEL;
	memcpy(buffer,&mensaje,SIZE_MSG);

	if((numbytes=write(sockfd,buffer,SIZE_MSG))<=0)
	{
		log_error(logger, "Error en el write en el socket UMV");
		return -1;
	}

	memset(buffer,'\0',MAXDATASIZE);

	if((numbytes=read(sockfd,buffer,size_msg_handshake))<=0)
	{
		log_error(logger, "Error en el read en el socket UMV");
		close(sockfd);
		return -1;
	}

	memcpy(&msj_handshake,buffer,size_msg_handshake);

	if(msj_handshake.tipo  == UMV)
	{
		log_info(logger, "Conexion Lograda con la UMV");
		free(buffer);
		return sockfd;
	}
	else
	{
		log_error(logger, "No recibi Handshake OK");
	}

	free(buffer);
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
	int cantidad_procesos_new = 0;
	int cantidad_procesos_exit = 0;

	t_pcb *element;
	for(;;)
	{
		log_info(logger,"planificador_sjn - sem_wait(&sem_plp)");
		sem_wait(&sem_plp);
		log_info(logger,"planificador_sjn - sem_wait(&sem_cpu_list)");
		sem_wait(&sem_cpu_list); // Tiene que haber un CPU conectado minimo

		log_info(logger,"planificador_sjn - sem_wait(&mutex_new_queue)");
		sem_wait(&mutex_new_queue);
		cantidad_procesos_new = list_size(list_pcb_new);
		sem_post(&mutex_new_queue);

		log_info(logger,"planificador_sjn - sem_wait(&mutex_exit_queue)");
		sem_wait(&mutex_exit_queue);
		cantidad_procesos_exit = list_size(list_pcb_exit);
		sem_post(&mutex_exit_queue);

		log_info(logger,"cantidad_procesos_new = %d", cantidad_procesos_new);
		log_info(logger,"cantidad_procesos_exit = %d", cantidad_procesos_exit);
		log_info(logger,"cantidad_procesos_sistema = %d", cantidad_procesos_sistema);

		if(cantidad_procesos_new > 0)
		{
			if(cantidad_procesos_sistema <= multiprogramacion)
			{
				sort_plp();
				sem_wait(&mutex_new_queue);
				element = list_get(list_pcb_new, 0);
				sem_post(&mutex_new_queue);

				sem_wait(&mutex_ready_queue);
				process_update(element->unique_id,PROCESS_NEW,PROCESS_READY);
				sem_post(&mutex_ready_queue);

				pthread_mutex_lock(&mutex_pedidos);
				queue_push(queue_rr,pedido_create(element->unique_id,PROCESS_READY,PROCESS_EXECUTE));
				pthread_mutex_unlock(&mutex_pedidos);

				log_info(logger, "PCB -> %d moved from New Queue to Ready Queue", element->unique_id);
				sem_post(&sem_pcp);
				cantidad_procesos_sistema++;
			}
			else
			{
				sem_post(&sem_plp); // Vuelvo a incrementar el semaforo porque no se movio el proceso de la cola de NEW
			}
		}

		if(cantidad_procesos_exit > 0)
		{
			sem_wait(&mutex_exit_queue);
			element = list_remove(list_pcb_exit, 0);
			sem_post(&mutex_exit_queue);
			program_exit(element->unique_id);
			cantidad_procesos_sistema--;
			log_info(logger, "Good Bye PCB %d", element->unique_id);
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

int escuchar_Nuevo_cpu(int sock_cpu)
{
	socklen_t sin_size;
	struct sockaddr_in my_addr;
	struct sockaddr_in their_addr;
	t_mensaje mensaje;
	int size_msg = sizeof(t_mensaje);
	char* buffer;

	int new_socket;
	int numbytes;

	if((buffer = (char*) malloc (sizeof(char) * MAXDATASIZE)) == NULL)
	{
		log_error(logger,"Error al reservar memoria para el buffer en escuchar_Nuevo_cpu");
		return -1;
	}

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

	if(mensaje.tipo==HANDSHAKE && mensaje.id_proceso ==CPU)
	{
		memset(buffer,'\0',MAXDATASIZE);
		mensaje.tipo=HANDSHAKE_OK;
		memcpy(buffer,&mensaje,size_msg);

		if((numbytes=write(new_socket,buffer,size_msg))<=0)
		{
			log_error(logger, "Error en el write de HANDSHAKE_OK en escuchar_Nuevo_CPU");
			close(new_socket);
			return -1;
		}

		memset(buffer,'\0',MAXDATASIZE);
		mensaje.datosNumericos = quantum;
		memcpy(buffer,&mensaje,size_msg);

		if((numbytes=write(new_socket,buffer,size_msg))<=0)
		{
			log_error(logger, "Error en el write del quantum en escuchar_Nuevo_CPU");
			close(new_socket);
			return -1;
		}

		log_info(logger, "Nueva conexion lograda con cpu");
		free(buffer);
		return new_socket;
	}
	else
	{
		log_error(logger, "No se pudo crear nueva conexion. Error en el handshake");
		free(buffer);
		return -1;
	}

}

/*
 * Function: escuchar_cpu
 * Purpose: Works with cpu
 * Created on: 03/05/2014
 * Author: SilverStack
*/

int escuchar_cpu(int sock_cpu)
{
	t_mensaje mensaje;
	int numbytes;
	int size_mensaje = sizeof(t_mensaje);
	char* buffer;

	if((buffer = (char*) malloc (sizeof(char) * MAXDATASIZE)) == NULL)
	{
		log_error(logger,"Error al reservar memoria para el buffer en escuchar_cpu");
		return -1;
	}

	memset(buffer,'\0',MAXDATASIZE);
	if((numbytes=read(sock_cpu,buffer,size_mensaje))<=0)
	{
		log_error(logger, "Error en el read en escuchar_cpu");
		return -1;
	}

	memcpy(&mensaje,buffer,size_mensaje);

	log_info(logger,"Recibi en escuchar_cpu mensaje.tipo = %d", mensaje.tipo);
	switch(mensaje.tipo)
	{
		case QUANTUMFINISH: finalizo_Quantum(sock_cpu); break;
		case ASIGNACION: asignar_valor_VariableCompartida(sock_cpu, mensaje.mensaje, mensaje.datosNumericos); break;
		case IMPRIMIR: imprimir(sock_cpu,mensaje.datosNumericos); break;
		case IMPRIMIRTEXTO: imprimirTexto(sock_cpu,mensaje.datosNumericos); break;
		case PROGRAMFINISH: process_finish(sock_cpu); break;
		case SIGNALSEM: semaphore_signal(sock_cpu,mensaje.mensaje); break;
		case WAITSEM: semaphore_wait(sock_cpu,mensaje.mensaje); break;
		/*case VARCOMREQUEST: obtener_valor_VariableCompartida(); break;
		case ENTRADASALIDA: io(); break;
		*/
	};

	free(buffer);
	return 0;
}

/*
 * Function: finalizo_Quantum
 * Purpose: Receives an update PCB from CPU. Update and move to correct queue.
 * Created on: 20/05/2014
 * Author: SilverStack
*/

void finalizo_Quantum(int sock_cpu)
{
	int numbytes;
	t_pcb *pcb;

	log_info(logger, "finalizo_Quantum");

	if((pcb = (t_pcb*) malloc (sizeof(t_pcb))) == NULL)
	{
		log_error(logger,"Error al reservar memoria para el pcb en finalizo_Quantum");
		return;
	}

	if((numbytes=read(sock_cpu,pcb,sizeof(t_pcb)))<=0)
	{
		log_error(logger, "Error en el read en finalizo_Quantum");
		return;
	};

	unsigned char status_Actual = process_get_status(pcb->unique_id);
	pcb_update(pcb,PROCESS_EXECUTE);

	pthread_mutex_lock(&mutex_pedidos);
	queue_push(queue_rr,pedido_create(pcb->unique_id,PROCESS_EXECUTE,status_Actual));
	pthread_mutex_unlock(&mutex_pedidos);

	//pcb_destroy(pcb); // NO se libera este puntero porque se agrego a la lista
	sem_post(&sem_pcp);

}

/*
 * Function: process_finish
 * Purpose: Receives an updated PCB from CPU. Finish Program Execution.
 * Created on: 10/06/2014
 * Author: SilverStack
*/

void process_finish(int sock_cpu)
{
	int numbytes;
	t_pcb* pcb;

	log_info(logger, "Begin process_finish");

	if((pcb = (t_pcb*) malloc (sizeof(t_pcb))) == NULL)
	{
		log_error(logger,"Error al reservar memoria para el pcb en process_finish");
		return;
	}

	if((numbytes=read(sock_cpu,pcb,sizeof(t_pcb)))<=0)
	{
		log_error(logger, "Error en el read en process_finish");
		return;
	}

	log_info(logger, "Antes del pcb_update");
	pcb_update(pcb,PROCESS_EXECUTE);
	log_info(logger, "Despues del pcb_update");

	pthread_mutex_lock(&mutex_pedidos);
	queue_push(queue_rr,pedido_create(pcb->unique_id,PROCESS_EXECUTE,PROCESS_EXIT));
	pthread_mutex_unlock(&mutex_pedidos);

	log_info(logger, "Finish process_finish");
	//pcb_destroy(pcb); NO se libera este puntero porque se agrego a la lista
	sem_post(&sem_pcp);

}

/*
 * Function: asignar_valor_VariableCompartida
 * Purpose: Receives a global variable and updaes its value.
 * 			In case of error, returns fails.
 * Created on: 20/05/2014
 * Author: SilverStack
*/

void asignar_valor_VariableCompartida(int sock_cpu, char* global_name, int value)
{
	t_mensaje mensaje;
	int numbytes;
	int size_mensaje = sizeof(t_mensaje);
	char* buffer;

	log_info(logger, "asignar_valor_VariableCompartida");
	if((buffer = (char*) malloc (sizeof(char) * MAXDATASIZE)) == NULL)
	{
		log_error(logger,"Error al reservar memoria para el buffer en asignar_valor_VariableCompartida");
		return;
	}

	mensaje.id_proceso = KERNEL;
	mensaje.datosNumericos = value;
	mensaje.tipo = ASIGNACION;

	if(global_update_value(global_name,value) == 0)
	{
		mensaje.tipo = ERROR;
	}

	memset(buffer,'\0',MAXDATASIZE);
	memcpy(buffer,&mensaje,size_mensaje);

	if((numbytes=write(sock_cpu,buffer,size_mensaje))<=0)
	{
		log_error(logger, "Error en el write en asignar_valor_VariableCompartida");
		close(sock_cpu);
		free(buffer);
		return;
	}

	//TODO: No tengo que abortar. Tengo que abortar en QUANTUM_FINISH

	if(mensaje.tipo == ERROR)
	{
		int pid = get_process_id_by_sock_cpu(sock_cpu);
		if(pid != -1)
		{
			pthread_mutex_lock(&mutex_pedidos);
			queue_push(queue_rr,pedido_create(pid,PROCESS_EXECUTE,PROCESS_EXIT));
			pthread_mutex_unlock(&mutex_pedidos);
			log_error(logger, "Variable Compartida no encontrada. Proceso %d exit", pid);
		}
	}

	free(buffer);
}

void imprimir(int sock_cpu,int valor)
{
	t_mensaje mensaje;
	int numbytes;
	int sock_prog;
	int size_msg = sizeof(t_mensaje);

	mensaje.tipo = IMPRIMIR;
	mensaje.datosNumericos = valor;
	//obtengo el sock program

	log_info(logger,"Envie mensaje imprimir a Programa");

	if((numbytes=write(sock_cpu,&mensaje,size_msg))<=0)
	{
		log_error(logger, "Verificar casos de error");
		// TODO: Verificar casos de error
		close(sock_cpu);
		cpu_remove(sock_cpu);
		return;
	}

	sock_prog = get_sock_prog_by_sock_cpu(sock_cpu);

	if((numbytes=write(sock_prog,&mensaje,size_msg))<=0)
	{
		log_error(logger, "Verificar casos de error");
		// TODO: Verificar casos de error
		close(sock_prog);
		return ;
	}
}

void imprimirTexto(int sock_cpu,int valor)
{
	t_mensaje mensaje;
	int numbytes;
	int sock_prog;
	char* buffer;
	int size_msg = sizeof(t_mensaje);

	if((buffer = (char*) malloc (sizeof(char) * MAXDATASIZE)) == NULL)
	{
		log_error(logger,"Error al reservar memoria para el buffer en imprimir texto");
		return;
	}

	if((numbytes=read(sock_cpu,buffer,valor))<=0)
	{
		log_error(logger, "Error en el read en imprimirTexto");
		return;
	}

	sock_prog = get_sock_prog_by_sock_cpu(sock_cpu);

	mensaje.id_proceso = KERNEL;
	mensaje.tipo = IMPRIMIRTEXTO;
	mensaje.datosNumericos = valor;

	if((numbytes=send(sock_prog,&mensaje,size_msg,0))<=0)
	{
		log_error(logger, "Error enviando tamanio al programa");
		close(sock_prog);
		return;
	}

	if((numbytes=send(sock_prog,buffer,mensaje.datosNumericos,0))<=0)
	{
		log_error(logger, "Error enviando el texto al programa");
		close(sock_prog);
		return;
	}

	mensaje.id_proceso = KERNEL;
	mensaje.tipo = IMPRIMIRTEXTO;
	mensaje.datosNumericos = 0;

	if((numbytes = write(sock_cpu, &mensaje, size_msg)) <= 0)
	{
		log_error(logger, "Error enviando confirmacion a cpu.");
		close(sock_cpu);
		return;
	}
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

	sem_wait(&mutex_cpu_list);
	list_iterate(list_cpu, (void*) _change_status);
	sem_post(&mutex_cpu_list);

	if(flag_found == 0)
	{
		log_error(logger, "CPU Socket %d no encontrado", socket);
		return;
	}

}

/*
 * Function: cpu_set_status
 * Purpose: update cpu's status.
 * Pre-Requisites: Must be called within semaphores
 * Created on: 11/06/2014
 * Author: SilverStack
*/

void cpu_set_status(int socket, unsigned char status)
{
	int flag_found = 0;
	int socket_cpu = socket;
	unsigned char nuevo_status = status;

	void _change_status(t_nodo_cpu *s)
	{
		if(s->socket == socket_cpu)
		{
			s->status = nuevo_status;
			flag_found = 1;
		}
	}

	list_iterate(list_cpu, (void*) _change_status);

	if(flag_found == 0)
	{
		log_error(logger, "CPU Socket %d no encontrado", socket);
		return;
	}

	return;
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
	new_process->current_cpu_socket = -1;
	return new_process;
}

/*
 * Function: process_update
 * Purpose: update process node, and moves the PCB to correct queue
 * Created on: 14/05/2014
 * Author: SilverStack
*/

void process_update(int process_id, unsigned char previous_status, unsigned char next_status)
{
	t_list* from;
	t_list* to;
	sem_t* mutex_list_previous;
	sem_t* mutex_list_next;
	int flag_found = 0;
	int i;
	t_process* process;
	t_pcb* pcb;

	log_info(logger,"Inicia process_update");
	switch(previous_status)
	{
		case PROCESS_NEW: from = list_pcb_new; mutex_list_previous = &mutex_new_queue; break;
		case PROCESS_READY: from = list_pcb_ready; mutex_list_previous = &mutex_ready_queue; break;
		case PROCESS_EXECUTE: from = list_pcb_execute; mutex_list_previous = &mutex_execute_queue; break;
		case PROCESS_BLOCKED: from = list_pcb_blocked; mutex_list_previous = &mutex_block_queue; break;
		case PROCESS_EXIT: from = list_pcb_exit; mutex_list_previous = &mutex_exit_queue; break;
	}

	switch(next_status)
	{
		case PROCESS_NEW: to = list_pcb_new; next_status = PROCESS_NEW; mutex_list_next = &mutex_block_queue; break;
		case PROCESS_READY: to = list_pcb_ready; next_status = PROCESS_READY; mutex_list_next = &mutex_block_queue; break;
		case PROCESS_EXECUTE: to = list_pcb_execute; next_status = PROCESS_EXECUTE; mutex_list_next = &mutex_block_queue; break;
		case PROCESS_BLOCKED: to = list_pcb_blocked; next_status = PROCESS_BLOCKED; mutex_list_next = &mutex_block_queue; break;
		case PROCESS_EXIT: to = list_pcb_exit; next_status = PROCESS_EXIT; mutex_list_next = &mutex_exit_queue; break;
	}

	sem_wait(&mutex_process_list);
	for(i=0;i < list_size(list_process);i++)
	{
		process = list_get(list_process,i);

		if(process->pid == process_id)
		{
			process->status = next_status;
			flag_found = 1;
			break;
		}
	}
	sem_post(&mutex_process_list);

	if(flag_found == 0)
		log_error(logger, "Process ID %d no encontrado en lista de procesos", process_id);

	flag_found = 0;

	sem_wait(mutex_list_previous);
	for(i=0;i < list_size(from);i++)
	{
		pcb = list_get(from,i);
		if(pcb->unique_id == process_id)
		{
			flag_found = 1;
			break;
		}
	}
	sem_post(mutex_list_previous);

	if(flag_found == 1)
	{
		sem_wait(mutex_list_next);
		list_add(to,pcb);
		sem_post(mutex_list_next);

		sem_wait(mutex_list_previous);
		list_remove(from,i);
		sem_post(mutex_list_previous);

		log_info(logger,"Termino Process Update");

		return;
	}

	log_error(logger,"Falló Process Update");
	return;
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
		log_error(logger, "PID %d no encontrado en pcb_move", pid);

	// TODO: Ya no se que probar
	//t_pcb *pcb = list_remove(from, indice_buscado);
	//list_add(to,pcb);

	t_pcb* pcb;
	t_pcb* old_pcb;

	if((pcb = (t_pcb*) malloc (sizeof(t_pcb))) == NULL)
	{
		log_error(logger,"Error al reservar memoria para el pcb en process_update");
		return;
	}

	old_pcb = list_get(from, indice_buscado);

	log_info(logger,"old_pcb = %d", old_pcb->unique_id);

	pcb->code_segment = old_pcb->code_segment;
	pcb->context_actual = old_pcb->context_actual;
	pcb->etiquetas_index = old_pcb->etiquetas_index;
	pcb->instruction_index = old_pcb->instruction_index;
	pcb->instruction_size = old_pcb->instruction_size;
	pcb->peso = old_pcb->peso;
	pcb->program_counter = old_pcb->program_counter;
	pcb->size_etiquetas_index = old_pcb->size_etiquetas_index;
	pcb->stack_pointer = old_pcb->stack_pointer;
	pcb->stack_segment = old_pcb->stack_segment;
	pcb->unique_id = old_pcb->unique_id;

	list_add(to,pcb);
	pcb_destroy(list_remove(from, indice_buscado));
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

	void _get_io_node(t_io *s)
	{
		if(strcmp(s->name, io_id) == 0)
		{
			io_node = s;
			queue_push(s->io_queue, io_queue_create(process_id,s->retardo * retardo));
			pthread_mutex_lock(&mutex_pedidos);
			queue_push(queue_rr,pedido_create(pid,PROCESS_EXECUTE,PROCESS_BLOCKED));
			pthread_mutex_unlock(&mutex_pedidos);
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
	struct timeval tv;

	void _get_io_node(t_io *s)
	{
		if(strcmp(s->name, name_io) == 0)
		{
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
		tv.tv_sec = 0;
		tv.tv_usec = io_queue_nodo->retardo * 1000;
		if (select(0, NULL, NULL, NULL, &tv) == -1)
		{
			log_error(logger, "Error en funcion select en retardo_io");;
			return;
		}
		pthread_mutex_lock(&mutex_pedidos);
		queue_push(queue_rr,pedido_create(io_queue_nodo->pcb,PROCESS_BLOCKED,PROCESS_READY));
		pthread_mutex_unlock(&mutex_pedidos);
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
	t_pedido* new_pedido;
	int cpu_socket;

	//	 TODO: Verificar queue_RR mandar siempre el mismo pcb al mismo CPU

	void _get_cpu_element(t_nodo_cpu *c)
	{
		if(flag_cpu_found == 0 && c->status == CPU_AVAILABLE)
		{
			cpu = c;
			flag_cpu_found = 1;
		}
	}

	for(;;)
	{
		log_info(logger,"planificador_rr - sem_wait(&sem_pcp)");
		sem_wait(&sem_pcp);

		pthread_mutex_lock(&mutex_pedidos);
		new_pedido = queue_pop(queue_rr);
		pthread_mutex_unlock(&mutex_pedidos);

		log_info(logger,"planificador_rr - new_pedido");

		switch(new_pedido->previous_status)
		{
			case PROCESS_READY:
			{
				switch(new_pedido->new_status)
				{
					case PROCESS_EXECUTE:
					{
						log_info(logger,"PID = %d - PROCESS_READY -> PROCESS_EXECUTE", new_pedido->process_id);
						sem_wait(&mutex_cpu_list);
						found_cpus_available();
						sem_post(&mutex_cpu_list);

						flag_cpu_found = 0;

						sem_wait(&mutex_cpu_list);
						list_iterate(list_cpu, (void*) _get_cpu_element);

						if(flag_cpu_found == 1) // Encontre un CPU AVAILABLE
						{
							process_update(new_pedido->process_id,PROCESS_READY, PROCESS_EXECUTE); //Mueve el pcb
							process_execute(new_pedido->process_id, cpu->socket);
							cpu_set_status(cpu->socket, CPU_WORKING); // Pone el CPU Working
							//
						}
						else
						{
							sem_post(&sem_pcp); // Incremento el semaforo porque no saqu2e el proceso
							log_error(logger, "No encontre CPU AVAILABLE");
							pthread_mutex_lock(&mutex_pedidos);
							queue_push(queue_rr,pedido_create(new_pedido->process_id,new_pedido->previous_status,new_pedido->new_status));
							pthread_mutex_unlock(&mutex_pedidos);
						}

						sem_post(&mutex_cpu_list);
						break;
					}
					default:
					{
						log_error(logger, "No se reconoce el new_pedido->next_status");
						break;
					}
				}
				break;
			}
			case PROCESS_EXECUTE:
			{
				switch(new_pedido->new_status)
				{
					case PROCESS_BLOCKED:
					{
						log_info(logger,"PID = %d - PROCESS_EXECUTE -> PROCESS_BLOCKED", new_pedido->process_id);
						process_update(new_pedido->process_id,PROCESS_EXECUTE,PROCESS_BLOCKED);
						break;
					}
					case PROCESS_READY:
					{
						log_info(logger,"PID = %d - PROCESS_EXECUTE -> PROCESS_READY", new_pedido->process_id);
						process_update(new_pedido->process_id,PROCESS_EXECUTE,PROCESS_READY);
						cpu_socket = get_sock_cpu_by_process_id(new_pedido->process_id);
						if(cpu_socket == -1)
						{
							log_error(logger, "No se encontro el cpu_socket");
							break;
						}

						sem_wait(&mutex_cpu_list);
						cpu_set_status(cpu_socket, CPU_AVAILABLE);
						sem_post(&mutex_cpu_list);

						pthread_mutex_lock(&mutex_pedidos);
						queue_push(queue_rr,pedido_create(new_pedido->process_id,PROCESS_READY,PROCESS_EXECUTE));
						pthread_mutex_unlock(&mutex_pedidos);

						sem_post(&sem_pcp);
						break;
					}
					case PROCESS_EXIT:

					{
						log_info(logger,"PID = %d - PROCESS_EXECUTE -> PROCESS_EXIT", new_pedido->process_id);
						process_update(new_pedido->process_id,PROCESS_EXECUTE,PROCESS_EXIT);
						cpu_socket = get_sock_cpu_by_process_id(new_pedido->process_id);
						if(cpu_socket == -1)
						{
							log_error(logger, "No se encontro el cpu_socket");
							break;
						}
						sem_wait(&mutex_cpu_list);
						cpu_set_status(cpu_socket, CPU_AVAILABLE);
						sem_post(&mutex_cpu_list);

						umv_destroy_segment(new_pedido->process_id); //Envio a la UMV el dato para que destruya segmentos
						sem_post(&sem_plp);
						break;
					}
					default:
					{
						log_error(logger, "No se reconoce el new_pedido->next_status");
						break;
					}
				}
				break;
			}
			case PROCESS_BLOCKED:
			{
				switch(new_pedido->new_status)
				{
					case PROCESS_READY:
					{
						log_info(logger,"PID = %d - PROCESS_BLOCKED -> PROCESS_READY", new_pedido->process_id);
						process_update(new_pedido->process_id,PROCESS_BLOCKED,PROCESS_READY);

						pthread_mutex_lock(&mutex_pedidos);
						queue_push(queue_rr,pedido_create(new_pedido->process_id,PROCESS_READY,PROCESS_EXECUTE));
						pthread_mutex_unlock(&mutex_pedidos);

						sem_post(&sem_pcp);
						break;
					}
					default:
					{
						log_error(logger, "No se reconoce el new_pedido->next_status");
						break;
					}
				}
				break;
			}
			default:
			{
				log_error(logger, "No se reconoce el new_pedido->previous_status");
				break;
			}
		} // switch(new_pedido->previous_status)
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

	void _get_cpu_element(t_nodo_cpu *c)
	{
		close(c->socket);
		log_info(logger,"Depuracion - Close CPU Socket %d", c->socket);
	}

	void _get_process_element(t_process *p)
	{
		close(p->program_socket);
		log_info(logger,"Depuracion - Close Process Socket %d", p->program_socket);
	}

	if(list_size(list_cpu) > 0)
		list_iterate(list_cpu, (void*) _get_cpu_element);

	if(list_size(list_process) > 0)
		list_iterate(list_process, (void*) _get_process_element);

	exit_status = 0;

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
	int i;
	t_process *p;

	for(i=0;i < list_size(list_process);i++)
	{
		p = list_get(list_process,i);
		if(p->program_socket == sock_program)
		{
			return 0;
		}
	}

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
	int sock_prog = socket;

	bool _is_process(t_process *p) {
		return p->program_socket == sock_prog;
	}

	sem_wait(&mutex_process_list);
	t_process *aux = list_remove_by_condition(list_process, (void*) _is_process);
	sem_post(&mutex_process_list);

	log_info(logger, "Se removio el programa %d de la lista de procesos", aux->pid);

	process_destroy(aux);

	return;
}

/*
 * Function: process_execute
 * Purpose: Update CPU Socket on Proces_List
 * Created on: 11/05/2014
 * Author: SilverStack
*/

void process_execute(int unique_id, int socket)
{
	int flag_process_found = 0;
	int flag_pcb_found = 0;
	int process_id = unique_id;
	int cpu_socket = socket;
	int numbytes;
	int index = 0;
	int indice_buscado = 0;
	t_pcb* pcb;



	void _get_process_element(t_process *p)
	{
		if(flag_process_found == 0 && p->pid == process_id )
		{
			p->current_cpu_socket = cpu_socket;
			flag_process_found = 1;
		}
	}

	void _get_pcb_element(t_pcb *s)
	{
		if(s->unique_id == process_id)
		{
			indice_buscado = index;
			flag_pcb_found = 1;
		}
		index++;
	}

	sem_wait(&mutex_execute_queue);
	list_iterate(list_pcb_execute, (void*) _get_pcb_element);
	sem_post(&mutex_execute_queue);

	sem_wait(&mutex_process_list);
	list_iterate(list_process, (void*) _get_process_element); // CPU_SOCKET
	sem_post(&mutex_process_list);

	if(flag_process_found == 0 || flag_pcb_found == 0)
	{
		log_error(logger, "PID %d no encontrado en process_execute", process_id);
		return;
	}

	sem_wait(&mutex_execute_queue);
	pcb = list_get(list_pcb_execute, indice_buscado);
	sem_post(&mutex_execute_queue);

	if((numbytes=write(cpu_socket,pcb,sizeof(t_pcb)))<=0)
	{
		log_error(logger, "Error en el write en process_execute");

		close(cpu_socket);
		cpu_remove(cpu_socket);
		pthread_mutex_lock(&mutex_pedidos);
		queue_push(queue_rr,pedido_create(process_id,PROCESS_EXECUTE,PROCESS_EXIT));
		pthread_mutex_unlock(&mutex_pedidos);
		return;
	}
	log_info(logger,"Se envia PCB = %d a CPU= %d", unique_id, cpu_socket);

	return;
}

/*
 * Function: process_destroy
 * Purpose: Free memory of current process node
 * Created on: 20/05/2014
 * Author: SilverStack
*/

void process_destroy(t_process *p)
{
	free(p);
	return;
}

/*
 * Function: pcb_update
 * Purpose: Updates PCB Information
 * Created on: 20/05/2014
 * Author: SilverStack
*/

void pcb_update(t_pcb* new_pcb, unsigned char previous_status)
{

	int flag_found = 0;
	int process_id = new_pcb->unique_id;
	t_list* pcb_from;
	sem_t* mutex_list;
	int i;
	t_pcb* pcb;

	log_info(logger,"Comienza el PCB Update");

	switch(previous_status)
	{
		case PROCESS_NEW: pcb_from = list_pcb_new; mutex_list = &mutex_new_queue; break;
		case PROCESS_READY: pcb_from = list_pcb_ready; mutex_list = &mutex_ready_queue; break;
		case PROCESS_EXECUTE: pcb_from = list_pcb_execute; mutex_list = &mutex_execute_queue; break;
		case PROCESS_BLOCKED: pcb_from = list_pcb_blocked; mutex_list = &mutex_block_queue; break;
		case PROCESS_EXIT: pcb_from = list_pcb_exit; mutex_list = &mutex_exit_queue; break;
	}

	sem_wait(mutex_list);
	for(i=0;i<list_size(pcb_from);i++)
	{
		pcb = list_get(pcb_from,i);
		if(pcb->unique_id == process_id)
		{
			flag_found = 1;
			break;
		}
	}

	if(flag_found == 0)
			log_error(logger, "PID %d no encontrado en pcb_update", process_id);

	list_remove(pcb_from, i);
	list_add(pcb_from,new_pcb);
	sem_post(mutex_list);
	log_info(logger,"Termino el PCB Update");

	return;
}

/*
 * Function: get_sock_cpu_by_process_id
 * Purpose: Returns the cpu_socket
 * Created on: 24/05/2014
 * Author: SilverStack
*/

int get_sock_cpu_by_process_id(int pid)
{
	int flag_process_found = 0;
	int process_id = pid;
	int cpu_socket = -1;

	void _get_process_element(t_process *p)
	{
		log_info(logger,"p->current_cpu_socket = %d", p->current_cpu_socket);
		if(flag_process_found == 0 && p->pid == process_id )
		{
			flag_process_found = 1;
			cpu_socket = p->current_cpu_socket;
		}
	}

	sem_wait(&mutex_process_list);
	list_iterate(list_process, (void*) _get_process_element);
	sem_post(&mutex_process_list);

	if(flag_process_found == 0)
	{
		log_error(logger, "PID %d no encontrado en get_sock_cpu_by_process_id", process_id);
		return -1;
	}

	return cpu_socket;
}

/*
 * Function: get_process_id_by_sock_cpu
 * Purpose: Returns the Process ID
 * Created on: 20/05/2014
 * Author: SilverStack
*/

int get_process_id_by_sock_cpu(int sock_cpu)
{
	int flag_process_found = 0;
	int process_id;
	int cpu_socket = sock_cpu;

	void _get_process_element(t_process *p)
	{
		if(flag_process_found == 0 && p->current_cpu_socket == cpu_socket )
		{
			flag_process_found = 1;
			process_id = p->pid;
		}
	}

	sem_wait(&mutex_process_list);
	list_iterate(list_process, (void*) _get_process_element);
	sem_post(&mutex_process_list);

	if(flag_process_found == 0)
	{
		log_error(logger, "PID %d no encontrado en get_process_id_by_sock_cpu", process_id);
		return -1;
	}
	return process_id;
}

int get_sock_prog_by_sock_cpu(int sock_cpu)
{
	int flag_process_found = 0;
	int sock_prog;
	int cpu_socket = sock_cpu;

	void _get_socket_program(t_process *p)
	{
		if(flag_process_found == 0 && p->current_cpu_socket == cpu_socket )
		{
			flag_process_found = 1;
			sock_prog = p->program_socket;
		}
	}

	sem_wait(&mutex_process_list);
	list_iterate(list_process, (void*) _get_socket_program);
	sem_post(&mutex_process_list);

	if(flag_process_found == 0)
	{
		log_error(logger, "socket program %d no encontrado en get_sock_prog_by_sock_cpu", sock_prog);
		return -1;
	}
		return sock_prog;

}

/*
 * Function: program_exit
 * Purpose: Send to Program Process Exit Call
 * Created on: 20/05/2014
 * Author: SilverStack
*/

void program_exit(int pid)
{
	int flag_process_found = 0;
	int process_id = pid;
	t_mensaje mensaje;
	char * buffer;
	int numbytes;
	int index = 0;
	int indice_buscado = 0;
	t_process* process;

	if((buffer = (char*) malloc (sizeof(char) * MAXDATASIZE)) == NULL)
	{
		log_error(logger,"Error al reservar memoria para el buffer en program_exit");
		return;
	}

	void _get_index_element_process(t_process *p)
	{
		if(flag_process_found == 0 && p->pid == process_id )
		{
			flag_process_found = 1;
			indice_buscado = index;
		}
		index++;
	}

	// Remuevo de la lista de Procesos
	sem_wait(&mutex_process_list);
	list_iterate(list_process, (void*) _get_index_element_process);
	sem_post(&mutex_process_list);

	sem_wait(&mutex_process_list);
	process = list_remove(list_process, indice_buscado);
	sem_post(&mutex_process_list);

	index = 0;
	indice_buscado = 0;

	if(flag_process_found == 0 )
	{
		log_error(logger, "PID %d no encontrado en lista de procesos", process_id);
		return;
	}

	mensaje.id_proceso = KERNEL;
	mensaje.datosNumericos = 0;
	mensaje.tipo = SALIR;
	memset(buffer,'\0',MAXDATASIZE);
	memcpy(buffer,&mensaje,SIZE_MSG);

	if((numbytes=write(process->program_socket,buffer,SIZE_MSG))<=0)
	{
		log_error(logger, "Error en el write en program_exit");
		close(process->program_socket);
		process_destroy(process);
		free(buffer);
		return;
	}

	close(process->program_socket);
	free(buffer);
	process_destroy(process);

	return;
}

/*
 * Function: pedido_create
 * Purpose: Create pedido node and add it to pedidos queue
 * Created on: 20/05/2014
 * Author: SilverStack
*/

t_pedido* pedido_create(int pid, unsigned char previous_status, unsigned char next_status)
{
	t_pedido* new_pedido;

	if ((new_pedido = (t_pedido*) malloc(sizeof(t_pedido)))== NULL)
	{
		log_error(logger, "No se pudo pedir memoria para el nuevo pedido");
		return NULL;
	}

	new_pedido->new_status = next_status;
	new_pedido->previous_status = previous_status;
	new_pedido->process_id = pid;

	return new_pedido;
}

/*
 * Function: fd_set_program_sockets
 * Purpose: FD_SET all program sockets
 * Created on: 06/06/2014
 * Author: SilverStack
*/

void fd_set_program_sockets(fd_set* descriptores)
{
	void _fd_set(t_process *p)
	{
		FD_SET(p->program_socket,descriptores);
	}

	sem_wait(&mutex_process_list);
	list_iterate(list_process, (void*) _fd_set);
	sem_post(&mutex_process_list);
}

/*
 * Function: fd_set_cpu_sockets
 * Purpose: FD_SET all cpu sockets
 * Created on: 06/06/2014
 * Author: SilverStack
*/

void fd_set_cpu_sockets(fd_set* descriptores)
{
	void _fd_set(t_nodo_cpu *cpu)
	{
		FD_SET(cpu->socket,descriptores);
	}

	sem_wait(&mutex_cpu_list);
	list_iterate(list_cpu, (void*) _fd_set);
	sem_post(&mutex_cpu_list);
}

void test_pcb(int process_id, unsigned char previous_status)
{

	int flag_found = 0;
	t_list* pcb_from;
	t_pcb* pcb;
	char PCB_QUEUE[20];
	int i;

	switch(previous_status)
	{
		case PROCESS_NEW: pcb_from = list_pcb_new; strcpy(PCB_QUEUE,"list_pcb_new");break;
		case PROCESS_READY: pcb_from = list_pcb_ready; strcpy(PCB_QUEUE,"list_pcb_ready");break;
		case PROCESS_EXECUTE: pcb_from = list_pcb_execute; strcpy(PCB_QUEUE,"list_pcb_execute"); break;
		case PROCESS_BLOCKED: pcb_from = list_pcb_blocked; strcpy(PCB_QUEUE,"list_pcb_blocked"); break;
		case PROCESS_EXIT: pcb_from = list_pcb_exit; strcpy(PCB_QUEUE,"list_pcb_exit"); break;
	}

	for(i=0;i < list_size(pcb_from);i++)
	{
		pcb = list_get(pcb_from,i);
		if(pcb->unique_id == process_id)
		{
			flag_found = 1;
			break;
		}
	}

	if(flag_found == 0)
	{
		log_error(logger, "PID %d no encontrado en test_pcb", process_id);
		return;
	}

	log_info(logger, "---------------- PCB QUEUE = %s", PCB_QUEUE);
	log_info(logger, "---------------- PCB QUEUE SIZE = %d", list_size(pcb_from));

	log_info(logger, "---------------- PCB ->unique_id = %d", pcb->unique_id);
	log_info(logger, "---------------- PCB ->code_segment = %d", pcb->code_segment);
	log_info(logger, "---------------- PCB ->context_actual = %d", pcb->context_actual);
	log_info(logger, "---------------- PCB ->etiquetas_index = %d", pcb->etiquetas_index);
	log_info(logger, "---------------- PCB ->instruction_index = %d", pcb->instruction_index);
	log_info(logger, "---------------- PCB ->instruction_size = %d", pcb->instruction_size);
	log_info(logger, "---------------- PCB ->peso = %d", pcb->peso);
	log_info(logger, "---------------- PCB ->program_counter = %d", pcb->program_counter);
	log_info(logger, "---------------- PCB ->size_etiquetas_index = %d", pcb->size_etiquetas_index);
	log_info(logger, "---------------- PCB ->stack_pointer = %d", pcb->stack_pointer);
	log_info(logger, "---------------- PCB ->stack_segment = %d", pcb->stack_segment);

	return;
}

/*
 * Function: process_set_status
 * Purpose: Set process status
 * Created on: 15/06/2014
 * Author: SilverStack
*/

void process_set_status(int process_id, unsigned char status)
{
	int i;
	t_process* process;

	sem_wait(&mutex_process_list);

	for(i=0; i < list_size(list_process);i++)
	{
		process = list_get(list_process,i);
		if(process->pid == process_id)
		{
			process->status = status;
			break;
		}
	}

	sem_post(&mutex_process_list);
	return;
}

/*
 * Function: process_get_status
 * Purpose: Get process status
 * Created on: 15/06/2014
 * Author: SilverStack
*/

unsigned char process_get_status(int process_id)
{
	int i;
	t_process* process;

	sem_wait(&mutex_process_list);
	for(i=0; i < list_size(list_process);i++)
	{
		process = list_get(list_process,i);
		if(process->pid == process_id)
		{
			sem_post(&mutex_process_list);
			return process->status;
		}
	}

	log_error(logger,"No se encontro el proceso = %d", process_id);
	sem_post(&mutex_process_list);
	return -1;
}

