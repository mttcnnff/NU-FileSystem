// based on cs3650 starter code

#ifndef DIRECTORY_H
#define DIRECTORY_H

#define DIR_NAME 48

#include "slist.h"
#include "pages.h"
#include "inode.h"

typedef struct dirent {
    char name[DIR_NAME];
    int  inum;
    int  used;
    char _reserved[12];
} dirent;

typedef struct directory {
	char dirname[DIR_NAME];
	int inum;
	dirent* entries;
} directory;

//void directory_init();
void directory_init(inode* dd, int inum, int parent_inum, char* name);
int directory_lookup(inode* dd, const char* name);
int tree_lookup(const char* path);
int get_parent_directory(const char* path);
int directory_put(inode* dd, const char* name, int inum);
dirent* directory_get_dirent(dirent* first, int entnum);
dirent* alloc_dirent(dirent* first);
void free_dirent(dirent* entry);
int directory_delete(inode* dd, const char* name);
slist* directory_list(const char* path);
void print_directory(inode* dd);

#endif

