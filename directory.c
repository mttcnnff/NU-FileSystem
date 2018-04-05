#include "directory.h"
#include "pages.h"
#include "slist.h"
#include "util.h"
#include <string.h>
#include <stdio.h>
/*
typedef struct dirent {
    char name[DIR_NAME];
    int  inum;
    int  used;
    char _reserved[12];
} dirent;
*/

const int MAX_ENTRIES = 100;

void 
directory_init(inode* dd, int inum, int parent_inum, char* name) {
	dd->ptrs[0] = alloc_page();
	directory_put(dd, ".", inum);
	directory_put(dd, "..", parent_inum);
}

dirent* 
directory_get_dirent(dirent* first, int entnum) {
	return (dirent*)first + (entnum*sizeof(dirent));
}

dirent* 
alloc_dirent(dirent* first) {
	//printf("Size of dirent: %ld\n", sizeof(dirent));
	dirent* curr = first;
	for (int i = 0; i < MAX_ENTRIES; i++) {
		if (curr->used == 1) {
			//printf("dirent %d used: %s\n", i, curr->name);
			curr = (void*)curr + sizeof(dirent);
			continue;
		}

		// printf("Free dirent found at: %d\n", i);
		// printf("@ address: %p\n", (void*)curr);
		return curr;

	}
	return NULL;
}

void
free_dirent(dirent* entry) {
	entry->used = 0;
	entry->inum = -1;
	memset(entry->name, '\0', sizeof(entry->name));
}

int
directory_put(inode* dd, const char* name, int inum) {
	int pnum = dd->ptrs[0];
	//printf("Putting into directory at page %d\n", pnum);
	void* page = pages_get_page(pnum);
	//printf("Page address: %p\n", page);

	dirent* first = (dirent*)page;
	//printf("First dirent: %s\n", first->name);
	//printf("@ address: %p\n", (void*)first);
	dirent* open = alloc_dirent(first);

	if (open == NULL) {
		printf("ERROR: No free dirents found!\n");
		return -1;
	}

	open->used = 1;
	open->inum = inum;
	memset(open->name, '\0', strlen(name + 1));
	strncpy(open->name, name, strlen(name));

	return 0;
}

int 
directory_delete(inode* dd, const char* name) {
	int pnum = dd->ptrs[0];
	void* page = pages_get_page(pnum);

	dirent* curr = (dirent*)page;
	for (int i = 0; i < MAX_ENTRIES; i++) {
		if(streq(curr->name, name)) {
			free_dirent(curr);
			return 0;
		}
		curr = (void*)curr + sizeof(dirent);
	}
	return -1;
}

int 
directory_lookup(inode* dd, const char* name) {
	int pnum = dd->ptrs[0];
	void* page = pages_get_page(pnum);

	dirent* curr = (dirent*)page;
	for (int i = 0; i < MAX_ENTRIES; i++) {
		
		if (strcmp(curr->name, name) == 0) {
			//printf("Found \"%s\" at entry %d\n", name, i);
			return curr->inum;
		}
		curr = (void*)curr + sizeof(dirent);
	}
	return -1;

}

int 
tree_lookup(const char* path) {
	printf("Tree lookup: %s\n", path);
	int rootinum = get_root_inode();
	inode* rootinode = get_inode(rootinum);

	if(strcmp(path, "/") == 0) {
		return rootinum;
	}

	slist* splitpath = s_split(path, '/');
	slist* curr = splitpath->next;

	int inum = -1;
	inode* currinode = rootinode;
	while (curr != NULL) {
		//printf("Looking at: %s\n", curr->data);

		inum = directory_lookup(currinode, curr->data);
		if (inum == -1) {
			break;
		}

		currinode = get_inode(inum);
		curr = curr->next;
	}

	if (curr == NULL) {
		//printf("Found %s\n", path);
		return inum;
	} else {
		printf("Nothing found at: %s\n", path);
		return -1;
	}
}


int
get_parent_directory(const char* path) {
	printf("getting_parent_dir: %s\n", path);
	char parenpath[200];
	get_parent_path(parenpath, path);
	printf("parent path found: %s\n", parenpath);

	return tree_lookup(parenpath);
}


slist* 
directory_list(const char* path) {
	int inum = tree_lookup(path);
	if (inum == -1) {
		printf("Can't ls %s bc the dir doesn't exist.\n", path);
		return NULL;
	}

	inode* dir = get_inode(inum);
	int pnum = dir->ptrs[0];
	void* page = pages_get_page(pnum);

	slist* contents = NULL;
	dirent* curr = (dirent*)page;
	for (int i = 0; i < MAX_ENTRIES; i++) {
		if (curr->used == 1) {
			//printf("Pushing %s\n", curr->name);
			contents = s_cons(curr->name, contents);
			//printf("Found \"%s\" at entry %d\n", name, i);
		}
		curr = (void*)curr + sizeof(dirent);
	}

	return contents;

}

void
print_directory(inode* dd) {
	int pnum = dd->ptrs[0];
	void* page = pages_get_page(pnum);

	printf("Printind Directory: \n");
	dirent* curr = (dirent*)page;
	for (int i = 0; i < MAX_ENTRIES; i++) {
		if (curr->used == 0) {
			continue;
		}
		printf("%s -> %d\n", curr->name, curr->inum);
		curr = (void*)curr + sizeof(dirent);
	}
}