/*
 * protocol.c
 *
 *  Created on: 17/04/2014
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
	char** AUX;
	int i = 0, j = 0,retardo_io;

	config = config_create(PATH_CONFIG);
	log_info(logger, "Cantidad de Keys = %d",config_keys_amount(config));

	strcpy(myip,config_get_string_value(config, "IP"));
	ID_HIO = config_get_array_value(config, "ID_HIO");
	HIO = config_get_array_value(config, "HIO");
	multiprogramacion=config_get_int_value(config, "MULTIPROGRAMACION");
	port_cpu=config_get_int_value(config, "PUERTO_CPU");
	port_program=config_get_int_value(config, "PUERTO_PROG");
	quantum=config_get_int_value(config, "QUANTUM");
	retardo=config_get_int_value(config, "RETARDO");

	AUX = ID_HIO;

	while (*ID_HIO != NULL)
	{
		ID_HIO++;
		i++;
	}

	ID_HIO = AUX;
	AUX = HIO;

	while (*HIO != NULL)
	{
		HIO++;
		j++;
	}

	HIO = AUX;

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

	log_info(logger, "Extraccion correcta del archivo de configuracion\n");
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
