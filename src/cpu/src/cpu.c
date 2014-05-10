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
	int seguirEjecutando = 1; // Mediante la señal SIGUSR1 se puede dejar de ejecutar el cpu
	t_mensaje mensaje;
	diccionario = list_create();

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
	ConectarA(&sockUmv, &port_umv, umvip, &umv_addr, logger);
	log_info(logger, "Conectado a la UMV.");

	// Handshake con el kernel
	mensaje.id_proceso = CPU;
	mensaje.tipo = HANDSHAKE;
	strcpy(mensaje.mensaje, "Hola kernel.");
	send(sockKernel, &mensaje, sizeof(t_mensaje), 0);
	log_info(logger, "Esperando rta de handshake del kernel.");
	recv(sockKernel, &mensaje, sizeof(t_mensaje), 0);
	if (mensaje.tipo == HANDSHAKEOK)
	{
		log_info(logger, "Handshake con kernel satisfactorio.");
	}
	else
	{
		log_info(logger, "Handshake con kernel erroneo.");
		exit(1);
	}

	// Handshake con la UMV
	mensaje.id_proceso = CPU;
	mensaje.tipo = HANDSHAKE;
	strcpy(mensaje.mensaje, "Hola UMV.");
	send(sockUmv, &mensaje, sizeof(t_mensaje), 0);
	log_info(logger, "Esperando rta de handshake de la umv.");
	recv(sockUmv, &mensaje, sizeof(t_mensaje), 0);
	if (mensaje.tipo == HANDSHAKEOK)
	{
		log_info(logger, "Handshake con UMV satisfactorio.");
	}
	else
	{
		log_info(logger, "Handshake con UMV erroneo.");
		exit(1);
	}

	char buf[82]; // Variable auxiliar para almacenar la linea de codigo
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
		mensaje.datosNumericos = (int)pcb.instruction_index.index->offset;
		send(sockUmv, &mensaje, sizeof(t_mensaje), 0);
		// Espero la respuesta de la UMV
		recv(sockUmv, &mensaje, sizeof(t_mensaje), 0);
		recv(sockUmv, buf, mensaje.datosNumericos, 0); // Recibo la instruccion en el buf
		// Analizo la instruccion
		analizadorLinea(strdup(buf), &primitivas, &primitivasKernel);
		// Aviso al kernel que termino el quantum del proceso y devuelvo pcb actualizado
		mensaje.id_proceso = CPU;
		mensaje.tipo = QUANTUMFINISH;
		send(sockKernel, &mensaje, sizeof(t_mensaje), 0);
		pcb.program_counter++;
		pcb.instruction_index.index = pcb.instruction_index.index + 8;
		send(sockKernel, &pcb, sizeof(t_pcb), 0);
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
	// 1) Reservar espacio en memoria para la variable
	// 2) Registrar variable en el Stack
	// 3) Registrar variable en el dicionario de variables
	// 4) Guardar contexto actual en el pcb
	// 5) Retornar la posicion de la variable
	// TODO Guardar el contexto en el pcb del programa
	t_mensaje msg;
	t_puntero ptr;
	msg.id_proceso = CPU;
	msg.tipo = MEMORIAREQUEST;
	msg.datosNumericos = 4;
	send(sockUmv, &msg, sizeof(t_mensaje), 0);
	recv(sockUmv, &msg, sizeof(t_mensaje), 0);
	msg.id_proceso = CPU;
	msg.tipo = VARIABLEREQUEST;
	msg.mensaje[0] = var;
	send(sockUmv, &msg, sizeof(t_mensaje), 0);
	recv(sockUmv, &msg, sizeof(t_mensaje), 0);
	ptr = msg.datosNumericos;
	t_variable variable;
	variable.nombre = var;
	variable.direccion = msg.datosNumericos;
	list_add(diccionario, &variable);
	return ptr;
}

