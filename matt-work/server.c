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

			printf("open(%s,%i,%i)", fpath, req.flags, req.mode);
			
			fd = open(fpath, req.flags, req.mode);
			rep.open = (fd >= 0) ? 0 : -errno;
			
			printf(" -> %i\n", rep.open);

			break;

		case FOP_RELEASE:

			printf("close(%i)", fd);
			
			rep.close = close(fd);
			if (rep.close != 0)
				rep.close = -errno;

			printf(" -> %i\n", rep.close);

			break;

		case FOP_READ:

			printf("read(%i, <buf>, %zi)", fd, req.count);

			payload = realloc(payload, req.count);
			
			if (payload == NULL) {
				/* TODO let client know */
				fprintf(stderr, "error allocating mem for payload\n");
				exit(EXIT_FAILURE);
			}

			rep.read = read(fd, payload, req.count);

			if (rep.read < 0)
				rep.read = -errno;
			
			printf(" -> %zi\n", rep.read);

			break;

		case FOP_WRITE:

			printf("write(%i, <buf>, %zi)", fd, req.count);
			
			payload = realloc(payload, req.count);
		
			if (payload == NULL) {
				/* TODO let client know */
				fprintf(stderr, "error allocating mem for payload\n");
				exit(EXIT_FAILURE);
			}
			
			err = read(sd, payload, req.count);
		
			if (err < 0) {
				perror("error reading from rx_buffer for write");
				exit(EXIT_FAILURE);
			}

			rep.write = write(fd, payload, req.count);
			
			if (rep.write < 0)
				rep.write = -errno;
			
			printf(" -> %zi\n", rep.write);

			break;

		default:

			printf("unknown fop: %i\n", req.call);
			rep.call = _FOP_ERROR;
		}

		
		if (rep.call != _FOP_ERROR)
			rep.call = req.call;

		err = write(sd, &rep, sizeof(rep));

		if (err < sizeof(rep)) {
			perror("error writing rep");
			exit(EXIT_FAILURE);
		}

		if (rep.call == FOP_READ && rep.read > 0) {

			err = write(sd, payload, rep.read);
			
			if (err != rep.read) {
				perror("error writing rep payload");
				exit(EXIT_FAILURE);
			}
		}

	}

	return EXIT_SUCCESS;
}
