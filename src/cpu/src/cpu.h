/*
 * cpu.h
 *
 *  Created on: 23/04/2014
 *      Author: utnso
 */

#ifndef CPU_H_
#define CPU_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>
#include <commons/log.h>
#include <commons/config.h>
#include <parser/parser.h>
#include <errno.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>

t_log *logger;
t_config *config;

int port_kernel;
int port_umv;
int sockPrin;
char myip[16];
char kernelip[16];
char umvip[16];

int sockKernel;
int sockUmv;
int numBytes;
char buf[256];
struct sockaddr_in their_addr;

void GetInfoConfFile(t_config *config, char *path);
void ConectarA(int sock);
t_puntero silverstack_definirVariable(t_nombre_variable var);
t_puntero silverstack_obtenerPosicionVariable(t_nombre_variable var);
t_valor_variable silverstack_dereferenciar(t_puntero dir_var);
void silverstack_asignar(t_puntero dir_var, t_valor_variable valor);
void silverstack_imprimir(t_valor_variable valor);
void silverstack_imprimirTexto(char *texto);
t_valor_variable silverstack_obtenerValorCompartida(t_nombre_compartida varCom);
void silverstack_entradaSalida(t_nombre_dispositivo dispositivo, int tiempo);
void silverstack_finalizar();
t_valor_variable silverstack_asignarValorCompartida(t_nombre_compartida varCom, t_valor_variable valor);
void silverstack_irAlLabel(t_nombre_etiqueta etiqueta);
void silverstack_llamarSinRetorno(t_nombre_etiqueta etiqueta);
void silverstack_llamarConRetorno(t_nombre_etiqueta etiqueta, t_puntero donde_retornar);
void silverstack_retornar(t_valor_variable valor);
void silverstack_signal(t_nombre_semaforo identificador_semaforo);
void silverstack_wait(t_nombre_semaforo identificador_semaforo);

#endif /* CPU_H_ */
