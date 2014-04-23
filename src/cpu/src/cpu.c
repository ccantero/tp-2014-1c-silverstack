/*
 * cpu.c
 *
 *  Created on: 23/04/2014
 *      Author: utnso
 */

// El proceso cpu recibe como primer parámetro el archivo de configuracion y
// como segundo parámetro el nombre de archivo de log.

#include "cpu.h"

// main
int main(int argc, char *argv[])
{
	// Se obtienen datos de archivo de configuracion y se crea el logger
	GetInfoConfFile(config, argv[1]);
	config_destroy(config);
	logger = log_create(argv[2], "cpu" , true, LOG_LEVEL_INFO);
	log_info(logger, "Se leyo el arch de config y se creo el logger satisfactoriamente.");

	ConectarAKernel();

	// Libero memoria del logger
	log_destroy(logger);

	return 0;
}


// Definicion de funciones
void GetInfoConfFile(t_config *config, char *path)
{
	config = config_create(path);
	strcpy(umvip, config_get_string_value(config, "UMV_IP"));
	strcpy(myip, config_get_string_value(config, "IP"));
	strcpy(kernelip, config_get_string_value(config, "KERNEL_IP"));
	port_kernel = config_get_int_value(config, "PORT_KERNEL");
	port_umv = config_get_int_value(config, "PORT_UMV");
	return;
}

void ConectarAKernel(void)
{
	// Creo el descriptor para el socket y compruebo errores
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		log_info(logger, "Error cuando se crea el socket.");
		exit(1);
	}
	// Asigno las variables de direccion a conectar
	their_addr.sin_family = AF_INET;
	their_addr.sin_port = htons(port_kernel);
	their_addr.sin_addr.s_addr = inet_addr(kernelip);
	memset(&(their_addr.sin_zero), '\0', 8);
	// Conecto el socket y compruebo errores
	if (connect(sockfd, (struct sockaddr *)&their_addr, sizeof(struct sockaddr)) == -1)
	{
		log_info(logger, "Error conectando el socket.");
		exit(1);
	}
	log_info(logger, "Conectado al kernel.");
}
