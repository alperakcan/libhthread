/*
 *  Copyright (c) 2009-2013 Alper Akcan <alper.akcan@gmail.com>
 *
 * This program is free software. It comes without any warranty, to
 * the extent permitted by applicable law. You can redistribute it
 * and/or modify it under the terms of the Do What The Fuck You Want
 * To Public License, Version 2, as published by Sam Hocevar. See
 * http://www.wtfpl.net/ for more details.
 */

#if !defined(HTHREAD_H)
#define HTHREAD_H 1

#if !defined(HTHREAD_DISABLE_YIELD)
#define HTHREAD_DISABLE_YIELD			0
#endif

#if !defined(HTHREAD_ENABLE_CALLSTACK)
#define HTHREAD_ENABLE_CALLSTACK		1
#endif

#if defined(__DARWIN__) && (__DARWIN__ == 1)
#undef HTHREAD_ENABLE_CALLSTACK
#define HTHREAD_ENABLE_CALLSTACK		0
#endif

#if !defined(HTHREAD_REPORT_CALLSTACK)
#define HTHREAD_REPORT_CALLSTACK		1
#endif
#define HTHREAD_REPORT_CALLSTACK_NAME		"hthread_report_callstack"

#if !defined(HTHREAD_ASSERT_ON_ERROR)
#define HTHREAD_ASSERT_ON_ERROR			1
#endif
#define HTHREAD_ASSERT_ON_ERROR_NAME		"hthread_assert_on_error"

#if !defined(HTHREAD_LOCK_THRESHOLD)
#define HTHREAD_LOCK_THRESHOLD			5000
#endif
#define HTHREAD_LOCK_THRESHOLD_NAME		"hthread_lock_threshold"

#if !defined(HTHREAD_LOCK_THRESHOLD_ASSERT)
#define HTHREAD_LOCK_THRESHOLD_ASSERT		0
#endif
#define HTHREAD_LOCK_THRESHOLD_ASSERT_NAME	"hthread_lock_threshold_assert"

#if !defined(HTHREAD_LOCK_TRY_THRESHOLD)
#define HTHREAD_LOCK_TRY_THRESHOLD		5000
#endif
#define HTHREAD_LOCK_TRY_THRESHOLD_NAME		"hthread_lock_try_threshold"

#if !defined(HTHREAD_LOCK_TRY_THRESHOLD_ASSERT)
#define HTHREAD_LOCK_TRY_THRESHOLD_ASSERT	0
#endif
#define HTHREAD_LOCK_TRY_THRESHOLD_ASSERT_NAME	"hthread_lock_try_threshold_assert"

struct hthread;
struct hthread_cond;
struct hthread_mutex;

#if defined(HTHREAD_DEBUG) && (HTHREAD_DEBUG == 1)

#if !defined(HTHREAD_INTERNAL) || (HTHREAD_INTERNAL == 0)

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <errno.h>

#if defined(HTHREAD_ENABLE_RACE_CHECK) && (HTHREAD_ENABLE_RACE_CHECK == 1)

#undef memset
#define memset(b, c, len) ({ \
	void *__r; \
	__r = hthread_memset(b, c, len); \
	__r; \
})

#undef memcpy
#define memcpy(s1, s2, n) ({ \
	void *__r; \
	__r = hthread_memcpy(s1, s2, n); \
	__r; \
})

#undef strdup
#define strdup(string) ({ \
	void *__r; \
	char __n[256]; \
	snprintf(__n, 256, "strdup-%p(%s %s:%d)", string, __FUNCTION__, __FILE__, __LINE__); \
	__r = hthread_strdup((const char *) __n, string); \
	__r; \
})

#undef strndup
#define strndup(string, size) ({ \
	void *__r; \
	char __n[256]; \
	snprintf(__n, 256, "strndup-%p-%d(%s %s:%d)", string, size, __FUNCTION__, __FILE__, __LINE__); \
	__r = hthread_strndup((const char *) __n, string, size); \
	__r; \
})

#undef malloc
#define malloc(size) ({ \
	void *__r; \
	char __n[256]; \
	snprintf(__n, 256, "malloc-%d(%s %s:%d)", size, __FUNCTION__, __FILE__, __LINE__); \
	__r = hthread_malloc((const char *) __n, size); \
	__r; \
})

