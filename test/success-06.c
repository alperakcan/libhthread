/*
 * Alper Akcan - 03.10.2009
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
