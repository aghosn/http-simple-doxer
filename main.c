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

#include "http_parser.h"
#include "response.h"

char *IP = "127.0.0.1";
int PORT = 11000;

static int avail_bytes(struct parsing_t *conn)
{
	return conn->buf_tail - conn->buf_head;
}

static inline long time_us(void){
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return (long) tv.tv_sec * 1000000 + (long) tv.tv_usec;
}

static int on_header_parser(http_parser *parser)
{
	struct parsing_t* parsing = (struct parsing_t*)(parser->data);
	parsing->state.content_length = parser->content_length;
	assert(parser->nread < BUFSIZE);
	parsing->state.size_header = parser->nread;
	parsing->state.header_done = 1;
	return 0;
}

static int on_data_parser(http_parser* parser)
{
	struct parsing_t* parsing = (struct parsing_t*)(parser->data);
	parsing->state.done = 1;
	int idx = parsing->buf_head + parsing->state.size_header;
	assert(idx > 0 && idx < BUFSIZE - parsing->state.content_length);
	parsing->state.content = &parsing->buffer[idx];
	parsing->buf_head = idx + parsing->state.content_length;
	return 1;
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

static void execute_iter2(int s, char* request, struct parsing_t *parsing) {
	size_t len = strlen(request);

	int written = write(s, request, len);

	while(!parsing->state.done) {
		int s_rcv = recv(s, &parsing->buffer[parsing->buf_tail], BUFSIZE - parsing->buf_tail, 0);
		if (s_rcv < 0) {
			fprintf(stderr, "Error while reading from the socket.");
			exit(1);
		}
		assert(parsing->buf_tail < BUFSIZE + s_rcv);
		parsing->buf_tail += s_rcv;

		parsing->parser.data = parsing;
		parsing->state.s_rcv = s_rcv;

		if (!parsing->state.header_done || !parsing->state.done) {
			http_parser_execute(&parsing->parser, &parsing->settings, &parsing->buffer[parsing->buf_head], s_rcv);
		}
	}

}

// Doxing a little bit.
static void execute_iter3(int s, char* request, struct parsing_t *parsing) {
	size_t len = strlen(request);

	for (int i = 0; i < 10; i++) {
		int written = write(s, request, len);
	}

	while(!parsing->state.done) {
		assert(BUFSIZE - parsing->buf_tail > 0);
		int s_rcv = recv(s, &parsing->buffer[parsing->buf_tail], BUFSIZE - parsing->buf_tail, 0);
		if (s_rcv < 0) {
			fprintf(stderr, "Error while reading from the socket.");
			exit(1);
		}
		assert(parsing->buf_tail <= BUFSIZE - s_rcv);
		parsing->buf_tail += s_rcv;

reparse:
		parsing->parser.data = parsing;
		parsing->state.s_rcv = s_rcv;

		if (!parsing->state.header_done || !parsing->state.done) {
			http_parser_execute(&parsing->parser, &parsing->settings, &parsing->buffer[parsing->buf_head], s_rcv);
		}

		if (parsing->state.header_done && parsing->state.done) {
			// Get back some space in the buffer.
			memcpy(parsing->buffer, &parsing->buffer[parsing->buf_head], parsing->buf_tail - parsing->buf_head);
			parsing->buf_tail -= parsing->buf_head;
			parsing->buf_head = 0;
			parsing->state = (struct parser_state_t){0};
			parsing->parser = (http_parser){0};
			http_parser_init(&parsing->parser, HTTP_RESPONSE);
			if (avail_bytes(parsing) == 0) {
				break;
			}
			s_rcv = avail_bytes(parsing);
			goto reparse;
		}
	}

}

int main(void) {

	char buffer[1024];
	char request[] = "POST / HTTP/1.1\r\nHost:127.0.0.1:11000\r\nContent-Length: 5\r\n\r\n12345\0";
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

		// Parsing the response.
		struct parsing_t parsing = {0};
		parsing.buf_head = 0;
		parsing.buf_tail = 0;

		parsing.settings = (http_parser_settings){0};
		parsing.settings.on_headers_complete = on_header_parser;
		parsing.settings.on_message_complete = on_data_parser;

		parsing.parser = (http_parser){0};
		parsing.state = (struct parser_state_t){0};

		http_parser_init(&parsing.parser, HTTP_RESPONSE);

		long start = time_us();
		execute_iter3(s, request, &parsing);
		//execute_iter(s, request);
		long end = time_us();
		printf("%ld\n", end - start);
        i++;
	}

	close(s);

	return 0;
}
