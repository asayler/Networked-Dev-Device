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

const char* CLIENT_DEV = "/dev/netchar/ctl0";
const char* SERVER_DEV = "/dev/random";

const useconds_t POLL_PERIOD = 1000;

int main()
{
	int cfd, sfd, i;
	struct netchar_msg msg;
	struct netchar_ret ret;

	cfd = open(CLIENT_DEV, O_RDWR, 0);

	if (cfd == -1) {

		printf("Error opening %s (%i): %s\n",
		       CLIENT_DEV, errno, strerror(errno));
		return EXIT_FAILURE;
	}

	while (1) {

		i = read(cfd, &msg, sizeof(msg));

		if (i == -1) {

			printf("Error reading %s (%i): %s\n",
			       CLIENT_DEV, errno, strerror(errno));

			return EXIT_FAILURE;
		
		} else if (i != sizeof(struct netchar_msg)) {

			printf("Weirdness reading %s, %i returned\n",
			       CLIENT_DEV, i);
		}

		switch(msg.type) {

		case FOP_NONE:
			break;;

		case FOP_OPEN:
			puts("FOP_OPEN");
			sfd = open(SERVER_DEV, O_RDONLY, 0); /* FIXME need to copy mode too */
			ret.type = msg.type;
			ret.val  = sfd;
			write(cfd, &ret, sizeof(ret));
			break;;

		default:
			puts("NOIMP");
		}

		msg.type = FOP_NONE;

		usleep(POLL_PERIOD);
	}

	return EXIT_SUCCESS;
}
