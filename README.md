# hthread #

  hthread is a thread error detector and helper library with synchronization errors detection support for
  c/c++ programs that use the pthreads.
  
  1. <a href="#1-overview">overview</a>
  2. <a href="#2-configuration">configuration</a>
  3. <a href="#3-error-reports">error reports</a>
  4. <a href="#4-test-cases">test cases</a>
  5. <a href="#5-usage-example">usage example</a>
  6. <a href="#6-contact">contact</a>
  7. <a href="#7-license">license</a>

## 1. overview ##
  
  hthread is a thread error detector and helper library with synchronization errors detection support for
  c/c++ programs that use the pthreads.

  hthread is a lightweight thread error detector and helper library with synchronization error detection
  support, specifically designed for embedded systems.
  
  main use case may include embedded systems where <a href="http://valgrind.org">valgrind</a> 
  <a href="http://valgrind.org/docs/manual/drd-manual.html">drd</a>, or
  <a href="http://valgrind.org/docs/manual/hg-manual.html>helgrind</a> support <b>is not</b> available.
  

### 1.1 misuses of pthreads api ###

  hthread is able to detect and report following errors. although some of them are too obvious, early detection is much better
  than to deal with hard-to-find bugs.
  
  1. destroying an invalid mutex - <a href="test/fail-00.c">fail-00.c</a>, <a href="#311-destroying-an-invalid-mutex">report</a>
  2. locking an invalid mutex - <a href="test/fail-01.c">fail-01.c</a>, <a href="#312-locking-an-invalid-mutex">report</a>
  3. unlocking an invalid mutex - <a href="test/fail-02.c">fail-02.c</a>, <a href="#313-unlocking-an-invalid-mutex">report</a>
  4. locking an already locked mutex - <a href="test/fail-03.c">fail-03.c</a>, <a href="#314-locking-an-already-locked-mutex">report</a>
  5. unlocking an unheld mutex - <a href="test/fail-05.c">fail-05.c</a>, <a href="#315-unlocking-an-unheld-mutex">report</a>
  6. destroying a locked mutex - <a href="test/fail-06.c">fail-06.c</a>, <a href="#316-destroying-a-locked-mutex">report</a>
  7. destroying an invalid condition - <a href="test/fail-20.c">fail-20.c</a>, <a href="#317-destroying-an-invalid-condition">report</a>
  8. signaling an invalid condition - <a href="test/fail-21.c">fail-21.c</a>, <a href="#318-signaling-an-invalid-condition">report</a>
  9. broadcasting an invalid condition - <a href="test/fail-22.c">fail-22.c</a>, <a href="#319-broadcasting-an-invalid-condition">report</a>
  10. [timed]waiting on an invalid condition - <a href="test/fail-23.c">fail-23.c</a>, <a href="#3110-timedwaiting-on-an-invalid-condition">report</a>
  11. [timed]waiting on an invalid mutex - <a href="test/fail-24.c">fail-24.c</a>, <a href="#3111-timedwaiting-on-an-invalid-mutex">report</a>
  12. [timed]waiting on an unheld mutex - <a href="test/fail-25.c">fail-25.c</a>, <a href="#3112-timedwaiting-on-an-unheld-mutex">report</a>
  13. join invalid thread - <a href="test/fail-40.c">fail-40.c</a>, <a href="#3113-join-invalid-thread">report</a>
  14. detach invalid thread - <a href="test/fail-41.c">fail-41.c</a>, <a href="#3113-join-invalid-thread">report</a>
  15. unlocking mutex that was held by other thread - <a href="test/fail-42.c">fail-42.c</a>, <a href="#3115-unlocking-mutex-that-was-held-by-other-thread">report</a>

### 1.2 lock ordering violation ###

  hthread is able to detect inconsistent locking orders, it is very useful because they usually result to deadlocks. they may never
  be discovered during testing and lead to hard-to-find bugs.
  
  hthread monitors locks/unlocks and stores them in separate tables for each thread. this allows hthread to build a global lock order
  table for running process. hthread checks for locking order violation against global order table for each lock request, and able to
  detect and report following errors.

  1. lock order violation in same thread - <a href="test/fail-04.c">fail-04.c</a>, <a href="#321-lock-order-violation-in-same-thread">report</a>
  2. lock order violation while [timed]waiting on condition - <a href="test/fail-26.c">fail-26.c</a>, <a href="#322-lock-order-violation-while-timedwaiting-on-condition">report</a>
  3. lock order violation between threads - <a href="test/fail-43.c">fail-43.c</a>, <a href="#323-lock-order-violation-between-threads">report</a>

### 1.3 lock contention ###

  hthread able to understand and print report about lock contentions - a thread has to wait until requested lock is released. monitoring
  lock contentions is handy because, they usually cause unwanted delays or they may point to an undetected potential deadlock. hthread
  can report followings.

  1. waiting to lock a mutex more than allowed threshold - <a href="test/fail-60.c">fail-60.c</a>, <a href="#331-waiting-to-lock-a-mutex-more-than-allowed-threshold">report</a>
  2. hold a mutex lock more than allowed threshold - <a href="test/fail-61.c">fail-61.c</a>, <a href="#332-hold-a-mutex-lock-more-than-allowed-threshold">report</a>

## 2. configuration ##

  1. <a href="#21-compile-time-options">compile-time options</a>
  2. <a href="#22-run-time-options">run-time options</a>

### 2.1 compile-time options ###

  hthread configuration parameters can be set using <tt>make flags</tt>, please check example section for demonstration.

  - HTHREAD_ENABLE_CALLSTACK
  
  
    default 1
    
    enable/disable reporting call trace information on error, useful but depends on <tt>libbdf</tt>, <tt>libdl</tt>, and <tt>backtrace function from glibc</tt>.
    may be disabled for toolchains which does not support backtracing.
  
  - HTHREAD_REPORT_CALLSTACK
  
  
    default 1
    
    dump callstack info (function call history) for error point.
  
  - HTHREAD_ASSERT_ON_ERROR
  
    
    default 1
    
    terminate the process on any pthreads api misuse and/or lock order violation.
  
  - HTHREAD_LOCK_THRESHOLD
  
    
    default 5000 miliseconds
    
    print report if lock is held longer than the specified time, in miliseconds.
  
  - HTHREAD_LOCK_THRESHOLD_ASSERT
  
    
    default 0
    
    terminate if lock is held longer than the specified time, in miliseconds.
  
  - HTHREAD_LOCK_TRY_THRESHOLD
  
    
    default 5000 miliseconds
    
    print report if locking operation takes longer than the specified time, in miliseconds.
  
  - HTHREAD_LOCK_TRY_THRESHOLD_ASSERT
  
    
    default 0
    
    terminate if locking operation takes longer than the specified time, in miliseconds.
 
