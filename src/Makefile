
target_o-y = \
	libhthread.o \
	libhthread-actual.o \
	libhthread-debug.o
	
libhthread.o_files-y = \
	libhthread-actual.o \
	libhthread-debug.o

libhthread-actual.o_files-y = \
	hthread.c

libhthread-actual.o_cflags-y = \
	-DHTHREAD_DEBUG=0

libhthread-debug.o_files-y = \
	hthread.c

libhthread-debug.o_cflags-y = \
	-DHTHREAD_DEBUG=1

include ../Makefile.lib
