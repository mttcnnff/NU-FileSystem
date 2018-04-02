// based on cs3650 starter code

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
#include <sys/stat.h>
//#include <dirent.h>
#include <bsd/string.h>
#include <assert.h>

#define FUSE_USE_VERSION 26
#include <fuse.h>

#include "storage.h"
#include "util.h"
#include "inode.h"
#include "directory.h"

// implementation for: man 2 access 
// Checks if a file exists. 

// DO I NEED A MASK???

//
// DONE?
//
int
nufs_access(const char *path, int mask) {     
    int status = storage_access(path); 
    int rv = 0;
    if (status == -1) {
        rv = -1;
    }

    printf("access(%s, %04o) -> %d\n", path, mask, rv);     
    return 0; 
}

//
// DONE
//
// implementation for: man 2 stat
// gets an object's attributes (type, permissions, size, etc)
int
nufs_getattr(const char *path, struct stat *st)
{
    int rv = storage_stat(path, st);

    if (rv == -1) {
        printf("ENONENT!\n");
        rv = -ENOENT;
    }

    printf("getattr(%s) -> (%d) {mode: %04o, size: %ld}\n", path, rv, st->st_mode, st->st_size);
    return rv;
}

//
// DONE
//
// implementation for: man 2 readdir
// lists the contents of a directory
int
nufs_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
             off_t offset, struct fuse_file_info *fi)
{
    slist* entries = storage_list(path);

    struct stat st;
    int rv = 0;

    slist* curr = entries;
    char fullpath[300];
    while(curr != NULL) {
        memset(fullpath, '\0', sizeof(buf));
        strcpy(fullpath, path);
        char* filename = curr->data;
        join_to_path(fullpath, filename);
        //printf("LS: %s\n", fullpath);

        storage_stat(fullpath, &st);
        filler(buf, filename, &st, 0);

        curr = curr->next;
    }

    printf("readdir(%s) -> %d\n", path, rv);
    return rv;
}

//
// DONE
//
// mknod makes a filesystem object like a file or directory
// called for: man 2 open, man 2 link
int
nufs_mknod(const char *path, mode_t mode, dev_t rdev)
{
    //ignore rdev
    int rv = storage_mknod(path, mode);

    printf("mknod(%s, %04o) -> %d\n", path, mode, rv);
    return rv;
}

//
// DONE
//
// most of the following callbacks implement
// another system call; see section 2 of the manual
int
nufs_mkdir(const char *path, mode_t mode)
{
    int rv = nufs_mknod(path, mode | 040000, 0);
    if (rv == 0) {
        int inum = tree_lookup(path);
        int parent_inum = get_parent_directory(path);
        inode* dirnode = get_inode(inum);
        directory_init(dirnode, inum, parent_inum, "");
    }
    printf("mkdir(%s) -> %d\n", path, rv);
    return rv;
}

//
// DONE
//
int
nufs_unlink(const char *path)
{
    
    int rv = storage_unlink(path);
    printf("unlink(%s) -> %d\n", path, rv);
    return rv;
}

int
nufs_link(const char *from, const char *to)
{
    int rv = -1;
    printf("link(%s => %s) -> %d\n", from, to, rv);
	return rv;
}

//
// DONE
//
int
nufs_rmdir(const char *path)
{
    int rv = nufs_unlink(path);
    printf("rmdir(%s) -> %d\n", path, rv);
    return rv;
}

