#ifndef MAIN_H
#define MAIN_H

#include "http.h"

#define BUFF_RECV_SIZE  1024
#define BUFF_SEND_SIZE  512

#define IP_ACCESOR      "192.168.15.100"    // default ip in whitelist
#define IP_ADDRESS	    "192.168.15.200"
#define MASK		    "255.255.255.0"
#define GATEWAY	        "192.168.1.1"
#define PORT	        443
#define DEF_USER        "YWRtaW46YWRtaW4="  // admin:admin

#define DEF_LOG_RATE    5000

const char PERS [] = "ssl_server";

void errorHandler();
void freeResources();

http_code_t router(method_t m, char *uri, char *content, char *response, FILE** file);

bool checkUser(char ** headers, int count);	

#endif /* MAIN_H */