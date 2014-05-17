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
	config = config_create("program.config");


	//defino algunas variables
		int sockfd;
		int puerto;
		char *direccionIp = (char *)malloc(16);
		char buf[256];
		int numBytes;
		int x=1;
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
				return 0;
			}
		t_mensaje msg;
		msg.tipo = HANDSHAKE;
		msg.id_proceso = PROGRAMA;
		strcpy(msg.mensaje,"Hola Kernel!!");
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

		//envio codigo ansisop
		stat(nombreArchivo,&stat_file);

		file = fopen (nombreArchivo,"r");

		char* buffer = (char*) malloc(sizeof(char) * (stat_file.st_size +1));

		if (file==NULL)
		{
			printf("Error de apertura de archivo");
		}
		fread(buffer, stat_file.st_size,1, file);

		msg.tipo = SENDFILE;
		msg.id_proceso = PROGRAMA;
		msg.datosNumericos = stat_file.st_size;

		send(sockfd, &msg, sizeof(t_mensaje), 0);

		send(sockfd, buffer, stat_file.st_size, 0);
		printf("Esperando respuesta...\n");
		while (x == 1)
		{
			puts("hola#");
			// Recibo y me fijo si hay errores
			if ((numBytes = recv(sockfd, &msg, sizeof(msg), 0)) == -1)
			{
				printf("Error recibiendo mensaje.");
				exit(1);
			}
			puts("hola#");

				//me fijo si es imprimir, imprimir texto o salir

			if(msg.tipo == IMPRIMIR)
			{
				printf("%d", msg.datosNumericos);
			}

			if (msg.tipo == IMPRIMIRTEXTO)
			{
				//recibo texto y me fijo si tiene error
				if ((numBytes = recv(sockfd, buf, sizeof(msg.datosNumericos), 0)) == -1)
							{
								printf("Error recibiendo mensaje.");
								exit(1);
							}
				//imprimo texto
				printf("%s", buf);

			}

			if (msg.tipo == SALIR)
			{
				x =2;
			}
		}
	//cierro archivo
		fclose(file);
	// Cierro el descriptor del socket
	close(sockfd);

	// Libero memoria
	free(direccionIp);
	config_destroy(config);

	return 0;
}

