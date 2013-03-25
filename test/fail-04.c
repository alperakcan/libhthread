/*
 * Alper Akcan - 03.10.2009
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

#define size(a) ((int) (sizeof(a) / sizeof(a[0])))

int main (int argc, char *argv[])
{
	int rc;
	int i;
	pthread_mutex_t m[5];
	(void) argc;
	(void) argv;
	for (i = 0; i < size(m); i++) {
		rc = pthread_mutex_init(&m[i], NULL);
		if (rc != 0) {
			fprintf(stderr, "pthread_mutex_init failed\n");
			exit(-1);
		}
	}
	for (i = 0; i < size(m); i++) {
		rc = pthread_mutex_lock(&m[i]);
		if (rc != 0) {
			fprintf(stderr, "pthread_mutex_lock failed\n");
			exit(-1);
		}
	}
	for (i = 0; i < size(m); i++) {
		rc = pthread_mutex_unlock(&m[i]);
		if (rc != 0) {
			fprintf(stderr, "pthread_mutex_unlock failed\n");
			exit(-1);
		}
	}
	for (i = size(m) - 1; i >= 0; i--) {
		rc = pthread_mutex_lock(&m[i]);
		if (rc != 0) {
			fprintf(stderr, "pthread_mutex_lock failed\n");
			exit(-1);
		}
	}
	for (i = size(m) - 1; i >= 0; i--) {
		rc = pthread_mutex_unlock(&m[i]);
		if (rc != 0) {
			fprintf(stderr, "pthread_mutex_unlock failed\n");
			exit(-1);
		}
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
