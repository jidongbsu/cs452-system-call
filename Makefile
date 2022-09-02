KERNEL_SOURCE=/lib/modules/`uname -r`/build
MY_CFLAGS += -g -DDEBUG -O0
ccflags-y += ${MY_CFLAGS}
CC += ${MY_CFLAGS}

debug:
	make -C ${KERNEL_SOURCE} M=`pwd` modules
	EXTRA_CFLAGS="$(MY_CFLAGS)"

all:
	make -C ${KERNEL_SOURCE} M=`pwd` modules

obj-m += tesla.o

clean:
	/bin/rm --force .tesla* tesla.o tesla.mod.c tesla.mod.o tesla.ko Module.symvers modules.order
	/bin/rm -fr .tmp_versions/
