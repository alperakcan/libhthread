# hthread #

  hthread is a threads helper library with synchronization errors detection support.
  
  1. <a href="#1-overview">overview</a>
  2. <a href="#2-configuration">configuration</a>
  3. <a href="#3-error-reports">error reports</a>
  4. <a href="#4-test-cases">test cases</a>
  5. <a href="#5-license">license</a>

## 1. overview ##

  hthread is for detecting synchronization errors in c/c++ programs that use the pthreads.

  can detect errors of:
  
  1. <a href="#11-misuses-of-pthreads-api">misuses of pthreads api</a>
  2. <a href="#12-lock-ordering-violation">lock ordering violation</a>
  3. <a href="#13-lock-contention">lock contention</a>

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

  hthread reads configuration parameters from environment via getenv function call. one can either set/change environment variables in source
  code of monitored project via setenv function call, or set them globally in running shell using export function.
  
  - hthread_assert_on_error
  
    > default yes
    
    > terminate the process on any pthreads api misuse and/or lock order violation.
  
  - hthread_lock_hreashold
  
    > default 5000 miliseconds
    
    > print report if lock is held longer than the specified time, in miliseconds.
  
  - hthread_lock_hreashold_assert
  
    > default no
    
    > terminate if lock is held longer than the specified time, in miliseconds.
  
  - hthread_lock_try_threshold
  
    > default 5000 miliseconds
    
    > print report if locking operation takes longer than the specified time, in miliseconds.
  
  - hthread_lock_try_threshold_assert
  
    > default no
    
    > terminate if locking operation takes longer than the specified time, in miliseconds.

## 3. error reports ##

### 3.1 misuses of pthreads api ###

#### 3.1.1. destroying an invalid mutex ####

  example code: <a href="test/fail-00.c">fail-00.c</a>
  
    # ./test/fail-00-debug 
    (hthread:22594) new thread created: 'root-process (0x2179010)'
    (hthread:22594)     at: (null) (null):0
    (hthread:22594) mutex destroy with invalid mutex: '0x7ffff1bf8938'
    (hthread:22594)     by: root-process (0x2179010)
    (hthread:22594)     at: main fail-00.c:23
    fail-00-debug: hthread.c:1202: debug_mutex_del: Assertion `(mt != ((void *)0)) && "invalid mutex"' failed.

#### 3.1.2. locking an invalid mutex ####

  example code: <a href="test/fail-01.c">fail-01.c</a>

    # ./test/fail-01-debug 
    (hthread:22639) new thread created: 'root-process (0x9dc010)'
    (hthread:22639)     at: (null) (null):0
    (hthread:22639) mutex lock with invalid mutex: '0x7fff281824d8'
    (hthread:22639)     by: root-process (0x9dc010)
    (hthread:22639)     at: main fail-01.c:23
    fail-01-debug: hthread.c:807: debug_mutex_add_lock: Assertion `(mt != ((void *)0)) && "invalid mutex"' failed.

#### 3.1.3. unlocking an invalid mutex ####

  example code: <a href="test/fail-02.c">fail-02.c</a>

    # ./test/fail-02-debug 
    (hthread:22655) new thread created: 'root-process (0xf61010)'
    (hthread:22655)     at: (null) (null):0
    (hthread:22655) mutex unlock with invalid mutex '0x7fffe8330fc8'
    (hthread:22655)     by: root-process (0xf61010)
    (hthread:22655)     at: main fail-02.c:23
    fail-02-debug: hthread.c:1083: debug_mutex_del_lock: Assertion `(mt != ((void *)0)) && "invalid mutex"' failed.

#### 3.1.4. locking an already locked mutex ####

  example code: <a href="test/fail-03.c">fail-03.c</a>

    # ./test/fail-03-debug 
    (hthread:22671) new thread created: 'root-process (0x112d0c0)'
    (hthread:22671)     at: (null) (null):0
    (hthread:22671) mutex lock with already held mutex: 'mutex(main fail-03.c:22) (0x112d010)'
    (hthread:22671)     by: root-process (0x112d0c0)
    (hthread:22671)     at: main fail-03.c:32
    (hthread:22671)   previously acquired
    (hthread:22671)     by: root-process (0x112d0c0)
    (hthread:22671)     at: main fail-03.c:27
    (hthread:22671)   created 'mutex(main fail-03.c:22) (0x112d010)'
    (hthread:22671)     at: main fail-03.c:22
    fail-03-debug: hthread.c:823: debug_mutex_add_lock: Assertion `(mt == ((void *)0)) && "mutex is already locked"' failed.

