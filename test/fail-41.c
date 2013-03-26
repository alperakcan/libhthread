/*
 *  Copyright (c) 2009-2013 Alper Akcan <alper.akcan@gmail.com>
 *
 * This program is free software. It comes without any warranty, to
 * the extent permitted by applicable law. You can redistribute it
 * and/or modify it under the terms of the Do What The Fuck You Want
 * To Public License, Version 2, as published by Sam Hocevar. See
 * http://www.wtfpl.net/ for more details.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <pthread.h>
#include <errno.h>

int main (int argc, char *argv[])
{
	int rc;
	pthread_t thread;
	(void) argc;
	(void) argv;
	thread = (pthread_t) argv;
	rc = pthread_detach(thread);
	if (rc != 0) {
		fprintf(stderr, "pthread_detach failed\n");
		exit(-1);
	}
	return 0;
}
