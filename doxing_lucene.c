#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "response.h"

static const char *taskfile = "term.tasks";
static char **queries = NULL;
static int nb_queries  = 0;

static int count_lines() {
	FILE *fp = fopen(taskfile, "r");
	if (fp == NULL)
		return -errno;
	int count = 0;
	for (char c = getc(fp); c != EOF; c = getc(fp)) {
		if (c == '\n') {
			count++;
		}
	}
	fclose(fp);
	return count;
}

void parse_queries() {
	int entries = count_lines();
	if (entries < 0) {
		fprintf(stderr, "Counting the number of lines in the file went wrong.\n");
		exit(1);
	}

	queries = calloc(sizeof(char*), (size_t) entries);
	if (queries == NULL) {
		fprintf(stderr, "Impossible to allocate the queries.\n");
		exit(1);
	}

	FILE *fp = fopen(taskfile, "r");
	if (fp == NULL) {
		fprintf(stderr, "Problem opening the file.\n");
		exit(1);
	}

	char buffer[100];
	int i = 0;
	while(fgets(buffer, 255, fp)) {
		size_t len = strlen(buffer);
		queries[i] = malloc(len * sizeof(char));
		if (queries[i] == NULL) {
			fprintf(stderr, "Unable to allocate memory for the request.\n");
			exit(1);
		}
		// Remove the \n at the end
		strncpy(queries[i], buffer, len);
		queries[i][len-1] = '\0';
		i++;
	}
	fclose(fp);
	nb_queries = entries;
}

void send_something(int s) {
	int ret;
	// generate the query.
	char *message =
"POST / HTTP/1.1\r\n\
Host: 127.0.0.1:11000\r\n\
Content-Type: text/plain\r\n\
Content-Length: %d\r\n\
\r\n\
%s";

	int i = rand() % nb_queries;
	char* query = queries[i];
	char request[2 * 1024];
	sprintf(request, message, strlen(query), query);
	ret = send(s, request, strlen(request), 0);
}

void execute_iter_lucene(int s, struct parsing_t *parsing) {
	send_something(s);

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