### 2.2 run-time options ###
  
  hthread reads configuration parameters from environment via getenv function call. one can either set/change environment variables in source
  code of monitored project via setenv function call, or set them globally in running shell using export function.
  
  please check example section for demonstration.

  - hthread_report_callstack
  
  	
  	default 1

  	dump callstack info (function call history) for error point.
  
  - hthread_assert_on_error
  
    
    default 1
    
    terminate the process on any pthreads api misuse and/or lock order violation.
  
  - hthread_lock_hreashold
  
    
    default 5000 miliseconds
    
    print report if lock is held longer than the specified time, in miliseconds.
  
  - hthread_lock_hreashold_assert
  
    
    default 0
    
    terminate if lock is held longer than the specified time, in miliseconds.
  
  - hthread_lock_try_threshold
  
    
    default 5000 miliseconds
    
    print report if locking operation takes longer than the specified time, in miliseconds.
  
  - hthread_lock_try_threshold_assert
  
    
    default 0
    
    terminate if locking operation takes longer than the specified time, in miliseconds.

## 3. error reports ##

### 3.1 misuses of pthreads api ###

  1. <a href="#311-destroying-an-invalid-mutex">destroying an invalid mutex</a>
  2. <a href="#312-locking-an-invalid-mutex">locking an invalid mutex</a>
  3. <a href="#313-unlocking-an-invalid-mutex">unlocking an invalid mutex</a>
  4. <a href="#314-locking-an-already-locked-mutex">locking an already locked mutex</a>
  5. <a href="#315-unlocking-an-unheld-mutex">unlocking an unheld mutex</a>
  6. <a href="#316-destroying-a-locked-mutex">destroying a locked mutex</a>
  7. <a href="#317-destroying-an-invalid-condition">destroying an invalid condition</a>
  8. <a href="#318-signaling-an-invalid-condition">signaling an invalid condition</a>
  9. <a href="#319-broadcasting-an-invalid-condition">broadcasting an invalid condition</a>
  10. <a href="#3110-timedwaiting-on-an-invalid-condition">[timed]waiting on an invalid condition</a>
  11. <a href="#3111-timedwaiting-on-an-invalid-mutex">[timed]waiting on an invalid mutex</a>
  12. <a href="#3112-timedwaiting-on-an-unheld-mutex">[timed]waiting on an unheld mutex</a>
  13. <a href="#3113-join-invalid-thread">join invalid thread</a>
  14. <a href="#3114-detach-invalid-thread">detach invalid thread</a>
  15. <a href="#3115-unlocking-mutex-that-was-held-by-other-thread">unlocking mutex that was held by other thread</a>

#### 3.1.1. destroying an invalid mutex ####

  example code: <a href="test/fail-00.c">fail-00.c</a>
  
    # ./test/fail-00-debug 
    (hthread:30786) new thread created: 'root-process (0x1427010)'
    (hthread:30786)     at: (null) (null):0
    (hthread:30786) mutex destroy with invalid mutex: '0x7fff1476d258'
    (hthread:30786)     by: root-process (0x1427010)
    (hthread:30786)     at: main fail-00.c:23
    (hthread:30786)         0x40c12f: hthread.c (debug_dump_callstack:829)
    (hthread:30786)         0x401b5f: fail-00.c (main:24)
    (hthread:30786)         0x401bbd: (null) (_start:0)
    fail-00-debug: hthread.c:1349: debug_mutex_del: Assertion `(mt != ((void *)0)) && "invalid mutex"' failed.

#### 3.1.2. locking an invalid mutex ####

  example code: <a href="test/fail-01.c">fail-01.c</a>

    # ./test/fail-01-debug 
    (hthread:30789) new thread created: 'root-process (0x1499010)'
    (hthread:30789)     at: (null) (null):0
    (hthread:30789) mutex lock with invalid mutex: '0x7fff394852f8'
    (hthread:30789)     by: root-process (0x1499010)
    (hthread:30789)     at: main fail-01.c:23
    (hthread:30789)         0x408e83: hthread.c (debug_dump_callstack:829)
    (hthread:30789)         0x40c2f4: hthread.c (debug_mutex_try_lock:1088)
    (hthread:30789)         0x401b5f: fail-01.c (main:24)
    (hthread:30789)         0x401bbd: (null) (_start:0)
    fail-01-debug: hthread.c:946: debug_mutex_add_lock: Assertion `(mt != ((void *)0)) && "invalid mutex"' failed.

#### 3.1.3. unlocking an invalid mutex ####

  example code: <a href="test/fail-02.c">fail-02.c</a>

    # ./test/fail-02-debug 
    (hthread:30792) new thread created: 'root-process (0x1f9e010)'
    (hthread:30792)     at: (null) (null):0
    (hthread:30792) mutex unlock with invalid mutex '0x7ffffa879c38'
    (hthread:30792)     by: root-process (0x1f9e010)
    (hthread:30792)     at: main fail-02.c:23
    (hthread:30792)         0x406a7c: hthread.c (debug_dump_callstack:829)
    (hthread:30792)         0x40d619: hthread.c (hthread_mutex_unlock_actual_debug:603)
    (hthread:30792)         0x401b5f: fail-02.c (main:24)
    (hthread:30792)         0x401bbd: (null) (_start:0)
    fail-02-debug: hthread.c:1226: debug_mutex_del_lock: Assertion `(mt != ((void *)0)) && "invalid mutex"' failed.

#### 3.1.4. locking an already locked mutex ####

  example code: <a href="test/fail-03.c">fail-03.c</a>

    # ./test/fail-03-debug 
    (hthread:30795) new thread created: 'root-process (0x1e670c0)'
    (hthread:30795)     at: (null) (null):0
    (hthread:30795) mutex lock with already held mutex: 'mutex(main fail-03.c:22) (0x1e67010)'
    (hthread:30795)     by: root-process (0x1e670c0)
    (hthread:30795)     at: main fail-03.c:32
    (hthread:30795)         0x408ef2: hthread.c (debug_dump_callstack:829)
    (hthread:30795)         0x40c4d4: hthread.c (debug_mutex_try_lock:1088)
    (hthread:30795)         0x401cb8: fail-03.c (main:33)
    (hthread:30795)         0x401da1: (null) (_start:0)
    (hthread:30795)   previously acquired
    (hthread:30795)     by: root-process (0x1e670c0)
    (hthread:30795)     at: main fail-03.c:27
    (hthread:30795)   created 'mutex(main fail-03.c:22) (0x1e67010)'
    (hthread:30795)     at: main fail-03.c:22
    fail-03-debug: hthread.c:963: debug_mutex_add_lock: Assertion `(mt == ((void *)0)) && "mutex is already locked"' failed.

