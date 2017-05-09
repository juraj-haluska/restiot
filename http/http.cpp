#include "http.h"

method_t http_parser(char * request, char ** ct, char ** uri, char ** headers, int headers_max, int * headers_count) {
	char delims [] = "\r\n\r\n";
	const int delims_length = strlen(delims);
	
	// parse content
	(*ct) = strstr(request, delims);
	if ((*ct) != NULL) {
		(*ct) = (*ct) + delims_length;
		if (strlen((*ct)) == 0) (*ct) = NULL;
	}


	// parse uri
	char * rline = strtok(request, delims);
	
	char * temp;
	char * method = strtok_r(rline, SPACE, &temp);
	(*uri) = strtok_r(NULL, SPACE, &temp);
	
	// parse method
	method_t m = GET;
	if (strstr(method, "GET") != NULL) {
		m = GET;
	} else if (strstr(method, "POST") != NULL) {
		m = POST;
	} else if (strstr(method, "PUT") != NULL) {
		m = PUT;
	} else if (strstr(method, "DELETE") != NULL) {
		m = DELETE;
	}
	
	(*headers_count) = 0;

	// parse headers
	char * token = strtok(NULL, delims);
	while ((token != NULL) && ((*headers_count) < headers_max)) {
		headers[(*headers_count)] = token;
		(*headers_count)++;
		if ((token + strlen(token)) < ((*ct) - delims_length)) {
			token = strtok(NULL, delims);
		} else {
			break ;
		}
	}
	
	return m;
}
