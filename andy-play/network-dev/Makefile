# Simple Kernel Moduel Makefule
# Using kbuild system (2.6+ kernel)

EXTRA_CFLAGS = -Wall
LOCAL_CFLAGS = -c -g -Wall -Wextra
LOCAL_LFLAGS = -Wall -Wextra
KERNELDIR ?= /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

obj-m += chardev.o
obj-m += netchardev.o

.PHONY: build clean

all: server client modules

server: server.o
	gcc $(LOCAL_LFLAGS) -o server server.o

server.o: server.c
	gcc $(LOCAL_CFLAGS) server.c

client: client.o
	gcc $(LOCAL_LFLAGS) -o client client.o

client.o: client.c
	gcc $(LOCAL_CFLAGS) client.c

modules:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules

clean:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) clean
	rm -f server client *.o
	rm -f *~
