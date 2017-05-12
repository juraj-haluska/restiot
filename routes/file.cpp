#include "routes.h"
#include "mbed.h"

#define PATH_LENGTH 255

http_code_t fileRoute(char *token, method_t m, char *content, char *response, FILE **file)
{

    token = strtok(NULL, "/");

    if (token == NULL)
    {
        if (m == GET)
        {
            sprintf(response, "[");
            DIR *dir = opendir("/fs/");

            struct dirent *d = readdir(dir);
            while (d != NULL)
            {
                sprintf(response + strlen(response), "\"%s\"", &(d->d_name)[0]);
                d = readdir(dir);
                if (d != NULL) {
                    sprintf(response + strlen(response), ",");
                }
            }

            sprintf(response + strlen(response), "]");
            closedir(dir);
            return _200_;
        }
    }
    else
    {
        char path[PATH_LENGTH];
        if (strlen(token) >= PATH_LENGTH) return _404_; 
        sprintf(path, "/fs/%s", token);

        if (m == GET)
        {
            *file = fopen(path, "r");
            if (*file != NULL)
            {
                printf("file opened\r\n");
                return _200_;
            }
        }
    }
    return _404_;
}