#### 3.1.5. unlocking an unheld mutex ####

  example code: <a href="test/fail-05.c">fail-05.c</a>

    # ./test/fail-05-debug 
    (hthread:30801) new thread created: 'root-process (0x9200c0)'
    (hthread:30801)     at: (null) (null):0
    (hthread:30801) mutex unlock with unheld mutex: 'mutex(main fail-05.c:22) (0x920010)'
    (hthread:30801)     by: root-process (0x9200c0)
    (hthread:30801)     at: main fail-05.c:27
    (hthread:30801)         0x406f94: hthread.c (debug_dump_callstack:829)
    (hthread:30801)         0x40d7d9: hthread.c (hthread_mutex_unlock_actual_debug:603)
    (hthread:30801)         0x401c95: fail-05.c (main:28)
    (hthread:30801)         0x401d81: (null) (_start:0)
    (hthread:30801)   created 'mutex(main fail-05.c:22) (0x920010)'
    (hthread:30801)     at: main fail-05.c:22
    fail-05-debug: hthread.c:1259: debug_mutex_del_lock: Assertion `(mtl != ((void *)0)) && "mutex is not locked"' failed.

#### 3.1.6. destroying a locked mutex ####

  example code: <a href="test/fail-06.c">fail-06.c</a>

    # ./test/fail-06-debug 
    (hthread:30804) new thread created: 'root-process (0x6420c0)'
    (hthread:30804)     at: (null) (null):0
    (hthread:30804) mutex destroy with currently locked mutex: '0x642010'
    (hthread:30804)     by: root-process (0x6420c0)
    (hthread:30804)     at: main fail-06.c:32
    (hthread:30804)         0x40c21c: hthread.c (debug_dump_callstack:829)
    (hthread:30804)         0x401cb8: fail-06.c (main:33)
    (hthread:30804)         0x401d81: (null) (_start:0)
    (hthread:30804)   lock observed
    (hthread:30804)     by: root-process (0x6420c0)
    (hthread:30804)     at: main fail-06.c:27
    (hthread:30804)   created 'mutex(main fail-06.c:22) (0x642010)
    (hthread:30804)     at: main fail-06.c:22
    fail-06-debug: hthread.c:1366: debug_mutex_del: Assertion `(mt == ((void *)0)) && "invalid mutex"' failed.

#### 3.1.7. destroying an invalid condition ####

  example code: <a href="test/fail-20.c">fail-20.c</a>

    # ./test/fail-20-debug 
    (hthread:30807) new thread created: 'root-process (0x1829010)'
    (hthread:30807)     at: (null) (null):0
    (hthread:30807) cond destroy with invalid condition: '0x7fff491eb5a8'
    (hthread:30807)     by: root-process (0x1829010)
    (hthread:30807)     at: main fail-20.c:23
    (hthread:30807)         0x40a413: hthread.c (debug_dump_callstack:829)
    (hthread:30807)         0x401b5f: fail-20.c (main:24)
    (hthread:30807)         0x401bbd: (null) (_start:0)
    fail-20-debug: hthread.c:1436: debug_cond_del: Assertion `(cv != ((void *)0)) && "invalid condition"' failed.

#### 3.1.8. signaling an invalid condition ####

  example code: <a href="test/fail-21.c">fail-21.c</a>

    # ./test/fail-21-debug 
    (hthread:30810) new thread created: 'root-process (0x2019010)'
    (hthread:30810)     at: (null) (null):0
    (hthread:30810) cond signal with invalid condition: '0x7fffd0b4ec78'
    (hthread:30810)     by: root-process (0x2019010)
    (hthread:30810)     at: main fail-21.c:23
    (hthread:30810)         0x405769: hthread.c (debug_dump_callstack:829)
    (hthread:30810)         0x40a5a9: hthread.c (hthread_cond_signal_actual_debug:457)
    (hthread:30810)         0x401b5f: fail-21.c (main:24)
    (hthread:30810)         0x401c01: (null) (_start:0)
    fail-21-debug: hthread.c:1469: debug_cond_check: Assertion `(cv != ((void *)0)) && "invalid condition"' failed.

#### 3.1.9. broadcasting an invalid condition ####

  example code: <a href="test/fail-22.c">fail-22.c</a>

    # ./test/fail-22-debug 
    (hthread:30813) new thread created: 'root-process (0x10b8010)'
    (hthread:30813)     at: (null) (null):0
    (hthread:30813) cond signal with invalid condition: '0x7fffafb97db8'
    (hthread:30813)     by: root-process (0x10b8010)
    (hthread:30813)     at: main fail-22.c:23
    (hthread:30813)         0x405769: hthread.c (debug_dump_callstack:829)
    (hthread:30813)         0x40a5d9: hthread.c (hthread_cond_broadcast_actual_debug:463)
    (hthread:30813)         0x401b5f: fail-22.c (main:24)
    (hthread:30813)         0x401c01: (null) (_start:0)
    fail-22-debug: hthread.c:1469: debug_cond_check: Assertion `(cv != ((void *)0)) && "invalid condition"' failed.

#### 3.1.10. [timed]waiting on an invalid condition ####

  example code: <a href="test/fail-23.c">fail-23.c</a>

    # ./test/fail-23-debug 
    (hthread:30816) new thread created: 'root-process (0x10340c0)'
    (hthread:30816)     at: (null) (null):0
    (hthread:30816) cond timedwait with invalid condition: '0x7fffaab0c968'
    (hthread:30816)     by: root-process (0x10340c0)
    (hthread:30816)     at: main fail-23.c:43
    (hthread:30816)         0x405a09: hthread.c (debug_dump_callstack:829)
    (hthread:30816)         0x40a8db: hthread.c (hthread_cond_timedwait_tspec_actual_debug:495)
    (hthread:30816)         0x401cf4: fail-23.c (main:43)
    (hthread:30816)         0x401ea5: (null) (_start:0)
    fail-23-debug: hthread.c:1469: debug_cond_check: Assertion `(cv != ((void *)0)) && "invalid condition"' failed.

