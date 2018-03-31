#include "inode.h"
#include "pages.h"
#include "bitmap.h"
#include <stdio.h>

// typedef struct inode {
//     int refs; // reference count
//     int mode; // permission & type
//     int size; // bytes
//     int ptrs[2]; // direct pointers
//     int iptr; // single indirect pointer
// } inode;

// void print_inode(inode* node);
inode* 
get_inode(int inum) {
	void* inode0 = get_inodes();
	return (inode*)(inode0 + (inum*sizeof(inode)));
}

int alloc_inode() {
	void* ibm = get_inode_bitmap();
	int inode_max = get_inode_max();
	for (int i = 2; i < inode_max; i++) {
		if (bitmap_get(ibm, i) == 1) {
			continue;
		}

		printf("First free inode found at: %d\n", i);
		return 0;
	}
	return 1;
}
// void free_inode();
// int grow_inode(inode* node, int size);
// int shrink_inode(inode* node, int size);
// int inode_get_pnum(inode* node, int fpn);