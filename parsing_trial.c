#include <stdio.h>
#include "http_parser.h"
#include "response.h"


int main(int argc, char const *argv[]) {
	char *response = "HTTP/1.1 200 OK\r\nServer: 127.0.0.1:11000\r\nContent-type: text/plain\r\nContent-Lenght: 5\r\n\r\n12345";

	return 0;
}
