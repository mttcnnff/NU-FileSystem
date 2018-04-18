
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
    int fd, rv;
    char* text;

    printf("1..4\n");

    fd = open("mnt/one.txt", O_CREAT | O_TRUNC | O_RDWR, 0644);
    assert(fd != -1);
    write_to(fd, "one");
    assert(rv != -1);
    rv = badop(fd);
    assert(rv == -1);
    rv = close(fd);
    assert(rv != -1);

    fd = open("mnt/one.txt", O_RDWR, 0644);
    assert(fd != -1);
    text = read_from(fd);
    printf("# text = [%s]\n", text);
    assert(strcmp(text, "one") == 0);
    rv = close(fd);
    assert(rv != -1);
    test_ok(1, "read/write");

    fd = open("mnt/one.txt", O_RDWR, 0644);
    assert(fd != -1);
    write_to(fd, "bad");
    assert(rv != -1);
    text = read_from(fd);
    assert(strcmp(text, "bad") == 0);
    printf("# text = [%s]\n", text);
    test_ok(1, "read/write in trans");
    rv = rollback(fd);
    assert(rv != -1);
    text = read_from(fd);
    printf("# text = [%s]\n", text);
    assert(strcmp(text, "one") == 0);
    test_ok(1, "rollback");
    rv = badop(fd);
    assert(rv == -1);
    rv = close(fd);
    assert(rv != -1);

    test_ok(1, "done");
    return 0;
}

