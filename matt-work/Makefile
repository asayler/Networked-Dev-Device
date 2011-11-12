
EXTRA_CFLAGS += -Wall

KDIR := /lib/modules/$(shell uname -r)/build

obj-m += vmouse.o
vmouse.ko: vmouse.c
	make -C $(KDIR) M=$(PWD) vmouse.ko

.PHONY: clean
clean:
	make -C $(KDIR) M=$(PWD) clean
