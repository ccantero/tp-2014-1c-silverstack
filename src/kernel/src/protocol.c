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
	char** AUX;
	int i = 0, j = 0,retardo_io, valor_semaforo;

	config = config_create(PATH_CONFIG);
	log_info(logger, "Cantidad de Keys = %d",config_keys_amount(config));

	strcpy(myip,config_get_string_value(config, "IP"));
	ID_HIO = config_get_array_value(config, "ID_HIO");
	HIO = config_get_array_value(config, "HIO");
	ID_Sem = config_get_array_value(config, "SEMAFOROS");
	Value_Sem = config_get_array_value(config, "VALOR_SEMAFORO");
	multiprogramacion=config_get_int_value(config, "MULTIPROGRAMACION");
	port_cpu=config_get_int_value(config, "PUERTO_CPU");
	port_program=config_get_int_value(config, "PUERTO_PROG");
	quantum=config_get_int_value(config, "QUANTUM");
	retardo=config_get_int_value(config, "RETARDO");

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
		log_info(logger, "Agregue a la io_queue ID = %s, Retardo = %d", *ID_HIO, retardo_io);
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

	while (*AUX != NULL) /* Cuenta cantidad de valores en I/O */
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
		log_info(logger, "Agregue a la list_semaphores ID = %s, Valor = %d", *ID_Sem, valor_semaforo);
		ID_Sem++;
		Value_Sem++;
	}


	log_info(logger, "Extraccion correcta del archivo de configuracion");
	config_destroy(config);
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
	new_io->name = io_name;
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
 * Function: semaphore
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
 * Function: servidor
 * Purpose: Check for new Program connections
 * Created on: 18/04/2014
 * Author: SilverStack
*/

void servidor_plp(void)
{
	fd_set descriptoresLectura;
	int sockf, fdmax, i, numbytes, sin_size, new_fd;
	struct sockaddr_in my_addr;
	struct sockaddr_in their_addr;
	char buf[MAXDATASIZE];
	thdr hdr;

	if( (sockf=socket(AF_INET,SOCK_STREAM,0))==-1)
	{
		log_error(logger, "Error en funcion socket");
		return;
	}

	my_addr.sin_port=htons(port_program);
	my_addr.sin_family=AF_INET;
	my_addr.sin_addr.s_addr=inet_addr(myip);
	memset(&(my_addr.sin_zero),0,8);

	if (bind(sockf,(struct sockaddr *)&my_addr,sizeof(struct sockaddr))==-1)
	{
		log_error(logger, "Error en funcion bind");
		return;
	}

	if (listen(sockf,backlog)==-1)
	{
		log_error(logger, "Error en funcion listen");
		return;
	}

	FD_ZERO (&descriptoresLectura);
	FD_SET (sockf, &descriptoresLectura);

	fdmax = sockf;

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
				if (i == sockf)
				{
					sin_size=sizeof(struct sockaddr_in);
					if((new_fd=accept(sockf,(struct sockaddr *)&their_addr,	&sin_size))==-1)
					{
						log_error(logger, "Error en funcion accept");
					}

					memset(buf,'\0',MAXDATASIZE);

					if((numbytes=read(new_fd,buf,SIZE_HDR))<=0)
					{
						log_error(logger, "Error en el read");
						close(new_fd);
						FD_CLR(i, &descriptoresLectura);
					}

					memcpy(&hdr,buf,SIZE_HDR);

					if(hdr.pay_desc==MSG_CON_PRG)
					{
						/* Es un nuevo programa que quiere conectarse */
						hdr.pay_desc=MSG_CON_PRG_OK;

						memset(buf,'\0',MAXDATASIZE);
						memcpy(buf,&hdr,SIZE_HDR);

						if((numbytes=write(new_fd,buf,SIZE_HDR))<=0)
						{
							log_error(logger, "Error en el write");
							close(new_fd);
							FD_CLR(i, &descriptoresLectura);
						}

						FD_SET(new_fd, &descriptoresLectura);
						if (new_fd > fdmax)
						{
							fdmax = new_fd;
						}
						log_info(logger, "Nueva conexion lograda con programa");
					}
					else
					{
						/* Handshake Fallo */
						hdr.pay_desc=MSG_CON_PRG_FAIL;
						memset(buf,'\0',MAXDATASIZE);
						memcpy(buf,&hdr,SIZE_HDR);

						if((numbytes=write(new_fd,buf,SIZE_HDR))<=0)
						{
							log_error(logger, "Error en el write");
						}

						FD_CLR(i, &descriptoresLectura);
						close(new_fd);
						log_error(logger, "No se pudo crear nueva conexion. Error en el handshake");
					}
				}
				else
				{
					log_info(logger, "Recepcion de datos desde Programa");
					memset(buf,'\0',MAXDATASIZE);
					if((numbytes=read(i,buf,SIZE_HDR))<=0)
					{
						log_error(logger, "Error en el read");
						close(i);
						FD_CLR(i, &descriptoresLectura);
					}

					memcpy(&hdr,buf,SIZE_HDR);

					if(hdr.pay_desc==MSG_CON_PRG_TXT)
					{
						/* Para tratamiento del envio de archivos o comandos*/
						hdr.pay_desc = MSG_CON_PRG_TXT_OK;
						memset(buf,'\0',MAXDATASIZE);
						memcpy(buf,&hdr,SIZE_HDR);

						if((write(i,buf,SIZE_HDR))<=0)
						{
							log_error(logger, "Error en el write");
						}

						if(hdr.pay_len>0)
						{
							if(hdr.pay_len >= MAXDATASIZE)
							{
								log_info(logger, "Se necesita recibir un archivo de gran tamanio");
								//recibirArchivo();
							}
							else
							{
								memset(buf,'\0',MAXDATASIZE);
								if((numbytes=read(i,buf,hdr.pay_len))<=0)
								{
									log_error(logger, "Error en el read");
									close(i);
									FD_CLR(i, &descriptoresLectura);
								}
								log_info(logger, "Se recibieron %d bytes desde programa", hdr.pay_len);
							}
						}
						else
						{
							log_error(logger, "Error en el descriptor");
							close(i);
							FD_CLR(i, &descriptoresLectura);
						}
					}
					else
					{
						log_info(logger, "Error en el descriptor");
						close(i);
						FD_CLR(i, &descriptoresLectura);
					}
				}
			}
			else
			{
				log_error(logger, "Terrible, hice el select por un socket que no esta!!1");
			}
		}
	}
}

