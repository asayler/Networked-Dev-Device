struct fop_request req;
struct fop_reply   rep;
while (1) {
	err = read(sd, &req, sizeof(req));
	switch(req.call) {
	case FOP_READ:
		payload = realloc(payload, req.count);
		rep.read = read(fd, payload, req.count);
		if (rep.read < 0)
			rep.read = -errno;
		break;
	}
	err = write(sd, &rep, sizeof(rep));
	if (rep.call == FOP_READ && rep.read > 0)
		err = write(sd, payload, rep.read)
}
