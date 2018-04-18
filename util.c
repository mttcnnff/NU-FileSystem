#include <string.h>

int
streq(const char* aa, const char* bb)
{
    return strcmp(aa, bb) == 0;
}

int
min(int x, int y)
{
    return (x < y) ? x : y;
}

int
max(int x, int y)
{
    return (x > y) ? x : y;
}

int
bytes_to_pages(int bytes)
{
    int quo = bytes / 4096;
    int rem = bytes % 4096;
    if (rem == 0) {
        return quo;
    }
    else {
        return quo + 1;
    }
}

void
join_to_path(char* buf, char* item)
{
    int nn = strlen(buf);
    if (buf[nn - 1] != '/') {
        strcat(buf, "/");
    }
    strcat(buf, item);
}


void
get_filename(char* buf, const char* path) {
    int length = strlen(path);
    int i = length - 1;
    for (; i > -1; i--) {
        if (path[i] == '/') {
            break;
        }
    }

    int size = sizeof(buf);
    memset(buf, '\0', size);
    strcpy(buf, path + i + 1);
}

void 
get_parent_path(char* buf, const char* path) {
    int length = strlen(path);
    int i = length - 1;
    for (; i > -1; i--) {
        if (path[i] == '/') {   
            break;
        }
    }

    int size = sizeof(buf);
    memset(buf, '\0', size);
    strcpy(buf, path);

    if (i == 0) {
        buf[1] = '\0';
    } else {
        buf[i] = '\0';
    }
}