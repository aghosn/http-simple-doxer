#include <time.h>
#include <errno.h>
#include <stdio.h>
#include <netdb.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/time.h>


char *IP = "127.0.0.1";
int PORT = 11000;

static inline long time_us(void){
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return (long) tv.tv_sec * 1000000 + (long) tv.tv_usec;
}

static void execute_iter(int s, char* request) {
	char buffer[2 * 1024];
    //memset(buffer, 0, 2 * 1024);
	size_t len = strlen(request);
    //printf("The request:\n%s\n", request);
	int written = write(s, request, len);
	//assert(written == len);
	int rcv = read(s, buffer, 2 * 1024);
    //printf("The response:\n%.*s\n", rcv, buffer);
}

int main(void) {
	
	char buffer[1024];
	char request[] = "POST / HTTP/1.1\r\nHost:127.0.0.1:11000\r\nContent-Length: 5\r\n\r\n12345";
	struct sockaddr_in srvr_addr;

	srvr_addr.sin_family = AF_INET;
	srvr_addr.sin_port = htons(PORT);
	srvr_addr.sin_addr.s_addr = inet_addr(IP);

	int s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (s < 0) {
		fprintf(stderr, "Error socket.\n");
		exit(-errno);
	}

	int ssd = connect(s, (struct sockaddr*)&srvr_addr, (socklen_t)(sizeof(srvr_addr)));
	if (ssd < 0) {
		fprintf(stderr, "Error connect\n");
		exit(-errno);
	}
    
    int i = 0;
	while(1) {
		long start = time_us();
		execute_iter(s, request);
		long end = time_us();
		printf("%ld\n", end - start);
        i++;
	}

	close(s);

	return 0;
}
