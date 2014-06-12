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
	int salir_bucle = 0;
	int inicio_instruccion = 0;
	int cantidad_letras_instruccion = 0;
	int pos_en_instruccion = 0;
	char buf[82]; // Variable auxiliar para almacenar la linea de codigo
	char instruccion[82];
	int bufferaux[2];
	recv(sockKernel, &mensaje, sizeof(t_mensaje), 0);
	quantum = mensaje.datosNumericos;
	// Bucle principal del proceso
	while(seguirEjecutando)
	{
		// Recibo el pcb del kernel
		recv(sockKernel, &pcb, sizeof(t_pcb), 0);
		log_info(logger,"Recibi PCB de Kernel");
		if(pcb.program_counter == 0)
		{
			pcb.program_counter++;
		}
		for (i = 0; i < quantum; i++)
		{
			if (proceso_bloqueado == 0 && proceso_finalizo == 0)
			{
				salir_bucle = 0;
				inicio_instruccion = 0;
				cantidad_letras_instruccion = 0;
				// Preparo mensaje para la UMV
				msg_solicitud_bytes.base = pcb.instruction_index;
				msg_solicitud_bytes.offset = 0 + ((pcb.program_counter - 1) * 8);
				msg_solicitud_bytes.tamanio = 8;
				msg_cambio_proceso_activo.id_programa = pcb.unique_id;
				mensaje.tipo = SOLICITUDBYTES;
				send(sockUmv, &mensaje, sizeof(t_mensaje), 0);
				log_info(logger, "Se envio solicitud de busqueda de instruccion.");
				send(sockUmv, &msg_cambio_proceso_activo, sizeof(t_msg_cambio_proceso_activo), 0);
				send(sockUmv, &msg_solicitud_bytes, sizeof(t_msg_solicitud_bytes), 0);
				// Espero la respuesta de la UMV
				recv(sockUmv, &mensaje, sizeof(t_mensaje), 0);
				recv(sockUmv, &bufferaux, 8, 0);
				// Preparo mensaje para la UMV
				msg_solicitud_bytes.base = pcb.code_segment;
				msg_solicitud_bytes.offset = bufferaux[0];
				msg_solicitud_bytes.tamanio = bufferaux[1];
				msg_cambio_proceso_activo.id_programa = pcb.unique_id;
				mensaje.tipo = SOLICITUDBYTES;
				log_info(logger, "Se envio solicitud de instruccion.");
				send(sockUmv, &mensaje, sizeof(t_mensaje), 0);
				send(sockUmv, &msg_cambio_proceso_activo, sizeof(t_msg_cambio_proceso_activo), 0);
				send(sockUmv, &msg_solicitud_bytes, sizeof(t_msg_solicitud_bytes), 0);
				// Espero la respuesta de la UMV
				recv(sockUmv, &mensaje, sizeof(t_mensaje), 0);
				recv(sockUmv, &buf, bufferaux[1], 0);
				buf[bufferaux[1]] = '\0';
				// Verifico limites de instruccion
				while (salir_bucle != 1)
				{
					if(buf[inicio_instruccion] == '\t' || buf[inicio_instruccion] == '\0')
					{
						inicio_instruccion++;
					}
					else
					{
						salir_bucle = 1;
					}
				}
				salir_bucle = 0;
				pos_en_instruccion = inicio_instruccion;
				while (salir_bucle != 1)
				{
					if(buf[pos_en_instruccion] != '\n')
					{
						cantidad_letras_instruccion++;
					}
					else
					{
						salir_bucle = 1;
					}
					pos_en_instruccion++;
				}
				memcpy(&instruccion[0], &buf[inicio_instruccion], cantidad_letras_instruccion);
				instruccion[cantidad_letras_instruccion] = '\0';
				// Analizo la instruccion y ejecuto primitivas necesarias
				log_info(logger, "Me llego la instruccion: %s.", instruccion);
				analizadorLinea(strdup(instruccion), &primitivas, &primitivasKernel);
				log_info(logger, "Se termino de procesar la instruccion: %s.", instruccion);
				// Actualizo el pcb
				pcb.program_counter++;
			}
			else
			{
				break;
			}
		}
		// Aviso al kernel que termino el quantum del proceso y devuelvo pcb actualizado
		if (proceso_finalizo == 1)
		{
			mensaje.id_proceso = CPU;
			mensaje.tipo = PROGRAMFINISH;
			send(sockKernel, &mensaje, sizeof(t_mensaje), 0);
			log_info(logger, "Envie PROGRAMFINISH al kernel.");
			send(sockKernel, &pcb, sizeof(t_pcb), 0);
			log_info(logger, "Envie PCB al kernel.");
			proceso_finalizo = 0;
		}
		else
		{
			mensaje.id_proceso = CPU;
			mensaje.tipo = QUANTUMFINISH;
			send(sockKernel, &mensaje, sizeof(t_mensaje), 0);
			log_info(logger,"Envie  QUANTUMFINISH al Kernel");
			send(sockKernel, &pcb, sizeof(t_pcb), 0);
			log_info(logger,"Envie  PCB al Kernel");
		}
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
	char buffaux[5];
	buffaux[0] = var;
	msg_cambio_proceso_activo.id_programa = pcb.unique_id;
	mensaje.tipo = ENVIOBYTES;
	msg_envio_bytes.base = pcb.stack_pointer;
	msg_envio_bytes.offset = 5 * pcb.context_actual;
	msg_envio_bytes.tamanio = 5;
	send(sockUmv, &mensaje, sizeof(t_mensaje), 0);
	send(sockUmv, &msg_cambio_proceso_activo, sizeof(t_msg_cambio_proceso_activo), 0);
	send(sockUmv, &msg_envio_bytes, sizeof(t_msg_envio_bytes), 0);
	send(sockUmv, buffaux, 5, 0);
	recv(sockUmv, &mensaje, sizeof(t_mensaje), 0);
	if (mensaje.tipo == ENVIOBYTES)
	{
		ptr = pcb.stack_pointer + (5 * pcb.context_actual);
		pcb.context_actual++;
	}
	else
	{
		// TODO Verificar errores de segmentation fault
	}
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
		mensaje.tipo = SOLICITUDBYTES;
		send(sockUmv, &mensaje, sizeof(t_mensaje), 0);
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
	// 1) Se lee el valor de la variable almacenada en dir_var
	t_valor_variable valor = 0;
	char buffer[5];
	msg_solicitud_bytes.base = pcb.stack_pointer;
	msg_solicitud_bytes.offset = dir_var - pcb.stack_pointer;
	msg_solicitud_bytes.tamanio = 5;
	msg_cambio_proceso_activo.id_programa = pcb.unique_id;
	mensaje.tipo = SOLICITUDBYTES;
	send(sockUmv, &mensaje, sizeof(t_mensaje), 0);
	send(sockUmv, &msg_cambio_proceso_activo, sizeof(t_msg_cambio_proceso_activo), 0);
	send(sockUmv, &msg_solicitud_bytes, sizeof(t_msg_solicitud_bytes), 0);
	recv(sockUmv, &mensaje, sizeof(t_mensaje), 0);
	recv(sockUmv, &buffer, sizeof(buffer), 0);
	memcpy(&valor, &buffer[1], 4);
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
	mensaje.tipo = ENVIOBYTES;
	msg_cambio_proceso_activo.id_programa = pcb.unique_id;
	send(sockUmv, &mensaje, sizeof(t_mensaje), 0);
	send(sockUmv, &msg_cambio_proceso_activo, sizeof(t_msg_cambio_proceso_activo), 0);
	send(sockUmv, &msg_envio_bytes, sizeof(t_msg_envio_bytes), 0);
	buffer = valor;
	send(sockUmv, &buffer, sizeof(buffer), 0);
	recv(sockUmv, &mensaje, sizeof(t_mensaje), 0);
}

