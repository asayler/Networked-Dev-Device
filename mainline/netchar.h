#ifndef _NETCHAR_H_
#define _NETCHAR_H_

enum FOPS{
	FOP_OPEN, FOP_RELEASE,
	FOP_READ, FOP_WRITE,
	_FOP_ERROR
};

enum TESTS{
	TEST1,
	TEST2
};

struct fop_request {

	enum FOPS call;

	union {
		struct {
			int     flags;
			mode_t  mode;
		};
		
		size_t  count;
	};
};

struct fop_reply {

	enum FOPS call;

	union {
		int      open;
		int      close;
		ssize_t  read;
		ssize_t  write;
	};
};

#endif /* _NETCHAR_H_ */
