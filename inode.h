// based on cs3650 starter code

#ifndef INODE_H
#define INODE_H

#include <time.h>

typedef struct inode {
    int refs; // reference count
    int mode; // permission & type
    int size; // bytes
    int ptrs[2]; // direct pointers to pages
    int iptr; // single indirect
    time_t timestamp; //last edited?
} inode;

void print_inode(inode* node);
inode* get_inode(int inum);
int alloc_inode();
int free_inode(int inum);
int grow_inode(inode* node, int size);
int shrink_inode(inode* node, int size);
int inode_get_pnum(inode* node, int fpn);
int is_file(inode* node);
int is_dir(inode* node);

#endif