#### 3.1.11. [timed]waiting on an invalid mutex ####

  example code: <a href="test/fail-24.c">fail-24.c</a>

    # ./test/fail-24-debug 
    (hthread:30819) new thread created: 'root-process (0x1e320c0)'
    (hthread:30819)     at: (null) (null):0
    (hthread:30819) cond timedwait with invalid mutex '0x7fff3e5d79d8'
    (hthread:30819)     by: root-process (0x1e320c0)
    (hthread:30819)     at: main fail-24.c:38
    (hthread:30819)         0x406d1c: hthread.c (debug_dump_callstack:829)
    (hthread:30819)         0x40a8b3: hthread.c (hthread_cond_timedwait_tspec_actual_debug:496)
    (hthread:30819)         0x401cd1: fail-24.c (main:38)
    (hthread:30819)         0x401e59: (null) (_start:0)
    fail-24-debug: hthread.c:1226: debug_mutex_del_lock: Assertion `(mt != ((void *)0)) && "invalid mutex"' failed.

#### 3.1.12. [timed]waiting on an unheld mutex ####

  example code: <a href="test/fail-25.c">fail-25.c</a>

    # ./test/fail-25-debug 
    (hthread:30822) new thread created: 'root-process (0x23af0c0)'
    (hthread:30822)     at: (null) (null):0
    (hthread:30822) cond timedwait with unheld mutex: 'mutex(main fail-25.c:32) (0x23afa00)'
    (hthread:30822)     by: root-process (0x23af0c0)
    (hthread:30822)     at: main fail-25.c:52
    (hthread:30822)         0x407184: hthread.c (debug_dump_callstack:829)
    (hthread:30822)         0x40a9c3: hthread.c (hthread_cond_timedwait_tspec_actual_debug:496)
    (hthread:30822)         0x401d72: fail-25.c (main:52)
    (hthread:30822)         0x401f6d: (null) (_start:0)
    (hthread:30822)   created 'mutex(main fail-25.c:32) (0x23afa00)'
    (hthread:30822)     at: main fail-25.c:32
    fail-25-debug: hthread.c:1259: debug_mutex_del_lock: Assertion `(mtl != ((void *)0)) && "mutex is not locked"' failed.

#### 3.1.13. join invalid thread ####

  example code: <a href="test/fail-40.c">fail-40.c</a>

    # ./test/fail-40-debug 
    (hthread:30828) new thread created: 'root-process (0xe20010)'
    (hthread:30828)     at: (null) (null):0
    (hthread:30828) join with invalid thread: '0x7fffecd84218'
    (hthread:30828)     by: root-process (0xe20010)
    (hthread:30828)     at: main fail-40.c:25
    (hthread:30828)         0x404ff5: hthread.c (debug_dump_callstack:829)
    (hthread:30828)         0x40daee: hthread.c (hthread_join_actual_debug:690)
    (hthread:30828)         0x401b5f: fail-40.c (main:26)
    (hthread:30828)         0x401bbd: (null) (_start:0)
    fail-40-debug: hthread.c:349: hthread_check: Assertion `(th == thread) && "invalid thread"' failed.

#### 3.1.14. detach invalid thread ####

  example code: <a href="test/fail-41.c">fail-41.c</a>

    # ./test/fail-41-debug 
    (hthread:30831) new thread created: 'root-process (0x258a010)'
    (hthread:30831)     at: (null) (null):0
    (hthread:30831) detach with invalid thread: '0x7fff81b0b8d8'
    (hthread:30831)     by: root-process (0x258a010)
    (hthread:30831)     at: main fail-41.c:25
    (hthread:30831)         0x404ff5: hthread.c (debug_dump_callstack:829)
    (hthread:30831)         0x40db7e: hthread.c (hthread_detach_actual_debug:704)
    (hthread:30831)         0x401b5f: fail-41.c (main:26)
    (hthread:30831)         0x401bbd: (null) (_start:0)
    fail-41-debug: hthread.c:349: hthread_check: Assertion `(th == thread) && "invalid thread"' failed.

#### 3.1.15. unlocking mutex that was held by other thread ####

  example code: <a href="test/fail-42.c">fail-42.c</a>

    # ./test/fail-42-debug 
    (hthread:30834) new thread created: 'root-process (0x1e560c0)'
    (hthread:30834)     at: (null) (null):0
    (hthread:30834) new thread created: 'thread(main fail-42.c:48) (0x1e56cd0)'
    (hthread:30834)     at: main fail-42.c:48
    (hthread:30834) mutex unlock with a mutex 'mutex(main fail-42.c:38) (0x1e56010)' currently hold by other thread
    (hthread:30834)     by: thread(main fail-42.c:48) (0x1e56cd0)
    (hthread:30834)     at: worker fail-42.c:23
    (hthread:30834)         0x406d9d: hthread.c (debug_dump_callstack:829)
    (hthread:30834)         0x40d929: hthread.c (hthread_mutex_unlock_actual_debug:603)
    (hthread:30834)         0x401f4f: fail-42.c (worker:24)
    (hthread:30834)         0x40da0a: hthread.c (thread_run:619)
    (hthread:30834)         0x7f51f6de1e9a: pthread_create.c (start_thread:308)
    (hthread:30834)   lock observed
    (hthread:30834)     by: root-process (0x1e560c0)
    (hthread:30834)     at: main fail-42.c:43
    (hthread:30834)   created 'mutex(main fail-42.c:38) (0x1e56010)'
    (hthread:30834)     at: main fail-42.c:38
    fail-42-debug: hthread.c:1247: debug_mutex_del_lock: Assertion `(mtl == ((void *)0)) && "mutex is locked by other thread"' failed.

### 3.2 lock ordering violation ###

  1. <a href="#321-lock-order-violation-in-same-thread">lock order violation in same thread</a>
  2. <a href="#322-lock-order-violation-while-timedwaiting-on-condition">lock order violation while [timed]waiting on condition</a>
  3. <a href="#323-lock-order-violation-between-threads">lock order violation between threads</a>

#### 3.2.1. lock order violation in same thread ####

  example code: <a href="test/fail-04.c">fail-04.c</a>

    # ./test/fail-04-debug 
    (hthread:30798) new thread created: 'root-process (0x1d530c0)'
    (hthread:30798)     at: (null) (null):0
    (hthread:30798) mutex lock order 'mutex(main fail-04.c:26) (0x1d53b60)' before 'mutex(main fail-04.c:26) (0x1d53c10)' violated
    (hthread:30798)   incorrect order is: acquisition of 'mutex(main fail-04.c:26) (0x1d53c10)'
    (hthread:30798)       by: root-process (0x1d530c0)
    (hthread:30798)       at: main fail-04.c:47
    (hthread:30798)     followed by a later acquisition of 'mutex(main fail-04.c:26) (0x1d53b60)'
    (hthread:30798)       by: root-process (0x1d530c0)
    (hthread:30798)       at: main fail-04.c:47
    (hthread:30798)           0x408b94: hthread.c (debug_dump_callstack:829)
    (hthread:30798)           0x40c5c4: hthread.c (debug_mutex_try_lock:1088)
    (hthread:30798)           0x401d20: fail-04.c (main:48)
    (hthread:30798)           0x401e89: (null) (_start:0)
    (hthread:30798)   required order is: acquisition of 'mutex(main fail-04.c:26) (0x1d53b60)'
    (hthread:30798)       by: root-process (0x1d530c0)
    (hthread:30798)       at: main fail-04.c:33
    (hthread:30798)     followed by a later acquisition of 'mutex(main fail-04.c:26) (0x1d53c10)'
    (hthread:30798)       by: root-process (0x1d530c0)
    (hthread:30798)       at: main fail-04.c:33
    (hthread:30798)   created 'mutex(main fail-04.c:26) (0x1d53c10)'
    (hthread:30798)     at: main fail-04.c:26
    (hthread:30798)   created 'mutex(main fail-04.c:26) (0x1d53b60)'
    (hthread:30798)     at: main fail-04.c:26
    fail-04-debug: hthread.c:1036: debug_mutex_add_lock: Assertion `(mto == ((void *)0)) && "lock order violation"' failed.