#### 3.1.5. unlocking an unheld mutex ####

  example code: <a href="test/fail-05.c">fail-05.c</a>

    # ./test/fail-05-debug 
    (hthread:22688) new thread created: 'root-process (0x106b0c0)'
    (hthread:22688)     at: (null) (null):0
    (hthread:22688) mutex unlock with unheld mutex: 'mutex(main fail-05.c:22) (0x106b010)'
    (hthread:22688)     by: root-process (0x106b0c0)
    (hthread:22688)     at: main fail-05.c:27
    (hthread:22688)   created 'mutex(main fail-05.c:22) (0x106b010)'
    (hthread:22688)     at: main fail-05.c:22
    fail-05-debug: hthread.c:1114: debug_mutex_del_lock: Assertion `(mtl != ((void *)0)) && "mutex is not locked"' failed.

#### 3.1.6. destroying a locked mutex ####

  example code: <a href="test/fail-06.c">fail-06.c</a>

    # ./test/fail-06-debug 
    (hthread:22704) new thread created: 'root-process (0xc7d0c0)'
    (hthread:22704)     at: (null) (null):0
    (hthread:22704) mutex destroy with currently locked mutex: '0xc7d010'
    (hthread:22704)     by: root-process (0xc7d0c0)
    (hthread:22704)     at: main fail-06.c:32
    (hthread:22704)   lock observed
    (hthread:22704)     by: root-process (0xc7d0c0)
    (hthread:22704)     at: main fail-06.c:27
    (hthread:22704)   created 'mutex(main fail-06.c:22) (0xc7d010)
    (hthread:22704)     at: main fail-06.c:22
    fail-06-debug: hthread.c:1218: debug_mutex_del: Assertion `(mt == ((void *)0)) && "invalid mutex"' failed.

#### 3.1.7. destroying an invalid condition ####

  example code: <a href="test/fail-20.c">fail-20.c</a>

    # ./test/fail-20-debug 
    (hthread:22720) new thread created: 'root-process (0x20ec010)'
    (hthread:22720)     at: (null) (null):0
    (hthread:22720) cond destroy with invalid condition: '0x7fff8a9a43e8'
    (hthread:22720)     by: root-process (0x20ec010)
    (hthread:22720)     at: main fail-20.c:23
    fail-20-debug: hthread.c:1287: debug_cond_del: Assertion `(cv != ((void *)0)) && "invalid condition"' failed.

#### 3.1.8. signaling an invalid condition ####

  example code: <a href="test/fail-21.c">fail-21.c</a>

    # ./test/fail-21-debug 
    (hthread:22737) new thread created: 'root-process (0x842010)'
    (hthread:22737)     at: (null) (null):0
    (hthread:22737) cond signal with invalid condition: '0x7fffa37ddd38'
    (hthread:22737)     by: root-process (0x842010)
    (hthread:22737)     at: main fail-21.c:23
    fail-21-debug: hthread.c:1319: debug_cond_check: Assertion `(cv != ((void *)0)) && "invalid condition"' failed.

#### 3.1.9. broadcasting an invalid condition ####

  example code: <a href="test/fail-22.c">fail-22.c</a>

    # ./test/fail-22-debug 
    (hthread:22753) new thread created: 'root-process (0xa24010)'
    (hthread:22753)     at: (null) (null):0
    (hthread:22753) cond signal with invalid condition: '0x7fff5c00acb8'
    (hthread:22753)     by: root-process (0xa24010)
    (hthread:22753)     at: main fail-22.c:23
    fail-22-debug: hthread.c:1319: debug_cond_check: Assertion `(cv != ((void *)0)) && "invalid condition"' failed.

#### 3.1.10. [timed]waiting on an invalid condition ####

  example code: <a href="test/fail-23.c">fail-23.c</a>

    # ./test/fail-23-debug 
    (hthread:22769) new thread created: 'root-process (0x1e700c0)'
    (hthread:22769)     at: (null) (null):0
    (hthread:22769) cond timedwait with invalid condition: '0x7fff3f3d3308'
    (hthread:22769)     by: root-process (0x1e700c0)
    (hthread:22769)     at: main fail-23.c:43
    fail-23-debug: hthread.c:1319: debug_cond_check: Assertion `(cv != ((void *)0)) && "invalid condition"' failed.