void silverstack_imprimir(t_valor_variable valor)
{
	// 1) Envio al kernel el valor para que lo imprima la correspondiente consola
	log_info(logger, "Comienzo primitiva silverstack_imprimir");
	t_mensaje msg;
	msg.id_proceso = CPU;
	msg.tipo = IMPRIMIR;
	msg.datosNumericos = valor;
	send(sockKernel, &msg, sizeof(t_mensaje), 0);
	recv(sockKernel, &msg, sizeof(t_mensaje), 0);
	log_info(logger, "Fin primitiva silverstack_imprimir");
}

void silverstack_imprimirTexto(char *texto)
{
	// 1) Envio al kernel la cadena de texto para que la reenvíe a la correspondiente consola
	log_info(logger, "Comienzo primitiva silverstack_imprimirTexto");
	t_mensaje msg;
	msg.id_proceso = CPU;
	msg.tipo = IMPRIMIRTEXTO;
	msg.datosNumericos = strlen(texto);
	send(sockKernel, &msg, sizeof(t_mensaje), 0);
	char buf[strlen(texto)];
	strcpy(buf, texto);
	send(sockKernel, buf, sizeof(buf), 0);
	recv(sockKernel, &msg, sizeof(t_mensaje), 0);
	log_info(logger, "Fin primitiva silverstack_imprimirTexto");
}