#### 3.2.2. lock order violation while [timed]waiting on condition ####

  example code: <a href="test/fail-26.c">fail-26.c</a>

    # ./test/fail-26-debug 
    (hthread:30825) new thread created: 'root-process (0x77b0c0)'
    (hthread:30825)     at: (null) (null):0
    (hthread:30825) cond wait order 'mutex(main fail-26.c:32) (0x77b010)' before 'mutex(main fail-26.c:32) (0x77ba00)' violated
    (hthread:30825)   incorrect order is: acquisition of 'mutex(main fail-26.c:32) (0x77ba00)'
    (hthread:30825)       by: root-process (0x77b0c0)
    (hthread:30825)       at: main fail-26.c:48
    (hthread:30825)     followed by a later acquisition of 'mutex(main fail-26.c:32) (0x77b010)'
    (hthread:30825)       by: root-process (0x77b0c0)
    (hthread:30825)       at: main fail-26.c:57
    (hthread:30825)           0x408cb4: hthread.c (debug_dump_callstack:829)
    (hthread:30825)           0x40aa3b: hthread.c (hthread_cond_timedwait_tspec_actual_debug:513)
    (hthread:30825)           0x401d95: fail-26.c (main:57)
    (hthread:30825)           0x401fb5: (null) (_start:0)
    (hthread:30825)   required order is: acquisition of 'mutex(main fail-26.c:32) (0x77b010)'
    (hthread:30825)       by: root-process (0x77b0c0)
    (hthread:30825)       at: main fail-26.c:43
    (hthread:30825)     followed by a later acquisition of 'mutex(main fail-26.c:32) (0x77ba00)'
    (hthread:30825)       by: root-process (0x77b0c0)
    (hthread:30825)       at: main fail-26.c:48
    (hthread:30825)   created 'mutex(main fail-26.c:32) (0x77ba00)'
    (hthread:30825)     at: main fail-26.c:32
    (hthread:30825)   created 'mutex(main fail-26.c:32) (0x77b010)'
    (hthread:30825)     at: main fail-26.c:32
    fail-26-debug: hthread.c:1036: debug_mutex_add_lock: Assertion `(mto == ((void *)0)) && "lock order violation"' failed.

#### 3.2.3. lock order violation between threads ####

  example code: <a href="test/fail-43.c">fail-43.c</a>

    # ./test/fail-43-debug 
    (hthread:30838) new thread created: 'root-process (0x14ae0c0)'
    (hthread:30838)     at: (null) (null):0
    (hthread:30838) new thread created: 'thread(main fail-43.c:73) (0x14af0f0)'
    (hthread:30838)     at: main fail-43.c:73
    (hthread:30838) mutex lock order 'mutex(main fail-43.c:53) (0x14ae010)' before 'mutex(main fail-43.c:58) (0x14aea00)' violated
    (hthread:30838)   incorrect order is: acquisition of 'mutex(main fail-43.c:58) (0x14aea00)'
    (hthread:30838)       by: thread(main fail-43.c:73) (0x14af0f0)
    (hthread:30838)       at: worker fail-43.c:23
    (hthread:30838)     followed by a later acquisition of 'mutex(main fail-43.c:53) (0x14ae010)'
    (hthread:30838)       by: thread(main fail-43.c:73) (0x14af0f0)
    (hthread:30838)       at: worker fail-43.c:28
    (hthread:30838)           0x408d24: hthread.c (debug_dump_callstack:829)
    (hthread:30838)           0x40c754: hthread.c (debug_mutex_try_lock:1088)
    (hthread:30838)           0x40206f: fail-43.c (worker:29)
    (hthread:30838)           0x40db5a: hthread.c (thread_run:619)
    (hthread:30838)           0x7f58cbd27e9a: pthread_create.c (start_thread:308)
    (hthread:30838)   required order is: acquisition of 'mutex(main fail-43.c:53) (0x14ae010)'
    (hthread:30838)       by: root-process (0x14ae0c0)
    (hthread:30838)       at: main fail-43.c:63
    (hthread:30838)     followed by a later acquisition of 'mutex(main fail-43.c:58) (0x14aea00)'
    (hthread:30838)       by: root-process (0x14ae0c0)
    (hthread:30838)       at: main fail-43.c:68
    (hthread:30838)   created 'mutex(main fail-43.c:58) (0x14aea00)'
    (hthread:30838)     at: main fail-43.c:58
    (hthread:30838)   created 'mutex(main fail-43.c:53) (0x14ae010)'
    (hthread:30838)     at: main fail-43.c:53
    fail-43-debug: hthread.c:1036: debug_mutex_add_lock: Assertion `(mto == ((void *)0)) && "lock order violation"' failed.

### 3.3 lock contention ###

  1. <a href="#331-waiting-to-lock-a-mutex-more-than-allowed-threshold">waiting to lock a mutex more than allowed threshold</a>
  2. <a href="#332-hold-a-mutex-lock-more-than-allowed-threshold">hold a mutex lock more than allowed threshold</a>