#undef calloc
#define calloc(nmemb, size) ({ \
	void *__r; \
	char __n[256]; \
	snprintf(__n, 256, "calloc-%d,%d(%s %s:%d)", nmemb, size, __FUNCTION__, __FILE__, __LINE__); \
	__r = hthread_calloc((const char *) __n, nmemb, size); \
	__r; \
})

#undef realloc
#define realloc(address, size) ({ \
	void *__r; \
	char __n[256]; \
	snprintf(__n, 256, "realloc-%p,%d(%s %s:%d)", address, size, __FUNCTION__, __FILE__, __LINE__); \
	__r = hthread_realloc((const char *) __n, address, size); \
	__r; \
})

#undef free
#define free(address) ({ \
	hthread_free(address); \
})

#endif

#define pthread_t struct hthread *

#define pthread_create(thread, attr, function, arg) ({ \
	int __r; \
	char __n[256]; \
	struct hthread *__t; \
	snprintf(__n, 256, "thread(%s %s:%d)", __FUNCTION__, __FILE__, __LINE__); \
	__t = hthread_create((const char *) __n, function, arg); \
	*thread = __t; \
	if (__t == NULL) { \
		__r = (int) -1; \
	} else { \
		__r = 0; \
	} \
	__r; \
})

#define pthread_join(thread, return) ({ \
	int __r; \
	struct hthread *__t; \
	__t = thread; \
	__r = hthread_join(__t); \
	__r; \
})

#define pthread_detach(thread) ({ \
	int __r; \
	struct hthread *__t; \
	__t = thread; \
	__r = hthread_detach(__t); \
	__r; \
})

#define pthread_mutex_t struct hthread_mutex *

#define pthread_mutex_init(mutex, attr) ({ \
	int __r; \
	char __n[256]; \
	struct hthread_mutex *__m; \
	snprintf(__n, 256, "mutex(%s %s:%d)", __FUNCTION__, __FILE__, __LINE__); \
	__m = hthread_mutex_init((const char *) __n); \
	*mutex = __m; \
	if (__m == NULL) { \
		__r = (int) -1; \
	} else { \
		__r = 0; \
	} \
	__r; \
})

#define pthread_mutex_destroy(mutex) ({ \
	int __r; \
	struct hthread_mutex *__m; \
	__m = *mutex; \
	__r = hthread_mutex_destroy(__m); \
	__r; \
})

#define pthread_mutex_lock(mutex) ({ \
	int __r; \
	struct hthread_mutex *__m; \
	__m = *mutex; \
	__r = hthread_mutex_lock(__m); \
	__r; \
})

#define pthread_mutex_unlock(mutex) ({ \
	int __r; \
	struct hthread_mutex *__m; \
	__m = *mutex; \
	__r = hthread_mutex_unlock(__m); \
	__r; \
})

#define pthread_cond_t struct hthread_cond *

#define pthread_cond_init(cond, attr) ({ \
	int __r; \
	char __n[256]; \
	struct hthread_cond *__c; \
	snprintf(__n, 256, "cond(%s %s:%d)", __FUNCTION__, __FILE__, __LINE__); \
	__c = hthread_cond_init(__n); \
	*cond = __c; \
	if (__c == NULL) { \
		__r = -1; \
	} else { \
		__r = 0; \
	} \
	__r; \
})

#define pthread_cond_destroy(cond) ({ \
	int __r; \
	struct hthread_cond *__c; \
	__c = *cond; \
	__r = hthread_cond_destroy(__c); \
	__r; \
})

#define pthread_cond_signal(cond) ({ \
	int __r; \
	struct hthread_cond *__c; \
	__c = *cond; \
	__r = hthread_cond_signal(__c); \
	__r; \
})

#define pthread_cond_broadcast(cond) ({ \
	int __r; \
	struct hthread_cond *__c; \
	__c = *cond; \
	__r = hthread_cond_broadcast(__c); \
	__r; \
})

#define pthread_cond_wait(cond, mutex) ({ \
	int __r; \
	struct hthread_cond *__c; \
	struct hthread_mutex *__m; \
	__c = *cond; \
	__m = *mutex; \
	__r = hthread_cond_wait(__c, __m); \
	if (__r == 1) { \
		__r = ETIMEDOUT; \
	} \
	__r; \
})

