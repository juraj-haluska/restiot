#include "whitelist.h"
#include "inet.h"

Whitelist::Whitelist(unsigned short max, const char *ip_default): max(max) { 
	this->addIp(ip_default);
}
	
int Whitelist::addIp(const char *ip) {
	uint32_t converted = this->convertIp(ip);
	if (converted != 0) {
		int result = this->checkIp(converted);
		if (result == -1) {
			if (this->ips.size() < this->max) {
				this->ips.push_back(converted);
				return ips.size() - 1;
			}
		}
	}
	return -1;
}
	
int Whitelist::removeIp(const char *ip) {
	unsigned short i;
	uint32_t converted = this->convertIp(ip);
	for (i = 0; i < this->ips.size(); i++) {
		if (converted == this->ips[i]) { 
			this->ips.erase(this->ips.begin() + i);
			return i;
		}
	}
	return -1;
}
	
int Whitelist::checkIp(uint32_t ip) {
	unsigned short i;
	for (i = 0; i < this->ips.size(); i++) {
		if (ip == this->ips[i]) return i;
	}
	return -1;
}

int Whitelist::checkIp(const char * ip) {
	uint32_t converted = this->convertIp(ip);
	if (converted != 0) {
		return this->checkIp(converted);
	}
	return -1;
}

uint32_t Whitelist::convertIp(const char * ip) {
	uint32_t out;
	if(inet_aton(ip, &out)) {
		return out;
	}
	return 0;
}

unsigned int Whitelist::getIpCount() {
	return this->ips.size();
}

char * Whitelist::getIp(unsigned int index) {
	if (index < this->ips.size()) {
		return inet_ntoa(this->ips[index]);
	}

	return NULL;
}
