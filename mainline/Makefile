# Mainline netchar Makefile
# Using kbuild system (2.6+ kernel) for modules

EXTRA_CFLAGS += -Wall
LOCAL_CFLAGS = -c -g -Wall -Wextra -O2
LOCAL_LFLAGS = -Wall -Wextra

KDIR ?= /lib/modules/$(shell uname -r)/build
PWD ?= $(shell pwd)

obj-m += netchar.o

.PHONY: all clean modules

all: server modules

modules: netchar.ko

netchar.ko: netchar.c netcharmod.h netcharproto.h
	$(MAKE) -C $(KDIR) M=$(PWD) $@

server: server.o
	$(CC) $(LOCAL_LFLAGS) -o $@ $<

server.o: server.c netcharproto.h
	$(CC) $(LOCAL_CFLAGS) -o $@ $<

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean
	$(RM) server
	$(RM) *.o
	$(RM) *~
	$(RM) \#*\#