#define pthread_cond_timedwait(cond, mutex, tspec) ({ \
	int __r; \
	struct hthread_cond *__c; \
	struct hthread_mutex *__m; \
	__c = *cond; \
	__m = *mutex; \
	__r = hthread_cond_timedwait_tspec(__c, __m, tspec); \
	if (__r == 1) { \
		__r = ETIMEDOUT; \
	} \
	__r; \
})

#endif

#define HTHREAD_FUNCTION_NAME(function) hthread_ ## function ## _debug

#else

#include <pthread.h>

#define HTHREAD_FUNCTION_NAME(function) hthread_ ## function

#endif

#define hthread_memset(a, b, c)               HTHREAD_FUNCTION_NAME(memset_actual)(a, b, c, __FUNCTION__, __FILE__, __LINE__)
#define hthread_memcpy(a, b, c)               HTHREAD_FUNCTION_NAME(memcpy_actual)(a, b, c, __FUNCTION__, __FILE__, __LINE__)

#define hthread_strdup(a, b)                  HTHREAD_FUNCTION_NAME(strdup_actual)(a, b, __FUNCTION__, __FILE__, __LINE__)
#define hthread_strndup(a, b, c)              HTHREAD_FUNCTION_NAME(strndup_actual)(a, b, c, __FUNCTION__, __FILE__, __LINE__)
#define hthread_malloc(a, b)                  HTHREAD_FUNCTION_NAME(malloc_actual)(a, b, __FUNCTION__, __FILE__, __LINE__)
#define hthread_calloc(a, b, c)               HTHREAD_FUNCTION_NAME(calloc_actual)(a, b, c, __FUNCTION__, __FILE__, __LINE__)
#define hthread_realloc(a, b, c)              HTHREAD_FUNCTION_NAME(realloc_actual)(a, b, c, __FUNCTION__, __FILE__, __LINE__)
#define hthread_free(a)                       HTHREAD_FUNCTION_NAME(free_actual)(a, __FUNCTION__, __FILE__, __LINE__)

#define hthread_self()                        HTHREAD_FUNCTION_NAME(self_actual)(__FUNCTION__, __FILE__, __LINE__)
#define hthread_sched_yield()                 HTHREAD_FUNCTION_NAME(sched_yield_actual)(__FUNCTION__, __FILE__, __LINE__)

#define hthread_create(a, b, c)               HTHREAD_FUNCTION_NAME(create_actual)(a, b, c, __FUNCTION__, __FILE__, __LINE__)
#define hthread_detach(a)                     HTHREAD_FUNCTION_NAME(detach_actual)(a, __FUNCTION__, __FILE__, __LINE__)
#define hthread_join(a)                       HTHREAD_FUNCTION_NAME(join_actual)(a, __FUNCTION__, __FILE__, __LINE__)

#define hthread_mutex_init(a)                 HTHREAD_FUNCTION_NAME(mutex_init_actual)(a, __FUNCTION__, __FILE__, __LINE__)
#define hthread_mutex_lock(a)                 HTHREAD_FUNCTION_NAME(mutex_lock_actual)(a, __FUNCTION__, __FILE__, __LINE__)
#define hthread_mutex_unlock(a)               HTHREAD_FUNCTION_NAME(mutex_unlock_actual)(a, __FUNCTION__, __FILE__, __LINE__)
#define hthread_mutex_destroy(a)              HTHREAD_FUNCTION_NAME(mutex_destroy_actual)(a, __FUNCTION__, __FILE__, __LINE__)

