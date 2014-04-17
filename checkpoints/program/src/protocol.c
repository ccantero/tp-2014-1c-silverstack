/*
 * protocol.c
 *
 *  Created on: 13/04/2014
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
    	printf("         Bienvenido a la consola de Programa           \n");
    	printf("-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-\n");

    	printf("Ingrese el comando: \n");
    	printf(">");

    	scanf("%s",comando);
    	c=strtok(comando,"\n");

   		if (flag_comandoOK == 0 && strcmp(c,"imprimir")==0 )
   		{
   			printf("Llamar a funcion imprimir\n");
   			flag_comandoOK = 1;
   		}

   		if (strcmp(c,"imprimirTexto")==0)
   		{
   			printf("Llamar a funcion imprimirTexto\n");
   			flag_comandoOK = 1;
   		}

   		if(flag_comandoOK == 0)
   		{
   			printf("Por favor verifique la sintaxis de los comandos utilizados\n");
   			printf("Los unicos comandos habilitados son: \n");
   			printf("\timprimir\n");
   			printf("\timprimirTexto\n");
   		}
   	}
	return;
}

/*
 * Function: GetInfoConfFile
 * Purpose: Parse Configuration File
 * Created on: 14/04/2014
 * Author: SilverStack
*/

void GetInfoConfFile(void)
{
	config = config_create(PATH_CONFIG);
	strcpy(myip,config_get_string_value(config, "IP"));
	strcpy(hostip,config_get_string_value(config, "HOSTIP"));
	port=config_get_int_value(config, "PORT");
}