#### 3.1.11. [timed]waiting on an invalid mutex ####

  example code: <a href="test/fail-24.c">fail-24.c</a>

    # ./test/fail-24-debug 
    (hthread:22785) new thread created: 'root-process (0x13300c0)'
    (hthread:22785)     at: (null) (null):0
    (hthread:22785) cond timedwait with invalid mutex '0x7fff760defe8'
    (hthread:22785)     by: root-process (0x13300c0)
    (hthread:22785)     at: main fail-24.c:38
    fail-24-debug: hthread.c:1083: debug_mutex_del_lock: Assertion `(mt != ((void *)0)) && "invalid mutex"' failed.

#### 3.1.12. [timed]waiting on an unheld mutex ####

  example code: <a href="test/fail-25.c">fail-25.c</a>

    # ./test/fail-25-debug 
    (hthread:22802) new thread created: 'root-process (0x22330c0)'
    (hthread:22802)     at: (null) (null):0
    (hthread:22802) cond timedwait with unheld mutex: 'mutex(main fail-25.c:32) (0x2233600)'
    (hthread:22802)     by: root-process (0x22330c0)
    (hthread:22802)     at: main fail-25.c:52
    (hthread:22802)   created 'mutex(main fail-25.c:32) (0x2233600)'
    (hthread:22802)     at: main fail-25.c:32
    fail-25-debug: hthread.c:1114: debug_mutex_del_lock: Assertion `(mtl != ((void *)0)) && "mutex is not locked"' failed.

#### 3.1.13. join invalid thread ####

  example code: <a href="test/fail-40.c">fail-40.c</a>

    # ./test/fail-40-debug 
    (hthread:22825) new thread created: 'root-process (0x1741010)'
    (hthread:22825)     at: (null) (null):0
    (hthread:22825) join with invalid thread: '0x7fff6b67cd98'
    (hthread:22825)     by: root-process (0x1741010)
    (hthread:22825)     at: main fail-40.c:25
    fail-40-debug: hthread.c:344: hthread_check: Assertion `(th == thread) && "invalid thread"' failed.

#### 3.1.14. detach invalid thread ####

  example code: <a href="test/fail-41.c">fail-41.c</a>

    # ./test/fail-41-debug 
    (hthread:22843) new thread created: 'root-process (0x2438010)'
    (hthread:22843)     at: (null) (null):0
    (hthread:22843) detach with invalid thread: '0x7fff672803c8'
    (hthread:22843)     by: root-process (0x2438010)
    (hthread:22843)     at: main fail-41.c:25
    fail-41-debug: hthread.c:344: hthread_check: Assertion `(th == thread) && "invalid thread"' failed.

#### 3.1.15. unlocking mutex that was held by other thread ####

  example code: <a href="test/fail-42.c">fail-42.c</a>

    # ./test/fail-42-debug 
    (hthread:22859) new thread created: 'root-process (0xb0a0c0)'
    (hthread:22859)     at: (null) (null):0
    (hthread:22859) new thread created: 'thread(main fail-42.c:48) (0xb0a8d0)'
    (hthread:22859)     at: main fail-42.c:48
    (hthread:22859) mutex unlock with a mutex 'mutex(main fail-42.c:38) (0xb0a010)' currently hold by other thread
    (hthread:22859)     by: thread(main fail-42.c:48) (0xb0a8d0)
    (hthread:22859)     at: worker fail-42.c:23
    (hthread:22859)   lock observed
    (hthread:22859)     by: root-process (0xb0a0c0)
    (hthread:22859)     at: main fail-42.c:43
    (hthread:22859)   created 'mutex(main fail-42.c:38) (0xb0a010)'
    (hthread:22859)     at: main fail-42.c:38
    fail-42-debug: hthread.c:1103: debug_mutex_del_lock: Assertion `(mtl == ((void *)0)) && "mutex is locked by other thread"' failed.

### 3.2 lock ordering violation ###

#### 3.2.1. lock order violation in same thread ####

  example code: <a href="test/fail-04.c">fail-04.c</a>

    # ./test/fail-04-debug 
    (hthread:22925) new thread created: 'root-process (0x9740c0)'
    (hthread:22925)     at: (null) (null):0
    (hthread:22925) mutex lock order 'mutex(main fail-04.c:26) (0x974760)' before 'mutex(main fail-04.c:26) (0x974810)' violated
    (hthread:22925)   incorrect order is: acquisition of 'mutex(main fail-04.c:26) (0x974810)'
    (hthread:22925)       by: root-process (0x9740c0)
    (hthread:22925)       at: main fail-04.c:47
    (hthread:22925)     followed by a later acquisition of 'mutex(main fail-04.c:26) (0x974760)'
    (hthread:22925)       by: root-process (0x9740c0)
    (hthread:22925)       at: main fail-04.c:47
    (hthread:22925)   required order is: acquisition of 'mutex(main fail-04.c:26) (0x974760)'
    (hthread:22925)       by: root-process (0x9740c0)
    (hthread:22925)       at: main fail-04.c:33
    (hthread:22925)     followed by a later acquisition of 'mutex(main fail-04.c:26) (0x974810)'
    (hthread:22925)       by: root-process (0x9740c0)
    (hthread:22925)       at: main fail-04.c:33
    (hthread:22925)   created 'mutex(main fail-04.c:26) (0x974810)'
    (hthread:22925)     at: main fail-04.c:26
    (hthread:22925)   created 'mutex(main fail-04.c:26) (0x974760)'
    (hthread:22925)     at: main fail-04.c:26
    fail-04-debug: hthread.c:895: debug_mutex_add_lock: Assertion `(mto == ((void *)0)) && "lock order violation"' failed.

