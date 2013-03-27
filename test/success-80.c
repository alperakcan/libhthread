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

int main (int argc, char *argv[])
{
	void *rc;
	(void) argc;
	(void) argv;
	rc = malloc(1024);
	if (rc == NULL) {
		fprintf(stderr, "malloc failed\n");
		exit(-1);
	}
	free(rc);
	rc = calloc(1, 1024);
	if (rc == NULL) {
		fprintf(stderr, "calloc failed\n");
		exit(-1);
	}
	free(rc);
	rc = realloc(NULL, 1024);
	if (rc == NULL) {
		fprintf(stderr, "realloc failed\n");
		exit(-1);
	}
	free(rc);
	rc = realloc(NULL, 1024);
	if (rc == NULL) {
		fprintf(stderr, "realloc failed\n");
		exit(-1);
	}
	rc = realloc(rc, 2048);
	if (rc == NULL) {
		fprintf(stderr, "realloc failed\n");
		exit(-1);
	}
	free(rc);
	return 0;
}