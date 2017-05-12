#include "routes.h"
#include "userlist.h"

Userlist::Userlist(unsigned int max, const char *defaultUser) : max(max)
{
    this->addUser(defaultUser);
}

/* returns zero if successful */
int Userlist::addUser(const char *user)
{
    int result = this->checkUser(user);
    if (result >= 0 || result == -2)
        return -1;
    if (this->users.size() >= this->max)
        return -3;

    char *newUser = new char[MAX_CHARS];
    memset(newUser, '\0', MAX_CHARS);

    strcpy(newUser, user);
    this->users.push_back(newUser);
    return 0;
}

/* returns zero if successful */
int Userlist::removeUser(const char *user)
{
    int result = this->checkUser(user);
    if (result < 0) return result;

    delete[] this->users[result];
    this->users.erase(this->users.begin() + result);
    return 0;
}

/* returns positive index of user if it's found, 
negative number for error */
int Userlist::checkUser(const char *user)
{
    if (strlen(user) > (MAX_CHARS - 1))
        return -2;
    for (int i = 0; i < this->users.size(); i++)
    {
        if (strcmp(user, this->users[i]) == 0)
        {
            return i;
        }
    }
    return -1;
}

int Userlist::getUsersCount()
{
    return this->users.size();
}

char * Userlist::getUser(int index)
{
    if (index > this->users.size())
        return NULL;
    return this->users[index];
}