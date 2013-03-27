
tests-success-y = \
	$(subst .c, , $(wildcard success-*.c))

tests-fail-y = \
	$(subst .c, , $(filter-out fail-env.c, $(wildcard fail-*.c)))

target-y = \
	${tests-success-y} \
	$(addsuffix -debug, ${tests-success-y}) \
	${tests-fail-y} \
	$(addsuffix -debug, ${tests-fail-y})

uname_S := $(shell sh -c 'uname -s 2>/dev/null || echo not')

define test-defaults
	$1_files-y = \
		$(addsuffix .c, $1)

	$1_includes-y = \
		../src
	
	$1_ldflags-y = \
		-lpthread
	
ifeq ($(uname_S),Linux)
	$1_ldflags-y += \
		-lrt
endif
endef

define test-debug-defaults
	$1_files-y = \
		$(addsuffix .c, $(subst -debug, , $1)) \
		../src/libhthread.o

	$1_cflags-y = \
		-O0 \
		-DHTHREAD_DEBUG=1 \
		-DHTHREAD_ENABLE_RACE_CHECK=1 \
		-include ../src/hthread.h
	
	$1_includes-y = \
		../src
	
	$1_ldflags-y = \
		-lpthread
		
	$1_ldflags-${HTHREAD_ENABLE_CALLSTACK} += \
		-rdynamic \
		-ldl \
		-lbfd
	
ifeq ($(uname_S),Linux)
	$1_ldflags-y += \
		-lrt
endif
endef

$(eval $(foreach T,$(target-y), $(eval $(call test-defaults,$T))))
$(eval $(foreach T,$(target-y), $(eval $(call test-debug-defaults,$(addsuffix -debug, $T)))))

include ../Makefile.lib
