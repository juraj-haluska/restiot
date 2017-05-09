#include <vector>
#include <inttypes.h>
#include <string.h>

class Whitelist {
private:
	std::vector<uint32_t> ips;
	unsigned short max;
	uint32_t convertIp(const char * ip);
public:
	Whitelist(unsigned short max, const char *ip_default);
	int addIp(const char *ip);
	int removeIp(const char *ip);
	int checkIp(const uint32_t ip);
	int checkIp(const char * ip);
	unsigned int getIpCount();
	char * getIp(unsigned int index);
};

extern Whitelist whitelist;