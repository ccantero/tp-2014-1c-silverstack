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

void GetInfoConfFile(void)
{
	t_config* config;
	char** HIO;
	char** ID_HIO;
	char** ID_Sem;
	char** Value_Sem;
	char** Globales;
	char** Global_Values;
	char** AUX;
	int i = 0, j = 0,retardo_io, valor_semaforo, valor_compartida;

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
	Global_Values = config_get_array_value(config, "VALORES");

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
		retardo_io = atoi(*HIO);
		queue_push(queue_io, io_create(*ID_HIO, retardo_io));
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

	AUX = Globales;

	i = 0;
	j = 0;

	while (*AUX != NULL) /* Cuenta cantidad de Variables Globales */
	{
		AUX++;
		i++;
	}

	AUX = Global_Values;

	while (*AUX != NULL) /* Cuenta cantidad de valores en Variables Globales */
	{
		AUX++;
		j++;
	}

	if(i != j)
	{
		log_error(logger, "No coinciden la cantidad de Variables Compartidas con la cantidad de valores");
		log_error(logger, "Extraccion incorrecta del archivo de configuracion");
		exit(1);
	}

	while(*Globales != NULL && *Global_Values != NULL)
	{
		valor_compartida = atoi(*Global_Values);
		list_add(list_globales, global_create(*Globales, valor_compartida));
		Globales++;
		Global_Values++;
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

t_global* global_create(char *global_name, int value)
{
	t_global* new_global = (t_global*) malloc(sizeof(t_global));
	new_global->identifier = (char*) malloc(sizeof(char) * (strlen(global_name) + 1));
	strcpy(new_global->identifier,global_name);
	new_global->value = value;
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
	char* global_id = (char*) malloc (sizeof(char) * (strlen(global_name) + 1));
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
	char* global_id = (char*) malloc (sizeof(char) * (strlen(global_name) + 1));
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
	t_io* new_io = (t_io*) malloc(sizeof(t_io));
	new_io->name = (char*) malloc(sizeof(char) * (strlen(io_name) + 1));
	strcpy(new_io->name,io_name);
	new_io->retardo = io_retardo;
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
	free(self);
}

/*
 * Function: semaphore_create
 * Purpose: Creates an semaphore
 * Created on: 26/04/2014
 * Author: SilverStack
*/

t_semaphore* semaphore_create(char* sem_name, int value)
{
	t_semaphore* new_sem = (t_semaphore*) malloc(sizeof(t_semaphore));
	new_sem->identifier = (char*) malloc (sizeof(char) * ( strlen(sem_name) + 1 ) );
 	strcpy(new_sem->identifier,sem_name);
	new_sem->value = value;
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
	free(self->identifier);
	free(self);
}

/*
 * Function: semphore_wait
 * Purpose: wait / down / P
 * Created on: 26/04/2014
 * Author: SilverStack
*/

void semaphore_wait(char* sem_name)
{
	int flag_mod = 0;
	char* semaphore_id = (char*) malloc (sizeof(char) * (strlen(sem_name) + 1));
	strcpy(semaphore_id,sem_name);

	void _get_semaphore(t_semaphore *s)
	{
		if(strcmp(semaphore_id,s->identifier) == 0)
		{
			s->value = s->value - 1;
			flag_mod = 1;
		}
	}

	list_iterate(list_semaphores, (void*) _get_semaphore);

	if(flag_mod == 0)
		log_error(logger, "Semaforo %s no encontrado", sem_name);

	free(semaphore_id);
}

/*
 * Function: semphore_signal
 * Purpose: signal / up / v
 * Created on: 01/05/2014
 * Author: SilverStack
*/

void semaphore_signal(char* sem_name)
{
	int flag_mod = 0;
	char* semaphore_id = (char*) malloc (sizeof(char) * (strlen(sem_name) + 1));
	strcpy(semaphore_id,sem_name);

	void _get_semaphore(t_semaphore *s)
	{
		if(strcmp(semaphore_id,s->identifier) == 0)
		{
			s->value = s->value + 1;
			flag_mod = 1;
		}
	}

	list_iterate(list_semaphores, (void*) _get_semaphore);

	if(flag_mod == 0)
			log_error(logger, "Semaforo %s no encontrado", sem_name);

	free(semaphore_id);
}

/*
 * Function: servidor
 * Purpose: Check all the sockets umv and program
 * Created on: 18/04/2014
 * Author: SilverStack
*/

void servidor_plp(void)
{
	fd_set descriptoresLectura;
	int sock_program, fdmax, i, new_socket;
	struct sockaddr_in my_addr;
	char buf[MAXDATASIZE];

	if( (sock_program=socket(AF_INET,SOCK_STREAM,0))==-1)
	{
		log_error(logger, "Error en funcion socket");
		return;
	}

	my_addr.sin_port=htons(port_program);
	my_addr.sin_family=AF_INET;
	my_addr.sin_addr.s_addr=inet_addr(myip);
	memset(&(my_addr.sin_zero),0,8);

	if (bind(sock_program,(struct sockaddr *)&my_addr,sizeof(struct sockaddr))==-1)
	{
		log_error(logger, "Error en funcion bind");
		return;
	}

	if (listen(sock_program,backlog)==-1)
	{
		log_error(logger, "Error en funcion listen");
		return;
	}

	FD_ZERO (&descriptoresLectura);
	FD_SET (sock_program, &descriptoresLectura);
	FD_SET (sock_umv, &descriptoresLectura);

	if(sock_umv < sock_program)
		fdmax = sock_program;
	else
		fdmax = sock_umv;

	for(;;)
	{
		if (select(fdmax + 1, &descriptoresLectura, NULL, NULL, NULL) == -1)
		{
			log_error(logger, "Error en funcion select");;
			exit(1);
		}

		for (i = 0; i <= fdmax; i++)
		{
			if (FD_ISSET(i, &descriptoresLectura))
			{
				if(i == sock_umv)
				{
					//escuchar_umv();
					continue;
				}

				if (i == sock_program)
				{
					new_socket = escuchar_Nuevo_Programa(sock_program,buf);
					if(new_socket == -1)
					{
						FD_CLR(i, &descriptoresLectura);
					}
					else
					{
						FD_SET(new_socket, &descriptoresLectura);
						if(fdmax < new_socket)
							fdmax = new_socket;
					}
					continue;
				}

				if(escuchar_Programa(i, buf) == -1)
				{
					close(i);
					FD_CLR(i, &descriptoresLectura);
				}
			}
		} /* for (i = 0; i <= fdmax; i++) */
	}/* for(;;) */
}

/*
 * Function: escuchar_Nuevo_Programa
 * Purpose: Check for new Program connections
 * Created on: 18/04/2014
 * Author: SilverStack
*/

int escuchar_Nuevo_Programa(int sock_program, char* buffer)
{
	socklen_t sin_size;
	struct sockaddr_in my_addr;
	struct sockaddr_in their_addr;
	thdr hdr;

	int new_socket;
	int numbytes;

	my_addr.sin_port=htons(port_program); /* No creo que sea necesario para el sizeof */
	my_addr.sin_family=AF_INET; /* No creo que sea necesario para el sizeof */
	my_addr.sin_addr.s_addr=inet_addr(myip); /* No creo que sea necesario para el sizeof */
	memset(&(my_addr.sin_zero),0,8); /* No creo que sea necesario para el sizeof */

	sin_size=sizeof(struct sockaddr_in);

	if((new_socket=accept(sock_program,(struct sockaddr *)&their_addr,	&sin_size))==-1)
	{
		log_error(logger, "Error en funcion accept en escuchar_Nuevo_Programa");
		return -1;
	}

	memset(buffer,'\0',MAXDATASIZE);

	if((numbytes=read(new_socket,buffer,SIZE_HDR))<=0)
	{
		log_error(logger, "Error en el read en escuchar_Nuevo_Programa");
		close(new_socket);
		return -1;
	}

	if(hdr.pay_desc==MSG_CON_PRG)
	{
		/* Es un nuevo programa que quiere conectarse */
		strcpy(hdr.desc_id,myip);
		hdr.pay_desc = MSG_CON_PRG_OK;
		hdr.pay_len = 0;
		memset(buffer,'\0',MAXDATASIZE);
		memcpy(buffer,&hdr,SIZE_HDR);

		if((numbytes=write(new_socket,buffer,SIZE_HDR))<=0)
		{
			log_error(logger, "Error en el write en escuchar_Nuevo_Programa");
			close(new_socket);
			return -1;
		}

		log_info(logger, "Nueva conexion lograda con programa");
		return new_socket;
	}
	else
	{
		/* Handshake Fallo */
		strcpy(hdr.desc_id,myip);
		hdr.pay_desc = MSG_CON_PRG_FAIL;
		hdr.pay_len = 0;
		memset(buffer,'\0',MAXDATASIZE);
		memcpy(buffer,&hdr,SIZE_HDR);

		if((numbytes=write(new_socket,buffer,SIZE_HDR))<=0)
		{
			log_error(logger, "Error en el write en escuchar_Nuevo_Programa. Error en el handshake");
			return -1;
		}

		log_error(logger, "No se pudo crear nueva conexion. Error en el handshake");
		return -1;
	}
}

/*
 * Function: escuchar_Programa
 * Purpose: Listen for program
 * Created on: 01/05/2014
 * Author: SilverStack
*/

int escuchar_Programa(int sock_program, char* buffer)
{
	thdr hdr;
	int numbytes;

	log_info(logger, "Recepcion de datos desde Programa");
	memset(buffer,'\0',MAXDATASIZE);
	if((numbytes=read(sock_program,buffer,SIZE_HDR))<=0)
	{
		log_error(logger, "Error en el read en escuchar_Programa");
		return -1;
	}

	memcpy(&hdr,buffer,SIZE_HDR);

	if(hdr.pay_desc==MSG_CON_PRG_TXT)
	{
		/* Para tratamiento del envio de archivos o comandos*/
		strcpy(hdr.desc_id,myip);
		hdr.pay_desc = MSG_CON_PRG_TXT_OK;
		hdr.pay_len = 0;
		memset(buffer,'\0',MAXDATASIZE);
		memcpy(buffer,&hdr,SIZE_HDR);

		if((write(sock_program,buffer,SIZE_HDR))<=0)
		{
			log_error(logger, "Error en el write en escuchar_Programa");
			return -1;
		}

		if((numbytes=read(sock_program,buffer,SIZE_HDR))<=0)
		{
			log_error(logger, "Error en el read en escuchar_Programa");
			return -1;
		}

		memcpy(&hdr,buffer,SIZE_HDR);

		if(hdr.pay_len>0)
		{
			if(hdr.pay_len >= MAXDATASIZE)
			{
				log_error(logger, "Se necesita recibir un archivo de gran tamanio. escuchar_Programa");
				return -1;
			}
			else
			{
				memset(buffer,'\0',MAXDATASIZE);
				if((numbytes=read(sock_program,buffer,hdr.pay_len))<=0)
				{
					log_error(logger, "Error en el read en escuchar_Programa");
					return -1;
				}

				log_info(logger, "Se recibieron %d bytes desde programa", hdr.pay_len);
				create_pcb(buffer,numbytes);
				return 0;
			}
		}
		else
		{
			log_error(logger, "Archivo de tamanio 0. escuchar_Programa");
			return -1;
		}
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

void create_pcb(char* buffer, int tamanio_buffer)
{
	t_medatada_program* metadata;
	t_pcb* new_pcb = (t_pcb*) malloc(sizeof(t_pcb));

	//send_umv_code_segment(sock_UMV, buffer);

	metadata = metadatada_desde_literal(buffer);
	new_pcb->unique_id = ++process_Id;
	new_pcb->code_segment.offset = tamanio_buffer;
	new_pcb->code_segment.start = get_Segment_Start(tamanio_buffer);
	new_pcb->code_segment.code_identifier = CODE_SEGMENT;

	list_add(list_segment, segment_create(new_pcb->code_segment.start,
										  new_pcb->code_segment.offset));

	new_pcb->stack_segment.code_identifier = STACK_SEGMENT;
	new_pcb->stack_segment.start = get_Segment_Start(100); /* 100 bytes de stack */
	new_pcb->stack_segment.offset = 100;

	list_add(list_segment, segment_create(new_pcb->stack_segment.start,
										  new_pcb->stack_segment.offset));

	new_pcb->stack_pointer = new_pcb->stack_segment.start;

	t_intructions*	indice_instrucciones = (t_intructions*) malloc (sizeof(t_intructions) * metadata->instrucciones_size);
	memcpy(indice_instrucciones,metadata->instrucciones_serializado,sizeof(t_intructions)*metadata->instrucciones_size);
	new_pcb->instruction_index.index = indice_instrucciones;
	new_pcb->instruction_index.size = metadata->instrucciones_size;

	char* etiquetas = (char*) malloc (sizeof(char) * metadata->etiquetas_size);
	memcpy(etiquetas,metadata->etiquetas,sizeof(char) * metadata->etiquetas_size);
	new_pcb->etiquetas_index.etiquetas = etiquetas;
	new_pcb->etiquetas_index.size = metadata->etiquetas_size;

	new_pcb->program_counter = metadata->instruccion_inicio;
	new_pcb->context_actual = 0;
	new_pcb->peso = 5 * metadata->cantidad_de_etiquetas + 3 * metadata->cantidad_de_funciones +
							metadata->instrucciones_size;

	list_add(list_pcb_new, new_pcb);

	metadata_destruir(metadata);
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
 * Function: get_Segment_Start
 * Purpose: Get a correct value for a new segment
 * Created on: 25/04/2014
 * Author: SilverStack
*/

int get_Segment_Start(int offset)
{
	int segment_start = 0;
	int segment_offset = offset;
	int segment_end = segment_start + offset;
	int segment_Available = 0;

	void _get_segment(t_nodo_segment *p)
	{
		int nodo_segment_end = p->start + p->offset;

		if(segment_Available == 0) /* No encontre un segmento libre */
		{
			if(segment_start < p->start && segment_end < p->start )
			{
				segment_Available = 1; /* Encontre un segmento */
			}
			else
			{
				segment_start = nodo_segment_end + 1;
				segment_end = segment_start + segment_offset;
			}
		}
	}


	if(list_size(list_segment) == 0)
	{
		return segment_start;
	}
	else
	{
		list_iterate(list_segment, (void*) _get_segment);
	}

	return segment_start;
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
	thdr hdr;

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

	strcpy(hdr.desc_id,myip);
	hdr.pay_desc=MSG_CON_UMV;
	hdr.pay_len=0;

	memcpy(buffer,&hdr,SIZE_HDR);

	if((numbytes=write(sockfd,buffer,SIZE_HDR))<=0)
	{
		log_error(logger, "Error en el write en el socket UMV");
		return -1;
	}

	memset(buffer,'\0',MAXDATASIZE);

	if((numbytes=read(sockfd,buffer,SIZE_HDR))<=0)
	{
		log_error(logger, "Error en el read en el socket UMV");
		close(sockfd);
		return -1;
	}

	memcpy(&hdr,buffer,SIZE_HDR);

	if(hdr.pay_desc==MSG_CON_UMV_OK)
	{
		log_info(logger, "Conexion Lograda con la UMV");
		return sockfd;
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
	sort_plp();
	t_list* sublist = list_take(list_pcb_new, 1);
	t_pcb* element = list_get(sublist, 0);

	if(list_size(list_pcb_ready) <= multiprogramacion)
	{
		list_add(list_pcb_new, element);
		log_info(logger, "PCB -> %s moved from New Queue to Ready Queue", element->unique_id);
	}
}
