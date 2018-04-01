#include "inode.h"
#include "pages.h"
#include "bitmap.h"
#include <stdio.h>
#include <sys/stat.h>

// typedef struct inode {
//     int refs; // reference count
//     int mode; // permission & type
//     int size; // bytes
//     int ptrs[2]; // direct pointers
//     int iptr; // single indirect pointer
// } inode;

void 
print_inode(inode* node) {
	printf("==== inode ====\n");
	printf("Refs: %d\n", node->refs);
	printf("Mode: %d\n", node->mode);
	printf("Size: %d\n", node->size);
}


inode* 
get_inode(int inum) {
	void* inode0 = get_inodes();
	return (inode*)(inode0 + (inum*sizeof(inode)));
}

int
alloc_inode() {
	void* ibm = get_inode_bitmap();
	int inode_max = get_inode_max();
	for (int i = 2; i < inode_max; i++) {
		if (bitmap_get(ibm, i) == 1) {
			continue;
		}

		printf("First free inode found at: %d\n", i);
		bitmap_put(ibm, i, 1);
		inode* node = get_inode(i);
		node->ptrs[0] = -1;
		node->ptrs[1] = -1;
		node->iptr = -1;
		return i;
	}
	return -1;
}


int 
free_inode(int inum) {
	if (inum < 2) {
		return -1;
	}
	void* ibm = get_inode_bitmap();
	int inode_max = get_inode_max();

	inode* node = get_inode(inum);
	if (node->refs > 1) {
		node->refs = node->refs - 1;
		return 0;
	}
	node->refs = 0;
	node->mode = 0;
	node->size = 0;
	free_page(node->ptrs[0]);
	free_page(node->ptrs[1]);
	return 0;
}

int 
is_file(inode* node) {
	return ((node->mode & S_IFMT) == S_IFREG);
}

int 
is_dir(inode* node) {
	return ((node->mode & S_IFMT) == S_IFDIR);
}
// int grow_inode(inode* node, int size);
// int shrink_inode(inode* node, int size);
// int inode_get_pnum(inode* node, int fpn);