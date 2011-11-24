#ifndef _NETCHAR_H_
#define _NETCHAR_H_

#define NETCHAR_NUM_DEVS    10

enum {
	FOP_OPEN, FOP_RELEASE,
	FOP_READ, FOP_WRITE,
	_FOP_ERROR
};

struct fop_request {

	int       call;
	size_t    seq;

	union {
		struct {
			int     flags;
			mode_t  mode;
		};
		
		size_t  count;
	};
};

struct fop_reply {

	int      call;
	size_t   seq;

	union {
		int      open;
		int      close;
		ssize_t  read;
		ssize_t  write;
	};
};

#endif /* _NETCHAR_H_ */