#### 3.3.1. waiting to lock a mutex more than allowed threshold ####

  example code: <a href="test/fail-60.c">fail-60.c</a>
  
  lock try threshold is set to 1 seconds, and enabled terminating if waited more than threshold time. 

    # hthread_lock_try_threshold=1000 hthread_lock_try_threshold_assert=1 ./test/fail-60-debug 
    (hthread:30880) new thread created: 'root-process (0x13670c0)'
    (hthread:30880)     at: (null) (null):0
    (hthread:30880) new thread created: 'thread(main fail-60.c:53) (0x1367cd0)'
    (hthread:30880)     at: main fail-60.c:53
    (hthread:30880) mutex lock still waiting for mutex: 'mutex(main fail-60.c:43) (0x1367010)'
    (hthread:30880)     by: thread(main fail-60.c:53) (0x1367cd0)
    (hthread:30880)     at: worker fail-60.c:23
    (hthread:30880)         0x40d6e5: hthread.c (debug_dump_callstack:829)
    (hthread:30880)         0x401fbf: fail-60.c (worker:24)
    (hthread:30880)         0x40da8a: hthread.c (thread_run:619)
    (hthread:30880)         0x7f5bf1a76e9a: pthread_create.c (start_thread:308)
    (hthread:30880)   currently locked
    (hthread:30880)     by: root-process (0x13670c0)
    (hthread:30880)     at: main fail-60.c:48
    (hthread:30880)   currently locked
    (hthread:30880)     by: thread(main fail-60.c:53) (0x1367cd0)
    (hthread:30880)     at: worker fail-60.c:23
    (hthread:30880)   created 'mutex(main fail-60.c:43) (0x1367010)'
    (hthread:30880)     at: main fail-60.c:43
    fail-60-debug: hthread.c:1166: debug_mutex_try_lock: Assertion `(a == 0) && "mutex try lock threshold reached"' failed.

#### 3.3.2. hold a mutex lock more than allowed threshold ####

  example code: <a href="test/fail-61.c">fail-61.c</a>
  
  lock hold threshold is set to 1 seconds, and enabled terminating if waited more than threshold time.
   
    # hthread_lock_threshold=1000 hthread_lock_threshold_assert=1 ./test/fail-61-debug 
    (hthread:30885) new thread created: 'root-process (0x15a20c0)'
    (hthread:30885)     at: (null) (null):0
    (hthread:30885) mutex unlock with a mutex 'mutex(main fail-61.c:22) (0x15a2010)' hold during 3001 ms
    (hthread:30885)     by: root-process (0x15a20c0)
    (hthread:30885)     at: main fail-61.c:33
    (hthread:30885)         0x406bc6: hthread.c (debug_dump_callstack:829)
    (hthread:30885)         0x40d849: hthread.c (hthread_mutex_unlock_actual_debug:603)
    (hthread:30885)         0x401d06: fail-61.c (main:34)
    (hthread:30885)         0x401ded: (null) (_start:0)
    (hthread:30885)   lock observed
    (hthread:30885)     by: root-process (0x15a20c0)
    (hthread:30885)     at: main fail-61.c:27
    (hthread:30885)   created 'mutex(main fail-61.c:22) (0x15a2010)'
    (hthread:30885)     at: main fail-61.c:22
    fail-61-debug: hthread.c:1286: debug_mutex_del_lock: Assertion `(a == 0) && "mutex lock threshold reached"' failed.

## 4. test cases ##

  1. <a href="#41-mutex-tests">mutex tests</a>
  2. <a href="#42-condition-tests">condition tests</a>
  3. <a href="#43-thread-tests">thread tests</a>
  4. <a href="#44-lock-contention-tests">lock contention tests</a>

### 4.1 mutex tests ###

  <table>
    <tr valign="top">
      <th>test</th>
      <th>sucess</th>
      <th>fail</th>
    </tr>
    <tr valign="top">
      <td>
      <a href="test/success-00.c">sucess-00.c</a><br> 
      <a href="test/fail-00.c">fail-00.c</a><br> 
      </td>
      <td>
      init mutex                             <br>
      destroy mutex                          <br>
      </td>
      <td>
      destroy mutex                          <br>
      ** invalid mutex **                    <br>
      </td>
    </tr>
    <tr valign="top">
      <td>
      <a href="test/success-01.c">sucess-01.c</a><br> 
      <a href="test/fail-01.c">fail-01.c</a><br> 
      </td>
      <td>
      init mutex                             <br>
      lock mutex                             <br>
      unlock mutex                           <br>
      destroy mutex                          <br>
      </td>
      <td>
      lock mutex                             <br>
      ** invalid mutex **                    <br>
      </td>
    </tr>
    <tr valign="top">
      <td>
      <a href="test/success-02.c">sucess-02.c</a><br> 
      <a href="test/fail-02.c">fail-02.c</a><br> 
      </td>
      <td>
      init mutex                             <br>
      lock mutex                             <br>
      unlock mutex                           <br>
      destroy mutex                          <br>
      </td>
      <td>
      unlock mutex                           <br>
      ** invalid mutex **                    <br>
      </td>
    </tr>
    <tr valign="top">
      <td>
      <a href="test/success-03.c">sucess-03.c</a><br> 
      <a href="test/fail-03.c">fail-03.c</a><br> 
      </td>
      <td>
      init mutex                             <br>
      lock mutex                             <br>
      unlock mutex                           <br>
      destroy mutex                          <br>
      </td>
      <td>
      init mutex                             <br>
      lock mutex                             <br>
      lock mutex                             <br>
      ** mutex is already locked **          <br>
      </td>
    </tr>
    <tr valign="top">
      <td>
      <a href="test/success-04.c">sucess-04.c</a><br> 
      <a href="test/fail-04.c">fail-04.c</a><br> 
      </td>
      <td>
      init mutexes                           <br>
      lock mutexes 0...n                     <br>
      unlock mutexes 0...n                   <br>
      lock mutexes 0...n                     <br>
      unlock mutexes 0...n                   <br>
      destroy mutexes                        <br>
      </td>
      <td>
      init mutexes                           <br>
      lock mutexes 0...n                     <br>
      unlock mutexes 0...n                   <br>
      lock mutexes n...0                     <br>
      ** lock order violation **             <br>
      </td>
    </tr>
    <tr valign="top">
      <td>
      <a href="test/success-05.c">sucess-05.c</a><br> 
      <a href="test/fail-05.c">fail-05.c</a><br> 
      </td>
      <td>
      init mutex                             <br>
      lock mutex                             <br>
      unlock mutex                           <br>
      destroy mutex                          <br>
      </td>
      <td>
      init mutex                             <br>
      unlock mutex                           <br>
      ** unlocking a unheld mutex **         <br>
      </td>
    </tr>
    <tr valign="top">
      <td>
      <a href="test/success-06.c">sucess-06.c</a><br> 
      <a href="test/fail-06.c">fail-06.c</a><br> 
      </td>
      <td>
      init mutex                             <br>
      lock mutex                             <br>
      unlock mutex                           <br>
      destroy mutex                          <br>
      </td>
      <td>
      init mutex                             <br>
      lock mutex                             <br>
      destroy mutex                          <br>
      ** destroying a locked mutex **        <br>
      </td>
    </tr>
  </table>

