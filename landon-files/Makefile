#Simple Kernel Module Makefile
# Using kbuild system (2.6+ kernel)

EXTRA_CFLAGS = -Wall
KERNELDIR ?= /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

obj-m := hi.o

.PHONY: build clean

build:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules

clean:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) clean
#	rm -rf *.o *~ *.ko *.mod.c .*.cmd *.order *.symvers .tmp*
