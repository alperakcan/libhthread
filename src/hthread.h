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

#define HTHREAD_DISABLE_YIELD			0

#define HTHREAD_LOCK_THRESHOLD			10000
#define HTHREAD_LOCK_THRESHOLD_NAME		"hthread_lock_threshold"

#define HTHREAD_LOCK_THRESHOLD_ASSERT		0
#define HTHREAD_LOCK_THRESHOLD_ASSERT_NAME	"hthread_lock_threshold_assert"

#define HTHREAD_LOCK_TRY_THRESHOLD		10000
#define HTHREAD_LOCK_TRY_THRESHOLD_NAME		"hthread_lock_try_threshold"

#define HTHREAD_LOCK_TRY_THRESHOLD_ASSERT	0
#define HTHREAD_LOCK_TRY_THRESHOLD_ASSERT_NAME	"hthread_lock_try_threshold_assert"

struct hthread;
struct hthread_cond;
struct hthread_mutex;

#if defined(HTHREAD_DEBUG) && (HTHREAD_DEBUG == 1)

#if !defined(HTHREAD_INTERNAL) || (HTHREAD_INTERNAL == 0)

#include <stdio.h>
#include <pthread.h>
#include <errno.h>

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

#define HTHREAD_FUNCTION_NAME(function) hthread_ ## function

#endif

#define hthread_self()                        HTHREAD_FUNCTION_NAME(self_actual)(, __FUNCTION__, __FILE__, __LINE__)
#define hthread_sched_yield()                 HTHREAD_FUNCTION_NAME(sched_yield_actual)(, __FUNCTION__, __FILE__, __LINE__)

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
