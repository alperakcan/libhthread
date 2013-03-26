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

static void * worker (void *arg)
{
	(void) arg;
	return NULL;
}

int main (int argc, char *argv[])
{
	int rc;
	pthread_t thread;
	(void) argc;
	(void) argv;
	rc = pthread_create(&thread, NULL, worker, NULL);
	if (rc != 0) {
		fprintf(stderr, "pthread_create failed\n");
		exit(-1);
	}
	rc = pthread_join(thread, NULL);
	if (rc != 0) {
		fprintf(stderr, "pthread_join failed\n");
		exit(-1);
	}
	return 0;
}
