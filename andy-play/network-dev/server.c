/* Andy Sayler
 * server.c
 * Creates 11/15/11
 * Basic dev server
 */

/* Original base source from:
 * http://www.linuxhowtos.org/C_C++/socket.htm
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>

#include <netinet/in.h>

#define FILEPATH "./testfile"

#define RCVBUFFERSIZE 256
#define SNDBUFFERSIZE 256

void error(const char *msg)
{
	perror(msg);
	exit(1);
}

int main(int argc, char *argv[])
{
	int sockfd, newsockfd, portno;
	socklen_t clilen;
	char rcvbuffer[RCVBUFFERSIZE];
	char sndbuffer[RCVBUFFERSIZE];
	struct sockaddr_in serv_addr, cli_addr;
	int n;

	int fd = -1;

	if (argc < 2) {
		fprintf(stderr, "ERROR, no port provided\n");
		exit(1);
	}

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) {
		error("ERROR opening socket");
	}

	bzero((char *) &serv_addr, sizeof(serv_addr));
	portno = atoi(argv[1]);
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(portno);

	if (bind(sockfd, (struct sockaddr *) &serv_addr,
		 sizeof(serv_addr)) < 0) {
		error("ERROR on binding");
	}

	listen(sockfd, 5);

	clilen = sizeof(cli_addr);
	newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
	if (newsockfd < 0) {
		error("ERROR on accept");
	}

	bzero(rcvbuffer, sizeof(rcvbuffer));
	n = read(newsockfd, rcvbuffer, sizeof(rcvbuffer));
	if (n < 0) {
		error("ERROR reading from socket");
	}
 	
	switch(rcvbuffer[0]) {
	case 'o':
	{
		sprintf(sndbuffer, "%s", "opening...");
		printf("%s\n", sndbuffer);
		fd = open(FILEPATH, O_RDWR|O_CLOEXEC|O_SYNC);
		if(fd < 0){
			error("ERROR opening file");
		}
		break;
	}
	case 'c':
	{
		sprintf(sndbuffer, "%s", "closing...");
		printf("%s\n", sndbuffer);
		break;
	}
	case 'r':
	{
		sprintf(sndbuffer, "%s", "reading...");
		printf("%s\n", sndbuffer);
		break;
	}
	case 'w':
	{
		sprintf(sndbuffer, "%s", "writing...");
		printf("%s\n", sndbuffer);
		break;
	}
	case '\0':
	default:
	{
		sprintf(sndbuffer, "%s", "Error: Unknown Operation");
		fprintf(stderr, "%s\n", sndbuffer);
		break;
	}
	}
	
	
	n = write(newsockfd, sndbuffer,
		strnlen(sndbuffer, sizeof(sndbuffer)));
	if (n < 0){
		error("ERROR writing to socket");
	}
	

	if(fd > 0){
		printf("fd still open. Closing fd...\n");
		if(close(fd) < 0){
			error("ERROR closing file");
		}
		else{
			fd = -1;
		}
	}

	sleep(3); /* Wait a bit for client to close first*/
	printf("Closing Sockets...\n");
	if(close(newsockfd) < 0){
		error("ERROR closing new socket");
	}
	if(close(sockfd) < 0){
		error("ERROR closing socket");
	}
	
	return 0;
}
