/*
 * Alper Akcan - 03.10.2009
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
	rc = pthread_join(thread, NULL);
	if (rc != 0) {
		fprintf(stderr, "pthread_join failed\n");
		exit(-1);
	}
	return 0;
}
