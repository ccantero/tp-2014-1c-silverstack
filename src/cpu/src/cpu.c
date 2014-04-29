/*
 * cpu.c
 *
 *  Created on: 23/04/2014
 *      Author: utnso
 */

// El proceso cpu recibe como primer parámetro el archivo de configuracion y
// como segundo parámetro el nombre de archivo de log.

#include "cpu.h"

// Defino las primitivas que vamos a usar, cada una está definida mas abajo con el resto de las funciones
AnSISOP_funciones primitivas = {
		.AnSISOP_definirVariable		 = silverstack_definirVariable,
		.AnSISOP_obtenerPosicionVariable = silverstack_obtenerPosicionVariable,
		.AnSISOP_dereferenciar			 = silverstack_dereferenciar,
		.AnSISOP_asignar				 = silverstack_asignar,
		.AnSISOP_imprimir				 = silverstack_imprimir,
		.AnSISOP_imprimirTexto			 = silverstack_imprimirTexto,
		.AnSISOP_obtenerValorCompartida  = silverstack_obtenerValorCompartida,
		.AnSISOP_entradaSalida           = silverstack_entradaSalida,
		.AnSISOP_finalizar               = silverstack_finalizar,
		.AnSISOP_asignarValorCompartida  = silverstack_asignarValorCompartida,
		.AnSISOP_irAlLabel               = silverstack_irAlLabel,
		.AnSISOP_llamarSinRetorno        = silverstack_llamarSinRetorno,
		.AnSISOP_llamarConRetorno        = silverstack_llamarConRetorno,
		.AnSISOP_retornar                = silverstack_retornar,
};
AnSISOP_kernel primitivasKernel = {
		.AnSISOP_signal = silverstack_signal,
		.AnSISOP_wait   = silverstack_wait,
};

// main
int main(int argc, char *argv[])
{
	// Definicion de variables
	t_log *logger;
	t_config *config;
	int port_kernel;
	int port_umv;
	char kernelip[16];
	char umvip[16];
	int sockKernel;
	int sockUmv;
	struct sockaddr_in kernel_addr;
	struct sockaddr_in umv_addr;
	int seguirEjecutando = 1; // Mediante la señal SIGUSR1 se puede dejar de ejecutar el cpu
	t_mensaje mensaje;
	struct _pcb pcb;

	// Obtengo datos de archivo de configuracion y se crea el logger
	config = config_create(argv[1]);
	strcpy(umvip, config_get_string_value(config, "UMV_IP"));
	strcpy(kernelip, config_get_string_value(config, "KERNEL_IP"));
	port_kernel = config_get_int_value(config, "PORT_KERNEL");
	port_umv = config_get_int_value(config, "PORT_UMV");
	config_destroy(config);
	logger = log_create(argv[2], "cpu", true, LOG_LEVEL_INFO);
	log_info(logger, "Se leyo el arch de config y se creo el logger satisfactoriamente.");

	// Me conecto al kernel
	ConectarA(&sockKernel, &port_kernel, kernelip, &kernel_addr, logger);
	log_info(logger, "Conectado al kernel.");

	// Obtengo datos de la umv
	// TODO Creo que tengo que intercambiar mensajes con el kernel para pedir datos de la umv
	ConectarA(&sockUmv, &port_umv, umvip, &umv_addr, logger);
	log_info(logger, "Conectado a la UMV.");

	// Handshake con el kernel
	mensaje.id_proceso = CPU;
	mensaje.tipo = HANDSHAKE;
	strcpy(mensaje.mensaje, "Hola kernel.");
	send(sockKernel, &mensaje, sizeof(t_mensaje), 0);
	recv(sockKernel, &mensaje, sizeof(t_mensaje), 0);

	// Handshake con la UMV
	mensaje.id_proceso = CPU;
	mensaje.tipo = HANDSHAKE;
	strcpy(mensaje.mensaje, "Hola UMV.");
	send(sockUmv, &mensaje, sizeof(t_mensaje), 0);
	recv(sockUmv, &mensaje, sizeof(t_mensaje), 0);

	// Bucle principal del proceso
	while(seguirEjecutando)
	{
		// Recibo el pcb del proceso a ejecutar
		recv(sockKernel, &pcb, sizeof(t_pcb), 0);
		// Preparo mensaje para la UMV
		mensaje.id_proceso = CPU;
		mensaje.tipo = INSTRUCCIONREQUEST;
		mensaje.datosNumericos = (int)pcb.instruction_index.index->start;
		send(sockUmv, &mensaje, sizeof(t_mensaje), 0);
		// Espero la respuesta de la UMV
		recv(sockUmv, &mensaje, sizeof(t_mensaje), 0);
		// Analizo la instruccion
		analizadorLinea(strdup(mensaje.mensaje), &primitivas, &primitivasKernel);
		// Aviso al kernel que termino el quantum
		mensaje.id_proceso = CPU;
		mensaje.tipo = QUANTUMFINISH;
		send(sockKernel, &mensaje, sizeof(t_mensaje), 0);
	}

	// Libero memoria del logger
	log_destroy(logger);

	// Cierro los sockets
	close(sockKernel);
	close(sockUmv);

	return 0;
}


// Definicion de funciones
void ConectarA(int *sock, int *puerto, char *ip, struct sockaddr_in *their_addr, t_log *logger)
{
	// Creo el descriptor para el socket y compruebo errores
	if ((*sock = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		log_info(logger, "Error cuando se crea el socket.");
		exit(1);
	}
	// Asigno las variables de direccion a conectar
	their_addr->sin_family = AF_INET;
	their_addr->sin_port = htons(*puerto);
	their_addr->sin_addr.s_addr = inet_addr(ip);
	memset(&(their_addr->sin_zero), '\0', 8);
	// Conecto el socket y compruebo errores
	if (connect(*sock, (struct sockaddr *)their_addr, sizeof(struct sockaddr)) == -1)
	{
		log_info(logger, "Error conectando el socket.");
		exit(1);
	}
}

t_puntero silverstack_definirVariable(t_nombre_variable var)
{
	t_puntero ptr = 0;
	return ptr;
}

t_puntero silverstack_obtenerPosicionVariable(t_nombre_variable var)
{
	t_puntero ptr = 0;
	return ptr;
}

t_valor_variable silverstack_dereferenciar(t_puntero dir_var)
{
	t_valor_variable valor = 0;
	return valor;
}

void silverstack_asignar(t_puntero dir_var, t_valor_variable valor)
{

}

void silverstack_imprimir(t_valor_variable valor)
{

}

void silverstack_imprimirTexto(char *texto)
{

}

t_valor_variable silverstack_obtenerValorCompartida(t_nombre_compartida varCom)
{
	t_valor_variable valor = 0;
	return valor;
}

void silverstack_entradaSalida(t_nombre_dispositivo dispositivo, int tiempo)
{

}

void silverstack_finalizar()
{

}

t_valor_variable silverstack_asignarValorCompartida(t_nombre_compartida varCom, t_valor_variable valor)
{
	return valor;
}

void silverstack_irAlLabel(t_nombre_etiqueta etiqueta)
{

}

void silverstack_llamarSinRetorno(t_nombre_etiqueta etiqueta)
{

}

void silverstack_llamarConRetorno(t_nombre_etiqueta etiqueta, t_puntero donde_retornar)
{

}

void silverstack_retornar(t_valor_variable valor)
{

}

void silverstack_signal(t_nombre_semaforo identificador_semaforo)
{

}

void silverstack_wait(t_nombre_semaforo identificador_semaforo)
{

}
