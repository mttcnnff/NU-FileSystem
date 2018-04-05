#include "inode.h"
#include "pages.h"
#include "bitmap.h"
#include "util.h"
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
	printf("ptr0: %d\n", node->ptrs[0]);
	printf("ptr1: %d\n", node->ptrs[1]);
	if (node->iptr == -1) {
		printf("iptr: %d\n", node->iptr);
	} else {
		int iptr_count = bytes_to_pages(node->size) - 2;
		int* indirects = pages_get_page(node->iptr);
		printf("iptr: %d\n", node->iptr);
		for (int i = 0; i < iptr_count; i++) {
			printf("iptr%d: %d\n", i, indirects[i]);
		}
	}
}


inode* 
get_inode(int inum) {
	void* inode0 = get_inodes();
	return inode0 + (inum * sizeof(inode));
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


// PAGE SIZE: 4096
int 
grow_inode(inode* node, int size) {
	//find end, grow from there, edit original size
	int new_page_count = bytes_to_pages(size);
	int curr_page_count = bytes_to_pages(node->size);
	int np;
	if (new_page_count > 0) {
		if (curr_page_count < 1) {
			np = alloc_page();
			if (np == -1) {
				return -1;
			}
			node->ptrs[0] = np;
			curr_page_count++;
		}
	}

	if (new_page_count > 1) {
		if (curr_page_count < 2) {
			np = alloc_page();
			if (np == -1) {
				return -1;
			}
			node->ptrs[1] = np;
			curr_page_count++;
		}
	} 

	if (new_page_count > 2) {
		if (curr_page_count < 3) {
			np = alloc_page();
			if (np == -1) {
				return -1;
			}
			node->iptr = np;
		}

		int* indirects = pages_get_page(node->iptr);
		for (int i = curr_page_count - 2; i < new_page_count - 2; i++) {
			np = alloc_page();
			if (np == -1) {
				return -1;
			}
			indirects[i] = np;
		}
	}
	node->size = size;
	printf("Successfully Grew Inode!\n");
	print_inode(node);
	return 0;

}
// int shrink_inode(inode* node, int size);
// int inode_get_pnum(inode* node, int fpn);

void 
get_pages(inode* node, int pages[], int* pages_count) {
	*pages_count = bytes_to_pages(node->size);
	int* indirects;
	if (*pages_count > 2) {
		indirects = pages_get_page(node->iptr);
	}
	
	for (int i = 0; i < *pages_count; i++) {
		if (i < 2) {
			pages[i] = node->ptrs[i];
			continue;
		}
		pages[i] = indirects[i-2];
	}
}