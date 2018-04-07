// based on cs3650 starter code

#define _GNU_SOURCE
#include <string.h>

#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <assert.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <stdint.h>

#include "pages.h"
#include "util.h"
#include "bitmap.h"
#include "inode.h"
#include "directory.h"

const int PAGE_COUNT = 256;
const int NUFS_SIZE  = 4096 * 256; // 1MB

static int   pages_fd   = -1;
static void* pages_base =  0;
static int   rootinum = 3;

void
pages_init(const char* path)
{
    pages_fd = open(path, O_CREAT | O_RDWR, 0644);
    assert(pages_fd != -1);

    int rv = ftruncate(pages_fd, NUFS_SIZE);
    assert(rv == 0);

    pages_base = mmap(0, NUFS_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, pages_fd, 0);
    assert(pages_base != MAP_FAILED);

    printf("inode size: %ld\n", sizeof(inode));

    void* pbm = get_pages_bitmap();
    void* ibm = get_inode_bitmap();
    if (bitmap_get(pbm, 0) == 0) {
        printf("This is a fresh mount.\n");

        for (int i = 0; i < rootinum; i++) {
            bitmap_put(pbm, i, 1);
            bitmap_put(ibm, i, 1);
        }

        // root
        bitmap_put(ibm, rootinum, 1);
        inode* root = get_inode(rootinum);
        root->ptrs[0] = -1;
        root->ptrs[1] = -1;
        root->iptr = -1;
        root->refs = 1;
        root->mode = 040755;
        root->size = 4096;
        u_acc_time(root);
        u_mod_time(root);
        u_ch_time(root);

        //directory* newdir = (directory*)page;
        directory_init(root, rootinum, rootinum, "/");
    } 
    else {
        printf("This is a remount.\n");
    }
    
    //printf("Made Directory: %s, in inode: %d\n", newdir->dirname, newdir->inum);
    //directory_put(root, ".", 2);
}

void
pages_free()
{
    int rv = munmap(pages_base, NUFS_SIZE);
    assert(rv == 0);
}

void*
pages_get_page(int pnum)
{
    return pages_base + 4096 * pnum;
}

void*
get_pages_bitmap()
{
    return pages_get_page(0);
}

void*
get_inode_bitmap()
{
    uint8_t* page = pages_get_page(0);
    return (void*)(page + 32);
}

void*
get_inodes()
{
    uint8_t* page = pages_get_page(0);
    return (void*)(page + 64);
}

int
get_root_inode() {
    //printf("Getting root inode %d!\n", rootinum);
    return rootinum;
}

int
get_inode_max() {
    return PAGE_COUNT;
}

int
alloc_page()
{
    void* pbm = get_pages_bitmap();

    // printf("BEFORE PAGE ALLOC:\n");
    // bitmap_print(pbm, 32);

    for (int ii = 2; ii < PAGE_COUNT; ii++) {
        if (bitmap_get(pbm, ii) == 0) {
            bitmap_put(pbm, ii, 1);
            printf("+ alloc_page() -> %d\n", ii);
            return ii;
        }
    }

    return -1;
}

void
free_page(int pnum)
{
    if (pnum == -1) {
        return;
    }
    printf("+ free_page(%d)\n", pnum);
    void* pbm = get_pages_bitmap();
    bitmap_put(pbm, pnum, 0);
}

