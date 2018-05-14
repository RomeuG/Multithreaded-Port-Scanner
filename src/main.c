#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <netdb.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>

#include <pthread.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define MIN_PORT 1
#define MAX_PORT 65535

int sock;
int err;
int min_port = MIN_PORT;
int max_port = MAX_PORT;

struct sockaddr_in sa;

typedef struct {
	int min_port;
	int max_port;
} thread_info_t;

int is_ip_address(char* s)
{
	int i = 0;
	int dot_count = 0;

	while(*s++) {
		if(i > 4) {
			return 0;
		}

		if(*s == '.') {
			if(++dot_count > 4) {
				return 0;
			}

			i = 0;
			continue;
		}

		if(strncmp(s, "", 1) == 0) {
			continue;
		}

		if(!isdigit(*s)) {
			return 0;
		}

		i++;
	}

	return 1;
}

void *port_scanning_thread(void *ptr)
{
	thread_info_t *info = (thread_info_t*)ptr;

	for(int i = info->min_port; i < info->max_port; i++) {
		sa.sin_port = htons(i);

		sock = socket(AF_INET, SOCK_STREAM, 0);

		if(sock < 0) {
			printf("Error opening socket\n");
			exit(EXIT_FAILURE);
		}

		err = connect(sock, (struct sockaddr*)&sa, sizeof(sa));

		if(err >= 0) {
			printf("%-5d open\n", i);
		}

		close(sock);
		printf("Finished testing: %d\n", i);
		sleep(1);
	}

	return NULL;
}

int main(int argc, char** argv)
{
	int copts;
	int t_error;
	int total_ports;

	int t1_min_port;
	int t1_max_port;
	int t2_min_port;
	int t2_max_port;

	char hostname[64];

	pthread_t t1;
	pthread_t t2;

	thread_info_t t_info1;
	thread_info_t t_info2;

	struct hostent *host;

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

	total_ports = max_port - min_port;

	t1_min_port = min_port;
	t1_max_port = max_port / 2;
	t2_min_port = (max_port / 2) + 1;
	t2_max_port = max_port;

	printf("t1_min_port=%d\nt1_max_port=%d\nt2_min_port=%d\nt2_max_port=%d\n",
		   t1_min_port, t1_max_port, t2_min_port, t2_max_port);

	t_info1.min_port = t1_min_port;
	t_info1.max_port = t1_max_port;
	t_info2.min_port = t2_min_port;
	t_info2.max_port = t2_max_port;

// TODO: create a specific function to check the whole IP address
	if(is_ip_address(hostname)) {
		sa.sin_addr.s_addr = inet_addr(hostname);
	} else if((host = gethostbyname(hostname))) {
		strncpy((char*)&sa.sin_addr, (char*)host->h_name, sizeof(sa.sin_addr));
	} else {
		herror(hostname);
		exit(EXIT_FAILURE);
	}

	if((t_error = pthread_create(&t1, NULL, &port_scanning_thread, &t_info1))) {
		printf("Starting thread failed: %d\n", t_error);
		exit(EXIT_FAILURE);
	}

	if((t_error = pthread_create(&t2, NULL, &port_scanning_thread, &t_info2))) {
		printf("Starting thread failed: %d\n", t_error);
		exit(EXIT_FAILURE);
	}

	pthread_join(t1, NULL);
	pthread_join(t2, NULL);

	return EXIT_SUCCESS;
}
