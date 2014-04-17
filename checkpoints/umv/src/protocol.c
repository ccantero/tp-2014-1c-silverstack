/*
 * protocol.c
 *
 *  Created on: 15/04/2014
 *  Author: SilverStack
*/

/* Header File */

#include "protocol.h"

/*
 * Function: consola
 * Purpose: Launch consola
 * Created on: 13/04/2014
 * Author: SilverStack
*/

void consola (void* param)
{

	char comando[100];
	char *c;
	int flag_comandoOK;

	for(;;)
	{
		//system("clear");
		flag_comandoOK = 0;

		log_info(logger, "Se lanzo el hilo de consola");
    	printf("-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-\n");
    	printf("         Bienvenido a la consola de UMV           \n");
    	printf("-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-\n");

    	printf("Ingrese el comando: \n");
    	printf(">");

    	scanf("%s",comando);
    	c=strtok(comando,"\n");

   		if (flag_comandoOK == 0 && strncmp(c,"operacion",strlen("operacion"))==0 )
   		{
   			printf("Llamar a funcion operacion\n");
   			flag_comandoOK = 1;
   		}

   		if (flag_comandoOK == 0 && strncmp(c,"retardo",strlen("retardo"))==0)
   		{
   			printf("Llamar a funcion retardo\n");
   			flag_comandoOK = 1;
   		}

   		if (flag_comandoOK == 0 && strncmp(c,"algoritmo",strlen("algoritmo"))==0)
   		{
   			printf("Llamar a funcion algoritmo\n");
   		   	flag_comandoOK = 1;
   		}

   		if (flag_comandoOK == 0 && strncmp(c,"compactacion",strlen("compactacion"))==0)
   		{
   			printf("Llamar a funcion compactacion\n");
   		   	flag_comandoOK = 1;
   		}

   		if (flag_comandoOK == 0 && strncmp(c,"dump",strlen("dump"))==0)
   		{
   			printf("Llamar a funcion dump\n");
   		   	flag_comandoOK = 1;
   		}

   		if(flag_comandoOK == 0)
   		{
   			printf("Por favor verifique la sintaxis de los comandos utilizados\n");
   			printf("Los unicos comandos habilitados son: \n");
   			printf("\toperacion\n");
   			printf("\tretardo\n");
   			printf("\talgoritmo\n");
   			printf("\tcompactacion\n");
   			printf("\tdump\n");
   		}
   	}
	return;
}

/*
 * Function: GetInfoConfFile
 * Purpose: Parse Configuration File
 * Created on: 15/04/2014
 * Author: SilverStack
*/

void GetInfoConfFile(void)
{
	t_config* config;

	config = config_create(PATH_CONFIG);
	strcpy(myip,config_get_string_value(config, "IP"));
	strcpy(hostip,config_get_string_value(config, "KERNEL_IP"));
	strcpy(algoritmo,config_get_string_value(config, "ALGORITMO"));
	port_kernel=config_get_int_value(config, "PORT_KERNEL");
	port_cpu=config_get_int_value(config, "PORT_CPU");
	space=config_get_int_value(config, "DISK_SPACE");
	return;
}
