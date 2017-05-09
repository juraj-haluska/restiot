#include "routes.h"
#include "mbed.h"
#include "utils.h"

#define TF(v)	((v == 0) ? "false" : "true")

DigitalOut led1(LED1);	
DigitalOut led2(LED2);
DigitalOut led3(LED3);

http_code_t ledRoute(method_t m, char * content, char * response) {

    switch (m) {
		case GET: {
			sprintf(response, "{\"led1\":%s,\"led2\":%s,\"led3\":%s}", TF(led1), TF(led2), TF(led3));
			return _200_;
		}
		case POST: {
			bool v = false;
				
			if(jgetBool("led1", content, &v)) {
				led1 = v;
			}
			if(jgetBool("led2", content, &v)) {
				led2 = v;
			}
			if(jgetBool("led3", content, &v)) {
				led3 = v;
			}
				
			return _200_;
		}
		
		default: return _404_;
	}
}

