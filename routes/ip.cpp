#include "routes.h"
#include "whitelist.h"

http_code_t ipRoute(char * token, method_t m, char * content, char * response) {
	if (m == GET) {
		sprintf(response, "{\"ips\":[");
        unsigned int count = whitelist.getIpCount();
        for(int i = 0; i < count; i++) {
            if (i == count - 1) {
                sprintf(response + strlen(response), "\"%s\"", whitelist.getIp(i));
            } else {
                sprintf(response + strlen(response), "\"%s\",", whitelist.getIp(i));
            }
        }
        sprintf(response + strlen(response), "]}");

        return _200_;
	}

    token = strtok(NULL, "/");
    
	if (token != NULL) {
		switch (m) {
			case POST: {
				int result = whitelist.addIp(token);
				if (result != -1) {
					sprintf(response, "{\"ip\":\"%s\"}", (result==-1)?"":token);
				} else {
					sprintf(response, "{\"ip\":\"\"}");
				}
				return _200_;
			}
			case DELETE: {					
                int result = whitelist.removeIp(token);
				sprintf(response, "{\"ip\":\"%s\"}", (result==-1)?"":token);
				return _200_;
			}
			default: return _404_;
		}
	} else return _404_;
}