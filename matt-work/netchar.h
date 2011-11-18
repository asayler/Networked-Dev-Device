#ifndef _NETCHAR_H_
#define _NETCHAR_H_

#define NETCHAR_NUM_DEVS    10

enum { FOP_NONE, FOP_OPEN, FOP_READ, FOP_WRITE, FOP_RELEASE };

struct netchar_read {
	size_t size;
	loff_t* offset;
};

struct netchar_write {
	/* nothing */
};

struct netchar_msg {
	int index;
	int type;
	
	union {
		struct netchar_read      read;
		struct netchar_write     write;
	} fop;
};

struct netchar_ret {
	int index;
	int type;
	int val;
};

#endif /* _NETCHAR_H_ */
