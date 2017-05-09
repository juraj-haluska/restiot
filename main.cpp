#include "mbed.h"
#include "rtos.h"
#include "main.h"
#include "cert.h"
#include "key.h"
#include "utils.h"
#include "routes.h"
#include "whitelist.h"
#include <EthernetInterface.h>

/* mbed TLS */
#include "mbedtls/entropy.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/certs.h"
#include "mbedtls/x509.h"
#include "mbedtls/ssl.h"
#include "mbedtls/ssl_cache.h"

const short bsize = 1024;
uint8_t * buf;
	
mbedtls_entropy_context entropy;
mbedtls_ctr_drbg_context ctr_drbg;
mbedtls_ssl_context ssl;
mbedtls_ssl_config conf;
mbedtls_x509_crt srvcert;
mbedtls_pk_context pkey;
mbedtls_ssl_cache_context cache;

Whitelist whitelist(10, IP_ACCESOR);

// callbacks for mbedTLS
static int ssl_recv(void *ctx, unsigned char *buf, size_t len) {
	TCPSocket * client = (TCPSocket *) ctx;
	int count;
	count = client->recv(buf, len);
	return count;
}

static int ssl_send(void *ctx, const unsigned char *buf, size_t len) {
	TCPSocket * client = (TCPSocket *) ctx;
	int count;
	count = client->send(buf, len);
	return count;
}

Thread signaliseThread;
Thread serverThread(osPriorityRealtime, 16000, NULL);

// tasks
void signaliseTask() {
	while(true) {
		Thread::wait(50);
	}
}

void serverTask() {	
	int ret = 0;

    ret = mbedtls_x509_crt_parse(&srvcert, (const unsigned char *) CERT, sizeof(CERT));
    if( ret != 0 ) {
		printf("error: parsing cert\r\n");
		errorHandler();
    }
    
    ret =  mbedtls_pk_parse_key(&pkey, (const unsigned char *) KEY, sizeof(KEY), NULL, 0);
    if( ret != 0 ) {
		printf("error: parsing key\r\n");
		errorHandler();
    }

    ret = mbedtls_ctr_drbg_seed( &ctr_drbg, mbedtls_entropy_func, &entropy,(const unsigned char *) PERS, sizeof(PERS));
    if( ret != 0 ) {
        printf("error: mbedtls_ctr_drbg_seed returned %d\r\n", ret);
        errorHandler();
    }
    
    ret = mbedtls_ssl_config_defaults( &conf, MBEDTLS_SSL_IS_SERVER, MBEDTLS_SSL_TRANSPORT_STREAM, MBEDTLS_SSL_PRESET_DEFAULT );
    if( ret != 0 ) {
        printf("error: mbedtls_ssl_config_defaults returned %d\r\n", ret);
        errorHandler();
    }

    mbedtls_ssl_conf_rng( &conf, mbedtls_ctr_drbg_random, &ctr_drbg);

    mbedtls_ssl_conf_ca_chain( &conf, srvcert.next, NULL);
    
    ret = mbedtls_ssl_conf_own_cert( &conf, &srvcert, &pkey);
    if( ret != 0 ) {
        printf("error: mbedtls_ssl_conf_own_cert returned %d\r\n", ret);
        errorHandler();
    }

	// set up session id cache for session resumption (faster handshake)
	mbedtls_ssl_cache_set_max_entries(&cache, 5);
	mbedtls_ssl_cache_set_timeout(&cache, 0); // no timeout
	mbedtls_ssl_conf_session_cache( &conf, &cache, mbedtls_ssl_cache_get, mbedtls_ssl_cache_set );


    ret = mbedtls_ssl_setup( &ssl, &conf );
    if( ret != 0 ) {
        printf("error: mbedtls_ssl_setup returned %d\n\n", ret);
        errorHandler();
    }

	EthernetInterface iface;
	TCPServer server;
	TCPSocket client;
	SocketAddress client_addr;
	
	iface.disconnect();
	
	iface.set_network(IP_ADDRESS, MASK, GATEWAY);	
	
	ret = iface.connect();
	if (ret == 0) {
		printf("interface connected, IP: %s\r\n", iface.get_ip_address());
	} else {
		printf("error: connecting to interface\r\n");
		errorHandler();
	}
	
    ret = server.open(&iface);
	if (ret == 0) {
		printf("server opened");
	} else {
		printf("error: opening server\r\n");
		errorHandler();		
	}

    ret = server.bind(iface.get_ip_address(), PORT);
	if (ret == 0) {
		printf("server bound to address\r\n");
	} else {
		printf("error: opening server\r\n");
		errorHandler();	
	}

    ret = server.listen();
	if (ret == 0) {
		printf("server is listening\r\n");
	} else {
		printf("error: server is not listening\r\n");
		errorHandler();	
	}

	buf = new uint8_t [bsize];
	mbedtls_ssl_session_reset( &ssl );
	
    while (true) {
		server.accept(&client, &client_addr);
		
        uint32_t ip = *((uint32_t *) client_addr.get_ip_bytes());
		
		if (whitelist.checkIp(ip) != -1) {

			printf("accepting client, IP: %x\r\n", (unsigned int) ip);
			mbedtls_ssl_set_bio(&ssl, &client, ssl_send, ssl_recv, NULL);
			
		    // SSL HANDSHAKE
			while( ( ret = mbedtls_ssl_handshake( &ssl ) ) != 0 ) {
			if( ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE )
				{	
					printf("error: mbedtls_ssl_handshake returned %d\r\n", ret);
					break ;
				}
			}

			// HANDSHAKE SUCCESSFUL
			if (ret == 0) {
				// read request
				memset(buf, 0, bsize);
				ret = mbedtls_ssl_read( &ssl, buf, bsize - 1);

				/* pointers of each header's line in content */
				const int headers_max = 10;
				char * headers[headers_max];
				int headers_count = 0;
				
				char * content;
				char * uri;
				
				method_t m = http_parser((char *) buf, &content, &uri, headers, headers_max, &headers_count);
				
				/* check user authorization */
				if (!checkUser(headers, headers_count)) {
					return ; // unauthorized
				}

				char response[512];
				memset(response, 0, 512);
				
				http_code_t code = router(m, uri, content, response);

				int len = 0;
				switch(code) {
					case _200_: {
						len = sprintf( (char *) buf, "%s%s\r\n", HTTP_200, response);
					} break;
					case _404_: {
						len = sprintf( (char *) buf, HTTP_404);
					} break;
				}

				// write response
				while( ( ret = mbedtls_ssl_write( &ssl, buf, len ) ) <= 0 ) {

					if( ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE )
					{
						printf("error: mbedtls_ssl_write returned %d\r\n", ret);
						break ;
					}
				}

				while(( ret = mbedtls_ssl_close_notify( &ssl )) < 0 ) {
					if(ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE) {
						break ;
					}
				}
			}
			
			mbedtls_ssl_session_reset( &ssl );
		}
		
		client.close();
    }
}

