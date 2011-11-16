
EXTRA_CFLAGS += -Wall

KDIR := /lib/modules/$(shell uname -r)/build

obj-m     += netchar.o
#netchar-y := netchar-control.o

netchar.ko: *.c
	make -C $(KDIR) M=$(PWD) $@

.PHONY: clean

clean:
	make -C $(KDIR) M=$(PWD) clean
