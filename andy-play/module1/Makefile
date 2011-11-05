#Simple Kernel Module Makefile

EXTRA_CFLAGS = -Wall

ifeq ($(KERNELRELEASE),)

KERNELDIR ?= /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

.PHONY: build clean

build:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules

clean:
	rm -rf *.o *~ *.ko *.mod.c .*.cmd *.order *.symvers .tmp*
else

$(info Building with KERNELRELEASE = ${KERNELRELEASE})
obj-m := hellomodule.o

endif
