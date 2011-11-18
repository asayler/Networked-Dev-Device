
EXTRA_CFLAGS += -Wall

KDIR := /lib/modules/$(shell uname -r)/build

obj-m     += netchar.o
#netchar-y := netchar-control.o

netchar.ko: netchar.c netchar.h
	make -C $(KDIR) M=$(PWD) $@

.PHONY: clean

clean:
	make -C $(KDIR) M=$(PWD) clean