#### 3.2.2. lock order violation while [timed]waiting on condition ####

  example code: <a href="test/fail-26.c">fail-26.c</a>

    # ./test/fail-26-debug 
    (hthread:22941) new thread created: 'root-process (0x21c80c0)'
    (hthread:22941)     at: (null) (null):0
    (hthread:22941) cond wait order 'mutex(main fail-26.c:32) (0x21c8010)' before 'mutex(main fail-26.c:32) (0x21c8600)' violated
    (hthread:22941)   incorrect order is: acquisition of 'mutex(main fail-26.c:32) (0x21c8600)'
    (hthread:22941)       by: root-process (0x21c80c0)
    (hthread:22941)       at: main fail-26.c:48
    (hthread:22941)     followed by a later acquisition of 'mutex(main fail-26.c:32) (0x21c8010)'
    (hthread:22941)       by: root-process (0x21c80c0)
    (hthread:22941)       at: main fail-26.c:57
    (hthread:22941)   required order is: acquisition of 'mutex(main fail-26.c:32) (0x21c8010)'
    (hthread:22941)       by: root-process (0x21c80c0)
    (hthread:22941)       at: main fail-26.c:43
    (hthread:22941)     followed by a later acquisition of 'mutex(main fail-26.c:32) (0x21c8600)'
    (hthread:22941)       by: root-process (0x21c80c0)
    (hthread:22941)       at: main fail-26.c:48
    (hthread:22941)   created 'mutex(main fail-26.c:32) (0x21c8600)'
    (hthread:22941)     at: main fail-26.c:32
    (hthread:22941)   created 'mutex(main fail-26.c:32) (0x21c8010)'
    (hthread:22941)     at: main fail-26.c:32
    fail-26-debug: hthread.c:895: debug_mutex_add_lock: Assertion `(mto == ((void *)0)) && "lock order violation"' failed.

#### 3.2.3. lock order violation between threads ####

  example code: <a href="test/fail-43.c">fail-43.c</a>

    # ./test/fail-43-debug 
    (hthread:22959) new thread created: 'root-process (0x13db0c0)'
    (hthread:22959)     at: (null) (null):0
    (hthread:22959) new thread created: 'thread(main fail-43.c:73) (0x13dbcf0)'
    (hthread:22959)     at: main fail-43.c:73
    (hthread:22959) mutex lock order 'mutex(main fail-43.c:53) (0x13db010)' before 'mutex(main fail-43.c:58) (0x13db600)' violated
    (hthread:22959)   incorrect order is: acquisition of 'mutex(main fail-43.c:58) (0x13db600)'
    (hthread:22959)       by: thread(main fail-43.c:73) (0x13dbcf0)
    (hthread:22959)       at: worker fail-43.c:23
    (hthread:22959)     followed by a later acquisition of 'mutex(main fail-43.c:53) (0x13db010)'
    (hthread:22959)       by: thread(main fail-43.c:73) (0x13dbcf0)
    (hthread:22959)       at: worker fail-43.c:28
    (hthread:22959)   required order is: acquisition of 'mutex(main fail-43.c:53) (0x13db010)'
    (hthread:22959)       by: root-process (0x13db0c0)
    (hthread:22959)       at: main fail-43.c:63
    (hthread:22959)     followed by a later acquisition of 'mutex(main fail-43.c:58) (0x13db600)'
    (hthread:22959)       by: root-process (0x13db0c0)
    (hthread:22959)       at: main fail-43.c:68
    (hthread:22959)   created 'mutex(main fail-43.c:58) (0x13db600)'
    (hthread:22959)     at: main fail-43.c:58
    (hthread:22959)   created 'mutex(main fail-43.c:53) (0x13db010)'
    (hthread:22959)     at: main fail-43.c:53
    fail-43-debug: hthread.c:895: debug_mutex_add_lock: Assertion `(mto == ((void *)0)) && "lock order violation"' failed.

