// based on cs3650 starter code

#ifndef UTIL_H
#define UTIL_H

#include <string.h>

static int
streq(const char* aa, const char* bb)
{
    return strcmp(aa, bb) == 0;
}

static int
min(int x, int y)
{
    return (x < y) ? x : y;
}

static int
max(int x, int y)
{
    return (x > y) ? x : y;
}

static int
clamp(int x, int v0, int v1)
{
    return max(v0, min(x, v1));
}

static int
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

static void
join_to_path(char* buf, char* item)
{
    int nn = strlen(buf);
    if (buf[nn - 1] != '/') {
        strcat(buf, "/");
    }
    strcat(buf, item);
}


static void
get_filename(char* buf, const char* path) {
    int length = strlen(path);
    int i = length - 1;
    for (i; i > -1; i--) {
        if (path[i] == '/') {
            break;
        }
    }

    memset(buf, '\0', sizeof(buf));
    strcpy(buf, path + i + 1);
}

static void 
get_parent_path(char* buf, const char* path) {
    int length = strlen(path);
    int i = length - 1;
    for (i; i > -1; i--) {
        if (path[i] == '/') {   
            break;
        }
    }

    memset(buf, '\0', sizeof(buf));
    strcpy(buf, path);

    if (i == 0) {
        buf[1] = '\0';
    } else {
        buf[i] = '\0';
    }
}



#endif
