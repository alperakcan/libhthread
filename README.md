# hthread #

  hthread is a threads helper library with synchronization errors detection support.

## 1. overview ##

  hthread is for detecting synchronization errors in c/c++ programs that use the pthreads.

  can detect errors of:
  
  1. misuses of pthreads api
  2. lock ordering violation
  3. lock contention

### 1.1 misuses of pthreads api ###

  hthread is able to detect and report following errors. although some of them are too obvious, early detection is much better
  than to deal with hard-to-find bugs.
  
  - destroying an invalid mutex - <a href="test/fail-00.c">fail-00.c</a>
  - locking an invalid mutex - <a href="test/fail-01.c">fail-01.c</a>
  - unlocking an invalid mutex - <a href="test/fail-02.c">fail-02.c</a>
  - locking an already locked mutex - <a href="test/fail-03.c">fail-03.c</a>
  - unlocking an unheld mutex - <a href="test/fail-05.c">fail-05.c</a>
  - destroying a locked mutex - <a href="test/fail-06.c">fail-06.c</a>
  - destroying an invalid condition - <a href="test/fail-20.c">fail-20.c</a>
  - signaling an invalid condition - <a href="test/fail-21.c">fail-21.c</a>
  - broadcasting an invalid condition - <a href="test/fail-22.c">fail-22.c</a>
  - [timed]waiting on an invalid condition - <a href="test/fail-23.c">fail-23.c</a>
  - [timed]waiting on an invalid mutex - <a href="test/fail-24.c">fail-24.c</a>
  - [timed]waiting on an unheld mutex - <a href="test/fail-25.c">fail-25.c</a>
  - join invalid thread - <a href="test/fail-40.c">fail-40.c</a>
  - detach invalid thread - <a href="test/fail-41.c">fail-41.c</a>
  - unlocking mutex that was held by other thread - <a href="test/fail-42.c">fail-42.c</a>

### 1.2 lock ordering violation ###

  hthread is able to detect inconsistent locking orders, it is very useful because they usually result to deadlocks. they may never
  be discovered during testing and lead to hard-to-find bugs.
  
  hthread monitors locks/unlocks and stores them in separate tables for each thread. this allows hthread to build a global lock order
  table for running process. hthread check for locking order violation against global order table for each lock request, and able to
  detect and report following errors.

  - lock order violation in same thread - <a href="test/fail-04.c">fail-04.c</a>
  - lock order violation while [timed]waiting on condition - <a href="test/fail-26.c">fail-26.c</a>
  - lock order violation between threads - <a href="test/fail-43.c">fail-43.c</a>

### 1.3 lock contention ###

  hthread able to understans and print report about lock contentions - a thread has to wait until requested lock is released. monitoring
  lock contentions is handy because, they usually cause unwanted delays or they may point to an undetected potential deadlock. hthread
  can report followings.

  - waiting to lock a mutex more than allowed threshold - <a href="test/fail-60.c">fail-60.c</a>
  - hold a mutex lock more than allowed threshold - <a href="test/fail-61.c">fail-61.c</a>

## 2. configuration ##

  hthread reads configuration parameters from environment via getenv function call. one can either set/change environment variables in source
  code of monitored project via setenv function call, or set them globally in running shell using export function.
  
  - hthread_assert_on_error (default yes): terminate the process on any pthreads api misuse and/or lock order violation. 
  - hthread_lock_hreashold (default 5000 miliseconds): print report if lock is held longer than the specified time, in miliseconds.
  - hthread_lock_hreashold_assert (default no): terminate if lock is held longer than the specified time, in miliseconds.
  - hthread_lock_try_threshold (default 5000 miliseconds): print report if locking operation takes longer than the specified time, in miliseconds.
  - hthread_lock_try_threshold_assert (default no): terminate if locking operation takes longer than the specified time, in miliseconds.

## 3. license ##

  Copyright (C) 2009-2013 Alper Akcan <alper.akcan@gmail.com>

  This work is free. It comes without any warranty, to the extent permitted
  by applicable law. You can redistribute it and/or modify it under the terms
  of the Do What The Fuck You Want To Public License, Version 2, as published
  by Sam Hocevar. See the COPYING file for more details.