//
// DONE
//
// implements: man 2 rename
// called to move a file within the same filesystem
int
nufs_rename(const char *from, const char *to)
{
    int rv;

    int inumfrom = tree_lookup(from);
    if (inumfrom == -1) {
        printf("ERROR-rename-from: %s d.n.e.\n", from);
        return -1;
    }
    inode* nodefrom = get_inode(inumfrom);

    int from_parent_inum = get_parent_directory(from);
    inode* node_from_parent = get_inode(from_parent_inum);

    int to_parent_inum = get_parent_directory(to);
    if (to_parent_inum == -1) {
        printf("ERROR-rename-to: %s d.n.e.\n", to);
    }
    inode* node_to_parent = get_inode(to_parent_inum);


    char fromname[200];
    get_filename(fromname, from);
    directory_delete(node_from_parent, fromname);

    int inumto = tree_lookup(to);
    if (inumto == -1) {
        // doesnt exist, add entry with new name in parent to directory
        char toname[200];
        get_filename(toname, to);
        directory_put(node_to_parent, toname, inumfrom);
        return 0;

    } 
    inode* nodeto = get_inode(inumto);

    // does exist

    if (is_file(nodeto) && is_file(nodefrom)) {
        // rename
        char newname[200];
        get_filename(newname, to);
        directory_put(node_to_parent, newname, inumfrom);
        return 0;
    }

    if (is_dir(nodeto)) {
        //if to is a directory -> put entry in to directory
        char fromnewname[200];
        get_filename(fromnewname, from);
        directory_put(node_to_parent, fromnewname, inumfrom);
        return 0;

    }
    return -1;
}

//
// DONE
//
int
nufs_chmod(const char *path, mode_t mode)
{
    int inum = tree_lookup(path);
    if (inum == -1) {
        printf("ERROR-Chmod: Can't find %s\n", path);
        return -1;
    }
    inode* node = get_inode(inum);
    node->mode = mode;
    int rv = 0;
    printf("chmod(%s, %04o) -> %d\n", path, mode, rv);
    return rv;
}

int
nufs_truncate(const char *path, off_t size)
{
    int rv = -1;
    printf("truncate(%s, %ld bytes) -> %d\n", path, size, rv);
    return rv;
}

// this is called on open, but doesn't need to do much
// since FUSE doesn't assume you maintain state for
// open files.
int
nufs_open(const char *path, struct fuse_file_info *fi)
{
    int rv = 0;
    printf("open(%s) -> %d\n", path, rv);
    return rv;
}

// Actually read data
int
nufs_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi)
{
    int rv = storage_read(path, buf, size, offset);
    printf("read(%s, %ld bytes, @+%ld) -> %d\n", path, size, offset, rv);
    return rv;
}

// Actually write data
int
nufs_write(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fi)
{
    int rv = storage_write(path, buf, size, offset);
    printf("write(%s, %ld bytes, @+%ld) -> %d\n", path, size, offset, rv);
    return rv;
}

//
// DONE
//
// Update the timestamps on a file or directory.
int
nufs_utimens(const char* path, const struct timespec ts[2])
{
    int rv = storage_set_time(path, ts);
    printf("utimens(%s, [%ld, %ld; %ld %ld]) -> %d\n",
           path, ts[0].tv_sec, ts[0].tv_nsec, ts[1].tv_sec, ts[1].tv_nsec, rv);
	return rv;
}

// Extended operations
int
nufs_ioctl(const char* path, int cmd, void* arg, struct fuse_file_info* fi,
           unsigned int flags, void* data)
{
    int rv = -1;
    printf("ioctl(%s, %d, ...) -> %d\n", path, cmd, rv);
    return rv;
}

void
nufs_init_ops(struct fuse_operations* ops)
{
    memset(ops, 0, sizeof(struct fuse_operations));
    ops->access   = nufs_access;
    ops->getattr  = nufs_getattr;
    ops->readdir  = nufs_readdir;
    ops->mknod    = nufs_mknod;
    ops->mkdir    = nufs_mkdir;
    ops->link     = nufs_link;
    ops->unlink   = nufs_unlink;
    ops->rmdir    = nufs_rmdir;
    ops->rename   = nufs_rename;
    ops->chmod    = nufs_chmod;
    ops->truncate = nufs_truncate;
    ops->open	  = nufs_open;
    ops->read     = nufs_read;
    ops->write    = nufs_write;
    ops->utimens  = nufs_utimens;
    ops->ioctl    = nufs_ioctl;
};

struct fuse_operations nufs_ops;

int
main(int argc, char *argv[])
{
    assert(argc > 2 && argc < 6);
    storage_init(argv[--argc]);
    nufs_init_ops(&nufs_ops);
    return fuse_main(argc, argv, &nufs_ops, NULL);
}