t_valor_variable silverstack_obtenerValorCompartida(t_nombre_compartida varCom)
{
	// 1) Solicito al kernel el valor de la variable varCom
	// 2) Devuelvo el valor recibido
	log_info(logger, "Comienzo primitiva silverstack_obtenerValorCompartida");
	t_valor_variable valor = 0;
	t_mensaje msg;
	msg.id_proceso = CPU;
	msg.tipo = VARCOMREQUEST;
	strcpy(msg.mensaje, varCom);
	send(sockKernel, &msg, sizeof(t_mensaje), 0);
	recv(sockKernel, &msg, sizeof(t_mensaje), 0);
	valor = msg.datosNumericos;
	log_info(logger, "Fin primitiva silverstack_obtenerValorCompartida");
	return valor;
}

void silverstack_entradaSalida(t_nombre_dispositivo dispositivo, int tiempo)
{
	// 1) Envio al kernel el tiempo de entrada/salida de dispositivo
	log_info(logger, "Comienzo primitiva silverstack_entradaSalida");
	t_mensaje msg;
	msg.id_proceso = CPU;
	msg.tipo = ENTRADASALIDA;
	msg.datosNumericos = tiempo;
	strcpy(msg.mensaje, dispositivo);
	send(sockKernel, &msg, sizeof(t_mensaje), 0);
	recv(sockKernel, &msg, sizeof(t_mensaje), 0);
	proceso_bloqueado = 1;
	log_info(logger, "Fin primitiva silverstack_entradaSalida");
}

void silverstack_finalizar()
{
	/*
	Cambia el Contexto de Ejecución Actual para volver al Contexto anterior al que se está
	ejecutando, recuperando el Cursor de Contexto Actual y el Program Counter previamente
	apilados en el Stack. En caso de estar finalizando el Contexto principal (el ubicado al inicio del
	Stack), deberá finalizar la ejecución del programa devolviendo el valor -1.
	*/
	if (pcb.stack_pointer == pcb.stack_segment)
	{
		proceso_finalizo = 1;
	}
	else
	{
		// TODO Verificar cuando se sale de contexto de funcion o procedimiento
	}
}

