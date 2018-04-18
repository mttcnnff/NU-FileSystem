
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>

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

int
main(int _argc, char* _argv[])
{
    int fd0, fd1, fd2, rv;
    char* text;

    printf("1..7\n");

    fd0 = open("mnt/four.txt", O_CREAT | O_TRUNC | O_RDWR, 0644);
    assert(fd0 != -1);
    write_to(fd0, "aaaa");
    assert(rv != -1);
    text = read_from(fd0);
    test_ok(strcmp(text, "aaaa") == 0, "buffer aaaa");

    fd1 = open("mnt/four.txt", O_RDWR);
    assert(fd1 != -1);
    text = read_from(fd1);
    test_ok(strcmp(text, "") == 0, "no early commit");
    write_to(fd1, "bbb");
    assert(rv != -1);
    text = read_from(fd1);
    test_ok(strcmp(text, "bbb") == 0, "buffer bbb");

    rv = close(fd1);
    test_ok(rv != -1, "transaction succeed");

    rv = close(fd0);
    test_ok(rv < 0, "transaction fail");

    fd2 = open("mnt/four.txt", O_RDWR);
    assert(fd2 != -1);
    text = read_from(fd2);
    test_ok(strcmp(text, "bbb") == 0, "reread bbb");
    rv = close(fd2);
    assert(rv != -1);

    test_ok(1, "done");
    return 0;
}

