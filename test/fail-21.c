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
	pthread_cond_t c;
	(void) argc;
	(void) argv;
	c = *((pthread_cond_t *) &argv);
	rc = pthread_cond_signal(&c);
	if (rc != 0) {
		fprintf(stderr, "pthread_cond_signal failed\n");
		exit(-1);
	}
	rc = pthread_cond_destroy(&c);
	if (rc != 0) {
		fprintf(stderr, "pthread_cond_destroy failed\n");
		exit(-1);
	}
	return 0;
}
