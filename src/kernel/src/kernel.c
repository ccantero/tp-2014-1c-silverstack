/*
 * kernel.c
 *
 *  Created on: 26/04/2014
 *  Author: SilverStack
 *	Compiler Include: Path to commons lib
 *  Linker Command: gcc -lpthread -pthread
 *  Linker Library: -commons
 */

/* Header File */

#include "protocol.h"

int main(int argc, char *argv[])
{
	int i,fdmax; /* for aux */

	int sock_program = 0, sock_cpu = 0;
	int new_socket;
	fd_set read_fds;
	fd_set master;

	pthread_t th_plp;
	pthread_t th_pcp;

	char* buffer;
	if ( (buffer = (char*) malloc (sizeof(char) * MAXDATASIZE) )== NULL  )
	{
	    printf("ERROR, No se pudo pedir memoria\n");
	    return -1;
	 }

	if(argc != 2)
	{
		printf("ERROR, la sintaxis de kernel es: ./kernel archivo_configuracion \n");
		return -1;
	}

	logger = log_create("Log.txt", "Kernel",false, LOG_LEVEL_INFO);
	list_io = list_create();
	list_segment = list_create();
	list_semaphores = list_create();
	list_pcb_new = list_create();
	list_pcb_ready = list_create();
	list_pcb_execute = list_create();
	list_pcb_blocked = list_create();
	list_globales = list_create();
	list_pcb_exit = list_create();
	list_cpu = list_create();
	list_process = list_create();

	queue_rr = queue_create();

	process_Id = 10000;
	cantidad_cpu = 0;
	cantidad_procesos_sistema = 0;

	sem_init(&mutex_new_queue, 0, 1);
	sem_init(&mutex_ready_queue, 0, 1);
	sem_init(&mutex_execute_queue, 0, 1);
	sem_init(&mutex_block_queue, 0, 1);
	sem_init(&mutex_exit_queue, 0, 1);

	sem_init(&mutex_process_list, 0, 1);

	sem_init(&free_io_queue, 0, 1);


	sem_init(&sem_plp, 0, 0); // Empieza en cero porque tiene que bloquearse hasta que aparezca algo
	sem_init(&sem_pcp, 0, 0); // Empieza en cero porque tiene que bloquearse hasta que aparezca algo
	sem_init(&sem_cpu_list, 0, 0); // Empieza en cero porque tiene que bloquearse hasta que un cpu se conecte
	sem_init(&mutex_cpu_list, 0, 1);

	if (pthread_mutex_init(&mutex_pedidos, NULL) != 0)
	{
		printf("ERROR - No se pudo inicializar semaforo mutex_pedidos\n");
		return 1;
	}

	GetInfoConfFile(argv[1]);

	signal(SIGINT,depurar);

	pthread_create(&th_plp,NULL,(void*)planificador_sjn,NULL);
	pthread_create(&th_pcp,NULL,(void*)planificador_rr,NULL);

	sock_umv = conectar_umv();
	sock_cpu = servidor_CPU();
	sock_program = servidor_Programa();

	if(sock_umv == -1 || sock_program == -1 || sock_cpu == -1)
	{
		printf("ERROR, Alguno de los sockets principales no pudo iniciar. \n");
		printf("ERROR, sock_umv = %d \n", sock_umv);
		printf("ERROR, sock_program = %d \n", sock_program);
		printf("ERROR, sock_cpu = %d \n", sock_cpu);
		return -1;
	}

	FD_ZERO (&master);
	FD_ZERO (&read_fds);

	FD_SET (sock_program, &master);
	FD_SET (sock_umv, &master);
	FD_SET (sock_cpu, &master);

	fdmax = buscar_Mayor(sock_program, sock_umv, sock_cpu);
	//log_info(logger, sock_umv = %d \n", sock_umv);
	for(;;)
	{
		read_fds = master;
		if (select(fdmax + 1, &read_fds, NULL, NULL, NULL) == -1)
		{
			log_error(logger, "Error en funcion select");
			exit(1);
		}

		for (i = 0; i <= fdmax; i++)
		{
			if (FD_ISSET(i, &read_fds))
			{
				if(i == sock_umv)
				{
					//escuchar_umv();
					log_error(logger,"Se cayo la UMV");
					close(sock_umv);
					continue;
				}

				if (i == sock_program)
				{
					new_socket = escuchar_Nuevo_Programa(sock_program);
					if(new_socket == -1)
					{
						continue;
					}

					FD_SET(new_socket, &master);
					if(fdmax < new_socket)
						fdmax = new_socket;

					continue;
				}

				if (i == sock_cpu)
				{
					new_socket = escuchar_Nuevo_cpu(i);

					if(new_socket == -1)
					{
						//FD_CLR(i, &descriptoresLectura);
						log_error(logger,"No se pudo agregar una cpu");
						continue;
					}

					FD_SET(new_socket, &master);
					cantidad_cpu++;
					sem_wait(&mutex_cpu_list);
					list_add(list_cpu, cpu_create(new_socket));
					sem_post(&mutex_cpu_list);

					log_info(logger,"Se agrego un cpu a la lista de CPU socket %d", new_socket);

					if(cantidad_cpu < multiprogramacion)
					{	sem_post(&sem_cpu_list); // Hay un nuevo CPU Disponible
					}

					if(fdmax < new_socket)
						fdmax = new_socket;

					continue;
				}

				if(is_Connected_Program(i) == 0)
				{
					log_info(logger,"Select detecto actividad en Programa Existente");
					if(escuchar_Programa(i, buffer) == -1)
					{
						close(i);
						process_remove_by_socket(i);
						log_info(logger,"Se removio un program de la lista de Programas");
						FD_CLR(i, &master);
					}
					continue;
				}

				if(is_Connected_CPU(i) != 0)
				{
					if(escuchar_cpu(i) == -1)
					{
						close(i);
						FD_CLR(i, &master);
						cpu_remove(i);
						sem_wait(&sem_cpu_list); // Hay un CPU Disponible menos
						log_info(logger,"Se removio un cpu de la lista de CPU");
						// TODO: ¿Abortar Programa en ejecucion?
						continue;
					}
					continue;
				}
			}
		} /* for (i = 0; i <= fdmax; i++) */
	}/* for(;;) */


	pause();
	log_destroy(logger);
	return 1;
}
