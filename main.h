#ifndef MAIN_H
#define MAIN_H

#include "http.h"

#define IP_ACCESOR  "192.168.58.115"   // default ip in whitelist
#define IP_ADDRESS	"192.168.58.173"
#define MASK		"255.255.255.0"
#define GATEWAY		"192.168.1.1"
#define PORT		443

const char PERS [] = "ssl_server";

void errorHandler();
void freeResources();
http_code_t router(method_t m, char *uri, char *content, char *response);

bool checkUser(char ** headers, int count);

#endif /* MAIN_H */