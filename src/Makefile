
target.o-y = \
	libhthread.o \
	libhthread-actual.o \
	libhthread-debug.o

target.so-y = \
	libhthread.so

libhthread.o_files-y = \
	libhthread-actual.o \
	libhthread-debug.o

libhthread.so_files-y = \
	libhthread.o

libhthread-actual.o_files-y = \
	hthread.c

libhthread-actual.o_cflags-y = \
	-DHTHREAD_DEBUG=0

libhthread-debug.o_files-y = \
	hthread.c

libhthread-debug.o_cflags-y = \
	-DHTHREAD_DEBUG=1

include ../Makefile.lib