### 4.2 condition tests ###

  <table>
    <tr valign="top">
      <th>test</th>
      <th>sucess</th>
      <th>fail</th>
    </tr>
    <tr valign="top">
      <td>
      <a href="test/success-20.c">sucess-20.c</a><br> 
      <a href="test/fail-20.c">fail-20.c</a><br> 
      </td>
      <td>
      init condition                         <br>
      destroy condition                      <br>
      </td>
      <td>
      destroy condition                      <br>
      ** invalid condition **                <br>
      </td>
    </tr>
    <tr valign="top">
      <td>
      <a href="test/success-21.c">sucess-21.c</a><br> 
      <a href="test/fail-21.c">fail-21.c</a><br> 
      </td>
      <td>
      init condition                         <br>
      signal condition                       <br>
      destroy condition                      <br>
      </td>
      <td>
      signal condition                       <br>
      ** invalid condition **                <br>
      </td>
    </tr>
    <tr valign="top">
      <td>
      <a href="test/success-22.c">sucess-22.c</a><br> 
      <a href="test/fail-22.c">fail-22.c</a><br> 
      </td>
      <td>
      init condition                         <br>
      broadcast condition                    <br>
      destroy condition                      <br>
      </td>
      <td>
      broadcast condition                    <br>
      ** invalid condition **                <br>
      </td>
    </tr>
    <tr valign="top">
      <td>
      <a href="test/success-23.c">sucess-23.c</a><br> 
      <a href="test/fail-23.c">fail-23.c</a><br> 
      </td>
      <td>
      init condition                         <br>
      init mutex                             <br>
      lock mutex                             <br>
      timed wait on condition, mutex         <br>
      unlock mutex                           <br>
      destroy condition                      <br>
      destroy mutex                          <br>
      </td>
      <td>
      init mutex                             <br>
      lock mutex                             <br>
      timed wait on condition, mutex         <br>
      ** invalid condition **                <br>
      </td>
    </tr>
    <tr valign="top">
      <td>
      <a href="test/success-24.c">sucess-24.c</a><br> 
      <a href="test/fail-24.c">fail-24.c</a><br> 
      </td>
      <td>
      init condition                         <br>
      init mutex                             <br>
      lock mutex                             <br>
      timed wait on condition, mutex         <br>
      unlock mutex                           <br>
      destroy condition                      <br>
      destroy mutex                          <br>
      </td>
      <td>
      init condition                         <br>
      timed wait on condition, mutex         <br>
      ** invalid mutex **                    <br>
      </td>
    </tr>
    <tr valign="top">
      <td>
      <a href="test/success-25.c">sucess-25.c</a><br> 
      <a href="test/fail-25.c">fail-25.c</a><br> 
      </td>
      <td>
      init mutexes                           <br>
      init condition                         <br>
      lock mutex 0                           <br>
      timed wait on condition, mutex 0       <br>
      unlock mutex 0                         <br>
      destroy condition                      <br>
      destroy mutexes                        <br>
      </td>
      <td>
      init mutexes                           <br>
      init condition                         <br>
      lock mutex 0                           <br>
      timed wait on condition, mutex 1       <br>
      ** mutex not locked **                 <br>
      </td>
    </tr>
    <tr valign="top">
      <td>
      <a href="test/success-26.c">sucess-26.c</a><br> 
      <a href="test/fail-26.c">fail-26.c</a><br> 
      </td>
      <td>
      init mutexes                           <br>
      init condition                         <br>
      lock mutex 0                           <br>
      lock mutex 1                           <br>
      unlock mutex 1                         <br>
      timed wait on condition, mutex 0       <br>
      unlock mutex 0                         <br>
      destroy condition                      <br>
      destroy mutexes                        <br>
      </td>
      <td>
      init mutexes                           <br>
      init condition                         <br>
      lock mutex 0                           <br>
      lock mutex 1                           <br>
      timed wait on condition, mutex 0       <br>
      ** lock order will be violated **      <br>
      </td>
    </tr>
  </table>

### 4.3 thread tests ###

  <table>
    <tr valign="top">
      <th>test</th>
      <th>sucess</th>
      <th>fail</th>
    </tr>
    <tr valign="top">
      <td>
      <a href="test/success-40.c">sucess-40.c</a><br> 
      <a href="test/fail-40.c">fail-40.c</a><br> 
      </td>
      <td>
      create thread                          <br>
      join thread                            <br>
      </td>
      <td>
      join thread                            <br>
      ** invalid thread **                   <br>
      </td>
    </tr>
    <tr valign="top">
      <td>
      <a href="test/success-41.c">sucess-41.c</a><br> 
      <a href="test/fail-41.c">fail-41.c</a><br> 
      </td>
      <td>
      create thread                          <br>
      detach thread                          <br>
      </td>
      <td>
      detach thread                          <br>
      ** invalid thread **                   <br>
      </td>
    </tr>
    <tr valign="top">
      <td>
      <a href="test/success-42.c">sucess-42.c</a><br> 
      <a href="test/fail-42.c">fail-42.c</a><br> 
      </td>
      <td>
      main  : init mutex                     <br>
      main  : lock mutex                     <br>
      main  : create thread                  <br>
      thread: lock mutex                     <br>
      thread: unlock mutex                   <br>
      main  : unlock mutex                   <br>
      main  : join thread                    <br>
      main  : destroy mutex                  <br>
      </td>
      <td>
      main  : init mutex                     <br>
      main  : lock mutex                     <br>
      main  : create thread                  <br>
      thread: unlock mutex                   <br>
      ** unlocking mutex currently held **   <br>
      ** by other thread **                  <br>
      </td>
    </tr>
    <tr valign="top">
      <td>
      <a href="test/success-43.c">sucess-43.c</a><br> 
      <a href="test/fail-43.c">fail-43.c</a><br> 
      </td>
      <td>
      main  : init mutex 0                   <br>
      main  : init mutex 1                   <br>
      main  : lock mutex 0                   <br>
      main  : lock mutex 1                   <br>
      main  : create thread                  <br>
      thread: lock mutex 0                   <br>
      thread: lock mutex 1                   <br>
      thread: unlock mutex 0                 <br>
      thread: unlock mutex 1                 <br>
      main  : unlock mutex 0                 <br>
      main  : unlock mutex 1                 <br>
      main  : join thread                    <br>
      main  : destroy mutex                  <br>
      </td>
      <td>
      main  : init mutex 0                   <br>
      main  : init mutex 1                   <br>
      main  : lock mutex 0                   <br>
      main  : lock mutex 1                   <br>
      main  : create thread                  <br>
      thread: lock mutex 1                   <br>
      thread: lock mutex 0                   <br>
      ** lock order violation **             <br>
      </td>
    </tr>
  </table>

