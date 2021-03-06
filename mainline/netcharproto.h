/* netcharproto.h - netchar client module
 * protocall header file
 * Matthew Monaco
 * Andy Sayler
 */

#ifndef _NETCHAR_H_
#define _NETCHAR_H_

#ifndef NCD_PORT
#define NCD_PORT 2000   /* default port */
#endif

enum FOPS{
	FOP_OPEN, FOP_RELEASE,
	FOP_READ, FOP_WRITE,
	_FOP_ERROR
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

enum nc_cmd {
	NC_CMD_ADD,
	NC_CMD_RM,
};

struct nc_admin {
	enum nc_cmd cmd;

	long       address;
	short      port;
	char       name[1024];
};

#endif /* _NETCHAR_H_ */
