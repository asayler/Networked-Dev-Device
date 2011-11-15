# Simple Kernel Moduel Makefule
# Using kbuild system (2.6+ kernel)

EXTRA_CFLAGS = -Wall
KERNELDIR ?= /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

obj-m += chardev.o

.PHONY: build clean

all: modules

modules:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules

clean:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) clean
	rm -f *~