### 3.3 lock contention ###

#### 3.3.1. waiting to lock a mutex more than allowed threshold ####

  example code: <a href="test/fail-60.c">fail-60.c</a>
  
  lock try threshold is set to 1 seconds, and enabled terminating if waited more than threshold time. 

    # hthread_lock_try_threshold=1000 hthread_lock_try_threshold_assert=1 ./test/fail-60-debug 
    (hthread:23001) new thread created: 'root-process (0x1c240c0)'
    (hthread:23001)     at: (null) (null):0
    (hthread:23001) new thread created: 'thread(main fail-60.c:53) (0x1c248d0)'
    (hthread:23001)     at: main fail-60.c:53
    (hthread:23001) mutex lock still waiting for mutex: 'mutex(main fail-60.c:43) (0x1c24010)'
    (hthread:23001)     by: thread(main fail-60.c:53) (0x1c248d0)
    (hthread:23001)     at: worker fail-60.c:23
    (hthread:23001)   currently locked
    (hthread:23001)     by: root-process (0x1c240c0)
    (hthread:23001)     at: main fail-60.c:48
    (hthread:23001)   currently locked
    (hthread:23001)     by: thread(main fail-60.c:53) (0x1c248d0)
    (hthread:23001)     at: worker fail-60.c:23
    (hthread:23001)   created 'mutex(main fail-60.c:43) (0x1c24010)'
    (hthread:23001)     at: main fail-60.c:43
    fail-60-debug: hthread.c:1024: debug_mutex_try_lock: Assertion `(a == 0) && "mutex try lock threshold reached"' failed.

#### 3.3.2. hold a mutex lock more than allowed threshold ####

  example code: <a href="test/fail-61.c">fail-61.c</a>
  
  lock hold threshold is set to 1 seconds, and enabled terminating if waited more than threshold time.
   
    # hthread_lock_threshold=1000 hthread_lock_threshold_assert=1 ./test/fail-61-debug 
    (hthread:23772) new thread created: 'root-process (0xc3f0c0)'
    (hthread:23772)     at: (null) (null):0
    (hthread:23772) mutex unlock with a mutex 'mutex(main fail-61.c:22) (0xc3f010)' hold during 3000 ms
    (hthread:23772)     by: root-process (0xc3f0c0)
    (hthread:23772)     at: main fail-61.c:33
    (hthread:23772)   lock observed
    (hthread:23772)     by: root-process (0xc3f0c0)
    (hthread:23772)     at: main fail-61.c:27
    (hthread:23772)   created 'mutex(main fail-61.c:22) (0xc3f010)'
    (hthread:23772)     at: main fail-61.c:22
    fail-61-debug: hthread.c:1140: debug_mutex_del_lock: Assertion `(a == 0) && "mutex lock threshold reached"' failed.

## 4. test cases ##

  <table>
    <tr>
      <th>test</th>
      <th>sucess</th>
      <th>fail</th>
    </tr>
    <tr>
      <td>00</td>
      <td>Tom Preston-Werner</td>
      <td>Awesome</td>
    </tr>
  </table>

## 5. license ##

  Copyright (C) 2009-2013 Alper Akcan <alper.akcan@gmail.com>

  This work is free. It comes without any warranty, to the extent permitted
  by applicable law. You can redistribute it and/or modify it under the terms
  of the Do What The Fuck You Want To Public License, Version 2, as published
  by Sam Hocevar. See the COPYING file for more details.
