enum FOPS {
	FOP_OPEN, FOP_RELEASE,
	FOP_READ, FOP_WRITE,
	_FOP_ERROR
};

struct fop_request {
	
	enum FOPS call;

	union {
		struct {
			int flags;
			mode_t mode;
		};
	
		size_t count;
	};
};

struct fop_reply {

	enum FOPS call;

	union {
		int     open;
		int     close;
		ssize_t read;
		ssize_t write;
	};
};
