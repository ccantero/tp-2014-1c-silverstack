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
		char *direccionIp = (char *)malloc(15);

	//veo ip y puerto
			puerto = config_get_int_value(config, "puerto");
			direccionIp = config_get_string_value(config, "ip");

	//creo socket
			if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
				{
					printf("Error cuando se crea el socket.");
					exit(1);
				}
	//asigno el socket al kernel



}

