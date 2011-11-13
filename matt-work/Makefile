
EXTRA_CFLAGS += -Wall

KDIR := /lib/modules/$(shell uname -r)/build

obj-m += vmouse.o
vmouse.ko: vmouse.c
	make -C $(KDIR) M=$(PWD) vmouse.ko

.PHONY: clean setup

clean:
	make -C $(KDIR) M=$(PWD) clean

setup: vmouse.ko
	sudo rmmod vmouse || true
	sudo insmod vmouse.ko
	sudo chmod a+rw /dev/vmouse
