#include "routes.h"
#include "userlist.h"

http_code_t userRoute(char * token, method_t m, char * content, char * response) {

	if (m == GET) {
		sprintf(response, "{\"users\":[");

        int count = userlist.getUsersCount();
        for(int i = 0; i < count; i++) {
            if (i == count - 1) {
                sprintf(response + strlen(response), "\"%s\"", userlist.getUser(i));
            } else {
                sprintf(response + strlen(response), "\"%s\",", userlist.getUser(i));
            }
        }
        sprintf(response + strlen(response), "]}");

        return _200_;
	}

    token = strtok(NULL, "/");
    
	if (token != NULL) {
		switch (m) {
			case POST: {
				int result = userlist.addUser(token);
				if (result != -1) {
					sprintf(response, "{\"user\":\"%s\"}", (result != 0)?"":token);
				} else {
					sprintf(response, "{\"user\":\"\"}");
				}
				return _200_;
			}
			case DELETE: {					
                int result = userlist.removeUser(token);
				sprintf(response, "{\"user\":\"%s\"}", (result != 0)?"":token);
				return _200_;
			}
			default: return _404_;
		}
	} else return _404_;
}