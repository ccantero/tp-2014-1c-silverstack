/*
 * servidor.c
 *
 *  Created on: 18/04/2014
 *      Author: utnso
 */

// El servidor recibe como primer parametro el archivo de configuracion
// y como segundo parametro recibe el nombre que va a tener el archivo de log.

#include "servidor.h"

int main(int argc, char *argv[])
{
	// Creo el logger
	logger = log_create(argv[2], "servidor" , true, LOG_LEVEL_INFO);

	// Configuracion del cliente
	config = config_create(argv[1]);

	// Definicion de variables
	fd_set master;
	fd_set read_fds;
	struct sockaddr_in my_addr;
	struct sockaddr_in remoteaddr;
	int fdmax;
	int listener;
	int newfd;
	char buf[256];
	int nbytes;
	int addrlen;
	int yes = 1;
	int puerto;
	int i;
	int j;

	// Borro conjuntos maestro y temporal
	FD_ZERO(&master);
	FD_ZERO(&read_fds);

	if ((listener = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		log_info(logger, "Error en la creacion del socket listener.");
		exit(1);
	}

	if (setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1)
	{
		log_info(logger, "Error en funcion setsockopt");
		exit(1);
	}

	// Obtengo datos del archivo de configuracion
	puerto = config_get_int_value(config, "puerto");
	log_info(logger, "Se leyeron satisfactoriamente los datos del archivo de configuracion.");

	// Asigno variables de mi direccion
	my_addr.sin_family = AF_INET;
	my_addr.sin_addr.s_addr = INADDR_ANY;
	my_addr.sin_port = htons(puerto);
	memset(&(my_addr.sin_zero), '\0', 8);

	if (bind(listener, (struct sockaddr *)&my_addr, sizeof(struct sockaddr)) == -1)
	{
		log_info(logger, "Error en funcion bind.");
		exit(1);
	}

	if (listen(listener, 10) == -1)
	{
		log_info(logger, "Error en funcion listen.");
		exit(1);
	}

	// Se añade el listener al conjunto maestro
	FD_SET(listener, &master);

	// Se inicializa el mayor descriptor (es un numero entero)
	fdmax = listener;

	// Loop principal donde se gestionan todas las conexiones entrantes
	while (1)
	{
		read_fds = master;
		if (select(fdmax + 1, &read_fds, NULL, NULL, NULL) == -1)
		{
			perror("select");
			exit(1);
		}
		for (i = 0; i <= fdmax; i++)
		{
			if (FD_ISSET(i, &read_fds))
			{
				// Hay datos
				if (i == listener)
				{
					// Gestiono la nueva conexion
					addrlen = sizeof(remoteaddr);
					if ((newfd = accept(listener, (struct sockaddr *)&remoteaddr, &addrlen)) == -1)
					{
						log_info(logger, "Error en funcion aceptar.");
					}
					else
					{
						FD_SET(newfd, &master);
						if (newfd > fdmax)
						{
							fdmax = newfd;
						}
						log_info(logger, "Nueva conexion desde: %s\n", inet_ntoa(remoteaddr.sin_addr));
						printf("Nueva conexion desde %s\n", inet_ntoa(remoteaddr.sin_addr));
					}
				}
				else
				{
					// Gestiono datos de cliente
					if ((nbytes = recv(i, buf, sizeof(buf), 0)) <= 0)
					{
						// Conexion cerrada o error
						if (nbytes == 0)
						{
							log_info(logger, "Conexion cerrada.");
							printf("Conexion cerrada.");
						}
						else
						{
							log_info(logger, "Error recibiendo datos.");
						}

						// Cierro el socket y lo saco del maestro
						close(i);
						FD_CLR(i, &master);
					}
					else
					{
						// Hay datos de cliente
						log_info(logger, "Se recibieron datos de cliente.");
						printf("Datos recibidos: %s", buf);
					}
				}
			}
		}
	}
	return 0;
}
