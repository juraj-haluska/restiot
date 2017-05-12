#ifndef USERLIST_H
#define USERLIST_H

#include <vector>
#include <inttypes.h>
#include <string.h>

#define MAX_CHARS	255

class Userlist {
private:
	std::vector<char *> users;
	unsigned int max;
public:
	Userlist(unsigned int max, const char *defaultUser);
	int addUser(const char *user);
	int removeUser(const char *user);
	int checkUser(const char *user);
	int getUsersCount();
	char * getUser(int index);
};

extern Userlist userlist;

#endif /* USERLIST_H */