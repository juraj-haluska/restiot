#ifndef HTTP_H
#define HTTP_H

#include "mbed.h"
#include <string.h>

#define SPACE " "

const char HTTP_404 [] =
	"HTTP/1.1 404 Not Found\r\n\r\n"\
	"<!DOCTYPE HTML PUBLIC \"-//IETF//DTD HTML 2.0//EN\">\n<html>\n<head>\n<title>404 Not Found</title>\n</head>\n<body>\n<h1>Not Found</h1>\n<p>"\
	"The requested URL was not found on this server.</p>\n</body>\n</html>\r\n";
	
const char HTTP_200 [] =
	"HTTP/1.0 200 OK\r\n"										\
	"Access-Control-Allow-Origin: *\r\n"						\
	"Access-Control-Allow-Methods: GET,POST,PUT,DELETE\r\n"		\
	"Access-Control-Allow-Headers: 								\
		Content-Type,											\
		Access-Control-Allow-Headers, 							\
		Authorization, 											\
		X-Requested-With\r\n"									\
	"Content-Type: text/html; charset=utf-8\r\n\r\n"			;

const char HTTP_ITWORKS [] =
	"HTTP/1.0 200 OK\r\n"										\
	"Content-Type: text/html; charset=utf-8\r\n\r\n"			\
	"<html>"													\
	"<body style=\"display:flex;text-align:center\">"			\
	"<div style=\"margin:auto\">"								\
	"<h1>Hello World</h1>"										\
	"<p>It works :(.....</p>"									\
	"</div>"													\
	"</body>"													\
	"</html>\r\n"												;
	
enum http_code_t { _200_, _404_ };	
	
enum method_t { GET, POST, PUT, DELETE };

method_t http_parser(char * request, char ** content, char ** uri, char ** headers, int headers_max, int * headers_count);

#endif /* HTTP_H */