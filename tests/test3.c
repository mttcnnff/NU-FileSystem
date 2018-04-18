
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "rollback.h"

static int test_i = 1;
void
test_ok(int cond, const char* text)
{
    if (cond) {
        printf("ok %d - %s\n", test_i++, text);
    }
    else {
        printf("not ok %d - %s\n", test_i++, text);
    }
}

int
write_to(int fd, const char* msg)
{
    int nn = strlen(msg);
    return write(fd, msg, nn);
}

static char read_buf[8192];
char*
read_from(int fd)
{
    int rv;
    memset(read_buf, 0, 8192);
    rv = lseek(fd, 0, SEEK_SET);
    assert(rv != -1);
    rv = read(fd, read_buf, 8192);
    assert(rv != -1);
    return read_buf;
}

void
increment(int fd)
{
    static char temp[20];
    char* text = read_from(fd);
    int nn = atoi(text);
    snprintf(temp, 20, "%d", nn + 1);
    int rv = lseek(fd, 0, SEEK_SET);
    assert(rv != -1);
    rv = write_to(fd, temp);
    assert(rv != -1);
}

int
main(int _argc, char* _argv[])
{
    int fd0, fd1, fd2, rv;
    char* text;

    printf("1..3\n");

    fd0 = open("mnt/count.txt", O_CREAT | O_TRUNC | O_RDWR, 0644);
    assert(fd0 != -1);
    rv = write_to(fd0, "0");
    assert(rv != -1);
    rv = close(fd0);
    test_ok(rv != -1, "write succeed");

    fd1 = open("mnt/count.txt", O_RDWR);
    assert(fd1 != -1);
    increment(fd1);
    rv = close(fd1);
    assert(rv != -1);

    fd1 = open("mnt/count.txt", O_RDWR);
    assert(fd1 != -1);
    increment(fd1);
    rv = close(fd1);
    assert(rv != -1);


    fd0 = open("mnt/count.txt", O_RDWR);
    assert(fd0 != -1);
    increment(fd0);

    fd1 = open("mnt/count.txt", O_RDWR);
    assert(fd1 != -1);
    increment(fd1);
    rv = close(fd1);
    assert(rv != -1);

    rv = fsync(fd0);
    assert(rv != 0);

    if (rv != 0) {
        rollback(fd0);
        increment(fd0);
        rv = close(fd0);
        assert(rv == 0);
    }

    fd2 = open("mnt/count.txt", O_RDWR);
    assert(fd2 != -1);
    text = read_from(fd2);
    int vv = atoi(text);
    test_ok(vv == 4, "four increments succeeded");

    rv = close(fd2);
    assert(rv != -1);

    test_ok(1, "done");
    return 0;
}

