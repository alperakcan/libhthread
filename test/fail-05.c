/*
 * Alper Akcan - 03.10.2009
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

int main (int argc, char *argv[])
{
	int rc;
	pthread_mutex_t m;
	(void) argc;
	(void) argv;
	rc = pthread_mutex_init(&m, NULL);
	if (rc != 0) {
		fprintf(stderr, "pthread_mutex_init failed\n");
		exit(-1);
	}
	rc = pthread_mutex_unlock(&m);
	if (rc != 0) {
		fprintf(stderr, "pthread_mutex_lock failed\n");
		exit(-1);
	}
	rc = pthread_mutex_destroy(&m);
	if (rc != 0) {
		fprintf(stderr, "pthread_mutex_destroy failed\n");
		exit(-1);
	}
	return 0;
}
