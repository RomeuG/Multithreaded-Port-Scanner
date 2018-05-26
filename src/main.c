#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <netdb.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <stdbool.h>
#include <pthread.h>

#include <sys/socket.h>
#include <sys/sysinfo.h>
#include <sys/syscall.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PRINT_DEC_I32(var) printf("%s=%d\n", #var, var);
#define PRINT_DEC_STR(var) printf("%s=%s\n", #var, var);

#define MIN_PORT 1
#define MAX_PORT 65535
#define FILE_NAME "results.txt"

int sock;
int err;
int min_port = MIN_PORT;
int max_port = MAX_PORT;

bool save = false;
FILE* f;

struct sockaddr_in sa;
pthread_mutex_t _mutex = PTHREAD_MUTEX_INITIALIZER;

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
	pid_t t_id = syscall(__NR_gettid);

	for(int i = info->min_port; i < info->max_port; i++) {
		pthread_mutex_lock(&_mutex);
		sa.sin_port = htons(i);
		pthread_mutex_unlock(&_mutex);

		sock = socket(AF_INET, SOCK_STREAM, 0);

		if(sock < 0) {
			printf("Error opening socket\n");
			exit(EXIT_FAILURE);
		}

		err = connect(sock, (struct sockaddr*)&sa, sizeof(sa));

		if(err >= 0) {
			if(save) {
				fprintf(f, "%-5d open!\n", i);
			}

			printf("%-5d open\n", i);
		}

		close(sock);
		printf("Finished testing: %d\n", i);
	}

	return NULL;
}

int main(int argc, char** argv)
{
	int copts;
	int t_error;
	int total_cpus;
	int total_ports;
	int equal_parts;

	char hostname[64];

	pthread_t *threads;
	thread_info_t *t_info;

	struct hostent *host;

	sa.sin_family = AF_INET;

	while((copts = getopt(argc, argv, "l:h:i:f:")) != -1) {
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
		case 'f':
			save = true;
			break;
		}
	}

	PRINT_DEC_STR(hostname);
	PRINT_DEC_I32(min_port);
	PRINT_DEC_I32(max_port);

	//total_cpus = get_nprocs();
	total_cpus = 6;

	if(is_ip_address(hostname)) {
		sa.sin_addr.s_addr = inet_addr(hostname);
	} else if((host = gethostbyname(hostname))) {
		strncpy((char*)&sa.sin_addr, (char*)host->h_name, sizeof(sa.sin_addr));
	} else {
		herror(hostname);
		exit(EXIT_FAILURE);
	}

	if(save) {
		f = fopen(FILE_NAME, "w");
	}

	threads = malloc(sizeof(pthread_t) * total_cpus);
	t_info = malloc(sizeof(thread_info_t) * total_cpus);

	total_ports = max_port - min_port;
	equal_parts = total_ports / total_cpus;

	PRINT_DEC_I32(total_ports);
	PRINT_DEC_I32(equal_parts);

	for(int i = 0; i < total_cpus; i++) {
		t_info[i].min_port = (i == 0) ? min_port : (equal_parts * i) + 1;
		t_info[i].max_port = (i == 0) ? equal_parts : (i + 1 == total_cpus) ? (equal_parts * total_cpus) + (max_port - (equal_parts * total_cpus)) : equal_parts * (i + 1);

		PRINT_DEC_I32(t_info[i].min_port);
		PRINT_DEC_I32(t_info[i].max_port); putchar('\n');

		if((t_error = pthread_create(&threads[i], NULL, &port_scanning_thread, &t_info[i])) != 0) {
			printf("Starting thread failed: %d\n", t_error);
			exit(EXIT_FAILURE);
		}
	}

	for(int i = 0; i < total_cpus; i++) {
		pthread_join(threads[i], NULL);
	}

	return EXIT_SUCCESS;
}
