#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

#include <fcntl.h>
#include <unistd.h>

#include "netchar.h"

/* id just like to note the I really appreciate the simply things in life,
 * like client and server having the same number of characters
 * */

#define CLIENT_DEV  "/dev/netchar/ctl0"
#define SERVER_DEV  "/dev/urandom"

const useconds_t POLL_PERIOD = 10000;

void printe(char* s)
{
	fprintf(stderr, "%s (%i): %s\n", s, errno, strerror(errno));
	exit(EXIT_FAILURE);
}

int main(void)
{
	int cfd, sfd, i;
	char* buffer;
	struct netchar_msg msg;

	cfd = open(CLIENT_DEV, O_RDWR, 0);

	if (cfd == -1)
		printe("error opening " CLIENT_DEV);

	while (1) {

		i = read(cfd, &msg, sizeof(msg));

		if (i == -1)
			printe("error reading " CLIENT_DEV);
		
		else if (i != sizeof(msg))
			printe("weirdness reading " CLIENT_DEV);

		switch(msg.type) {

		case FOP_NONE:
			break;

		case FOP_OPEN:
			
			puts("FOP_OPEN");
			
			sfd = open(SERVER_DEV, O_RDONLY, 0); /* FIXME need to copy mode too */
			
			printf("open() returned: %i\n", sfd);
			
			msg.status   = FOP_STAT_RET;
			msg.ret.open = sfd;
			
			i = write(cfd, &msg, sizeof(msg));
			printf("write() returned: %i\n", i);
			
			break;

		case FOP_RELEASE:

			puts("FOP_RELASE");
			
			sfd = close(sfd);

			printf("close() returned: %i\n", sfd);

			msg.status      = FOP_STAT_RET;
			msg.ret.release = sfd;

			i = write(cfd, &msg, sizeof(msg));
			printf("write() returned: %i\n", i);

			break;

		case FOP_READ:

			puts("FOP_READ");

			printf("asked to read %zi bytes\n", msg.bufsiz);

			buffer = malloc(sizeof(msg.bufsiz));
			i = read(sfd, buffer, msg.bufsiz);

			printf("read() returned: %i\n", i);

			printf("<");
			write(0, buffer, i);
			puts(">");

			msg.status   = FOP_STAT_RET_DATA;
			msg.ret.read = i;

			i = write(cfd, &msg, sizeof(msg));
			printf("write() returned: %i\n", i);

			msg.status = FOP_STAT_RET;

			i = write(cfd, buffer, i);
			printf("write()2 returned: %i\n", i);

			break;

		default:
			puts("NOIMP");
		}

		msg.type = FOP_NONE;

		usleep(POLL_PERIOD);
	}

	return EXIT_SUCCESS;
}