t_puntero silverstack_obtenerPosicionVariable(t_nombre_variable var)
{
	// 1) Pedir a la UMV la posicion de la variable var
	// 2) Calcular el desplazamiento respecto del stack
	t_mensaje msg;
	t_puntero ptr = 0;
	int posAux;
	msg.id_proceso = CPU;
	msg.tipo = POSICIONREQUEST;
	msg.mensaje[0] = var;
	send(sockUmv, &msg, sizeof(t_mensaje), 0);
	recv(sockUmv, &msg, sizeof(t_mensaje), 0);
	if (msg.tipo == REQUESTOK)
	{
		posAux = msg.datosNumericos;
	}
	else
	{
		ptr = -1;
		return ptr;
	}
	ptr = posAux - pcb.context_actual;
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
	// 1) Mando a la UMV el valor de la variable junto con su direccion
	// 2) Actualizo diccionario de variables
	t_mensaje msg;
	msg.id_proceso = CPU;
	msg.tipo = ASIGNACION;
	msg.datosNumericos = dir_var;
	send(sockUmv, &msg, sizeof(t_mensaje), 0);
	msg.datosNumericos = valor;
	send(sockUmv, &msg, sizeof(t_mensaje), 0);
	recv(sockUmv, &msg, sizeof(t_mensaje), 0);
	int i;
	t_variable *varAux;
	// Busco la variable en el diccionario para actualizarla
	for (i = 0; i < list_size(diccionario); i++)
	{
		varAux = (t_variable *)list_get(diccionario, i);
		if (varAux->direccion == (int)dir_var)
		{
			break;
		}
	}
	varAux->contenido = valor;
}

void silverstack_imprimir(t_valor_variable valor)
{
	// 1) Envio al kernel el valor para que lo imprima la correspondiente consola
	t_mensaje msg;
	msg.id_proceso = CPU;
	msg.tipo = IMPRIMIR;
	msg.datosNumericos = valor;
	send(sockKernel, &msg, sizeof(t_mensaje), 0);
	recv(sockKernel, &msg, sizeof(t_mensaje), 0);
}

void silverstack_imprimirTexto(char *texto)
{
	// 1) Envio al kernel la cadena de texto para que la reenvíe a la correspondiente consola
	t_mensaje msg;
	msg.id_proceso = CPU;
	msg.tipo = IMPRIMIRTEXTO;
	msg.datosNumericos = strlen(texto);
	send(sockKernel, &msg, sizeof(t_mensaje), 0);
	char buf[strlen(texto)];
	strcpy(buf, texto);
	send(sockKernel, buf, sizeof(buf), 0);
	recv(sockKernel, &msg, sizeof(t_mensaje), 0);
}

t_valor_variable silverstack_obtenerValorCompartida(t_nombre_compartida varCom)
{
	// 1) Solicito al kernel el valor de la variable varCom
	// 2) Devuelvo el valor recibido
	t_valor_variable valor = 0;
	t_mensaje msg;
	msg.id_proceso = CPU;
	msg.tipo = VARCOMREQUEST;
	msg.mensaje[0] = *varCom;
	send(sockKernel, &msg, sizeof(t_mensaje), 0);
	recv(sockKernel, &msg, sizeof(t_mensaje), 0);
	valor = msg.datosNumericos;
	return valor;
}

void silverstack_entradaSalida(t_nombre_dispositivo dispositivo, int tiempo)
{
	// 1) Envio al kernel el tiempo de entrada/salida de dispositivo
	t_mensaje msg;
	msg.id_proceso = CPU;
	msg.tipo = ENTRADASALIDA;
	msg.datosNumericos = tiempo;
	strcpy(msg.mensaje, dispositivo);
	send(sockKernel, &msg, sizeof(t_mensaje), 0);
	recv(sockKernel, &msg, sizeof(t_mensaje), 0);
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
	// 1) Envio al kernel el valor de la variable compartida a asignar
	// 2) Devuelvo el valor asignado
	t_mensaje msg;
	msg.id_proceso = CPU;
	msg.tipo = ASIGNACION;
	msg.datosNumericos = valor;
	msg.mensaje[0] = *varCom;
	send(sockKernel, &msg, sizeof(t_mensaje), 0);
	recv(sockKernel, &msg, sizeof(t_mensaje), 0);
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
	// 1) Envio al kernel el semaforo para que ejecute signal en él
	t_mensaje msg;
	msg.id_proceso = CPU;
	msg.tipo = SIGNALSEM;
	strcpy(msg.mensaje, identificador_semaforo);
	send(sockKernel, &msg, sizeof(t_mensaje), 0);
	recv(sockKernel, &msg, sizeof(t_mensaje), 0);
}

void silverstack_wait(t_nombre_semaforo identificador_semaforo)
{
	// 1) Envio al kernel el semaforo para que se ejecute wait en él
	t_mensaje msg;
	msg.id_proceso = CPU;
	msg.tipo = WAITSEM;
	strcpy(msg.mensaje, identificador_semaforo);
	send(sockKernel, &msg, sizeof(t_mensaje), 0);
	recv(sockKernel, &msg, sizeof(t_mensaje), 0);
}
