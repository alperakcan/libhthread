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
	int rc;
	pthread_mutex_t *m;
	m = arg;
	rc = pthread_mutex_lock(m);
	if (rc != 0) {
		fprintf(stderr, "pthread_mutex_lock failed\n");
		exit(-1);
	}
	rc = pthread_mutex_unlock(m);
	if (rc != 0) {
		fprintf(stderr, "pthread_mutex_lock failed\n");
		exit(-1);
	}
	return NULL;
}

int main (int argc, char *argv[])
{
	int rc;
	pthread_t t;
	pthread_mutex_t m;
	(void) argc;
	(void) argv;
	rc = pthread_mutex_init(&m, NULL);
	if (rc != 0) {
		fprintf(stderr, "pthread_mutex_init failed\n");
		exit(-1);
	}
	rc = pthread_mutex_lock(&m);
	if (rc != 0) {
		fprintf(stderr, "pthread_mutex_lock failed\n");
		exit(-1);
	}
	rc = pthread_create(&t, NULL, worker, &m);
	if (rc != 0) {
		fprintf(stderr, "pthread_create failed\n");
		exit(-1);
	}
	rc = pthread_mutex_unlock(&m);
	if (rc != 0) {
		fprintf(stderr, "pthread_mutex_lock failed\n");
		exit(-1);
	}
	rc = pthread_join(t, NULL);
	if (rc != 0) {
		fprintf(stderr, "pthread_join failed\n");
		exit(-1);
	}
	rc = pthread_mutex_destroy(&m);
	if (rc != 0) {
		fprintf(stderr, "pthread_mutex_destroy failed\n");
		exit(-1);
	}
	return 0;
}