#define hthread_cond_init(a)                  HTHREAD_FUNCTION_NAME(cond_init_actual)(a, __FUNCTION__, __FILE__, __LINE__)
#define hthread_cond_wait(a, b)               HTHREAD_FUNCTION_NAME(cond_wait_actual)(a, b, __FUNCTION__, __FILE__, __LINE__)
#define hthread_cond_timedwait(a, b, c)       HTHREAD_FUNCTION_NAME(cond_timedwait_actual)(a, b, c, __FUNCTION__, __FILE__, __LINE__)
#define hthread_cond_timedwait_tspec(a, b, c) HTHREAD_FUNCTION_NAME(cond_timedwait_tspec_actual)(a, b, c, __FUNCTION__, __FILE__, __LINE__)
#define hthread_cond_signal(a)                HTHREAD_FUNCTION_NAME(cond_signal_actual)(a, __FUNCTION__, __FILE__, __LINE__)
#define hthread_cond_broadcast(a)             HTHREAD_FUNCTION_NAME(cond_broadcast_actual)(a, __FUNCTION__, __FILE__, __LINE__)
#define hthread_cond_destroy(a)               HTHREAD_FUNCTION_NAME(cond_destroy_actual)(a, __FUNCTION__, __FILE__, __LINE__)

void * HTHREAD_FUNCTION_NAME(memset_actual) (void *destination, int c, size_t len, const char *func, const char *file, const int line);
void * HTHREAD_FUNCTION_NAME(memcpy_actual) (void *destination, void *source, size_t len, const char *func, const char *file, const int line);

char * HTHREAD_FUNCTION_NAME(strdup_actual) (const char *name, const char *string, const char *func, const char *file, const int line);
char * HTHREAD_FUNCTION_NAME(strndup_actual) (const char *name, const char *string, size_t size, const char *func, const char *file, const int line);
void * HTHREAD_FUNCTION_NAME(malloc_actual) (const char *name, size_t size, const char *func, const char *file, const int line);
void * HTHREAD_FUNCTION_NAME(calloc_actual) (const char *name, size_t nmemb, size_t size, const char *func, const char *file, const int line);
void * HTHREAD_FUNCTION_NAME(realloc_actual) (const char *name, void *address, size_t size, const char *func, const char *file, const int line);
void HTHREAD_FUNCTION_NAME(free_actual) (void *address, const char *func, const char *file, const int line);

struct hthread * HTHREAD_FUNCTION_NAME(self_actual) (const char *func, const char *file, const int line);
int HTHREAD_FUNCTION_NAME(sched_yield_actual) (const char *func, const char *file, const int line);

struct hthread * HTHREAD_FUNCTION_NAME(create_actual) (const char *name, void * (*function) (void *), void *arg, const char *func, const char *file, const int line);
int HTHREAD_FUNCTION_NAME(detach_actual) (struct hthread *thread, const char *func, const char *file, const int line);
int HTHREAD_FUNCTION_NAME(join_actual) (struct hthread *thread, const char *func, const char *file, const int line);

struct hthread_mutex * HTHREAD_FUNCTION_NAME(mutex_init_actual) (const char *name, const char *func, const char *file, const int line);
int HTHREAD_FUNCTION_NAME(mutex_lock_actual) (struct hthread_mutex *mutex, const char *func, const char *file, const int line);
int HTHREAD_FUNCTION_NAME(mutex_unlock_actual) (struct hthread_mutex *mutex, const char *func, const char *file, const int line);
int HTHREAD_FUNCTION_NAME(mutex_destroy_actual) (struct hthread_mutex *mutex, const char *func, const char *file, const int line);

struct hthread_cond * HTHREAD_FUNCTION_NAME(cond_init_actual) (const char *name, const char *func, const char *file, const int line);
int HTHREAD_FUNCTION_NAME(cond_wait_actual) (struct hthread_cond *cond, struct hthread_mutex *mutex, const char *func, const char *file, const int line);
int HTHREAD_FUNCTION_NAME(cond_timedwait_actual) (struct hthread_cond *cond, struct hthread_mutex *mutex, int msec, const char *func, const char *file, const int line);
int HTHREAD_FUNCTION_NAME(cond_timedwait_tspec_actual) (struct hthread_cond *cond, struct hthread_mutex *mutex, struct timespec *tspec, const char *func, const char *file, const int line);
int HTHREAD_FUNCTION_NAME(cond_signal_actual) (struct hthread_cond *cond, const char *func, const char *file, const int line);
int HTHREAD_FUNCTION_NAME(cond_broadcast_actual) (struct hthread_cond *cond, const char *func, const char *file, const int line);
int HTHREAD_FUNCTION_NAME(cond_destroy_actual) (struct hthread_cond *cond, const char *func, const char *file, const int line);

#endif