t_valor_variable silverstack_asignarValorCompartida(t_nombre_compartida varCom, t_valor_variable valor)
{
	// 1) Envio al kernel el valor de la variable compartida a asignar
	// 2) Devuelvo el valor asignado
	log_info(logger, "Comienzo primitiva silverstack_asignarValorCompartida");
	t_mensaje msg;
	msg.id_proceso = CPU;
	msg.tipo = ASIGNACION;
	msg.datosNumericos = valor;
	strcpy(msg.mensaje, varCom);
	send(sockKernel, &msg, sizeof(t_mensaje), 0);
	recv(sockKernel, &msg, sizeof(t_mensaje), 0);
	log_info(logger, "Fin primitiva silverstack_asignarValorCompartida");
	return valor;
}

void silverstack_irAlLabel(t_nombre_etiqueta etiqueta)
{
	/*
	Devuelve el número de la primer instrucción ejecutable de etiqueta y -1 en caso de error.
	*/
	int salir = 0;
	int dir_instruccion;
	char buffer[pcb.size_etiquetas_index];
	msg_solicitud_bytes.base = pcb.etiquetas_index;
	msg_solicitud_bytes.offset = 0;
	msg_solicitud_bytes.tamanio = pcb.size_etiquetas_index;
	msg_cambio_proceso_activo.id_programa = pcb.unique_id;
	mensaje.tipo = SOLICITUDBYTES;
	send(sockUmv, &mensaje, sizeof(t_mensaje), 0);
	send(sockUmv, &msg_cambio_proceso_activo, sizeof(t_msg_cambio_proceso_activo), 0);
	send(sockUmv, &msg_solicitud_bytes, sizeof(t_msg_solicitud_bytes), 0);
	recv(sockUmv, &mensaje, sizeof(t_mensaje), 0);
	recv(sockUmv, &buffer, sizeof(buffer), 0);
	dir_instruccion = metadata_buscar_etiqueta(etiqueta, buffer, pcb.size_etiquetas_index);
	pcb.program_counter = dir_instruccion;
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
	log_info(logger, "Comienzo primitiva silverstack_llamarSinRetorno");

	log_info(logger, "Fin primitiva silverstack_llamarSinRetorno");
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
	log_info(logger, "Comienzo primitiva silverstack_llamarConRetorno");

	log_info(logger, "Fin primitiva silverstack_llamarConRetorno");
}

void silverstack_retornar(t_valor_variable valor)
{
	/*
	Modifica el Contexto de Ejecución Actual por el Contexto anterior al que se está ejecutando,
	recuperando el Cursor de Contexto Actual, el Program Counter y la direccion donde retornar,
	asignando el valor de retorno en esta, previamente apilados en el Stack.
	*/
	log_info(logger, "Comienzo primitiva silverstack_retornar");

	log_info(logger, "Fin primitiva silverstack_retornar");
}

void silverstack_signal(t_nombre_semaforo identificador_semaforo)
{
	// 1) Envio al kernel el semaforo para que ejecute signal en él
	log_info(logger, "Comienzo primitiva silverstack_signal");
	t_mensaje msg;
	msg.id_proceso = CPU;
	msg.tipo = SIGNALSEM;
	strcpy(msg.mensaje, identificador_semaforo);
	send(sockKernel, &msg, sizeof(t_mensaje), 0);
	recv(sockKernel, &msg, sizeof(t_mensaje), 0);
	log_info(logger, "Fin primitiva silverstack_signal");
}

void silverstack_wait(t_nombre_semaforo identificador_semaforo)
{
	// 1) Envio al kernel el semaforo para que se ejecute wait en él
	log_info(logger, "Comienzo primitiva silverstack_wait");
	t_mensaje msg;
	msg.id_proceso = CPU;
	msg.tipo = WAITSEM;
	strcpy(msg.mensaje, identificador_semaforo);
	send(sockKernel, &msg, sizeof(t_mensaje), 0);
	recv(sockKernel, &msg, sizeof(t_mensaje), 0);
	if (msg.tipo == BLOCK)
	{
		proceso_bloqueado = 1;
	}
	log_info(logger, "Fin primitiva silverstack_wait");
}
