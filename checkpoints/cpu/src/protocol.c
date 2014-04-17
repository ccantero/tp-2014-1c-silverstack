/*
 * protocol.c
 *
 *  Created on: 16/04/2014
 *  Author: SilverStack
*/

/* Header File */

#include "protocol.h"

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
	strcpy(umvip,config_get_string_value(config, "UMV_IP"));
	strcpy(myip,config_get_string_value(config, "IP"));
	strcpy(kernelip,config_get_string_value(config, "KERNEL_IP"));
	port_kernel=config_get_int_value(config, "PORT_KERNEL");
	port_umv=config_get_int_value(config, "PORT_UMV");
	return;
}
