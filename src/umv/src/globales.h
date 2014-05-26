/*
 * globales.h
 *
 *  Created on: 26/05/2014
 *      Author: utnso
 */

#ifndef GLOBALES_H_
#define GLOBALES_H_

#include <pthread.h>

pthread_mutex_t semRetardo = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t semAlgoritmo = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t semCompactacion = PTHREAD_MUTEX_INITIALIZER;

#endif /* GLOBALES_H_ */