/*
 * Function: create_pcb
 * Purpose: create pcb node
 * Created on: 18/04/2014
 * Author: SilverStack
*/

void create_pcb(char* buffer, char* name_program)
{
	t_medatada_program* metadata;
	int tamanio_buffer;
	t_pcb* new_pcb = (t_pcb*) malloc(sizeof(t_pcb));
	tamanio_buffer = strlen(buffer) + 1; /* Se suma el caracter '\0' */

	//send_umv_code_segment(sock_UMV, buffer);

	metadata = metadatada_desde_literal(buffer);

	new_pcb->unique_id = (char*) malloc (sizeof(char) * strlen(name_program) + 1);
	strcpy(new_pcb->unique_id,name_program);

	new_pcb->code_segment.offset = tamanio_buffer;
	new_pcb->code_segment.start = get_Segment_Start(tamanio_buffer);
	new_pcb->code_segment.code_identifier = CODE_SEGMENT;

	list_add(list_segment, segment_create(new_pcb->code_segment.start,
											new_pcb->code_segment.offset));

	new_pcb->stack_segment.code_identifier = STACK_SEGMENT;
	new_pcb->stack_segment.start = get_Segment_Start(100); /* 100 bytes de stack */
	new_pcb->stack_segment.offset = 100;

	list_add(list_segment, segment_create(new_pcb->stack_segment.start,
										  new_pcb->stack_segment.start + 100));

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

	list_add(list_pcb, new_pcb);
	metadata_destruir(metadata);
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
	int i, j;
	int flag = 0; /* 0 es false 1 es true */
	srand(time(NULL));
	i = rand() % 1024; /* 1024 no tengo idea porqué */
	j = 1;
	while(flag)
	{
		i = j * i;
		if(is_Segment_Available(i,offset))
		{
			flag = 1;
		}
		else
		{
			j = j + offset;
		}
	}
	return i;
}

/*
 * Function: is_Segment_Available
 * Purpose: Should iterate on segment list and finds if current segment is correct
 * Created on: 25/04/2014
 * Author: SilverStack
*/

int is_Segment_Available(int start, int offset)
{
	return 1;
}
