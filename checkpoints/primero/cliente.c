/*
 * cliente.c
 *
 *  Created on: 18/04/2014
 *      Author: utnso
 */

// El cliente recibe como primer parametro el archivo de configuracion
// y como segundo parametro recibe el nombre que va a tener el archivo de log.

#include "cliente.h"

int main(int argc, char *argv[])
{
	// Creo el logger
	logger = log_create(argv[2], "cliente" , true, LOG_LEVEL_INFO);

	// Configuracion del cliente
	config = config_create(argv[1]);

	// Definicion de variables
	int sockfd;
	int numBytes;
	int puerto;
	char *direccionIp = (char *)malloc(15);
	char buf[256];
	char entrada[256];
	struct sockaddr_in their_addr;

	// Creo el descriptor para el socket y compruebo errores
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		log_info(logger, "Error cuando se crea el socket.");
		exit(1);
	}

	// Obtengo datos del archivo de configuracion
	puerto = config_get_int_value(config, "puerto");
	direccionIp = config_get_string_value(config, "ip");
	log_info(logger, "Se leyeron satisfactoriamente los datos del archivo de configuracion.");

	// Asigno las variables de direccion a conectar
	their_addr.sin_family = AF_INET;
	their_addr.sin_port = htons(puerto);
	their_addr.sin_addr.s_addr = inet_addr(direccionIp);
	memset(&(their_addr.sin_zero), '\0', 8);

	// Conecto el socket y compruebo errores
	if (connect(sockfd, (struct sockaddr *)&their_addr, sizeof(struct sockaddr)) == -1)
	{
		log_info(logger, "Error conectando el socket.");
		exit(1);
	}

	while(1)
	{


		// Pido mensaje a enviar
		printf("Yo: ");
		fgets(buf, 255, stdin);
		send(sockfd, buf, 255, 0);
		printf("Esperando respuesta...\n");

		// Recibo la respuesto y compruebo errores
		if ((numBytes = recv(sockfd, (void *)buf, 255, 0)) == -1)
		{
			log_info(logger, "Error recibiendo mensaje.");
			exit(1);
		}
		// log_info(logger, "Se recibieron %d bytes satisfactoriamente.", numBytes);

		// Muestro en pantalla la respuesta al mensaje enviado
		printf("Recibido: %s\n", buf);
	}

	// Cierro el descriptor del socket
	close(sockfd);

	// Libero memoria
	free(direccionIp);
	config_destroy(config);
	log_destroy(logger);

	return 0;
}
