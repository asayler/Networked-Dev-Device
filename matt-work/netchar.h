#ifndef _NETCHAR_H_
#define _NETCHAR_H_

#define NETCHAR_NUM_DEVS    10

enum { FOP_OPEN, FOP_READ, FOP_WRITE, FOP_RELEASE };

struct netchar_open {
	/* nothing */
};

struct netchar_read {
	size_t size;
	loff_t* offset;
};

struct netchar_msg {
	int index;
	int type;
	union {
		struct netchar_open      open;
	} fop;
};

#endif /* _NETCHAR_H_ */
