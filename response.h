#pragma once

#include "http_parser.h"

#define BUFSIZE 80240

/*HTTP protocol struct*/
struct parser_state_t {
	int done;
	int header_done;

	size_t s_rcv;
	size_t size_header;

	char *content;
	size_t content_length;
};

struct parsing_t {

	int buf_head;
	int buf_tail;
	char buffer[BUFSIZE];

	http_parser parser;
	struct parser_state_t state;
	http_parser_settings settings;
};
