#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <netdb.h>
#include <string.h>
#include <ctype.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int main(int argc, char** argv)
{
	if(argc != 3) {
		exit(EXIT_FAILURE);
	}

	char hostname[64];

	struct hostent *host;
	struct sockaddr_in sa;
	sa.sin_family = AF_INET;

	// TODO: create a specific function to check the whole IP address
	if(isdigit(hostname[0])) {
		sa.sin_addr.s_addr = inet_addr(hostname);
	} else if((host = gethostbyname(hostname))) {
		strncpy((char*)&sa.sin_addr, (char*)host->h_name, sizeof(sa.sin_addr));
	} else {
		herror(hostname);
		exit(EXIT_FAILURE);
	}

	return EXIT_SUCCESS;
}
