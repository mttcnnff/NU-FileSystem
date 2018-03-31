#include "bitmap.h"
#include <stdint.h>
#include <stdio.h>


static void 
toBinary(uint8_t a) {
	uint8_t i;

	for(i=0x80;i!=0;i>>=1) {
		printf("%c", (a&i)?'1':'0');
	}
	printf("\n");
}

static int
getBit(uint8_t a, int i) {
	return ((a >> i) & 0x01);
}

static void 
setBit(uint8_t* a, int i, int v) {
	if (v == 0) {
		*a &= ~(0x01 << 7 - i);
	} else {
		*a |= 0x01 << 7 - i;
	}
}

int 
bitmap_get(void* bm, int ii) {
	int byte = ii / 8;
	int bit = ii % 8;
	int offset = byte * sizeof(uint8_t);
	int b = getBit(*(uint8_t*)bm + offset, 7 - bit);
	return b;
}

void 
bitmap_put(void* bm, int ii, int vv) {
	uint8_t* tests = (uint8_t*)bm;
	int byte = ii / 8;
	int bit = ii % 8;
	setBit(&tests[byte], bit, vv);

}

// void bitmap_print(void* bm, int size) {

// }