/* THIS IS MESSY, I WILL NOT YET BE HELD RESPONSIBLE FOR ERRORS OR GENERAL GRIPES */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <fcntl.h>
#include <unistd.h>
#include <arpa/inet.h>

#include "netcharproto.h"

static const char* NC_CONTROL = "/dev/netchar/control";

static void print_usage(char* argv0, int fail)
{
	printf("usage: %s command [args]\n\n", argv0);
	
	printf("  The following commands are supported:\n\n"

	       "  add server port [name]\n"
	       "    server - an ip address\n"
	       "    port   - a port \n"
	       "    name   - optional name relative to /dev. if no name is\n"
	       "             specified /dev/netchar/import# will be used\n"
	       "\n"
	       "  rm name\n"
	       "    name   - the device relative to /dev to remove\n"
	       "\n"
	);

	if (fail)
		exit(EXIT_FAILURE);
}

int main(int argc, char* argv[])
{
	struct nc_admin admin;
	struct in_addr  addr;
	int             fd, err;

	if (argc < 3)
		print_usage(argv[0], 1);

	
	if (strcmp(argv[1], "add")) {

		admin.cmd = NC_CMD_ADD;

		if (argc < 4)
			print_usage(argv[0], 1);

		inet_aton(argv[2], &addr);

		admin.address = addr.s_addr; 
		admin.port    = (short) atoi(argv[3]);

		if (argc > 4)
			strncpy(admin.name, argv[4], sizeof(admin.name));

	} else if (strcmp(argv[1], "rm")) {

		admin.cmd = NC_CMD_RM;

		strncpy(admin.name, argv[2], sizeof(admin.name));

	} else {
		print_usage(argv[0], 1);
	}

	fd = open(NC_CONTROL, O_WRONLY);

	if (fd < 0) {
		perror("The netchar module is not available");
		exit(EXIT_FAILURE);
	}

	err = write(fd, &admin, sizeof(admin));

	if (err != sizeof(admin)) {
		perror("Error!");
		exit(EXIT_FAILURE);
	}

	return EXIT_SUCCESS;
}
