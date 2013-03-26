# hthread #

hthread is a threads helper library with synchronization errors detection support.

## 1. overview ##

hthread is for detecting synchronization errors in c/c++ programs that use the pthreads.

can detect errors of:
  1. misuses of pthreads api
  2. lock ordering violation
  3. lock contention

### 1.1 misuses of pthreads api ###

  - destroying an invalid mutex (<a href="https://github.com/anhanguera/libhthread/blob/master/test/fail-00.c">test 00</a>)
  - locking an invalid mutex (test 01)
  - unlocking an invalid mutex (test 02)
  - locking an already locked mutex (test 03)
  - unlocking an unheld mutex (test 05)
  - destroying a locked mutex (test 06)
  - destroying an invalid condition (test 20)
  - signaling an invalid condition (test 21)
  - broadcasting an invalid condition (test 22)
  - [timed]waiting on an invalid condition (test 23)
  - [timed]waiting on an invalid mutex (test 24)
  - [timed]waiting on an unheld mutex (test 25)
  - join invalid thread (test 40)
  - detach invalid thread (test 41)
  - unlocking mutex that was held by other thread (test 42)

### 1.2 lock ordering violation ###

  - lock order violation in same thread (test 04)
  - lock order violation while [timed]waiting on condition (test 26)
  - lock order violation between threads (test 43)

### 1.3 lock contention ###

  - waiting to lock a mutex more than allowed threshold (test 60)
  - hold a mutex lock more than allowed threshold (test 61)

## 2. configuration ##

## 3. license ##

Copyright (C) 2009-2013 Alper Akcan <alper.akcan@gmail.com>

This work is free. It comes without any warranty, to the extent permitted
by applicable law. You can redistribute it and/or modify it under the terms
of the Do What The Fuck You Want To Public License, Version 2, as published
by Sam Hocevar. See the COPYING file for more details.
