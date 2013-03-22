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
	m = *((pthread_mutex_t *) &argv);
	rc = pthread_mutex_destroy(&m);
	if (rc != 0) {
		fprintf(stderr, "pthread_mutex_destroy failed\n");
		exit(-1);
	}
	return 0;
}
