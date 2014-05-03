/*
 * programa.c
 *
 *  Created on: 30/04/2014
 *      Author: utnso
 */
#include "programa.h"

int main (int argc, char *argv[])
{
	// recibo nombre del archivo a procesar

	char *nombreArchivo = (char*) malloc (20);
	nombreArchivo = argv[1];

	// leo archivo configuracion
	config = config_create(argv[2]);

	//defino algunas variables
		int sockfd;
		int puerto;
		char *direccionIp = (char *)malloc(16);
		char buf[256];
		int numBytes;
		FILE *file;


	//veo ip y puerto
			puerto = config_get_int_value(config, "PUERTO");
			direccionIp = config_get_string_value(config, "IP");

	//creo socket
			if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
				{
					printf("Error cuando se crea el socket.");
					exit(1);
				}
	//asigno el socket al kernel
			struct sockaddr_in their_addr;

			their_addr.sin_family = AF_INET;
			their_addr.sin_port = htons(puerto);
			their_addr.sin_addr.s_addr = inet_addr(direccionIp);
			memset(&(their_addr.sin_zero), '\0', 8);

	// Conecto el socket y compruebo errores
			if (connect(sockfd, (struct sockaddr *)&their_addr, sizeof(struct sockaddr)) == -1)
			{
				printf("Error conectando el socket");
				exit(1);
			}
			printf("hola program 20000\n");
		t_mensaje msg;
		msg.tipo = HANDSHAKE;
		msg.id_proceso = PROGRAMA;
		strcpy(msg.mensaje,"Hola Kernel!!");
		printf("hola program 98798798790\n");
		//handshake
		printf("Yo: ");
		send(sockfd, &msg, sizeof(msg), 0);
		printf("Esperando respuesta...\n");

		// Respuesta handshake y compruebo errores
		if ((numBytes = recv(sockfd, &msg, sizeof(msg), 0)) == -1)
		{
			printf("Error recibiendo mensaje.");
			exit(1);
		}
		//imprimo respuesta handshake
			printf("Recibido: %d\n", msg.tipo);

			struct stat stat_file;
	while (1)
	{
		//envio codigo ansisop
		stat(nombreArchivo,&stat_file);

		file = fopen (nombreArchivo,"r");

		char* buffer = (char*) malloc(sizeof(char) * (stat_file.st_size +1));

		if (file==NULL)
		{
			printf("Error de apertura de archivo");
		}
		printf("Yo: ");
		fread(buffer, stat_file.st_size,1, file);

		msg.tipo = SENDFILE;
		msg.id_proceso = PROGRAMA;
		msg.datosNumericos = stat_file.st_size;

		send(sockfd, &msg, sizeof(t_mensaje), 0);

		send(sockfd, buffer, stat_file.st_size, 0);
		printf("Esperando respuesta...\n");

		// Recibo orden de impresion y compruebo errores
		if ((numBytes = recv(sockfd, (void *)buf, 255, 0)) == -1)
		{
			printf("Error recibiendo mensaje.");
			exit(1);
		}
		//muestro imprimir
		printf ("%s", buf);

		fclose(file);
	}

	// Cierro el descriptor del socket
	close(sockfd);

	// Libero memoria
	free(direccionIp);
	config_destroy(config);


	return 0;
}

