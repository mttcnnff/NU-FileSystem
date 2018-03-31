// #include "bitmap.h"
// #include "inode.h"
// #include <stdint.h>
// #include <stdio.h>
// #include <stdlib.h>


// int
// main(int argc, char *argv[])
// { 
// 	// 80 bits
//     uint8_t* tests = malloc(sizeof(uint8_t) * 10);

//     for (int i = 0; i < 10; i++) {
//     	tests[i] = i;
//     }
//     for (int i = 0; i < 10; i++) {
//     	toBinary(tests[i]);
//     }

//     printf("Getting Bit 0 -> %d\n", bitmap_get((void*)tests, 0));
//     printf("Getting Bit 15 -> %d\n", bitmap_get((void*)tests, 15));
//     printf("Getting Bit 22 -> %d\n", bitmap_get((void*)tests, 22));
//     printf("Getting Bit 23 -> %d\n", bitmap_get((void*)tests, 23));

//     bitmap_put((void*)tests, 0, 1);
//     bitmap_put((void*)tests, 4, 1);
//     bitmap_put((void*)tests, 8, 1);

//     for (int i = 0; i < 10; i++) {
//     	toBinary(tests[i]);
//     }

//     free(tests);





//     return 0;
// }