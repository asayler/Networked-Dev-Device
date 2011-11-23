#ifndef _NETCHAR_H_
#define _NETCHAR_H_

#define NETCHAR_NUM_DEVS    10

enum {
	FOP_NONE,
	FOP_OPEN, FOP_RELEASE,
	FOP_READ, FOP_WRITE
};

enum {
	FOP_STAT_NONE,
	FOP_STAT_WAIT,
	FOP_STAT_RET_DATA,
	FOP_STAT_RET,
};


struct netchar_msg {

	int type;
	int status;

	char*   buffer;
	size_t  bufsiz;

	union {
		int     open;
		int     release;
		ssize_t read;
		ssize_t write;
	} ret;
};

#endif /* _NETCHAR_H_ */
