
EXTRA_CFLAGS += -Wall

KDIR := /lib/modules/$(shell uname -r)/build

obj-m     += netchar.o
#netchar-y := netchar-control.o

netchar.ko: netchar.c netchar.h
	make -C $(KDIR) M=$(PWD) $@

loopback: loopback.c netchar.h
	gcc -Wall -g $< -o $@

server: server.c netchar.h
	gcc -Wall -O2 -g $< -o $@

.PHONY: clean

clean:
	make -C $(KDIR) M=$(PWD) clean
	$(RM) loopback server
