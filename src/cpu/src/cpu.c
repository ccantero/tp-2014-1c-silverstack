/*
 * cpu.c
 *
 *  Created on: 23/04/2014
 *      Author: utnso
 */


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
	// Obtengo datos de archivo de configuracion y se crea el logger
	config = config_create("../cpu.config");
	strcpy(umvip, config_get_string_value(config, "UMV_IP"));
	strcpy(kernelip, config_get_string_value(config, "KERNEL_IP"));
	port_kernel = config_get_int_value(config, "PORT_KERNEL");
	port_umv = config_get_int_value(config, "PORT_UMV");
	config_destroy(config);
	logger = log_create("../logcpu.log", "CPU", true, LOG_LEVEL_INFO);
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
	msg_handshake.tipo = CPU;
	send(sockUmv, &msg_handshake, sizeof(t_msg_handshake), 0);
	recv(sockUmv, &msg_handshake, sizeof(t_msg_handshake), 0);
	if (msg_handshake.tipo == UMV)
	{
		log_info(logger, "Handshake con UMV satisfactorio.");
	}
	else
	{
		log_info(logger, "Handshake con UMV erroneo.");
		exit(1);
	}
	int i;
	int quantum;
	char buf[82]; // Variable auxiliar para almacenar la linea de codigo
	recv(sockKernel, &mensaje, sizeof(t_mensaje), 0);
	quantum = mensaje.datosNumericos;
	// Bucle principal del proceso
	while(seguirEjecutando)
	{
		// Recibo el pcb del kernel
		recv(sockKernel, &pcb, sizeof(t_pcb), 0);
		for (i = 0; i < quantum; i++)
		{
			// Preparo mensaje para la UMV
			msg_solicitud_bytes.base = pcb.code_segment;
			msg_solicitud_bytes.offset = pcb.instruction_index;
			msg_solicitud_bytes.tamanio = pcb.instruction_index + 4;
			msg_cambio_proceso_activo.id_programa = pcb.unique_id;
			mensaje.tipo = SOLICITUDBYTES;
			send(sockUmv, &mensaje, sizeof(t_mensaje), 0);
			send(sockUmv, &msg_cambio_proceso_activo, sizeof(t_msg_cambio_proceso_activo), 0);
			send(sockUmv, &msg_solicitud_bytes, sizeof(t_msg_solicitud_bytes), 0);
			// Espero la respuesta de la UMV
			recv(sockUmv, &mensaje, sizeof(t_mensaje), 0);
			recv(sockUmv, &buf, pcb.instruction_index + 4, 0);
			// Analizo la instruccion y ejecuto primitivas necesarias
			analizadorLinea(strdup(buf), &primitivas, &primitivasKernel);
			// Actualizo el pcb
			pcb.program_counter++;
			pcb.instruction_index += 8;
		}
		// Aviso al kernel que termino el quantum del proceso y devuelvo pcb actualizado
		mensaje.id_proceso = CPU;
		mensaje.tipo = QUANTUMFINISH;
		send(sockKernel, &mensaje, sizeof(t_mensaje), 0);
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
	// 3) Guardar contexto actual en el pcb
	// 4) Retornar la posicion de la variable
	t_puntero ptr;
	char buffer;
	char buffaux[5];
	msg_solicitud_bytes.base = pcb.stack_pointer;
	msg_solicitud_bytes.offset = pcb.context_actual;
	msg_solicitud_bytes.tamanio = 5;
	msg_cambio_proceso_activo.id_programa = pcb.unique_id;
	send(sockUmv, &msg_cambio_proceso_activo, sizeof(t_msg_cambio_proceso_activo), 0);
	send(sockUmv, &msg_solicitud_bytes, sizeof(t_msg_solicitud_bytes), 0);
	recv(sockUmv, &mensaje, sizeof(t_mensaje), 0);
	recv(sockUmv, &buffaux, sizeof(buffaux), 0);
	msg_envio_bytes.base = pcb.stack_pointer;
	msg_envio_bytes.offset = pcb.context_actual;
	msg_envio_bytes.tamanio = 5;
	send(sockUmv, &msg_cambio_proceso_activo, sizeof(t_msg_cambio_proceso_activo), 0);
	send(sockUmv, &msg_envio_bytes, sizeof(t_msg_envio_bytes), 0);
	send(sockUmv, &buffer, sizeof(buffer), 0);
	recv(sockUmv, &msg_envio_bytes, sizeof(t_msg_envio_bytes), 0);
	ptr = pcb.stack_pointer + pcb.context_actual;
	pcb.context_actual += 5;
	return ptr;
}

t_puntero silverstack_obtenerPosicionVariable(t_nombre_variable var)
{
	// 1) Pedir a la UMV la posicion de la variable var
	// 2) Calcular el desplazamiento respecto del stack
	t_puntero ptr = 0;
	char buffer[5];
	int offset = 0;
	// Busco la variable en el diccionario de variables en el stack
	int no_encontre = 1;
	while(no_encontre)
	{
		msg_solicitud_bytes.base = pcb.stack_pointer;
		msg_solicitud_bytes.offset = offset;
		msg_solicitud_bytes.tamanio = 5;
		msg_cambio_proceso_activo.id_programa = pcb.unique_id;
		send(sockUmv, &msg_cambio_proceso_activo, sizeof(t_msg_cambio_proceso_activo), 0);
		send(sockUmv, &msg_solicitud_bytes, sizeof(t_msg_solicitud_bytes), 0);
		recv(sockUmv, &mensaje, sizeof(t_mensaje), 0);
		recv(sockUmv, &buffer, sizeof(buffer), 0);
		if (buffer[0] == var)
		{
			no_encontre = 0;
		}
		else
		{
			offset += 5;
		}
	}
	ptr = msg_solicitud_bytes.base + offset;
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
	int buffer;
	msg_envio_bytes.base = pcb.stack_pointer;
	msg_envio_bytes.offset = dir_var - pcb.stack_pointer + 1;
	msg_envio_bytes.tamanio = 4;
	msg_cambio_proceso_activo.id_programa = pcb.unique_id;
	send(sockUmv, &msg_cambio_proceso_activo, sizeof(t_msg_cambio_proceso_activo), 0);
	send(sockUmv, &msg_envio_bytes, sizeof(t_msg_envio_bytes), 0);
	buffer = valor;
	send(sockUmv, &buffer, sizeof(buffer), 0);
	recv(sockUmv, &msg_envio_bytes, sizeof(t_msg_envio_bytes), 0);
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
	strcpy(msg.mensaje, varCom);
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
	strcpy(msg.mensaje, varCom);
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
