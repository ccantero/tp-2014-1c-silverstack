/*
 * program.c
 *
 *  Created on: 13/04/2014
 *  Author: SilverStack
 *  Compiler Include: Path to commons lib
 *  Linker Library: -commons
*/

#include "protocol.h"

int main(void)
{
	logger = log_create("Log.txt", "CPU",false, LOG_LEVEL_INFO);

	GetInfoConfFile();

	return 1;

}
