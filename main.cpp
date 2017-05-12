#include "mbed.h"
#include "rtos.h"
#include "main.h"
#include "cert.h"
#include "key.h"
#include "utils.h"
#include "routes.h"
#include "whitelist.h"
#include "userlist.h"
#include "hts221.h"
#include "LPS25H.h"
#include "EthernetInterface.h"
#include "FATFileSystem.h"
#include "SDBlockDevice.h"
#include <stdio.h>

// mbed TLS
#include "mbedtls/entropy.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/ssl.h"
#include "mbedtls/pk.h"
#include "mbedtls/x509_crt.h"
#include "mbedtls/ssl_cache.h"

Whitelist whitelist(10, IP_ACCESOR);
Userlist userlist(10, DEF_USER);

// sensors
I2C i2c(PB_9, PB_8); // (sda, scl)
HTS221 tempHumi(i2c);
LPS25H tempPres(i2c, LPS25H_G_CHIP_ADDR);

// sd-card and filesystem
SDBlockDevice bd(PC_12, PC_11, PC_10, PC_9);
FATFileSystem fs("fs");

// callbacks for mbedTLS
static int ssl_recv(void *socket, unsigned char *buf, size_t len)
{
	TCPSocket *client = (TCPSocket *)socket;
	int count = client->recv(buf, len);
	return count;
}

static int ssl_send(void *socket, const unsigned char *buf, size_t len)
{
	TCPSocket *client = (TCPSocket *)socket;
	int count = client->send(buf, len);
	return count;
}

// thread objects
Thread sensorsThread;
Thread serverThread(osPriorityRealtime, 16000, NULL);

/* task of this thread is to measure
sensors value and save it to file on sd card */
void sensorTask()
{
	float temp, humi, press;
	FILE *fd = fopen("/fs/log.data", "w");
	if (fd != NULL)
	{
		fprintf(fd, "time,temp,humi,press\r\n");
		fclose(fd);
	}

	while (true)
	{
		time_t utime = time(NULL);

		tempHumi.ReadTempHumi(&temp, &humi);
		tempPres.get();
		press = tempPres.pressure();

		FILE *fd = fopen("/fs/log.data", "a");
		if (fd != NULL)
		{
			fprintf(fd, "%d,%f,%f,%f\r\n", utime, temp, humi, press);
			fclose(fd);
		}

		Thread::wait(DEF_LOG_RATE);
	}
}

