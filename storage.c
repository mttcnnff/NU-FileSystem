#include "storage.h"
#include "pages.h"
#include "inode.h"
#include "directory.h"
#include "util.h"
#include <stdio.h>
#include <string.h>

void storage_init(const char* path) {
	//printf("Initializing Storage: %s\n", path);
	pages_init(path);
}

int storage_stat(const char* path, struct stat* st) {

	int inum = tree_lookup(path);
	if (inum == -1) {
		return -1;
	}

	inode* info = get_inode(inum);

	memset(st, 0, sizeof(struct stat));
	st->st_uid = getuid();
	st->st_gid = getgid();
	st->st_nlink = 1;
	st->st_mode = info->mode;
	st->st_atime = time(NULL);
	st->st_mtime = time(NULL);
	st->st_size = info->size;
	return 0;
}
// int    storage_read(const char* path, char* buf, size_t size, off_t offset);
// int    storage_write(const char* path, const char* buf, size_t size, off_t offset);
// int    storage_truncate(const char *path, off_t size);
int storage_mknod(const char* path, int mode) {
	int inum = alloc_inode();
	if (inum == -1) {
		printf("ERROR: No free inode found!\n");
		return -1;
	}

	printf("MODE RECIEVED: %d\n", mode);

	inode* newnode = get_inode(inum);
	newnode->refs = 1;
	newnode->mode = mode;
	newnode->size = 0;

	int parent_inum = get_parent_directory(path);
	if (parent_inum == -1) {
		printf("ERROR: Path doesn't exist!\n");
		return -1;
	}

	inode* parent_inode = get_inode(parent_inum);
	char filename[200];
	get_filename(filename, path);
	directory_put(parent_inode, filename, inum);

	printf("Made inode: \n");
	print_inode(newnode);

	return 0;
}
// int    storage_unlink(const char* path);
// int    storage_link(const char *from, const char *to);
// int    storage_rename(const char *from, const char *to);
int storage_set_time(const char* path, const struct timespec ts[2]) {
	int inum = tree_lookup(path);
	if (inum == -1) {
		printf("Can't set time, can't find file.\n");
		return inum;
	}

	inode* node = get_inode(inum);

	node->timestamp = ts->tv_sec;
	return 0;
}


slist* storage_list(const char* path) {
	return directory_list(path);
}

int storage_access(const char* path) {
	return tree_lookup(path);
}