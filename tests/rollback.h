#ifndef HUSKYFS_H
#define HUSKYFS_H

#include <sys/ioctl.h>

inline
static
int
rollback(int fd)
{
    // Go read the macros in.
    // /usr/include/asm-generic/ioctl.h
    // Major number 16 shouldn't conflict.
    const long ROLLBACK = _IO(16, 1);
    return ioctl(fd, ROLLBACK, 0);
}

inline
static
int
badop(int fd)
{
    return ioctl(fd, _IO(16, 2), 0);
}

#endif
