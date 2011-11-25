#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <fcntl.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "netchar.h"


int main(int argc, char *argv[])
{
	int                 port, sd, sfd, fd = -1, err;
	char*               fpath;
	struct sockaddr_in  server_addr, client_addr;
	socklen_t           client_len;

	struct fop_request  req;
	struct fop_reply    rep;
	void*               payload = NULL;

	/**
	 * parse command line
	**/

	if (argc != 3) {
		fprintf(stderr, "usage: %s port filepath\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	port  = atoi(argv[1]);
	fpath = malloc(strlen(argv[2]) + 1);

	if (fpath == NULL) {
		fprintf(stderr, "not enough memory for fpath!\n");
		exit(EXIT_FAILURE);
	}

	strcpy(fpath, argv[2]);

	printf("hosting '%s' on port %i\n\n", fpath, port);

	/**
	 * create socketchar
	**/

	sfd = socket(AF_INET, SOCK_STREAM, 0);

	if (sfd < 0) {
		perror("error opening socket");
		exit(EXIT_FAILURE);
	}

	bzero(&server_addr, sizeof(server_addr));

	server_addr.sin_family       = AF_INET;
	server_addr.sin_addr.s_addr  = htonl(INADDR_ANY);
	server_addr.sin_port         = htons(port);

	/**
	 * bind to all interfaces on given port
	**/

	err = bind(sfd, (struct sockaddr*) &server_addr, sizeof(server_addr));

	if (err < 0) {
		perror("error binding server address to socket");
		exit(EXIT_FAILURE);
	}

	/**
	 * setup socket for listening
	**/

	err = listen(sfd, SOMAXCONN);

	if (err < 0) {
		perror("error setting socket to listen");
		exit(EXIT_FAILURE);
	}

	/**
	 * begin accepting connections
	*/

	client_len = sizeof(client_addr);
	sd = accept(sfd, (struct sockaddr*) &client_addr, &client_len);

	if (sd < 0) {
		perror("error on accept");
		exit(EXIT_FAILURE);
	}

	while (1) {

		err = read(sd, &req, sizeof(req));

		if (err < 0) {
			perror("error reading from rx_buffer");
			exit(EXIT_FAILURE);
		}

		switch (req.call) {

		case FOP_OPEN:

			printf("request: open(%s, flags=%i, mode=%i)\n",
			       fpath, req.flags, req.mode);
			
			fd = open(fpath, req.flags, req.mode);
			printf("\tfd = %i\n", fd);
			
			rep.open = (fd >= 0) ? 0 : -errno;
			printf("\treply: %i\n", rep.open);

			break;

		case FOP_RELEASE:

			printf("request: close()\n");
			printf("\tfd == %i\n", fd);

			rep.close = close(fd);
			if (rep.close != 0)
				rep.close = -errno;

			printf("\treply: %i\n", rep.close);

			break;

		case FOP_READ:

			printf("request: read(count=%zi)\n", req.count);
			printf("\tfd == %i\n", fd);

			payload = realloc(payload, req.count);

			if (payload == NULL) {
				printf("\terror allocating payload: %s\n",
				       strerror(errno));
				rep.call = _FOP_ERROR;
				rep.read = -EIO;
				break;
			}
			
			rep.read = read(fd, payload, req.count);

			if (rep.read < 0)
				rep.read = -errno;

			printf("\treply: %zi\n", rep.read);
			
			break;

		case FOP_WRITE:

			printf("request: write(count=%zi)\n", req.count);
			printf("\tfd == %i\n", fd);

			payload = realloc(payload, req.count);
		
			if (payload == NULL) {
				printf("\terror allocating payload: %s\n",
				       strerror(errno));
				rep.call = _FOP_ERROR;
				rep.read = -EIO;
				break;
			}
			
			err = read(sd, payload, req.count);
		
			if (err < 0) {
				printf("\terror reading rom rx_buffer: %s\n",
				       strerror(errno));
				rep.call = _FOP_ERROR;
				rep.read = -EIO;
				break;
			}

			rep.write = write(fd, payload, req.count);
			
			if (rep.write < 0)
				rep.write = -errno;
			
			printf("\treply: %zi\n", rep.write);

			break;

		default:

			printf("unknown request fop: %i\n", req.call);
			rep.call = _FOP_ERROR;
		}

		printf("\n");

		if (rep.call != _FOP_ERROR)
			rep.call = req.call;

		err = write(sd, &rep, sizeof(rep));

		if (err < sizeof(rep)) {
			perror("error writing reply");
			continue;
		}

		if (rep.call == FOP_READ && rep.read > 0) {

			err = write(sd, payload, rep.read);
			
			if (err != rep.read) {
				perror("error writing read payload");
				continue;
			}
		}

	}

	return EXIT_SUCCESS;
}
