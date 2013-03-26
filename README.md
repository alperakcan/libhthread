# hthread #

  hthread is a thread error detector and helper library with synchronization errors detection support.
  
  1. <a href="#1-overview">overview</a>
  2. <a href="#2-configuration">configuration</a>
  3. <a href="#3-error-reports">error reports</a>
  4. <a href="#4-test-cases">test cases</a>
  5. <a href="#5-usage-example">usage example</a>
  6. <a href="#6-contact">contact</a>
  7. <a href="#7-license">license</a>

## 1. overview ##

  hthread is thread error detector for detecting synchronization errors in c/c++ programs that use the pthreads.

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

  using hthread is pretty simple, just add <tt>-include hthread.h -DHTHREAD_DEBUG=1</tt> to cflags, and link with <tt>-lhthread -lrt</tt>
  
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

    # gcc -include src/hthread.h -DHTHREAD_DEBUG=1 -o app-debug main.c -Lsrc -lhthread -lrt -lpthread
    # ./app-debug
    
    # LD_LIBRARY_PATH=src ./app-debug
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
    app-debug: hthread.c:823: debug_mutex_add_lock: Assertion `(mt == ((void *)0)) && "mutex is already locked"' failed.
    
  hthread detected and reported the error: application was trying to lock an already locked mutex at line 22, which was
  previously locked at line 17, and was created at line 12.

## 6. contact ##

  if you are using the software and/or have any questions, suggestions, etc. please contact with me at alper.akcan@gmail.com

## 7. license ##

  Copyright (C) 2009-2013 Alper Akcan <alper.akcan@gmail.com>

  This work is free. It comes without any warranty, to the extent permitted
  by applicable law. You can redistribute it and/or modify it under the terms
  of the Do What The Fuck You Want To Public License, Version 2, as published
  by Sam Hocevar. See the COPYING file for more details.