int main() {

	sensorsInit();

	signaliseThread.start(signaliseTask); 	
    
    mbedtls_entropy_init( &entropy );
    mbedtls_ctr_drbg_init( &ctr_drbg );
    mbedtls_ssl_init( &ssl );
    mbedtls_ssl_config_init( &conf );
    mbedtls_x509_crt_init( &srvcert );
    mbedtls_pk_init( &pkey );
	mbedtls_ssl_cache_init( &cache );
    
    serverThread.start(serverTask);
}

void freeResources() {
    mbedtls_entropy_free( &entropy );
    mbedtls_ctr_drbg_free( &ctr_drbg );
    mbedtls_ssl_free( &ssl );
    mbedtls_ssl_config_free( &conf );
    mbedtls_x509_crt_free( &srvcert );
    mbedtls_pk_free( &pkey );
}

void errorHandler() {
	while(true) {
		printf("error\r\n");
		Thread::wait(500);
	}
}

http_code_t router(method_t m, char *uri, char *content, char *response) {
	http_code_t retval = _404_;
	char * token = strtok(uri, "/");	

	/* ROUTE "ip" */
	if (strcmp(token, "ip") == 0) {
		retval = ipRoute(token, m, content, response);
	}
	/* ROUTE "led" */
	else if (strcmp(token, "led") == 0) {
		retval = ledRoute(m, content, response);
	}
	/* ROUTE "sensor" */
	else if (strcmp(token, "sensor") == 0) {
		retval = sensorRoute(token, m, content, response);
	}
	/* ROUTE "check" */
	else if (strcmp(token, "check") == 0) {
		sprintf(response, "I'm just fine :)");
		retval = _200_;
	}
	/* ROUTE "*" */
	else {
		retval = _404_;
	}

	return retval;
}

bool checkUser(char ** headers, int count) {
	const char auth [] = "Authorization: Basic ";
	for (int i = 0; i < count; i++) {
		if (strncmp(headers[i], auth, strlen(auth)) == 0) {
			char * encoded = headers[i] + strlen(auth);
			
			/* perform user lookup */

			return true;
		}
	}

	return false;
}
