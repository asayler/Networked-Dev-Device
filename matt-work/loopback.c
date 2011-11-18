#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

#include <linux/fanotify.h>
#include <linux/fcntl.h>
#include <unistd.h>
#include <sys/syscall.h>

/* id just like to note the I really appreciate the simply things in life,
 * like client and server having the same number of characters
 * */

const char* CLIENT_DEV = "/dev/netchar/ctl0";
const char* SERVER_DEV = "/dev/random";

int main()
{
	int fd, ret;
	struct fanotify_event_metadata* fem;

	fd = syscall(SYS_fanotify_init, FAN_ACCESS | FAN_MODIFY, 0, 0);

	if (fd < 0) {
		fprintf(stderr, "fanotify_init resulted in an error (%i): %s\n", errno, strerror(errno));
		return EXIT_FAILURE;
	}

	
	ret = syscall(SYS_fanotify_mark, fd, FAN_MARK_ADD, FAN_ACCESS | FAN_MODIFY,
	              AT_FDCWD, CLIENT_DEV);

	if (ret < 0) {
		fprintf(stderr, "fanotify_mark resulted in an error (%i): %s\n", errno, strerror(errno));
		return EXIT_FAILURE;
	}

	fem = malloc(sizeof(struct fanotify_event_metadata));

	while (1) {
		
		ret = read(fd, fem, sizeof(struct fanotify_event_metadata));

		if (ret < 0) {
			fprintf(stderr, "read resulted in error (%i): %s\n", errno, strerror(errno));
		} else {
		
			printf("%i read and got: %llu\n", ret, fem->mask);

			if (fem->mask != 0) {
				return EXIT_SUCCESS;
			}
		}
	}

	return EXIT_SUCCESS;
}
