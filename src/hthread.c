/*
 * Alper Akcan - 03.10.2009
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <errno.h>
#include <unistd.h>
#include <pthread.h>
#include <assert.h>
#include <sys/queue.h>

#define HTHREAD_INTERNAL	1
#define HTHREAD_DISABLE_YIELD	0

#include "hthread.h"
#include "uthash.h"

static pthread_mutex_t debugf_mutex = PTHREAD_MUTEX_INITIALIZER;
#if 0
#define hdebugf(a...) { \
	pthread_mutex_lock(&debugf_mutex); \
	fprintf(stderr, "hthread::debug: "); \
	fprintf(stderr, a); \
	fprintf(stderr, " (%s %s:%d)\n", __FUNCTION__, __FILE__, __LINE__); \
	pthread_mutex_unlock(&debugf_mutex); \
}
#else
#define hdebugf(a...)
#endif

#define hinfof(a...) { \
	pthread_mutex_lock(&debugf_mutex); \
	fprintf(stderr, "(hthread) "); \
	fprintf(stderr, a); \
	fprintf(stderr, "\n"); \
	pthread_mutex_unlock(&debugf_mutex); \
}

#define herrorf(a...) { \
	pthread_mutex_lock(&debugf_mutex); \
	fprintf(stderr, "hthread::error: "); \
	fprintf(stderr, a); \
	fprintf(stderr, " (%s %s:%d)\n", __FUNCTION__, __FILE__, __LINE__); \
	pthread_mutex_unlock(&debugf_mutex); \
}

#define hassert(a) { \
	assert(a); \
}

#define hassertf(a...) { \
	pthread_mutex_lock(&debugf_mutex); \
	fprintf(stderr, "hthread::assert: "); \
	fprintf(stderr, a); \
	fprintf(stderr, " (%s %s:%d)\n", __FUNCTION__, __FILE__, __LINE__); \
	assert(0); \
	pthread_mutex_unlock(&debugf_mutex); \
}

#if defined(HTHREAD_DEBUG) && (HTHREAD_DEBUG == 1)

static inline int debug_thread_add (struct hthread *thread, const char *func, const char *file, const int line);
static inline int debug_thread_del (struct hthread *thread, const char *command, const char *func, const char *file, const int line);
static inline int debug_mutex_add_lock (struct hthread_mutex *mutex, const char *command, const char *func, const char *file, const int line);
static inline int debug_mutex_find_lock (struct hthread_mutex *mutex, const char *func, const char *file, const int line);
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
#define debug_thread_add(a...)
#define debug_thread_del(a...)          debug_thread_unused()
#define debug_mutex_add_lock(a...)	debug_thread_unused()
#define debug_mutex_del_lock(a...)	debug_thread_unused()
#define debug_mutex_add(a...)		debug_thread_unused()
#define debug_mutex_del(a...)           debug_thread_unused()
#define debug_cond_add(a...)		debug_thread_unused()
#define debug_cond_del(a...)            debug_thread_unused()
#define debug_cond_check(a...)          debug_thread_unused()

#endif

struct hthread_arg {
	char *name;
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
#if defined(HTHREAD_DEBUG) && (HTHREAD_DEBUG == 1)
	LIST_ENTRY(hthread) list;
	struct hthread_mutex_lock *locks;
	struct hthread_cond *conds;
	const char *func;
	const char *file;
	int line;
#endif
        pthread_t thread;
        char name[0];
};

struct hthread_cond {
#if defined(HTHREAD_DEBUG) && (HTHREAD_DEBUG == 1)
	void *address;
	UT_hash_handle hh;
	const char *func;
	const char *file;
	int line;
#endif
        pthread_cond_t cond;
        char name[0];
};

struct hthread_mutex {
#if defined(HTHREAD_DEBUG) && (HTHREAD_DEBUG == 1)
	void *address;
	UT_hash_handle hh;
	const char *func;
	const char *file;
	int line;
#endif
	pthread_mutex_t mutex;
	char name[0];
};

struct hthread_cond * HTHREAD_FUNCTION_NAME(cond_init_actual) (const char *name, const char *func, const char *file, const int line)
{
	struct hthread_cond *c;
	c = (struct hthread_cond *) malloc(sizeof(struct hthread_cond) + strlen(name) + 1);
	if (c == NULL) {
		hassertf("malloc failed");
		return NULL;
	}
	memset(c, 0, sizeof(struct hthread_cond) + strlen(name) + 1);
	memcpy(c->name, name, strlen(name) + 1);
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
	struct hthread_mutex *m;
	m = (struct hthread_mutex *) malloc(sizeof(struct hthread_mutex) + strlen(name) + 1);
	if (m == NULL) {
		hassertf("malloc failed");
		return NULL;
	}
	memset(m, 0, sizeof(struct hthread_mutex) + strlen(name) + 1);
	memcpy(m->name, name, strlen(name) + 1);
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
	int r;
	debug_mutex_add_lock(mutex, "mutex lock", func, file, line);
#if defined(HTHREAD_DEBUG) && (HTHREAD_DEBUG == 1)
	unsigned int t = 0;
	while (1) {
		r = pthread_mutex_trylock(&mutex->mutex);
		if (r == 0) {
			break;
		}
		usleep(20000);
		t += 1;
		if (t >= (1000000 / 20000) * 10) {
			herrorf("still waiting for %s mutex @ (%s %s:%d)", mutex->name, func, file, line);
			debug_mutex_find_lock(mutex, func, file, line);
			t = 0;
		}
	}
#else
	r = pthread_mutex_lock(&mutex->mutex);
#endif
	return r;
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
        debug_thread_add(arg->thread, arg->func, arg->file, arg->line);
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
        struct hthread *tid;
        struct hthread_arg *arg;
        tid = (struct hthread *) malloc(sizeof(struct hthread) + strlen(name) + 1);
        if (tid == NULL) {
        	hassertf("malloc failed");
        	return NULL;
        }
        memset(tid, 0, sizeof(struct hthread) + strlen(name) + 1);
        memcpy(tid->name, name, strlen(name) + 1);
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
        arg->name = tid->name;
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
	debug_thread_del(thread, "join", func, file, line);
	pthread_join(thread->thread, NULL);
	free(thread);
        return 0;
}

int HTHREAD_FUNCTION_NAME(detach_actual) (struct hthread *thread, const char *func, const char *file, const int line)
{
	if (thread == NULL) {
		return 0;
	}
	debug_thread_del(thread, "detach", func, file, line);
	pthread_detach(thread->thread);
	free(thread);
        return 0;
}

unsigned int HTHREAD_FUNCTION_NAME(self_actual) (const char *func, const char *file, const int line)
{
	(void) func;
	(void) file;
	(void) line;
	return (unsigned int) pthread_self();
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

#if defined(HTHREAD_DEBUG) && (HTHREAD_DEBUG == 1)

struct hthread_mutex_lock {
	struct hthread_mutex *mutex;
	struct hthread *thread;
	const char *func;
	const char *file;
	int line;
	UT_hash_handle hh;
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

static pthread_mutex_t debug_mutex = PTHREAD_MUTEX_INITIALIZER;
#define debug_thread_lock()   pthread_mutex_lock(&debug_mutex)
#define debug_thread_unlock() pthread_mutex_unlock(&debug_mutex)
#define debug_thread_self()   pthread_self()

static LIST_HEAD(debug_threads, hthread) debug_threads = LIST_HEAD_INITIALIZER(debug_threads);
static struct hthread_cond *debug_conds = NULL;
static struct hthread_mutex *debug_mutexes = NULL;
static struct hthread_mutex_order *debug_orders = NULL;

static inline int debug_thread_add_actual (struct hthread *thread, const char *func, const char *file, const int line)
{
	struct hthread *th;
	(void) func;
	(void) file;
	(void) line;
	LIST_FOREACH(th, &debug_threads, list) {
		if (th == thread) {
			hassertf("thread: %s is already in list", thread->name);
			debug_thread_unlock();
			return -1;
		}
	}
	thread->func = func;
	thread->file = file;
	thread->line = line;
	thread->conds = NULL;
	thread->locks = NULL;
	LIST_INSERT_HEAD(&debug_threads, thread, list);
	hinfof("thread: %s (%p) created", thread->name, thread);
	hinfof("    at: %s %s:%d", func, file, line);
	return 0;
}

static inline struct hthread * debug_thread_add_root (const char *command)
{
	struct hthread *th;
	LIST_FOREACH(th, &debug_threads, list) {
		if (strcmp(th->name, "root-process") != 0) {
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
		hinfof("%s within unknown thread", command);
		hassert(0 && "invalid thread");
	}
	th = malloc(sizeof(struct hthread) + strlen("root-process") + 1);
	if (th == NULL) {
		hassertf("malloc failed");
		return NULL;
	}
	memset(th, 0, sizeof(struct hthread) + strlen("root-process") + 1);
	memcpy(th->name, "root-process", strlen("root-process") + 1);
	th->thread = debug_thread_self();
	debug_thread_add_actual(th, NULL, NULL, 0);
	return th;
}

static inline int debug_thread_add (struct hthread *thread, const char *func, const char *file, const int line)
{
	(void) func;
	(void) file;
	(void) line;
	debug_thread_lock();
	debug_thread_add_actual(thread, func, file, line);
	debug_thread_unlock();
	return 0;
}

static inline int debug_thread_del (struct hthread *thread, const char *command, const char *func, const char *file, const int line)
{
	struct hthread *th;
	struct hthread *sth;
	(void) func;
	(void) file;
	(void) line;
	debug_thread_lock();
	LIST_FOREACH(sth, &debug_threads, list) {
		if (sth->thread == debug_thread_self()) {
			goto found_sth;
		}
	}
	sth = debug_thread_add_root(command);
found_sth:
	LIST_FOREACH(th, &debug_threads, list) {
		if (th == thread) {
			goto found_th;
		}
	}
	hinfof("thread: %s (%p): %s with invalid argument '%p'", sth->name, sth, command, thread);
	hinfof("    at: %s %s:%d", func, file, line);
	hassert(0 && "invalid thread");
	debug_thread_unlock();
	return -1;
found_th:
	hinfof("thread: %s (%p) deleted", thread->name, thread);
	hinfof("  created at: %s %s:%d", thread->func, thread->file, thread->line);
	hinfof("  deleted by: %s (%p)", sth->name, sth);
	hinfof("          at: %s %s:%d", func, file, line);
	LIST_REMOVE(th, list);
	debug_thread_unlock();
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
	debug_thread_lock();
	LIST_FOREACH(th, &debug_threads, list) {
		if (th->thread == debug_thread_self()) {
			goto found_th;
		}
	}
	th = debug_thread_add_root(command);
found_th:
	HASH_FIND_PTR(debug_mutexes, &mutex, mt);
	if (mt != NULL) {
		goto found_mt;
	}
	hinfof("thread: %s (%p): %s with invalid mutex '%p'", th->name, th, command, mutex);
	hinfof("    at: %s %s:%d", func, file, line);
	hassert(0 && "invalid mutex");
	debug_thread_unlock();
	return -1;
found_mt:
	HASH_FIND_PTR(th->locks, mutex, mtl);
	if (mtl != NULL) {
		hinfof("thread: %s (%p): %s with already hold mutex %s (%p)", th->name, th, command, mutex->name, mutex);
		hinfof("    at: %s %s:%d", func, file, line);
		hinfof("  previously acquired");
		hinfof("    by: %s (%p)", mtl->thread->name, mtl->thread);
		hinfof("    at: %s %s:%d", mtl->func, mtl->file, mtl->line);
		hassert(0 && "mutex is already locked");
		debug_thread_unlock();
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
			hinfof("thread: %s (%p): %s order '%s (%p) before %s (%p)' violated", th->name, th, command, mutex->name, mutex, mtl->mutex->name, mtl->mutex);
			hinfof("  incorrect order is: acquisition of %s (%p)", mtl->mutex->name, mtl->mutex);
			hinfof("      by: %s (%p)", th->name, th);
			hinfof("      at: %s %s:%d", mtl->func, mtl->file, mtl->line);
			hinfof("    followed by a later acquisition of %s (%p)", mutex->name, mutex);
			hinfof("      by: %s (%p)", th->name, th);
			hinfof("      at: %s %s:%d", func, file, line);
			hinfof("  required order is: acquisition of %s (%p)", mto->key.first->name, mto->key.first);
			hinfof("      by: %s (%p)", mto->info.first.thread->name, mto->info.first.thread);
			hinfof("      at: %s %s:%d", mto->info.first.func, mto->info.first.file, mto->info.first.line);
			hinfof("    followed by a later acquisition of %s (%p)", mto->key.second->name, mto->key.second);
			hinfof("      by: %s (%p)", mto->info.second.thread->name, mto->info.second.thread);
			hinfof("      at: %s %s:%d", mto->info.second.func, mto->info.second.file, mto->info.second.line);
			hassert(0 && "lock order violation");
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
		debug_thread_unlock();
		return -1;
	}
	memset(nmtl, 0, sizeof(struct hthread_mutex_lock));
	nmtl->thread = th;
	nmtl->mutex = mutex;
	nmtl->func = func;
	nmtl->file = file;
	nmtl->line = line;
	HASH_ADD_PTR(th->locks, mutex, nmtl);
	hdebugf("added lock mutex: %s @ %p, to thread: %s @ %p", mutex->name, mutex, th->name, th);
	debug_thread_unlock();
	return 0;
}

static inline int debug_mutex_find_lock (struct hthread_mutex *mutex, const char *func, const char *file, const int line)
{
	struct hthread *th;
	struct hthread_mutex *mt;
	struct hthread_mutex_lock *mtl;
	(void) func;
	(void) file;
	(void) line;
	HASH_FIND_PTR(debug_mutexes, &mutex, mt);
	if (mt != NULL) {
		goto found_mt;
	}
	hassertf("can not find mutex: %s in list", mutex->name);
	return -1;
found_mt:
	LIST_FOREACH(th, &debug_threads, list) {
		HASH_FIND_PTR(th->locks, mutex, mtl);
		if (mtl != NULL) {
			herrorf("mutex: %s is already locked @ (%s %s:%d)", mutex->name, mtl->func, mtl->file, mtl->line);
		}
	}
	return 0;
}

static inline int debug_mutex_del_lock (struct hthread_mutex *mutex, const char *command, const char *func, const char *file, const int line)
{
	struct hthread *th;
	struct hthread_mutex *mt;
	struct hthread_mutex_lock *mtl;
	debug_thread_lock();
	LIST_FOREACH(th, &debug_threads, list) {
		if (th->thread == debug_thread_self()) {
			goto found_th;
		}
	}
	th = debug_thread_add_root(command);
found_th:
	HASH_FIND_PTR(debug_mutexes, &mutex, mt);
	if (mt != NULL) {
		goto found_mt;
	}
	hinfof("thread: %s (%p): %s with invalid mutex '%p'", th->name, th, command, mutex);
	hinfof("    at: %s %s:%d", func, file, line);
	hassert(0 && "invalid mutex");
	debug_thread_unlock();
	return -1;
found_mt:
	HASH_FIND_PTR(th->locks, mutex, mtl);
	if (mtl != NULL) {
		goto found_lc;
	}
	hinfof("thread: %s (%p): %s with un-held mutex '%s (%p)'", th->name, th, command, mutex->name, mutex);
	hinfof("  by: %s (%p)", th->name, th);
	hinfof("  at: %s %s:%d", func, file, line);
	hassert(0 && "mutex is not locked");
	debug_thread_unlock();
	return -1;
found_lc:
	HASH_FIND_PTR(th->locks, mutex, mtl);
	if (mtl != NULL) {
		HASH_DEL(th->locks, mtl);
		hdebugf("deleted lock mutex: %s @ %p, from thread: %s @ %p", mutex->name, mutex, th->name, th);
		free(mtl);
	}
	debug_thread_unlock();
	return 0;
}

static inline int debug_mutex_add (struct hthread_mutex *mutex, const char *command, const char *func, const char *file, const int line)
{
	struct hthread *th;
	struct hthread_mutex *mt;
	debug_thread_lock();
	LIST_FOREACH(th, &debug_threads, list) {
		if (th->thread == debug_thread_self()) {
			goto found_th;
		}
	}
	th = debug_thread_add_root(command);
found_th:
	HASH_FIND_PTR(debug_mutexes, &mutex, mt);
	if (mt != NULL) {
		hassertf("mutex: %s is already in list", mutex->name);
		debug_thread_unlock();
		return -1;
	}
	hdebugf("adding mutex: %s", mutex->name);
	mutex->address = mutex;
	mutex->func = func;
	mutex->file = file;
	mutex->line = line;
	HASH_ADD_PTR(debug_mutexes, address, mutex);
	debug_thread_unlock();
	return 0;
}

static inline int debug_mutex_del (struct hthread_mutex *mutex, const char *command, const char *func, const char *file, const int line)
{
	struct hthread *th;
	struct hthread_mutex *mt;
	struct hthread_mutex_lock *mtl;
	struct hthread_mutex_order *mto;
	struct hthread_mutex_order *nmto;
	(void) func;
	(void) file;
	(void) line;
	debug_thread_lock();
	LIST_FOREACH(th, &debug_threads, list) {
		if (th->thread == debug_thread_self()) {
			goto found_th;
		}
	}
	th = debug_thread_add_root(command);
found_th:
	HASH_FIND_PTR(debug_mutexes, &mutex, mt);
	if (mt != NULL) {
		goto found_mt;
	}
	hinfof("thread: %s (%p): %s with invalid mutex '%p'", th->name, th, command, mutex);
	hinfof("    at: %s %s:%d", func, file, line);
	hassert(0 && "invalid mutex");
	debug_thread_unlock();
	return -1;
found_mt:
	HASH_FIND_PTR(th->locks, mutex, mtl);
	if (mtl != NULL) {
		hassertf("mutex: %s is still locked", mutex->name);
		debug_thread_unlock();
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
	debug_thread_unlock();
	return 0;
}

static inline int debug_cond_add (struct hthread_cond *cond, const char *command, const char *func, const char *file, const int line)
{
	struct hthread *th;
	struct hthread_cond *cv;
	debug_thread_lock();
	LIST_FOREACH(th, &debug_threads, list) {
		if (th->thread == debug_thread_self()) {
			goto found_th;
		}
	}
	th = debug_thread_add_root(command);
found_th:
	HASH_FIND_PTR(debug_conds, &cond, cv);
	if (cv != NULL) {
		hassertf("cond: %s is already in list", cond->name);
		debug_thread_unlock();
		return -1;
	}
	hdebugf("adding cond: %s", cond->name);
	cond->address = cond;
	cond->func = func;
	cond->file = file;
	cond->line = line;
	HASH_ADD_PTR(debug_conds, address, cond);
	debug_thread_unlock();
	return 0;
}

static inline int debug_cond_del (struct hthread_cond *cond, const char *command, const char *func, const char *file, const int line)
{
	struct hthread *th;
	struct hthread_cond *cv;
	debug_thread_lock();
	LIST_FOREACH(th, &debug_threads, list) {
		if (th->thread == debug_thread_self()) {
			goto found_th;
		}
	}
	th = debug_thread_add_root(command);
found_th:
	HASH_FIND_PTR(debug_conds, &cond, cv);
	if (cv != NULL) {
		goto found_cv;
	}
	hinfof("thread: %s (%p): %s with invalid condition '%p'", th->name, th, command, cond);
	hinfof("    at: %s %s:%d", func, file, line);
	hassert((cv != NULL) && "invalid condition");
	debug_thread_unlock();
	return -1;
found_cv:
	hdebugf("deleting cond: %s", cond->name);
	HASH_DEL(debug_conds, cv);
	debug_thread_unlock();
	return 0;
}

static inline int debug_cond_check (struct hthread_cond *cond, const char *command, const char *func, const char *file, const int line)
{
	struct hthread *th;
	struct hthread_cond *cv;
	debug_thread_lock();
	LIST_FOREACH(th, &debug_threads, list) {
		if (th->thread == debug_thread_self()) {
			goto found_th;
		}
	}
	th = debug_thread_add_root(command);
found_th:
	HASH_FIND_PTR(debug_conds, &cond, cv);
	if (cv != NULL) {
		goto found_cv;
	}
	hinfof("thread: %s (%p): %s with invalid condition '%p'", th->name, th, command, cond);
	hinfof("    at: %s %s:%d", func, file, line);
	hassert((cv != NULL) && "invalid condition");
	debug_thread_unlock();
	return -1;
found_cv:
	debug_thread_unlock();
	return 0;
}

#endif
