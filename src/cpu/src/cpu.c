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
	/*
	Reserva en el Contexto de Ejecución Actual el espacio necesario para una variable llamada
	var y la registra tanto en el Stack como en el Diccionario de Variables,
	retornando la posición del valor de esta nueva variable del stack
	El valor de la variable queda indefinido: no deberá inicializarlo con ningún valor default.
	Esta función se invoca una vez por variable, a pesar de que este varias veces en una línea. Por
	ejemplo, evaluar "variables a, b, c" llamará tres veces a esta función con los parámetros "a",
	"b" y "c"
	*/
	t_puntero ptr = 0;
	return ptr;
}

t_puntero silverstack_obtenerPosicionVariable(t_nombre_variable var)
{
	/*
	Devuelve el desplazamiento respecto al inicio del segmento Stack en que se encuentra el valor
	de la variable var del contexto actual. En caso de error, retorna -1.
	*/
	t_puntero ptr = 0;
	return ptr;
}

t_valor_variable silverstack_dereferenciar(t_puntero dir_var)
{
	/*
	Obtiene el valor resultante de leer a partir de dir_var, sin importar cual fuera el
	contexto actual.
	*/
	t_valor_variable valor = 0;
	return valor;
}

void silverstack_asignar(t_puntero dir_var, t_valor_variable valor)
{
	/*
	Copia un valor en la variable ubicada en dir_var.
	*/
}

void silverstack_imprimir(t_valor_variable valor)
{
	/*
	Envía al Kernel el contenido de valor, para que este le reenvíe a la correspondiente
	consola del Programa en ejecución. Devuelve la cantidad de dígitos impresos.
	*/
}

void silverstack_imprimirTexto(char *texto)
{
	/*
	Envía al Kernel una cadena de texto para que este la reenvíe a la correspondiente consola del
	Programa en ejecución. No admite parámetros adicionales, secuencias de escape o variables.
	Devuelve la cantidad de dígitos impresos.
	*/
}

t_valor_variable silverstack_obtenerValorCompartida(t_nombre_compartida varCom)
{
	/*
	Solicita al kernel el valor de una variable compartida.
	*/
	t_valor_variable valor = 0;
	return valor;
}

void silverstack_entradaSalida(t_nombre_dispositivo dispositivo, int tiempo)
{
	/*
	Informa al kernel que el Programa actual pretende utilizar el dispositivo durante tiempo
	unidades de tiempo.
	*/
}

void silverstack_finalizar()
{
	/*
	Cambia el Contexto de Ejecución Actual para volver al Contexto anterior al que se está
	ejecutando, recuperando el Cursor de Contexto Actual y el Program Counter previamente
	apilados en el Stack. En caso de estar finalizando el Contexto principal (el ubicado al inicio del
	Stack), deberá finalizar la ejecución del programa devolviendo el valor -1.
	*/
}

t_valor_variable silverstack_asignarValorCompartida(t_nombre_compartida varCom, t_valor_variable valor)
{
	/*
	Solicita al kernel asignar el valor a la variable compartida. Devuelve el valor asignado.
	*/
	return valor;
}

void silverstack_irAlLabel(t_nombre_etiqueta etiqueta)
{
	/*
	Devuelve el número de la primer instrucción ejecutable de etiqueta y -1 en caso de error.
	*/
}

void silverstack_llamarSinRetorno(t_nombre_etiqueta etiqueta)
{
	/*
	Preserva el contexto de ejecución actual para poder retornar luego. Modifica las estructuras
	correspondientes para mostrar un nuevo contexto vacío. Retorna el numero de instrucción a
	ejecutar.
	Los parámetros serán definidos luego de esta instrucción de la misma manera que una variable
	local, con identificadores numéricos empezando por el 0.
	*/
}

void silverstack_llamarConRetorno(t_nombre_etiqueta etiqueta, t_puntero donde_retornar)
{
	/*
	Preserva el contexto de ejecución actual para poder retornar luego al mismo, junto con la
	posicion de la variable entregada por donde_retornar . Modifica las estructuras
	correspondientes para mostrar un nuevo contexto vacío. Retorna el número de instrucción a
	ejecutar.
	Los parámetros serán definidos luego de esta instrucción de la misma manera que una variable
	local, con identificadores numéricos empezando por el 0.
	No se pretende que se pueda retornar a una variable global. Sí a un parámetro o variable local
	*/
}

void silverstack_retornar(t_valor_variable valor)
{
	/*
	Modifica el Contexto de Ejecución Actual por el Contexto anterior al que se está ejecutando,
	recuperando el Cursor de Contexto Actual, el Program Counter y la direccion donde retornar,
	asignando el valor de retorno en esta, previamente apilados en el Stack.
	*/
}

void silverstack_signal(t_nombre_semaforo identificador_semaforo)
{
	/*
	Comunica al kernel que ejecute la función signal para el semáforo con el nombre
	identificador_semaforo. El kernel decidirá si esto conlleva desbloquear a otros procesos.
	*/
}

void silverstack_wait(t_nombre_semaforo identificador_semaforo)
{
	/*
	Informa al kernel que ejecute la función wait para el semáforo con el nombre
	identificador_semaforo. El kernel deberá decidir si bloquearlo o no.
	*/
}
