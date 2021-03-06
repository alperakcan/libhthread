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

#define size(a) ((int) (sizeof(a) / sizeof(a[0])))

int main (int argc, char *argv[])
{
	int i;
	int rc;
	int msec;
	pthread_cond_t c;
	pthread_mutex_t m[2];
        struct timeval tval;
        struct timespec tspec;
	(void) argc;
	(void) argv;
	for (i = 0; i < size(m); i++) {
		rc = pthread_mutex_init(&m[i], NULL);
		if (rc != 0) {
			fprintf(stderr, "pthread_mutex_init failed\n");
			exit(-1);
		}
	}
	rc = pthread_cond_init(&c, NULL);
	if (rc != 0) {
		fprintf(stderr, "pthread_cond_init failed\n");
		exit(-1);
	}
	rc = pthread_mutex_lock(&m[0]);
	if (rc != 0) {
		fprintf(stderr, "pthread_mutex_lock failed\n");
		exit(-1);
	}
	msec = 1000;
	gettimeofday(&tval, NULL);
	tspec.tv_sec = tval.tv_sec + (msec / 1000);
	tspec.tv_nsec = (tval.tv_usec + ((msec % 1000) * 1000)) * 1000;
	rc = pthread_cond_timedwait(&c, &m[1], &tspec);
	if (rc != ETIMEDOUT) {
		fprintf(stderr, "pthread_cond_timedwait failed (rc: %d)\n", rc);
		exit(-1);
	}
	rc = pthread_mutex_unlock(&m[0]);
	if (rc != 0) {
		fprintf(stderr, "pthread_mutex_unlock failed\n");
		exit(-1);
	}
	rc = pthread_cond_destroy(&c);
	if (rc != 0) {
		fprintf(stderr, "pthread_cond_destroy failed\n");
		exit(-1);
	}
	for (i = 0; i < size(m); i++) {
		rc = pthread_mutex_destroy(&m[i]);
		if (rc != 0) {
			fprintf(stderr, "pthread_mutex_destroy failed\n");
			exit(-1);
		}
	}
	return 0;
}
