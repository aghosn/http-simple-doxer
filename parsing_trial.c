#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "http_parser.h"
#include "response.h"

static int on_header_parser(http_parser *parser)
{
	struct parsing_t* parsing = (struct parsing_t*)(parser->data);
	parsing->state.content_length = parser->content_length;
	assert(parser->nread < BUFSIZE);
	parsing->state.size_header = parser->nread;
	parsing->state.header_done = 1;
	printf("Parsed the header correctly\n");
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
	printf("Parsed the data as well.\n");
	return 0;
}

int main(int argc, char const *argv[]) {
	char *message =  "HTTP/1.1 200 OK\r\nServer: 127.0.0.1:11000\r\nContent-type: text/plain\r\nContent-Length: 5\r\n\r\n12345";

	struct parsing_t parsing = {0};
	parsing.buf_head = 0;
	parsing.buf_tail = 0;

	parsing.settings = (http_parser_settings){0};
	parsing.settings.on_headers_complete = on_header_parser;
	parsing.settings.on_message_complete = on_data_parser;

	parsing.parser = (http_parser){0};
	parsing.state = (struct parser_state_t){0};

	memcpy(parsing.buffer, message, strlen(message));

	http_parser_init(&parsing.parser, HTTP_RESPONSE);

	parsing.parser.data = &parsing;
	parsing.state.s_rcv = strlen(message);
	parsing.buf_tail += strlen(message);

	http_parser_execute(&parsing.parser, &parsing.settings, &parsing.buffer[parsing.buf_head], strlen(message));

	return 0;
}
