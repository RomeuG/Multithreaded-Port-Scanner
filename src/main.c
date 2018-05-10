#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <netdb.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define MIN_PORT 1
#define MAX_PORT 65535

int main(int argc, char** argv)
{
	if(argc != 3) {
		exit(EXIT_FAILURE);
	}

	int copts;
	int min_port = MIN_PORT;
	int max_port = MAX_PORT;
	char hostname[64];

	struct hostent *host;
	struct sockaddr_in sa;
	sa.sin_family = AF_INET;

	while((copts = getopt(argc, argv, "l:h:i:")) != -1) {
		switch(copts) {
		case 'l':
			min_port = atoi(optarg);
			break;
		case 'h':
			max_port = atoi(optarg);
			break;
		case 'i':
			strcpy(hostname, optarg);
			break;
		}
	}

	printf("IP: %s\nMin Port: %d\nMax Port: %d\n", hostname, min_port, max_port);

	// TODO: create a specific function to check the whole IP address
	if(isdigit(hostname[0])) {
		sa.sin_addr.s_addr = inet_addr(hostname);
	} else if((host = gethostbyname(hostname))) {
		strncpy((char*)&sa.sin_addr, (char*)host->h_name, sizeof(sa.sin_addr));
	} else {
		herror(hostname);
		exit(EXIT_FAILURE);
	}

	for(int i = 0; i < 65535; i++) {

	}

	return EXIT_SUCCESS;
}