void serverTask()
{
	mbedtls_entropy_context entropy;
	mbedtls_ctr_drbg_context ctr_drbg;
	mbedtls_ssl_context ssl;
	mbedtls_ssl_config conf;
	mbedtls_x509_crt cert;
	mbedtls_pk_context key;
	mbedtls_ssl_cache_context cache;

	mbedtls_entropy_init(&entropy);
	mbedtls_ctr_drbg_init(&ctr_drbg);
	mbedtls_ssl_init(&ssl);
	mbedtls_ssl_config_init(&conf);
	mbedtls_x509_crt_init(&cert);
	mbedtls_pk_init(&key);
	mbedtls_ssl_cache_init(&cache);

	int ret = 0;

	ret = mbedtls_x509_crt_parse(&cert, (const unsigned char *)CERT, sizeof(CERT));
	if (ret != 0)
	{
		printf("error while parsing cert\r\n");
		errorHandler();
	}

	ret = mbedtls_pk_parse_key(&key, (const unsigned char *)KEY, sizeof(KEY), NULL, 0);
	if (ret != 0)
	{
		printf("error while parsing key\r\n");
		errorHandler();
	}

	ret = mbedtls_ctr_drbg_seed(&ctr_drbg, mbedtls_entropy_func, &entropy, (const unsigned char *)PERS, sizeof(PERS));
	if (ret != 0)
	{
		printf("error while setting trng seed\r\n");
		errorHandler();
	}

	ret = mbedtls_ssl_config_defaults(&conf, MBEDTLS_SSL_IS_SERVER, MBEDTLS_SSL_TRANSPORT_STREAM, MBEDTLS_SSL_PRESET_DEFAULT);
	if (ret != 0)
	{
		printf("error while setting default configuration\r\n");
		errorHandler();
	}

	mbedtls_ssl_conf_rng(&conf, mbedtls_ctr_drbg_random, &ctr_drbg);

	mbedtls_ssl_conf_ca_chain(&conf, cert.next, NULL);

	ret = mbedtls_ssl_conf_own_cert(&conf, &cert, &key);
	if (ret != 0)
	{
		printf("error while setting certificate\r\n");
		errorHandler();
	}

	// set up session id cache for session resumption
	mbedtls_ssl_cache_set_max_entries(&cache, 5);
	mbedtls_ssl_cache_set_timeout(&cache, 0); // no timeout
	mbedtls_ssl_conf_session_cache(&conf, &cache, mbedtls_ssl_cache_get, mbedtls_ssl_cache_set);

	ret = mbedtls_ssl_setup(&ssl, &conf);
	if (ret != 0)
	{
		printf("error while setting up tls\n\n");
		errorHandler();
	}

	EthernetInterface iface;
	TCPServer server;
	TCPSocket client;
	SocketAddress client_addr;

	iface.disconnect();

	iface.set_network(IP_ADDRESS, MASK, GATEWAY);

	ret = iface.connect();
	if (ret == 0)
	{
		printf("interface connected, IP: %s\r\n", iface.get_ip_address());
	}
	else
	{
		printf("error while connecting to interface\r\n");
		errorHandler();
	}

	ret = server.open(&iface);
	if (ret == 0)
	{
		printf("server opened");
	}
	else
	{
		printf("error while opening server\r\n");
		errorHandler();
	}

	ret = server.bind(iface.get_ip_address(), PORT);
	if (ret == 0)
	{
		printf("server bound to address\r\n");
	}
	else
	{
		printf("error while opening server\r\n");
		errorHandler();
	}

	ret = server.listen();
	if (ret == 0)
	{
		printf("server is listening\r\n");
	}
	else
	{
		printf("error, server is not listening\r\n");
		errorHandler();
	}

	uint8_t buffer[BUFF_RECV_SIZE]; // buffer for incoming and outcoming data
	char response[512];				// response, will be copied to buffer
	FILE *file;						// for special case when file should be transfered

	mbedtls_ssl_session_reset(&ssl);

	while (true)
	{
		server.accept(&client, &client_addr);

		uint32_t ip = *((uint32_t *)client_addr.get_ip_bytes());

		if (whitelist.checkIp(ip) != -1)
		{

			printf("accepting client, IP: %x\r\n", (unsigned int)ip);
			mbedtls_ssl_set_bio(&ssl, &client, ssl_send, ssl_recv, NULL);

			// SSL HANDSHAKE
			while ((ret = mbedtls_ssl_handshake(&ssl)) != 0)
			{
				if (ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE)
				{
					printf("error: mbedtls_ssl_handshake returned %d\r\n", ret);
					break;
				}
			}

			// HANDSHAKE SUCCESSFUL
			if (ret == 0)
			{
				// read request
				memset(buffer, 0, BUFF_RECV_SIZE);
				ret = mbedtls_ssl_read(&ssl, buffer, BUFF_RECV_SIZE - 1);

				// pointer of each header's line in content
				const int headers_max = 10;
				char *headers[headers_max];
				int headers_count = 0;

				char *content;
				char *uri;

				method_t m = http_parser((char *)buffer, &content, &uri, headers, headers_max, &headers_count);

				memset(response, 0, BUFF_SEND_SIZE);
				file = NULL;

				http_code_t code;

				// handle OPTIONS preflight requests
				if (m == OPTIONS)
				{
					code = _200_;
				}
				else
				{
					// check user authorization
					if (!checkUser(headers, headers_count))
					{
						// unauthorized
						code = _401_;
					}
					else
					{
						// authorization ok
						code = router(m, uri, content, response, &file);
					}
				}

				int len = 0;
				switch (code)
				{
				case _200_:
				{
					if (file == NULL)
					{
						len = sprintf((char *)buffer, "%s\r\n%s\r\n", HTTP_200, response);
					}
					else
					{
						len = sprintf((char *)buffer, "%s%s\r\n%s\r\n", HTTP_200, CONTENT_STREAM, response);
					}
				}
				break;
				case _401_:
				{
					len = sprintf((char *)buffer, "%s\r\n", HTTP_401);
				}
				break;
				case _404_:
				{
					len = sprintf((char *)buffer, "%s\r\n", HTTP_404);
				}
				break;
				}

				// send normal response
				while ((ret = mbedtls_ssl_write(&ssl, buffer, len)) <= 0)
				{
					if (ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE)
					{
						printf("error while sending data\r\n", ret);
						break;
					}
				}

				// send file
				if (file != NULL)
				{
					memset(response, 0, BUFF_SEND_SIZE);
					while (!feof(file))
					{
						int size = fread(response, 1, BUFF_SEND_SIZE, file);
						while ((ret = mbedtls_ssl_write(&ssl, (uint8_t *)response, size)) <= 0)
						{
							if (ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE)
							{
								printf("error while sending file\r\n");
								break;
							}
						}
						if (ret < 0)
							break;
					}
					fclose(file);
				}

				while ((ret = mbedtls_ssl_close_notify(&ssl)) < 0)
				{
					if (ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE)
					{
						break;
					}
				}
			}

			mbedtls_ssl_session_reset(&ssl);
		}

		client.close();
	}
}

