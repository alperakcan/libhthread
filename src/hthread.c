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
#include <string.h>
#include <sys/time.h>
#include <errno.h>
#include <unistd.h>
#include <pthread.h>
#include <assert.h>
#if defined(__DARWIN__) && (__DARWIN__ == 1)
#include <mach/mach_time.h>
#endif
#if defined(HTHREAD_ENABLE_CALLSTACK) && (HTHREAD_ENABLE_CALLSTACK == 1)
#include <bfd.h>
#include <dlfcn.h>
#include <execinfo.h>
#endif

#define HTHREAD_INTERNAL			1
#define HTHREAD_CALLSTACK_MAX			128

#include "hthread.h"
#include "uthash.h"

static pthread_mutex_t debugf_mutex = PTHREAD_MUTEX_INITIALIZER;

#define hdebug_lock() pthread_mutex_lock(&debugf_mutex);
#define hdebug_unlock() pthread_mutex_unlock(&debugf_mutex);

#if 0
#define hdebugf(a...) { \
	hdebug_lock(); \
	fprintf(stderr, "hthread::debug: "); \
	fprintf(stderr, a); \
	fprintf(stderr, " (%s %s:%d)\n", __FUNCTION__, __FILE__, __LINE__); \
	hdebug_unlock(); \
}
#else
#define hdebugf(a...)
#endif

#define hinfof(a...) { \
	fprintf(stderr, "(hthread:%d) ", getpid()); \
	fprintf(stderr, a); \
	fprintf(stderr, "\n"); \
}

#define herrorf(a...) { \
	hdebug_lock(); \
	fprintf(stderr, "hthread::error: "); \
	fprintf(stderr, a); \
	fprintf(stderr, " (%s %s:%d)\n", __FUNCTION__, __FILE__, __LINE__); \
	hdebug_unlock(); \
}

#define hassert(a) { \
	unsigned int v; \
	v = hthread_getenv_int(HTHREAD_ASSERT_ON_ERROR_NAME); \
	if (v == (unsigned int) -1) { \
		v = HTHREAD_ASSERT_ON_ERROR; \
	} \
	if (v) { \
		assert(a); \
	} else { \
		herrorf(# a); \
	} \
}

#define hassertf(a...) { \
	hdebug_lock(); \
	fprintf(stderr, "hthread::assert: "); \
	fprintf(stderr, a); \
	fprintf(stderr, " (%s %s:%d)\n", __FUNCTION__, __FILE__, __LINE__); \
	assert(0); \
	hdebug_unlock(); \
}

struct hthread_arg {
#if defined(HTHREAD_DEBUG) && (HTHREAD_DEBUG == 1)
	char *name;
#endif
	int flag;
	void *arg;
	void * (*r) (void *);
	void * (*f) (void *);
	const char *func;
	const char *file;
	int line;
	struct hthread_cond *cond;
	struct hthread_mutex *mutex;
	struct hthread *thread;
};

struct hthread {
        pthread_t thread;
	UT_hash_handle hh;
#if defined(HTHREAD_DEBUG) && (HTHREAD_DEBUG == 1)
	struct hthread_mutex_lock *locks;
	struct hthread_cond *conds;
	const char *func;
	const char *file;
	int line;
	int frames;
	void *callstack[HTHREAD_CALLSTACK_MAX];
        char name[0];
#endif
};

struct hthread_cond {
        pthread_cond_t cond;
#if defined(HTHREAD_DEBUG) && (HTHREAD_DEBUG == 1)
	void *address;
	UT_hash_handle hh;
	const char *func;
	const char *file;
	int line;
        char name[0];
#endif
};

struct hthread_mutex {
	pthread_mutex_t mutex;
#if defined(HTHREAD_DEBUG) && (HTHREAD_DEBUG == 1)
	void *address;
	UT_hash_handle hh;
	const char *func;
	const char *file;
	int line;
	char name[0];
#endif
};

#define hthread_lock()			pthread_mutex_lock(&hthread_mutex)
#define hthread_unlock()		pthread_mutex_unlock(&hthread_mutex)
#define hthread_self_pthread()		pthread_self()

static struct hthread *hthreads		= NULL;
static struct hthread *hthread_root	= NULL;
static pthread_mutex_t hthread_mutex	= PTHREAD_MUTEX_INITIALIZER;

#if defined(HTHREAD_DEBUG) && (HTHREAD_DEBUG == 1)
static inline int hthread_check (struct hthread *thread, const char *command, const char *func, const char *file, const int line);
#endif
static inline int hthread_add (struct hthread *thread, const char *func, const char *file, const int line);
static inline int hthread_del (struct hthread *thread, const char *command, const char *func, const char *file, const int line);

#if defined(HTHREAD_DEBUG) && (HTHREAD_DEBUG == 1)

static inline int debug_dump_callstack (const char *prefix);
static inline int debug_memory_add (const char *name, void *address, size_t size, const char *command, const char *func, const char *file, const int line);
static inline int debug_memory_del (void *address, const char *command, const char *func, const char *file, const int line);
static inline int debug_mutex_add_lock (struct hthread_mutex *mutex, const char *command, const char *func, const char *file, const int line);
static inline int debug_mutex_try_lock (struct hthread_mutex *mutex, const char *command, const char *func, const char *file, const int line);
static inline struct hthread_mutex_lock * debug_mutex_find_lock (struct hthread_mutex *mutex, const char *func, const char *file, const int line);
static inline int debug_mutex_del_lock (struct hthread_mutex *mutex, const char *command, const char *func, const char *file, const int line);
static inline int debug_mutex_add (struct hthread_mutex *mutex, const char *command, const char *func, const char *file, const int line);
static inline int debug_mutex_del (struct hthread_mutex *mutex, const char *command, const char *func, const char *file, const int line);
static inline int debug_cond_add (struct hthread_cond *mutex, const char *command, const char *func, const char *file, const int line);
static inline int debug_cond_del (struct hthread_cond *mutex, const char *command, const char *func, const char *file, const int line);
static inline int debug_cond_check (struct hthread_cond *mutex, const char *command, const char *func, const char *file, const int line);
#else

#define debug_thread_unused() \
	(void) func; \
	(void) file; \
	(void) line;
#define debug_memory_add(a...)		(void) name; debug_thread_unused()
#define debug_memory_del(a...)		debug_thread_unused()
#define debug_mutex_add_lock(a...)	debug_thread_unused()
#define debug_mutex_del_lock(a...)	debug_thread_unused()
#define debug_mutex_add(a...)		debug_thread_unused()
#define debug_mutex_del(a...)           debug_thread_unused()
#define debug_cond_add(a...)		debug_thread_unused()
#define debug_cond_del(a...)            debug_thread_unused()
#define debug_cond_check(a...)          debug_thread_unused()

#endif

#define HTHREAD_ROOT_PROCESS_NAME	"root-process"

static inline int hthread_getenv_int (const char *name)
{
	int r;
	const char *e;
	if (name == NULL) {
		return -1;
	}
	e = getenv(name);
	if (e == NULL) {
		return -1;
	}
	r = atoi(e);
	return r;
}

