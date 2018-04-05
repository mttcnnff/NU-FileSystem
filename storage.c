#include "storage.h"
#include "pages.h"
#include "inode.h"
#include "directory.h"
#include "util.h"
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <bsd/string.h>

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
	printf("Making inode for %s.\n", path);
	int inum = alloc_inode();
	if (inum == -1) {
		printf("ERROR: No free inode found!\n");
		return -1;
	}


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
	print_directory(parent_inode);

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

int
storage_unlink(const char* path) {
	if (streq(path, "/")) {
		printf("Can't remove root dir.\n");
		return -1;
	}

	int inum = tree_lookup(path);
	int rv = free_inode(inum);
	if (rv == -1) {
		return -1;
	}

	int parent_inum = get_parent_directory(path);
	inode* parent_inode = get_inode(parent_inum);

	char filename[200];
	get_filename(filename, path);

	return directory_delete(parent_inode, filename);

}

int    
storage_write(const char* path, const char* buf, size_t size, off_t offset) {
	//printf("WRITING: %s\n", buf);


	int inum = tree_lookup(path);
	if (inum == -1) {
		printf("ERROR:STORAGE-WRITE: %s d.n.e.\n", path);
		return -1;
	}

	inode* node = get_inode(inum);
	int pages[20] = {0};
	int pages_count;
	if (offset + size > node->size) {
		grow_inode(node, offset + size);
	}
	get_pages(node, pages, &pages_count);

	printf("Writing to pages: \n");
	for (int i = 0; i < pages_count; i++) {
		printf("%d\n", pages[i]);
	}

	int pageindex = offset / 4096;
	int pageoffset = offset % 4096;
	
	int bytes_written = 0;
	for (int i = pageindex; i < pages_count; i++) {
		int pnum = pages[i];
		printf("Writing to page %d:%d\n", i, pnum);
		char* p = pages_get_page(pnum);
		for (int i = 0; i < 4096; i++) {
			if (p[i] != 0) {

				printf("%s\n", p + (i*sizeof(char)));
			} 
		}


		void* writepoint = pages_get_page(pnum) + pageoffset;
		int blockspace = 4096 - pageoffset;
		printf("blockspace: %d\n", blockspace);

		if (size > blockspace) {
			printf("Size left was bigger than blockspace\n");
			memcpy(writepoint, buf + bytes_written, blockspace);
			size = size - blockspace;
			bytes_written = bytes_written + blockspace;
		} else {
			printf("Size left was < blockspace\n");

			//we're able to write all the bytes written
			memcpy(writepoint, buf + bytes_written, size);
			//printf("WROTE: %s\n", (char*)writepoint);
			bytes_written = bytes_written + size;
			break;
		}
		pageoffset = 0;
	}

	return bytes_written;
}

int
storage_read(const char* path, char* buf, size_t size, off_t offset) {
	printf("Buf: %s\n", buf);


	int inum = tree_lookup(path);
	if (inum == -1) {
		printf("ERROR:STORAGE-READ: %s d.n.e.\n", path);
		return -1;
	}

	inode* node = get_inode(inum);
	int pages[20] = {0};
	int pages_count;
	if (offset > node->size) {
		printf("Offset at past eof!");
		return 0;
	}
	get_pages(node, pages, &pages_count);

	int pageindex = offset / 4096;
	int pageoffset = offset % 4096;
	
	int bytes_read = 0;
	for (int i = pageindex; i < pages_count; i++) {
		int pnum = pages[i];
		printf("Reading page %d:%d\n", i, pnum);
		void* readpoint = pages_get_page(pnum) + pageoffset;
		char* readpointstring = ((char*) readpoint);
		int blockspace = 4096 - pageoffset;
		printf("blockspace: %d\n", blockspace);

		if (size > blockspace) {
			printf("Size left was bigger than blockspace\n");
			memcpy(buf + bytes_read, readpointstring, blockspace);
			size = size - blockspace;
			bytes_read = bytes_read + blockspace;
		} else {
			printf("Size left was <= 0\n");
			//we're able to write all the bytes written
			memcpy(buf + bytes_read, readpointstring, size);
			bytes_read = bytes_read + size;
			break;
		}
		pageoffset = 0;
	}

	return bytes_read;
}

int    
storage_rename(const char *from, const char *to) {
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

int    
storage_link(const char *from, const char *to) {
	int rv;

	int inumfrom = tree_lookup(from);
	if (inumfrom == -1) {
        printf("ERROR-link-from: %s d.n.e.\n", from);
        return -ENOENT;
    }
    inode* nodefrom = get_inode(inumfrom);

    if (is_file(nodefrom) != 1) {
    	printf("ERROR-link-from: %s is not a file.\n", from);
    	return -EPERM;
    }

    int inumto = tree_lookup(to);
    inode* nodeto;
    char newentry[200];
    if (inumto == -1) {
    	char parentpath[200];
    	get_parent_path(parentpath, to);
    	inumto = tree_lookup(parentpath);
    	if (inumto == -1) {
    		printf("ERROR-link-to: %s d.n.e.\n", parentpath);
    		return -ENONET;
    	}
    	nodeto = get_inode(inumto);
    	get_filename(newentry, to);
    } else {
    	nodeto = get_inode(inumto);
    	if (is_file(nodeto)) {
    		printf("ERROR-link-to: %s is an already existing file.", to);
    		return -EEXIST;
    	}
    	get_filename(newentry, from);
    }

    //nodeto is directory to place entry

    nodefrom->refs++;
    directory_put(nodeto, newentry, inumfrom);
    return 0;


    // nodeto is defined




}