int main()
{
	// sd card initialization
	int retval = fs.mount(&bd);
	if (retval == 0)
		printf("sd-card mounted\r\n");

	// initializations of sensor
	tempHumi.init();

	sensorsThread.start(sensorTask);
	serverThread.start(serverTask);
}

void errorHandler()
{
	while (true)
	{
		printf("error\r\n");
		Thread::wait(500);
	}
}

http_code_t router(method_t m, char *uri, char *content, char *response, FILE **file)
{
	http_code_t retval = _404_;
	char *token = strtok(uri, "/");

	// ROUTE "ip"
	if (strcmp(token, "ip") == 0)
	{
		retval = ipRoute(token, m, content, response);
	}
	// ROUTE "time"
	else if (strcmp(token, "time") == 0)
	{
		retval = timeRoute(token, m, content, response);
	}
	// ROUTE "user"
	else if (strcmp(token, "user") == 0)
	{
		retval = userRoute(token, m, content, response);
	}
	// ROUTE "led"
	else if (strcmp(token, "led") == 0)
	{
		retval = ledRoute(m, content, response);
	}
	// ROUTE "sensor"
	else if (strcmp(token, "sensor") == 0)
	{
		retval = sensorRoute(token, m, content, response);
	}
	// ROUTE "check"
	else if (strcmp(token, "check") == 0)
	{
		sprintf(response, "I'm just fine :)");
		retval = _200_;
	}
	// ROUTE "file"
	else if (strcmp(token, "file") == 0)
	{
		retval = fileRoute(token, m, content, response, file);
	}
	// ROUTE "*"
	else
	{
		retval = _404_;
	}

	return retval;
}

/* This function checks if authorization header
is present. If it is, it will perform user lookup */
bool checkUser(char **headers, int count)
{
	const char auth[] = "Authorization: Basic ";
	for (int i = 0; i < count; i++)
	{
		if (strncmp(headers[i], auth, strlen(auth)) == 0)
		{
			char *encoded = headers[i] + strlen(auth);
			if (userlist.checkUser(encoded) >= 0)
			{
				return true;
			}
			return false;
		}
	}
	return false;
}