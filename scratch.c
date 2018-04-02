
// #include "bitmap.h"
// #include <stdio.h>
// #include <stdlib.h>

// int alloc_page(void* bm);


// int
// main(int argc, char *argv[])
// { 
// 	//uint8_t* bm = malloc(sizeof(uint8_t)*32);
// 	uint8_t bm[32] = {0};

// 	bitmap_print((void*)bm, 32);
// 	alloc_page((void*)bm);
// 	alloc_page((void*)bm);
// 	alloc_page((void*)bm);
// 	alloc_page((void*)bm);
// 	alloc_page((void*)bm);
// 	alloc_page((void*)bm);
// 	alloc_page((void*)bm);
// 	alloc_page((void*)bm);


//     return 0;
// }

// int
// alloc_page(void* bm)
// {
//     printf("BEFORE PAGE ALLOC:\n");
//     bitmap_print(bm, 32);

//     for (int ii = 2; ii < 256; ii++) {
//         if (bitmap_get(bm, ii) == 0) {
//             bitmap_put(bm, ii, 1);
//             printf("+ alloc_page() -> %d\n", ii);
//             printf("AFTER PAGE ALLOC:\n");
//     		bitmap_print(bm, 32);
//             return ii;
//         }
//     }

//     return -1;
// }