### 4.4 lock contention tests ###

  <table>
    <tr valign="top">
      <th>test</th>
      <th>sucess</th>
      <th>fail</th>
    </tr>
    <tr valign="top">
      <td>
      <a href="test/success-60.c">sucess-60.c</a><br> 
      <a href="test/fail-60.c">fail-60.c</a><br> 
      </td>
      <td>
      hthread_lock_try_threshold = 1000      <br>
      hthread_lock_try_threshold_assert = 0  <br>
      main  : init mutex                     <br>
      main  : lock mutex                     <br>
      main  : create thread                  <br>
      main  : sleep 3                        <br>
      thread: lock mutex                     <br>
      ** try lock threshold reached **       <br>
      ** still waiting on mutex **           <br>
      thread: unlock mutex                   <br>
      main  : unlock mutex                   <br>
      main  : join thread                    <br>
      main  : destroy mutex                  <br>
      </td>
      <td>
      hthread_lock_try_threshold = 1000      <br>
      hthread_lock_try_threshold_assert = 1  <br>
      main  : init mutex                     <br>
      main  : lock mutex                     <br>
      main  : create thread                  <br>
      main  : sleep 3                        <br>
      thread: lock mutex                     <br>
      ** try lock threshold reached **       <br>
      ** still waiting on mutex **           <br>
      ** assert requested **                 <br>
      </td>
    </tr>
    <tr valign="top">
      <td>
      <a href="test/success-61.c">sucess-61.c</a><br> 
      <a href="test/fail-61.c">fail-61.c</a><br> 
      </td>
      <td>
      hthread_lock_threshold = 1000          <br>
      hthread_lock_threshold_assert = 0      <br>
      init mutex                             <br>
      lock mutex                             <br>
      sleep 3                                <br>
      unlock mutex                           <br>
      ** lock threshold reached **           <br>
      destroy mutex                          <br>
      </td>
      <td>
      hthread_lock_threshold = 1000          <br>
      hthread_lock_assert = 1                <br>
      init mutex                             <br>
      lock mutex                             <br>
      sleep 3                                <br>
      unlock mutex                           <br>
      ** lock threshold reached **           <br>
      ** assert requested **                 <br>
      </td>
    </tr>
  </table>

## 5. usage example ##

  using hthread is pretty simple, just clone libhthread and build, add <tt>-include hthread.h -DHTHREAD_DEBUG=1 -g -O1</tt>
  to target cflags and link with <tt>-lhthread -lrt</tt> if HTHREAD_ENABLE_CALLSTACK is 0 or link with
  <tt>-lhthread -lrt -ldl -lbfd</tt> if HTHREAD_ENABLE_CALLSTACK is 1

  compile libhthread with callstack support

    # git clone git://github.com/anhanguera/libhthread.git
    # cd libhthread
    # HTHREAD_ENABLE_CALLSTACK=1 make
  
  compile libhthread without callstack support

    # git clone git://github.com/anhanguera/libhthread.git
    # cd libhthread
    # HTHREAD_ENABLE_CALLSTACK=0 make
  
  let below is the source code - with double lock error - to be monitored:
  
    1  #include <stdio.h>
    2  #include <stdlib.h>
    3  #include <unistd.h>
    4  #include <pthread.h>
    5  
    6  int main (int argc, char *argv[])
    7  {
    8    int rc;
    9    pthread_mutex_t m;
    10   (void) argc;
    11   (void) argv;
    12   rc = pthread_mutex_init(&m, NULL);
    13   if (rc != 0) {
	14     fprintf(stderr, "pthread_mutex_init failed\n");
	15     exit(-1);
    16   }
    17   rc = pthread_mutex_lock(&m);
    18   if (rc != 0) {
	19     fprintf(stderr, "pthread_mutex_lock failed\n");
	20     exit(-1);
	21   }
	22   rc = pthread_mutex_lock(&m);
	23   if (rc != 0) {
    24     fprintf(stderr, "pthread_mutex_lock failed\n");
	25     exit(-1);
	26   }
	27   rc = pthread_mutex_destroy(&m);
	28   if (rc != 0) {
    29     fprintf(stderr, "pthread_mutex_destroy failed\n");
	30     exit(-1);
	31   }
	32   return 0;
    32 }
    
  compile and run as usual:
  
    # gcc -o app main.c -lpthread
    # ./app
    
  application will not exit, because it is trying to lock a already locked mutex. now, enable
  monitoring with hthread:

    # gcc -include src/hthread.h -DHTHREAD_DEBUG=1 -g -O1 -o app-debug main.c -Lsrc -lhthread -lrt -ldl -lbfd -lpthread
    # LD_LIBRARY_PATH=src ./app-debug
    (hthread:30967) new thread created: 'root-process (0x11850b0)'
    (hthread:30967)     at: (null) (null):0
    (hthread:30967) mutex lock with already held mutex: 'mutex(main main.c:12) (0x1185010)'
    (hthread:30967)     by: root-process (0x11850b0)
    (hthread:30967)     at: main main.c:22
    (hthread:30967)         0x7f744a1d0d52: hthread.c (debug_dump_callstack:829)
    (hthread:30967)         0x7f744a1d4334: hthread.c (debug_mutex_try_lock:1088)
    (hthread:30967)         0x400986: main.c (main:23)
    (hthread:30967)         0x4007e9: (null) (_start:0)
    (hthread:30967)   previously acquired
    (hthread:30967)     by: root-process (0x11850b0)
    (hthread:30967)     at: main main.c:17
    (hthread:30967)   created 'mutex(main main.c:12) (0x1185010)'
    (hthread:30967)     at: main main.c:12
    app-debug: hthread.c:963: debug_mutex_add_lock: Assertion `(mt == ((void *)0)) && "mutex is already locked"' failed.
    
  hthread detected and reported the error: application was trying to lock an already locked mutex at line 22, which was
  previously locked at line 17, and was created at line 12. and terminated the process.

  program termination on error can be disabled with <tt>hthread_assert_on_error</tt> configuration parameter
  
    # hthread_assert_on_error=0 LD_LIBRARY_PATH=src ./app-debug
    (hthread:32648) new thread created: 'root-process (0xab10b0)'
    (hthread:32648)     at: (null) (null):0
    (hthread:32648) mutex lock with already held mutex: 'mutex(main main.c:12) (0xab1010)'
    (hthread:32648)     by: root-process (0xab10b0)
    (hthread:32648)     at: main main.c:22
    (hthread:32648)   previously acquired
    (hthread:32648)     by: root-process (0xab10b0)
    (hthread:32648)     at: main main.c:17
    (hthread:32648)   created 'mutex(main main.c:12) (0xab1010)'
    (hthread:32648)     at: main main.c:12
    hthread::error: (mt == NULL) && "mutex is already locked" (debug_mutex_add_lock hthread.c:823)
    
  this time hthread detected and reported the error, but not terminated the process.

## 6. contact ##

  if you are using the software and/or have any questions, suggestions, etc. please contact with me at alper.akcan@gmail.com

## 7. license ##

  Copyright (C) 2009-2013 Alper Akcan <alper.akcan@gmail.com>

  This work is free. It comes without any warranty, to the extent permitted
  by applicable law. You can redistribute it and/or modify it under the terms
  of the Do What The Fuck You Want To Public License, Version 2, as published
  by Sam Hocevar. See the COPYING file for more details.