static inline int hthread_add_actual (struct hthread *thread, const char *func, const char *file, const int line)
{
	struct hthread *th;
	(void) func;
	(void) file;
	(void) line;
	HASH_FIND(hh, hthreads, &thread->thread, sizeof(thread->thread), th);
	if (th != NULL) {
#if defined(HTHREAD_DEBUG) && (HTHREAD_DEBUG == 1)
		hassertf("thread: %s (%p) is already in list", thread->name, thread);
#else
		hassertf("thread: 0x%08llx (%p) is already in list", (unsigned long long) thread->thread, thread);
#endif
		hthread_unlock();
		return -1;
	}
#if defined(HTHREAD_DEBUG) && (HTHREAD_DEBUG == 1)
	thread->func = func;
	thread->file = file;
	thread->line = line;
	thread->conds = NULL;
	thread->locks = NULL;
#endif
	HASH_ADD(hh, hthreads, thread, sizeof(thread->thread), thread);
#if defined(HTHREAD_DEBUG) && (HTHREAD_DEBUG == 1)
	hdebug_lock();
	hinfof("new thread created: '%s (%p)'", thread->name, thread);
	hinfof("    at: %s %s:%d", func, file, line);
	hdebug_unlock();
#endif
	return 0;
}

static inline struct hthread * hthread_add_root (const char *command)
{
	unsigned int s;
	struct hthread *th;
#if 0
	struct hthread *nth;
	HASH_ITER(hh, hthreads, th, nth) {
		if (strcmp(th->name, HTHREAD_ROOT_PROCESS_NAME) != 0) {
			continue;
		}
		if (th->func != NULL) {
			continue;
		}
		if (th->file != NULL) {
			continue;
		}
		if (th->line != 0) {
			continue;
		}
#else
	if (hthread_root != NULL) {
#endif
		hdebug_lock();
		hinfof("%s within unknown thread", command);
		hdebug_unlock();
		hassert(0 && "invalid thread");
	}
#if defined(HTHREAD_DEBUG) && (HTHREAD_DEBUG == 1)
	s = sizeof(struct hthread) + strlen(HTHREAD_ROOT_PROCESS_NAME) + 1;
#else
	s = sizeof(struct hthread);
#endif
	th = malloc(s);
	if (th == NULL) {
		hassertf("malloc failed");
		return NULL;
	}
	memset(th, 0, s);
#if defined(HTHREAD_DEBUG) && (HTHREAD_DEBUG == 1)
	memcpy(th->name, HTHREAD_ROOT_PROCESS_NAME, strlen(HTHREAD_ROOT_PROCESS_NAME) + 1);
#endif
	th->thread = hthread_self_pthread();
	hthread_root = th;
	hthread_add_actual(th, NULL, NULL, 0);
	return th;
}

static inline struct hthread * hthread_find_self (const char *command)
{
	struct hthread *th;
#if 0
	struct hthread *nth;
	HASH_ITER(hh, hthreads, th, nth) {
		if (th->thread == hthread_self_pthread()) {
			goto found_th;
		}
	}
#else
	pthread_t self;
	self = hthread_self_pthread();
	HASH_FIND(hh, hthreads, &self, sizeof(self), th);
	if (th != NULL) {
		goto found_th;
	}
#endif
	th = hthread_add_root(command);
found_th:
	return th;
}

#if defined(HTHREAD_DEBUG) && (HTHREAD_DEBUG == 1)

static inline struct hthread * hthread_find (pthread_t *thread, const char *command)
{
	struct hthread *th;
	(void) command;
#if 0
	struct hthread *nth;
	HASH_ITER(hh, hthreads, th, nth) {
		if (th->thread == *thread) {
			goto found_th;
		}
	}
#else
	HASH_FIND(hh, hthreads, thread, sizeof(*thread), th);
	if (th != NULL) {
		goto found_th;
	}
#endif
	return NULL;
found_th:
	return th;
}

static inline int hthread_check (struct hthread *thread, const char *command, const char *func, const char *file, const int line)
{
	struct hthread *th;
	struct hthread *sth;
	hthread_lock();
	sth = hthread_find_self(command);
	if (sth != NULL) {
		goto found_sth;
	}
	hassert((sth != NULL) && "can not find self");
	hthread_unlock();
	return -1;
found_sth:
	th = hthread_find(&thread->thread, command);
	if (th != NULL) {
		goto found_th;
	}
	hdebug_lock();
	hinfof("%s with invalid thread: '%p'", command, thread);
	hinfof("    by: %s (%p)", sth->name, sth);
	hinfof("    at: %s %s:%d", func, file, line);
	debug_dump_callstack("        ");
	hdebug_unlock();
	hassert((th == thread) && "invalid thread");
	hthread_unlock();
	return -1;
found_th:
	hthread_unlock();
	return 0;
}
#endif

static inline int hthread_add (struct hthread *thread, const char *func, const char *file, const int line)
{
	hthread_lock();
	hthread_add_actual(thread, func, file, line);
	hthread_unlock();
	return 0;
}

static inline int hthread_del (struct hthread *thread, const char *command, const char *func, const char *file, const int line)
{
	struct hthread *th;
	struct hthread *nth;
	struct hthread *sth;
	struct hthread *nsth;
	hthread_lock();
	HASH_ITER(hh, hthreads, sth, nsth) {
		if (sth->thread == hthread_self_pthread()) {
			goto found_sth;
		}
	}
	sth = hthread_add_root(command);
found_sth:
	HASH_ITER(hh, hthreads, th, nth) {
		if (th == thread) {
			goto found_th;
		}
	}
#if defined(HTHREAD_DEBUG) && (HTHREAD_DEBUG == 1)
	hdebug_lock();
	hinfof("thread: %s (%p): %s with invalid argument '%p'", sth->name, sth, command, thread);
	hinfof("    at: %s %s:%d", func, file, line);
	hdebug_unlock();
#else
	(void) func;
	(void) file;
	(void) line;
#endif
	hassert((th == thread) && "invalid thread");
	hthread_unlock();
	return -1;
found_th:
#if defined(HTHREAD_DEBUG) && (HTHREAD_DEBUG == 1)
	hdebug_lock();
	hinfof("thread: %s (%p) deleted", thread->name, thread);
	hinfof("  created at: %s %s:%d", thread->func, thread->file, thread->line);
	hinfof("  deleted by: %s (%p)", sth->name, sth);
	hinfof("          at: %s %s:%d", func, file, line);
	hdebug_unlock();
#else
	(void) func;
	(void) file;
	(void) line;
#endif
	HASH_DEL(hthreads, th);
	hthread_unlock();
	return 0;
}

struct hthread_cond * HTHREAD_FUNCTION_NAME(cond_init_actual) (const char *name, const char *func, const char *file, const int line)
{
	unsigned int s;
	struct hthread_cond *c;
#if defined(HTHREAD_DEBUG) && (HTHREAD_DEBUG == 1)
	s = sizeof(struct hthread_cond) + strlen(name) + 1;
#else
	s = sizeof(struct hthread_cond);
#endif
	c = (struct hthread_cond *) malloc(s);
	if (c == NULL) {
		hassertf("malloc failed");
		return NULL;
	}
	memset(c, 0, s);
#if defined(HTHREAD_DEBUG) && (HTHREAD_DEBUG == 1)
	memcpy(c->name, name, strlen(name) + 1);
#endif
        if (pthread_cond_init(&c->cond, NULL) != 0) {
        	hassertf("can not create cond '%s'", name);
        	free(c);
        	return NULL;
        }
	debug_cond_add(c, "cond init", func, file, line);
        return c;
}

int HTHREAD_FUNCTION_NAME(cond_destroy_actual) (struct hthread_cond *cond, const char *func, const char *file, const int line)
{
	if (cond == NULL) {
		return 0;
	}
	debug_cond_del(cond, "cond destroy", func, file, line);
        pthread_cond_destroy(&cond->cond);
        free(cond);
        return 0;
}

int HTHREAD_FUNCTION_NAME(cond_signal_actual) (struct hthread_cond *cond, const char *func, const char *file, const int line)
{
	debug_cond_check(cond, "cond signal", func, file, line);
        return pthread_cond_signal(&cond->cond);
}

int HTHREAD_FUNCTION_NAME(cond_broadcast_actual) (struct hthread_cond *cond, const char *func, const char *file, const int line)
{
	debug_cond_check(cond, "cond signal", func, file, line);
	return pthread_cond_broadcast(&cond->cond);
}

int HTHREAD_FUNCTION_NAME(cond_wait_actual) (struct hthread_cond *cond, struct hthread_mutex *mutex, const char *func, const char *file, const int line)
{
	int r;
#if defined(HTHREAD_DEBUG) && (HTHREAD_DEBUG == 1)
	unsigned int t = 0;
	while (1) {
		r = HTHREAD_FUNCTION_NAME(cond_timedwait_actual)(cond, mutex, 1000, func, file, line);
		if (r == 0) {
			break;
		}
		t += 1;
		if (t >= 10) {
			herrorf("still waiting for %s cond, %s mutex @ %s:%s:%d", cond->name, mutex->name, func, file, line);
			t = 0;
		}
	}
#else
	debug_cond_check(cond, "cond wait", func, file, line);
	debug_mutex_del_lock(mutex, "cond wait", func, file, line);
	r = pthread_cond_wait(&cond->cond, &mutex->mutex);
	debug_mutex_add_lock(mutex, "cond wait", func, file, line);
#endif
	return r;
}

int HTHREAD_FUNCTION_NAME(cond_timedwait_tspec_actual) (struct hthread_cond *cond, struct hthread_mutex *mutex, struct timespec *tspec, const char *func, const char *file, const int line)
{
	int ret;
	debug_cond_check(cond, "cond timedwait", func, file, line);
	debug_mutex_del_lock(mutex, "cond timedwait", func, file, line);
again:  ret = pthread_cond_timedwait(&cond->cond, &mutex->mutex, tspec);
	switch (ret) {
		case EINTR:
			goto again;
			break;
		case ETIMEDOUT:
			ret = 1;
			break;
		case 0:
			break;
		default:
			assert(0);
			ret = -1;
			break;
	}
	debug_mutex_add_lock(mutex, "cond wait", func, file, line);
	return ret;
}

int HTHREAD_FUNCTION_NAME(cond_timedwait_actual) (struct hthread_cond *cond, struct hthread_mutex *mutex, int msec, const char *func, const char *file, const int line)
{
	int ret;
        struct timeval tval;
        struct timespec tspec;
	if (msec < 0) {
		return HTHREAD_FUNCTION_NAME(cond_wait_actual)(cond, mutex, func, file, line);
	}
	gettimeofday(&tval, NULL);
	tspec.tv_sec = tval.tv_sec + (msec / 1000);
	tspec.tv_nsec = (tval.tv_usec + ((msec % 1000) * 1000)) * 1000;
	if (tspec.tv_nsec >= 1000000000) {
		tspec.tv_sec += 1;
		tspec.tv_nsec -= 1000000000;
	}
	debug_cond_check(cond, "cond timedwait", func, file, line);
	debug_mutex_del_lock(mutex, "cond timedwait", func, file, line);
again:  ret = pthread_cond_timedwait(&cond->cond, &mutex->mutex, &tspec);
	switch (ret) {
		case EINTR:
			goto again;
			break;
		case ETIMEDOUT:
			ret = 1;
			break;
		case 0:
			break;
		default:
			assert(0);
			ret = -1;
			break;
	}
	debug_mutex_add_lock(mutex, "cond timedwait", func, file, line);
	return ret;
}

struct hthread_mutex * HTHREAD_FUNCTION_NAME(mutex_init_actual) (const char *name, const char *func, const char *file, const int line)
{
	unsigned int s;
	struct hthread_mutex *m;
#if defined(HTHREAD_DEBUG) && (HTHREAD_DEBUG == 1)
	s = sizeof(struct hthread_mutex) + strlen(name) + 1;
#else
	s = sizeof(struct hthread_mutex);
#endif
	m = (struct hthread_mutex *) malloc(s);
	if (m == NULL) {
		hassertf("malloc failed");
		return NULL;
	}
	memset(m, 0, s);
#if defined(HTHREAD_DEBUG) && (HTHREAD_DEBUG == 1)
	memcpy(m->name, name, strlen(name) + 1);
#endif
	if (pthread_mutex_init(&m->mutex, NULL) != 0) {
		hassertf("can not create mutex '%s'", name);
		free(m);
		return NULL;
	}
	debug_mutex_add(m, "mutex init", func, file, line);
	return m;
}

int HTHREAD_FUNCTION_NAME(mutex_destroy_actual) (struct hthread_mutex *mutex, const char *func, const char *file, const int line)
{
	if (mutex == NULL) {
		return 0;
	}
	debug_mutex_del(mutex, "mutex destroy", func, file, line);
        pthread_mutex_destroy(&mutex->mutex);
        free(mutex);
        return 0;
}

int HTHREAD_FUNCTION_NAME(mutex_lock_actual) (struct hthread_mutex *mutex, const char *func, const char *file, const int line)
{
	debug_mutex_add_lock(mutex, "mutex lock", func, file, line);
#if defined(HTHREAD_DEBUG) && (HTHREAD_DEBUG == 1)
	return debug_mutex_try_lock(mutex, "mutex lock", func, file, line);
#else
	return pthread_mutex_lock(&mutex->mutex);
#endif
}

int HTHREAD_FUNCTION_NAME(mutex_unlock_actual) (struct hthread_mutex *mutex, const char *func, const char *file, const int line)
{
	int r;
	debug_mutex_del_lock(mutex, "mutex unlock", func, file, line);
	r = pthread_mutex_unlock(&mutex->mutex);
	return r;
}

static void * thread_run (void *farg)
{
	struct hthread_arg *arg = (struct hthread_arg *) farg;
        void *p = arg->arg;
        void * (*f) (void *) = arg->f;
        hthread_add(arg->thread, arg->func, arg->file, arg->line);
        hthread_mutex_lock(arg->mutex);
        arg->flag = 1;
        hthread_cond_signal(arg->cond);
        hthread_mutex_unlock(arg->mutex);
        f(p);
        return NULL;
}

struct hthread * HTHREAD_FUNCTION_NAME(create_actual) (const char *name, void * (*function) (void *), void *farg, const char *func, const char *file, const int line)
{
        int ret;
        unsigned int s;
        struct hthread *tid;
        struct hthread_arg *arg;
#if defined(HTHREAD_DEBUG) && (HTHREAD_DEBUG == 1)
	s = sizeof(struct hthread) + strlen(name) + 1;
#else
	s = sizeof(struct hthread);
#endif
        tid = (struct hthread *) malloc(s);
        if (tid == NULL) {
        	hassertf("malloc failed");
        	return NULL;
        }
        memset(tid, 0, s);
#if defined(HTHREAD_DEBUG) && (HTHREAD_DEBUG == 1)
        memcpy(tid->name, name, strlen(name) + 1);
#endif
        arg = (struct hthread_arg *) malloc(sizeof(struct hthread_arg));
        if (arg == NULL) {
        	hassertf("malloc failed");
        	return NULL;
        }
        memset(arg, 0, sizeof(struct hthread_arg));
        arg->r = &thread_run;
        arg->f = function;
        arg->arg = farg;
        arg->func = func;
        arg->file = file;
        arg->line = line;
#if defined(HTHREAD_DEBUG) && (HTHREAD_DEBUG == 1)
        arg->name = tid->name;
#endif
        arg->cond = hthread_cond_init("arg->cond");
        arg->mutex = hthread_mutex_init("arg->mutex");
        arg->flag = 0;
        arg->thread = tid;
        hthread_mutex_lock(arg->mutex);
        ret = pthread_create(&(tid->thread), NULL, arg->r, arg);
        if (ret != 0) {
        	hassertf("can not create thread '%s'", name);
                hthread_mutex_unlock(arg->mutex);
                hthread_cond_destroy(arg->cond);
                hthread_mutex_destroy(arg->mutex);
                free(arg);
                return NULL;
        }
        while (arg->flag != 1) {
                hthread_cond_wait(arg->cond, arg->mutex);
        }
        hthread_mutex_unlock(arg->mutex);

        hthread_cond_destroy(arg->cond);
        hthread_mutex_destroy(arg->mutex);
        free(arg);

        return tid;
}

int HTHREAD_FUNCTION_NAME(join_actual) (struct hthread *thread, const char *func, const char *file, const int line)
{
	if (thread == NULL) {
		return 0;
	}
#if defined(HTHREAD_DEBUG) && (HTHREAD_DEBUG == 1)
	hthread_check(thread, "join", func, file, line);
#endif
	pthread_join(thread->thread, NULL);
	hthread_del(thread, "join", func, file, line);
	free(thread);
        return 0;
}

int HTHREAD_FUNCTION_NAME(detach_actual) (struct hthread *thread, const char *func, const char *file, const int line)
{
	if (thread == NULL) {
		return 0;
	}
#if defined(HTHREAD_DEBUG) && (HTHREAD_DEBUG == 1)
	hthread_check(thread, "detach", func, file, line);
#endif
	pthread_detach(thread->thread);
	hthread_del(thread, "detach", func, file, line);
	free(thread);
        return 0;
}

struct hthread * HTHREAD_FUNCTION_NAME(self_actual) (const char *func, const char *file, const int line)
{
	struct hthread *th;
	(void) func;
	(void) file;
	(void) line;
	hthread_lock();
	th = hthread_find_self("self");
	hthread_unlock();
	return th;
}

int HTHREAD_FUNCTION_NAME(sched_yield_actual) (const char *func, const char *file, const int line)
{
	(void) func;
	(void) file;
	(void) line;
#if defined(HTHREAD_DISABLE_YIELD) && (HTHREAD_DISABLE_YIELD == 0)
	return 0;
#else
	return sched_yield();
#endif
}


void * HTHREAD_FUNCTION_NAME(memset_actual) (void *destination, int c, size_t len, const char *func, const char *file, const int line)
{
	void *rc;
	(void) func;
	(void) file;
	(void) line;
	rc = memset(destination, c, len);
	return rc;
}

void * HTHREAD_FUNCTION_NAME(memcpy_actual) (void *destination, void *source, size_t len, const char *func, const char *file, const int line)
{
	void *rc;
	(void) func;
	(void) file;
	(void) line;
	rc = memcpy(destination, source, len);
	return rc;
}

char * HTHREAD_FUNCTION_NAME(strdup_actual) (const char *name, const char *string, const char *func, const char *file, const int line)
{
	void *rc;
	if (string == NULL) {
#if defined(HTHREAD_DEBUG) && (HTHREAD_DEBUG == 1)
		struct hthread *th;
		hthread_lock();
		th = hthread_find_self("strdup");
		hdebug_lock();
		hinfof("strdup with invalid argument '%p'", string);
		hinfof("    by: %s (%p)", th->name, th);
		hinfof("    at: %s %s:%d", func, file, line);
		hdebug_unlock();
		hassert((string != NULL) && "invalid strdup parameter");
		hthread_unlock();
#endif
		return NULL;
	}
	rc = strdup(string);
	debug_memory_add(name, rc, strlen(rc) + 1, "strdup", func, file, line);
	return rc;
}

char * HTHREAD_FUNCTION_NAME(strndup_actual) (const char *name, const char *string, size_t size, const char *func, const char *file, const int line)
{
	void *rc;
	if (string == NULL) {
#if defined(HTHREAD_DEBUG) && (HTHREAD_DEBUG == 1)
		struct hthread *th;
		hthread_lock();
		th = hthread_find_self("strndup");
		hdebug_lock();
		hinfof("strdup with invalid argument '%p'", string);
		hinfof("    by: %s (%p)", th->name, th);
		hinfof("    at: %s %s:%d", func, file, line);
		hdebug_unlock();
		hassert((string != NULL) && "invalid strdup parameter");
		hthread_unlock();
#endif
		return NULL;
	}
	rc = strndup(string, size);
	debug_memory_add(name, rc, strlen(rc) + 1, "strndup", func, file, line);
	return rc;
}

void * HTHREAD_FUNCTION_NAME(malloc_actual) (const char *name, size_t size, const char *func, const char *file, const int line)
{
	void *rc;
	rc = malloc(size);
	debug_memory_add(name, rc, size, "malloc", func, file, line);
	return rc;
}

void * HTHREAD_FUNCTION_NAME(calloc_actual) (const char *name, size_t nmemb, size_t size, const char *func, const char *file, const int line)
{
	void *rc;
	rc = calloc(nmemb, size);
	debug_memory_add(name, rc, nmemb * size, "calloc", func, file, line);
	return rc;
}

void * HTHREAD_FUNCTION_NAME(realloc_actual) (const char *name, void *address, size_t size, const char *func, const char *file, const int line)
{
	void *rc;
	rc = realloc(address, size);
	if (rc == NULL) {
		return rc;
	}
	debug_memory_del(address, "realloc", func, file, line);
	debug_memory_add(name, rc, size, "realloc", func, file, line);
	return rc;
}

void HTHREAD_FUNCTION_NAME(free_actual) (void *address, const char *func, const char *file, const int line)
{
	debug_memory_del(address, "free", func, file, line);
	free(address);
}

#if defined(HTHREAD_DEBUG) && (HTHREAD_DEBUG == 1)

struct hthread_mutex_lock {
	struct hthread_mutex *mutex;
	UT_hash_handle hh;
	struct hthread *thread;
	const char *func;
	const char *file;
	int line;
	unsigned long long timeval;
};

struct hthread_mutex_order_item {
	struct hthread *thread;
	const char *func;
	const char *file;
	int line;
};

struct hthread_mutex_order_key {
	struct hthread_mutex *first;
	struct hthread_mutex *second;
};

struct hthread_mutex_order_info {
	struct hthread_mutex_order_item first;
	struct hthread_mutex_order_item second;
};

struct hthread_mutex_order {
	struct hthread_mutex_order_key key;
	struct hthread_mutex_order_info info;
	struct hthread *thread;
	UT_hash_handle hh;
};

static struct hthread_cond *debug_conds = NULL;
static struct hthread_mutex *debug_mutexes = NULL;
static struct hthread_mutex_order *debug_orders = NULL;

static inline unsigned long long debug_getclock (void)
{
	struct timespec ts;
	unsigned long long tsec;
	unsigned long long tusec;
	unsigned long long _clock;
#if defined(__DARWIN__) && (__DARWIN__ == 1)
	(void) ts;
	(void) tsec;
	(void) tusec;
	_clock = mach_absolute_time();
	_clock /= 1000 * 1000;
#elif defined(__LINUX__) && (__LINUX__ == 1)
	if (clock_gettime(CLOCK_MONOTONIC, &ts) < 0) {
		return 0;
	}
	tsec = ((unsigned long long) ts.tv_sec) * 1000;
	tusec = ((unsigned long long) ts.tv_nsec) / 1000 / 1000;
	_clock = tsec + tusec;
#else
	#error "unknown os"
#endif
	return _clock;
}

struct stackinfo {
	const char *file;
	const char *func;
	unsigned int line;
	void *func_addr;
};

static inline int debug_dump_callstack (const char *prefix)
{
#if defined(HTHREAD_ENABLE_CALLSTACK) && (HTHREAD_ENABLE_CALLSTACK == 1)
	int i;
	int rc;
	int size;
	int frames;
	char **strs;
	bfd *bfd;
	bfd_vma ofs;
	bfd_vma start;
	Dl_info dlinfo;
	asymbol **syms;
	asection *secp;
	unsigned int v;
	const char *fname;
	const char *func;
	unsigned int line;
	struct stackinfo stackinfo;
	void *callstack[HTHREAD_CALLSTACK_MAX];
	v = hthread_getenv_int(HTHREAD_REPORT_CALLSTACK_NAME);
	if (v == (unsigned int) -1) {
		v = HTHREAD_REPORT_CALLSTACK;
	}
	if (v == 0) {
		return 0;
	}
	frames = backtrace(callstack, HTHREAD_CALLSTACK_MAX);
	strs = backtrace_symbols(callstack, frames);
	for (i = 1; i < frames; i++) {
		if (dladdr(callstack[i], &dlinfo)) {
			memset(&stackinfo, 0, sizeof(struct stackinfo));
			stackinfo.func = dlinfo.dli_sname;
			stackinfo.func_addr = dlinfo.dli_saddr;
			bfd = bfd_openr(dlinfo.dli_fname, NULL);
			if (bfd == NULL) {
				continue;
			}
			rc = bfd_check_format(bfd, bfd_object);
			if (rc == 0) {
				bfd_close(bfd);
				continue;
			}
			size = bfd_get_symtab_upper_bound(bfd);
			if (size <= 0) {
				bfd_close(bfd);
				continue;
			}
			syms = malloc(size);
			if (syms == NULL) {
				bfd_close(bfd);
				continue;
			}
			rc = bfd_canonicalize_symtab(bfd, syms);
			if (rc <= 0) {
				free(syms);
				bfd_close(bfd);
				continue;
			}
			#define ELF_DYNAMIC	0x40
			if (bfd->flags & ELF_DYNAMIC) {
				ofs = callstack[i] - dlinfo.dli_fbase;
			} else {
				ofs = callstack[i] - (void *) 0;
			}
			for (secp = bfd->sections; secp != NULL; secp = secp->next) {
				if (!(bfd_get_section_flags(bfd, secp) & SEC_ALLOC)) {
					continue;
				}
				start = bfd_get_section_vma(bfd, secp);
				if (ofs < start) {
					continue;
				}
				size = bfd_get_section_size(secp);
				if (ofs >= start + size) {
					continue;
				}
				if (bfd_find_nearest_line(bfd, secp, syms, ofs - start, &fname, &func, &line)) {
					stackinfo.file = fname;
					if (func != NULL) {
						stackinfo.func = func;
					}
					stackinfo.line = line;
					if (!stackinfo.func_addr && stackinfo.func) {
						asymbol **asymp;
						for (asymp = syms; *asymp; asymp++) {
							if (strcmp (bfd_asymbol_name (*asymp), stackinfo.func) == 0) {
								stackinfo.func_addr = bfd_asymbol_value (*asymp) + (void *) 0;
								break;
							}
						}
					}
				}

				break;
			}
			hinfof("%s%p: %s (%s:%d)", prefix, callstack[i], (stackinfo.file == NULL) ? "(null)" : ((strrchr(stackinfo.file, '/') == NULL) ? stackinfo.file : (strrchr(stackinfo.file, '/') + 1)), stackinfo.func, stackinfo.line);
			free(syms);
			bfd_close(bfd);
		}
	}
#if 0
	for (i = 0; i < frames; i++) {
		hinfof("%s%s", prefix, strs[i]);
	}
#endif
	free(strs);
#else
	(void) prefix;
#endif
	return 0;
}

static inline int debug_mutex_add_lock (struct hthread_mutex *mutex, const char *command, const char *func, const char *file, const int line)
{
	struct hthread *th;
	struct hthread_mutex *mt;
	struct hthread_mutex_lock *mtl;
	struct hthread_mutex_lock *dmtl;
	struct hthread_mutex_lock *nmtl;
	struct hthread_mutex_order *mto;
	struct hthread_mutex_order *nmto;
	struct hthread_mutex_order *dmto;
	struct hthread_mutex_order *ndmto;
	struct hthread_mutex_order_key key;
	hthread_lock();
	th = hthread_find_self(command);
	if (th != NULL) {
		goto found_th;
	}
	hassert((th != NULL) && "can not find self");
	hthread_unlock();
	return -1;
found_th:
	HASH_FIND(hh, debug_mutexes, &mutex->address, sizeof(mutex->address), mt);
	if (mt != NULL) {
		goto found_mt;
	}
	hdebug_lock();
	hinfof("%s with invalid mutex: '%p'", command, mutex);
	hinfof("    by: %s (%p)", th->name, th);
	hinfof("    at: %s %s:%d", func, file, line);
	debug_dump_callstack("        ");
	hdebug_unlock();
	hassert((mt != NULL) && "invalid mutex");
	hthread_unlock();
	return -1;
found_mt:
	HASH_FIND(hh, th->locks, &mutex, sizeof(mutex), mtl);
	if (mtl != NULL) {
		hdebug_lock();
		hinfof("%s with already held mutex: '%s (%p)'", command, mutex->name, mutex);
		hinfof("    by: %s (%p)", th->name, th);
		hinfof("    at: %s %s:%d", func, file, line);
		debug_dump_callstack("        ");
		hinfof("  previously acquired");
		hinfof("    by: %s (%p)", mtl->thread->name, mtl->thread);
		hinfof("    at: %s %s:%d", mtl->func, mtl->file, mtl->line);
		hinfof("  created '%s (%p)'", mutex->name, mutex);
		hinfof("    at: %s %s:%d", mutex->func, mutex->file, mutex->line);
		hdebug_unlock();
		hassert((mt == NULL) && "mutex is already locked");
		hthread_unlock();
		return -1;
	}
	HASH_ITER(hh, th->locks, mtl, nmtl) {
		memset(&key, 0, sizeof(key));
		key.first = mtl->mutex;
		key.second = mutex;
		HASH_FIND(hh, debug_orders, &key, sizeof(key), mto);
		if (mto != NULL) {
			continue;
		}
		memset(&key, 0, sizeof(key));
		key.first = mutex;
		key.second = mtl->mutex;
		HASH_FIND(hh, debug_orders, &key, sizeof(key), mto);
		if (mto != NULL) {
			hdebugf("locks: %u", HASH_COUNT(th->locks));
			HASH_ITER(hh, th->locks, dmtl, nmtl) {
				hdebugf("  %s (%p) @ %s, %s:%s:%d",
					dmtl->mutex->name,
					dmtl->mutex,
					dmtl->thread->name,
					dmtl->func, dmtl->file, dmtl->line);
			}
			hdebugf("orders: %u", HASH_COUNT(debug_orders));
			HASH_ITER(hh, debug_orders, dmto, ndmto) {
				hdebugf("  %s (%p) -> %s (%p) @ %s, %s:%s:%d, %s:%s:%d",
					  dmto->key.first->name,
					  dmto->key.first,
					  dmto->key.second->name,
					  dmto->key.second,
					  dmto->thread->name,
					  dmto->info.first.func, dmto->info.first.file, dmto->info.first.line,
					  dmto->info.second.func, dmto->info.second.file, dmto->info.second.line);
			}
			hdebugf("mutex order violated\n"
				  "  before: %s (%p) -> %s (%p) @ %s, %s:%s:%d, %s:%s:%d\n"
				  "  after : %s (%p) -> %s (%p) @ %s, %s:%s:%d, %s:%s:%d",
				  mto->key.first->name,
				  mto->key.first,
				  mto->key.second->name,
				  mto->key.second,
				  mto->thread->name,
				  mto->info.first.func, mto->info.first.file, mto->info.first.line,
				  mto->info.second.func, mto->info.second.file, mto->info.second.line,
				  mtl->mutex->name,
				  mtl->mutex,
				  mutex->name,
				  mutex,
				  th->name,
				  func, file, line,
				  mtl->func, mtl->file, mtl->line);
			hdebug_lock();
			hinfof("%s order '%s (%p)' before '%s (%p)' violated", command, mutex->name, mutex, mtl->mutex->name, mtl->mutex);
			hinfof("  incorrect order is: acquisition of '%s (%p)'", mtl->mutex->name, mtl->mutex);
			hinfof("      by: %s (%p)", mtl->thread->name, mtl->thread);
			hinfof("      at: %s %s:%d", mtl->func, mtl->file, mtl->line);
			hinfof("    followed by a later acquisition of '%s (%p)'", mutex->name, mutex);
			hinfof("      by: %s (%p)", th->name, th);
			hinfof("      at: %s %s:%d", func, file, line);
			debug_dump_callstack("          ");
			hinfof("  required order is: acquisition of '%s (%p)'", mto->key.first->name, mto->key.first);
			hinfof("      by: %s (%p)", mto->info.first.thread->name, mto->info.first.thread);
			hinfof("      at: %s %s:%d", mto->info.first.func, mto->info.first.file, mto->info.first.line);
			hinfof("    followed by a later acquisition of '%s (%p)'", mto->key.second->name, mto->key.second);
			hinfof("      by: %s (%p)", mto->info.second.thread->name, mto->info.second.thread);
			hinfof("      at: %s %s:%d", mto->info.second.func, mto->info.second.file, mto->info.second.line);
			hinfof("  created '%s (%p)'", mtl->mutex->name, mtl->mutex);
			hinfof("    at: %s %s:%d", mtl->mutex->func, mtl->mutex->file, mtl->mutex->line);
			hinfof("  created '%s (%p)'", mutex->name, mutex);
			hinfof("    at: %s %s:%d", mutex->func, mutex->file, mutex->line);
			hdebug_unlock();
			hassert((mto == NULL) && "lock order violation");
			continue;
		}
		nmto = malloc(sizeof(struct hthread_mutex_order));
		if (nmto == NULL) {
			hassertf("malloc failed");
			continue;
		}
		memset(nmto, 0, sizeof(struct hthread_mutex_order));
		nmto->key.first = mtl->mutex;
		nmto->key.second = mutex;
		nmto->info.first.thread = mtl->thread;
		nmto->info.first.func = mtl->func;
		nmto->info.first.file = mtl->file;
		nmto->info.first.line = mtl->line;
		nmto->info.second.thread = th;
		nmto->info.second.func = func;
		nmto->info.second.file = file;
		nmto->info.second.line = line;
		nmto->thread = th;
		hdebugf("adding order %s -> %s", nmto->key.first->name, nmto->key.second->name);
		HASH_ADD(hh, debug_orders, key, sizeof(key), nmto);
	}
	nmtl = malloc(sizeof(struct hthread_mutex_lock));
	if (nmtl == NULL) {
		hassertf("malloc failed");
		hthread_unlock();
		return -1;
	}
	memset(nmtl, 0, sizeof(struct hthread_mutex_lock));
	nmtl->thread = th;
	nmtl->mutex = mutex;
	nmtl->func = func;
	nmtl->file = file;
	nmtl->line = line;
	nmtl->timeval = debug_getclock();
	HASH_ADD(hh, th->locks, mutex, sizeof(mutex), nmtl);
	hdebugf("added lock mutex: %s @ %p, to thread: %s @ %p", mutex->name, mutex, th->name, th);
	hthread_unlock();
	return 0;
}

static int debug_mutex_try_lock (struct hthread_mutex *mutex, const char *command, const char *func, const char *file, const int line)
{
	int r;
	unsigned int a;
	unsigned int v;
	unsigned int t;
	struct hthread *th;
	struct hthread *nth;
	struct hthread_mutex *mt;
	struct hthread_mutex_lock *mtl;
	hthread_lock();
	th = hthread_find_self(command);
	if (th != NULL) {
		goto found_th;
	}
	hassert((th != NULL) && "can not find self");
	hthread_unlock();
	return -1;
found_th:
	HASH_FIND(hh, debug_mutexes, &mutex->address, sizeof(mutex->address), mt);
	if (mt != NULL) {
		goto found_mt;
	}
	hdebug_lock();
	hinfof("%s with invalid mutex: '%p'", command, mutex);
	hinfof("    by: %s (%p)", th->name, th);
	hinfof("    at: %s %s:%d", func, file, line);
	hdebug_unlock();
	hassert((mt != NULL) && "invalid mutex");
	hthread_unlock();
	return -1;
found_mt:
	hthread_unlock();
	t = 0;
	while (1) {
		r = pthread_mutex_trylock(&mutex->mutex);
		if (r == 0) {
			break;
		}
		usleep(10000);
		t += 1;
		v = hthread_getenv_int(HTHREAD_LOCK_TRY_THRESHOLD_NAME);
		if (v == (unsigned int) -1) {
			v = HTHREAD_LOCK_TRY_THRESHOLD;
		}
		a = hthread_getenv_int(HTHREAD_LOCK_TRY_THRESHOLD_ASSERT_NAME);
		if (a == (unsigned int) -1) {
			a = HTHREAD_LOCK_TRY_THRESHOLD_ASSERT;
		}
		if (t >= ((v * 1000) / 10000) && (v != 0)) {
			hthread_lock();
			th = hthread_find_self(command);
			if (th != NULL) {
				goto found_th_;
			}
			hassert((th != NULL) && "can not find self");
			hthread_unlock();
			return -1;
found_th_:
			HASH_FIND(hh, debug_mutexes, &mutex->address, sizeof(mutex->address), mt);
			if (mt != NULL) {
				goto found_mt_;
			}
			hdebug_lock();
			hinfof("%s with invalid mutex: '%p'", command, mutex);
			hinfof("    by: %s (%p)", th->name, th);
			hinfof("    at: %s %s:%d", func, file, line);
			hdebug_unlock();
			hassert((mt != NULL) && "invalid mutex");
			hthread_unlock();
			return -1;
found_mt_:
			hdebug_lock();
			hinfof("%s still waiting for mutex: '%s (%p)'", command, mutex->name, mutex);
			hinfof("    by: %s (%p)", th->name, th);
			hinfof("    at: %s %s:%d", func, file, line);
			debug_dump_callstack("        ");
			HASH_ITER(hh, hthreads, th, nth) {
				HASH_FIND(hh, th->locks, &mutex, sizeof(mutex), mtl);
				if (mtl != NULL) {
					hinfof("  currently locked");
					hinfof("    by: %s (%p)", mtl->thread->name, mtl->thread);
					hinfof("    at: %s %s:%d", mtl->func, mtl->file, mtl->line);
				}
			}
			hinfof("  created '%s (%p)'", mutex->name, mutex);
			hinfof("    at: %s %s:%d", mutex->func, mutex->file, mutex->line);
			hdebug_unlock();
			hassert((a == 0) && "mutex try lock threshold reached");
			t = 0;
			hthread_unlock();
		}
	}
	return 0;
}

static inline struct hthread_mutex_lock * debug_mutex_find_lock (struct hthread_mutex *mutex, const char *func, const char *file, const int line)
{
	struct hthread *th;
	struct hthread *nth;
	struct hthread_mutex *mt;
	struct hthread_mutex_lock *mtl;
	(void) func;
	(void) file;
	(void) line;
	HASH_FIND(hh, debug_mutexes, &mutex->address, sizeof(mutex->address), mt);
	if (mt != NULL) {
		goto found_mt;
	}
	hassertf("can not find mutex: %s in list", mutex->name);
	return NULL;
found_mt:
	HASH_ITER(hh, hthreads, th, nth) {
		HASH_FIND(hh, th->locks, &mutex, sizeof(mutex), mtl);
		if (mtl != NULL) {
			return mtl;
		}
	}
	return NULL;
}

static inline int debug_mutex_del_lock (struct hthread_mutex *mutex, const char *command, const char *func, const char *file, const int line)
{
	struct hthread *th;
	struct hthread_mutex *mt;
	struct hthread_mutex_lock *mtl;
	unsigned int v;
	unsigned int a;
	unsigned long long timeval;
	hthread_lock();
	th = hthread_find_self(command);
	if (th != NULL) {
		goto found_th;
	}
	hassert((th != NULL) && "can not find self");
	hthread_unlock();
	return -1;
found_th:
	HASH_FIND(hh, debug_mutexes, &mutex->address, sizeof(mutex->address), mt);
	if (mt != NULL) {
		goto found_mt;
	}
	hdebug_lock();
	hinfof("%s with invalid mutex '%p'", command, mutex);
	hinfof("    by: %s (%p)", th->name, th);
	hinfof("    at: %s %s:%d", func, file, line);
	debug_dump_callstack("        ");
	hdebug_unlock();
	hassert((mt != NULL) && "invalid mutex");
	hthread_unlock();
	return -1;
found_mt:
	HASH_FIND(hh, th->locks, &mutex, sizeof(mutex), mtl);
	if (mtl != NULL) {
		goto found_lc;
	}
	mtl = debug_mutex_find_lock(mutex, func, file, line);
	if (mtl != NULL) {
		hdebug_lock();
		hinfof("%s with a mutex '%s (%p)' currently hold by other thread", command, mutex->name, mutex);
		hinfof("    by: %s (%p)", th->name, th);
		hinfof("    at: %s %s:%d", func, file, line);
		debug_dump_callstack("        ");
		hinfof("  lock observed");
		hinfof("    by: %s (%p)", mtl->thread->name, mtl->thread);
		hinfof("    at: %s %s:%d", mtl->func, mtl->file, mtl->line);
		hinfof("  created '%s (%p)'", mutex->name, mutex);
		hinfof("    at: %s %s:%d", mutex->func, mutex->file, mutex->line);
		hdebug_unlock();
		hassert((mtl == NULL) && "mutex is locked by other thread");
		hthread_unlock();
		return -1;
	}
	hdebug_lock();
	hinfof("%s with unheld mutex: '%s (%p)'", command, mutex->name, mutex);
	hinfof("    by: %s (%p)", th->name, th);
	hinfof("    at: %s %s:%d", func, file, line);
	debug_dump_callstack("        ");
	hinfof("  created '%s (%p)'", mutex->name, mutex);
	hinfof("    at: %s %s:%d", mutex->func, mutex->file, mutex->line);
	hdebug_unlock();
	hassert((mtl != NULL) && "mutex is not locked");
	hthread_unlock();
	return -1;
found_lc:
	HASH_DEL(th->locks, mtl);
	timeval = debug_getclock();
	v = hthread_getenv_int(HTHREAD_LOCK_THRESHOLD_NAME);
	if (v == (unsigned int) -1) {
		v = HTHREAD_LOCK_THRESHOLD;
	}
	a = hthread_getenv_int(HTHREAD_LOCK_THRESHOLD_ASSERT_NAME);
	if (a == (unsigned int) -1) {
		a = HTHREAD_LOCK_THRESHOLD_ASSERT;
	}
	if ((mtl->timeval > timeval) ||
	    (mtl->timeval + v) < timeval) {
		hdebug_lock();
		hinfof("%s with a mutex '%s (%p)' hold during %llu ms", command, mutex->name, mutex, timeval - mtl->timeval);
		hinfof("    by: %s (%p)", th->name, th);
		hinfof("    at: %s %s:%d", func, file, line);
		debug_dump_callstack("        ");
		hinfof("  lock observed");
		hinfof("    by: %s (%p)", mtl->thread->name, mtl->thread);
		hinfof("    at: %s %s:%d", mtl->func, mtl->file, mtl->line);
		hinfof("  created '%s (%p)'", mutex->name, mutex);
		hinfof("    at: %s %s:%d", mutex->func, mutex->file, mutex->line);
		hdebug_unlock();
		hassert((a == 0) && "mutex lock threshold reached");
	}
	hdebugf("deleted lock mutex: %s @ %p, from thread: %s @ %p", mutex->name, mutex, th->name, th);
	free(mtl);
	hthread_unlock();
	return 0;
}

static inline int debug_mutex_add (struct hthread_mutex *mutex, const char *command, const char *func, const char *file, const int line)
{
	struct hthread *th;
	struct hthread_mutex *mt;
	hthread_lock();
	th = hthread_find_self(command);
	if (th != NULL) {
		goto found_th;
	}
	hassert((th != NULL) && "can not find self");
	hthread_unlock();
	return -1;
found_th:
	HASH_FIND(hh, debug_mutexes, &mutex->address, sizeof(mutex->address), mt);
	if (mt != NULL) {
		hassertf("mutex: %s is already in list", mutex->name);
		hthread_unlock();
		return -1;
	}
	hdebugf("adding mutex: %s", mutex->name);
	mutex->address = mutex;
	mutex->func = func;
	mutex->file = file;
	mutex->line = line;
	HASH_ADD(hh, debug_mutexes, address, sizeof(mutex->address), mutex);
	hthread_unlock();
	return 0;
}

static inline int debug_mutex_del (struct hthread_mutex *mutex, const char *command, const char *func, const char *file, const int line)
{
	struct hthread *th;
	struct hthread_mutex *mt;
	struct hthread_mutex_lock *mtl;
	struct hthread_mutex_order *mto;
	struct hthread_mutex_order *nmto;
	hthread_lock();
	th = hthread_find_self(command);
	if (th != NULL) {
		goto found_th;
	}
	hassert((th != NULL) && "can not find self");
	hthread_unlock();
	return -1;
found_th:
	HASH_FIND(hh, debug_mutexes, &mutex->address, sizeof(mutex->address), mt);
	if (mt != NULL) {
		goto found_mt;
	}
	hdebug_lock();
	hinfof("%s with invalid mutex: '%p'", command, mutex);
	hinfof("    by: %s (%p)", th->name, th);
	hinfof("    at: %s %s:%d", func, file, line);
	debug_dump_callstack("        ");
	hdebug_unlock();
	hassert((mt != NULL) && "invalid mutex");
	hthread_unlock();
	return -1;
found_mt:
	HASH_FIND(hh, th->locks, &mutex, sizeof(mutex), mtl);
	if (mtl != NULL) {
		hdebug_lock();
		hinfof("%s with currently locked mutex: '%p'", command, mutex);
		hinfof("    by: %s (%p)", th->name, th);
		hinfof("    at: %s %s:%d", func, file, line);
		debug_dump_callstack("        ");
		hinfof("  lock observed");
		hinfof("    by: %s (%p)", mtl->thread->name, mtl->thread);
		hinfof("    at: %s %s:%d", mtl->func, mtl->file, mtl->line);
		hinfof("  created '%s (%p)", mtl->mutex->name, mtl->mutex);
		hinfof("    at: %s %s:%d", mtl->mutex->func, mtl->mutex->file, mtl->mutex->line);
		hdebug_unlock();
		hassert((mt == NULL) && "invalid mutex");
		hthread_unlock();
		return -1;
	}
	HASH_ITER(hh, debug_orders, mto, nmto) {
		if (mto->key.first == mutex ||
		    mto->key.second == mutex) {
			hdebugf("deleting order %s -> %s", mto->key.first->name, mto->key.second->name);
			HASH_DEL(debug_orders, mto);
			free(mto);
		}
	}
	hdebugf("deleting mutex: %s", mutex->name);
	HASH_DEL(debug_mutexes, mt);
	hthread_unlock();
	return 0;
}

static inline int debug_cond_add (struct hthread_cond *cond, const char *command, const char *func, const char *file, const int line)
{
	struct hthread *th;
	struct hthread_cond *cv;
	hthread_lock();
	th = hthread_find_self(command);
	if (th != NULL) {
		goto found_th;
	}
	hassert((th != NULL) && "can not find self");
	hthread_unlock();
	return -1;
found_th:
	HASH_FIND(hh, debug_conds, &cond, sizeof(cond), cv);
	if (cv != NULL) {
		hassertf("cond: %s is already in list", cond->name);
		hthread_unlock();
		return -1;
	}
	hdebugf("adding cond: %s", cond->name);
	cond->address = cond;
	cond->func = func;
	cond->file = file;
	cond->line = line;
	HASH_ADD(hh, debug_conds, address, sizeof(cond), cond);
	hthread_unlock();
	return 0;
}

static inline int debug_cond_del (struct hthread_cond *cond, const char *command, const char *func, const char *file, const int line)
{
	struct hthread *th;
	struct hthread_cond *cv;
	hthread_lock();
	th = hthread_find_self(command);
	if (th != NULL) {
		goto found_th;
	}
	hassert((th != NULL) && "can not find self");
	hthread_unlock();
	return -1;
found_th:
	HASH_FIND(hh, debug_conds, &cond, sizeof(cond), cv);
	if (cv != NULL) {
		goto found_cv;
	}
	hdebug_lock();
	hinfof("%s with invalid condition: '%p'", command, cond);
	hinfof("    by: %s (%p)", th->name, th);
	hinfof("    at: %s %s:%d", func, file, line);
	debug_dump_callstack("        ");
	hdebug_unlock();
	hassert((cv != NULL) && "invalid condition");
	hthread_unlock();
	return -1;
found_cv:
	hdebugf("deleting cond: %s", cond->name);
	HASH_DEL(debug_conds, cv);
	hthread_unlock();
	return 0;
}

static inline int debug_cond_check (struct hthread_cond *cond, const char *command, const char *func, const char *file, const int line)
{
	struct hthread *th;
	struct hthread_cond *cv;
	hthread_lock();
	th = hthread_find_self(command);
	if (th != NULL) {
		goto found_th;
	}
	hassert((th != NULL) && "can not find self");
	hthread_unlock();
	return -1;
found_th:
	HASH_FIND(hh, debug_conds, &cond, sizeof(cond), cv);
	if (cv != NULL) {
		goto found_cv;
	}
	hdebug_lock();
	hinfof("%s with invalid condition: '%p'", command, cond);
	hinfof("    by: %s (%p)", th->name, th);
	hinfof("    at: %s %s:%d", func, file, line);
	debug_dump_callstack("        ");
	hdebug_unlock();
	hassert((cv != NULL) && "invalid condition");
	hthread_unlock();
	return -1;
found_cv:
	hthread_unlock();
	return 0;
}

struct hthread_memory {
	void *address;
	UT_hash_handle hh;
	const char *func;
	const char *file;
	int line;
	size_t size;
	char name[0];
};

static struct hthread_memory *debug_memory = NULL;

static int debug_memory_add (const char *name, void *address, size_t size, const char *command, const char *func, const char *file, const int line)
{
	unsigned int s;
	struct hthread_memory *m;
	if (address == NULL) {
		return 0;
	}
	hthread_lock();
	HASH_FIND_PTR(debug_memory, &address, m);
	if (m != NULL) {
		hdebug_lock();
		hinfof("%s with invalid memory (%p)", command, address);
		hinfof("    at: %s (%s:%d)", func, file, line);
		hinfof("  ");
		hinfof("  it is essential for correct operation of hthread that there");
		hinfof("  are no memory errors such as dangling pointers in process.");
		hinfof("  ");
		hinfof("  which means that it is a good idea to make sure that program");
		hinfof("  is clean before analyzing with hthread. it is possible however");
		hinfof("  that some memory errors are caused by data races.")
		hinfof("  ");
		hinfof("  if it is certain that program is memory bug free, then hthread");
		hinfof("  may have a serious bug that needs to be fixed urgent. please close");
		hinfof("  race condition checking for now '-DHTHREAD_ENABLE_RACE_CHECK=0',");
		hinfof("  and inform author");
		hinfof("    at: alper.akcan@gmail.com");
		hdebug_unlock();
		hassert((m == NULL) && "invalid memory");
		hthread_unlock();
		return -1;
	}
	s = sizeof(struct hthread_memory) + strlen(name) + 1;
	m = malloc(s);
	memset(m, 0, s);
	memcpy(m->name, name, strlen(name) + 1);
	m->address = address;
	m->size = size;
	m->func = func;
	m->file = file;
	m->line = line;
	HASH_ADD_PTR(debug_memory, address, m);
	hdebugf("%s added memory: %s, address: %p, size: %zd, func: %s, file: %s, line: %d", command, m->name, m->address, m->size, m->func, m->file, m->line);
	hthread_unlock();
	return 0;
}

static int debug_memory_del (void *address, const char *command, const char *func, const char *file, const int line)
{
	struct hthread_memory *m;
	if (address == NULL) {
		return 0;
	}
	hthread_lock();
	HASH_FIND_PTR(debug_memory, &address, m);
	if (m != NULL) {
		goto found_m;
	}
	hdebug_lock();
	hinfof("%s with invalid memory (%p)", command, address);
	hinfof("    at: %s (%s:%d)", func, file, line);
	hinfof("  ");
	hinfof("  it is essential for correct operation of hthread that there");
	hinfof("  are no memory errors such as dangling pointers in process.");
	hinfof("  ");
	hinfof("  which means that it is a good idea to make sure that program");
	hinfof("  is clean before analyzing with hthread. it is possible however");
	hinfof("  that some memory errors are caused by data races.")
	hinfof("  ");
	hinfof("  if it is certain that program is memory bug free, then hthread");
	hinfof("  may have a serious bug that needs to be fixed urgent. please close");
	hinfof("  race condition checking for now '-DHTHREAD_ENABLE_RACE_CHECK=0',");
	hinfof("  and inform author");
	hinfof("    at: alper.akcan@gmail.com");
	hdebug_unlock();
	hassert((m != NULL) && "invalid memory");
	hthread_unlock();
	return -1;
found_m:
	HASH_DEL(debug_memory, m);
	hdebugf("%s deleted memory: %s, address: %p, size: %zd, func: %s, file: %s, line: %d", command, m->name, m->address, m->size, m->func, m->file, m->line);
	free(m);
	hthread_unlock();
	return 0;
}

#endif
