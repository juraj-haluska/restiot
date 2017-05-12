#ifndef HTTP_H
#define HTTP_H

#include "mbed.h"
#include <string.h>

#define SPACE " "

/* const char arrays are stored in flash memory */

const char HTTP_401 [] =
	"HTTP/1.1 401 Unauthorized\r\n"\
	"WWW-Authenticate: Basic realm=\"nucleo\"\r\n"\
	"Access-Control-Allow-Origin: *\r\n"						\
	"Access-Control-Allow-Methods: GET,POST,PUT,DELETE\r\n"		\
	"Access-Control-Allow-Headers:" 							\
		"Content-Type,"											\
		"Access-Control-Allow-Headers," 						\
		"Authorization," 										\
		"X-Requested-With\r\n"								;

const char HTTP_404 [] =
	"HTTP/1.1 404 Not Found\r\n"							;
	
const char HTTP_200 [] =
	"HTTP/1.0 200 OK\r\n"										\
	"Access-Control-Allow-Origin: *\r\n"						\
	"Access-Control-Allow-Methods: GET,POST,PUT,DELETE\r\n"		\
	"Access-Control-Allow-Headers:" 							\
		"Content-Type,"											\
		"Access-Control-Allow-Headers," 						\
		"Authorization," 										\
		"X-Requested-With\r\n"									;

const char CONTENT_STREAM [] = "Content-Type: application/octet-stream\r\n";

enum http_code_t { _200_, _401_, _404_ };	
enum method_t { GET, POST, PUT, DELETE, OPTIONS };

method_t http_parser(char * request, char ** content, char ** uri, char ** headers, int headers_max, int * headers_count);

#endif /* HTTP_H */