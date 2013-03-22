hthread
========

hthread is a threads helper library with synchronization errors detection support.

overview
--------

hthread is for detecting synchronization errors in c/c++ programs that use the pthreads.

can detect errors of:
  1. misuses of pthreads api
  2. potential deadlocks arising from lock ordering mismatches

license
-------

Copyright (C) 2009-2013 Alper Akcan <alper.akcan@gmail.com>

This work is free. It comes without any warranty, to the extent permitted
by applicable law. You can redistribute it and/or modify it under the terms
of the Do What The Fuck You Want To Public License, Version 2, as published
by Sam Hocevar. See the COPYING file for more details.
