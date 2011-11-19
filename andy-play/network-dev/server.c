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

#define RCVBUFFERSIZE 256
#define SNDBUFFERSIZE 256

void error(const char *msg)
{
	perror(msg);
}

int main(int argc, char *argv[])
{
	int sockfd, newsockfd, portno;
	socklen_t clilen;
	char rcvbuffer[RCVBUFFERSIZE];
	char sndbuffer[RCVBUFFERSIZE];
	struct sockaddr_in serv_addr, cli_addr;
	ssize_t n;
	int run = 1;

	char* port;
	char* filepath;
	
	int fd = -1;
	ssize_t sndsize = -1;

	if (argc < 3) {
		fprintf(stderr, "usage: %s port filepath\n",
			argv[0]);
		exit(1);
	}

	port = argv[1];
	filepath = argv[2];

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) {
		error("ERROR opening socket");
	}

	bzero((char *) &serv_addr, sizeof(serv_addr));
	portno = atoi(port);
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

	while(run){

		/* Zero Buffers */
		bzero(rcvbuffer, sizeof(rcvbuffer));
		bzero(sndbuffer, sizeof(sndbuffer));

		/* Receive Messege */
		n = read(newsockfd, rcvbuffer, sizeof(rcvbuffer));
		if (n < 0) {
			error("ERROR reading from socket");
		}
 	
		/* Handle Action */
		switch(rcvbuffer[0]) {
		case 'o':
		{
			if(fd < 0){
				fd = open(filepath, O_RDWR|O_CLOEXEC|O_SYNC);
				if(fd < 0){
					error("ERROR opening file");
					sprintf(sndbuffer, "%s",
						"ERROR opening fd");
					sndsize = strnlen(sndbuffer,
							sizeof(sndbuffer));
				}
				else{
					sprintf(sndbuffer, "%s: %d",
						"fd opened", fd);
					sndsize = strnlen(sndbuffer,
							sizeof(sndbuffer));
				}
			}
			else{
				sprintf(sndbuffer, "%s",
					"WARNING fd already open");
				sndsize = strnlen(sndbuffer,
						sizeof(sndbuffer));
			}
			break;
		}
		case 'c':
		{
			if(fd > 0){
				if(close(fd) < 0){
					error("ERROR closing fd");
					sprintf(sndbuffer, "%s",
						"ERROR closing fd");
					sndsize = strnlen(sndbuffer,
							sizeof(sndbuffer));
				}
				else{
					fd = -1;
					sprintf(sndbuffer, "%s", "fd closed");
					sndsize = strnlen(sndbuffer,
							sizeof(sndbuffer));
				}
			}
			else{
				sprintf(sndbuffer, "%s",
					"WARNING no fd open");
				sndsize = strnlen(sndbuffer,
						sizeof(sndbuffer));
			}
			break;
		}
		case 'r':
		{
			if(fd > 0){
				sndsize = read(fd, sndbuffer,
					sizeof(sndbuffer));
				fprintf(stderr, "read %zd bytes\n", sndsize);
				if(sndsize == 0){
					sprintf(sndbuffer, "%s",
						"WARNING EOF reached");
					sndsize = strnlen(sndbuffer,
							sizeof(sndbuffer));
				}
				else if(sndsize < 0){
					error("ERROR reading");
					sprintf(sndbuffer, "%s",
						"ERROR reading");
					sndsize = strnlen(sndbuffer,
							sizeof(sndbuffer));
				}
			}
			else{
				sprintf(sndbuffer, "%s",
					"WARNING no fd open");
				sndsize = strnlen(sndbuffer,
						sizeof(sndbuffer));
			}
			break;
		}
		case 'w':
		{
			if(fd > 0){
				fprintf(stderr, "writing %zd bytes\n", n-2);
				sndsize = write(fd, &(rcvbuffer[2]), n-2);
				sprintf(sndbuffer, "%zd bytes written",
					sndsize);
				sndsize = strnlen(sndbuffer,
						sizeof(sndbuffer));
				if(sndsize < 0){
					error("ERROR reading");
					sprintf(sndbuffer, "%s",
						"ERROR reading");
					sndsize = strnlen(sndbuffer,
							sizeof(sndbuffer));
				}
			}
			else{
				sprintf(sndbuffer, "%s",
					"WARNING no fd open");
				sndsize = strnlen(sndbuffer,
						sizeof(sndbuffer));
			}
			break;
		}
		case 'q':
		{
			sprintf(sndbuffer, "%s", "quiting...");
			sndsize = strnlen(sndbuffer,
					sizeof(sndbuffer));
			run = 0;
			break;
		}
		case '\0':
		default:
		{
			sprintf(sndbuffer, "%s", "Error: Unknown Operation");
			sndsize = strnlen(sndbuffer,
					sizeof(sndbuffer));
			break;
		}
		}
	
		/* Send response */
		fprintf(stderr, "%s\n", sndbuffer);
		n = write(newsockfd, sndbuffer, sndsize);
		if (n < 0){
			error("ERROR writing to socket");
		}
	}	


	if(fd > 0){
		printf("WARNING fd still open.\n");
		printf("Closing fd...\n");
		if(close(fd) < 0){
			error("ERROR closing file");
		}
		else{
			fd = -1;
		}
	}

	sleep(1); /* Wait a bit for client to close first*/
	printf("Closing Sockets...\n");
	if(close(newsockfd) < 0){
		error("ERROR closing new socket");
	}
	if(close(sockfd) < 0){
		error("ERROR closing socket");
	}
	
	return 0;
}
