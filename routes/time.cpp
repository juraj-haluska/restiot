#include "mbed.h"
#include "routes.h"

http_code_t timeRoute(char * token, method_t m, char * content, char * response) {
    
    if (m == GET) {

        time_t utime = time(NULL);
		sprintf(response, "{\"time\":%d}", utime);

        return _200_;
	}

    token = strtok(NULL, "/");

    if (m == PUT) {

        time_t utime = atol(token);
        if (utime != 0) {
            set_time(utime);
            utime = time(NULL);
            sprintf(response, "{\"time\":%d}", utime);
            return _200_;
        }

        return _404_;
	}
}