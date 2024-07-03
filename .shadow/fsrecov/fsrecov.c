#include <stdio.h>
#include <assert.h>
#include <sys/mman.h>
#include "fat32.h"

struct fat32dent_long {
	u8 LDIR_Ord;
	u16 LDIR_Name1[5];
	u8 LDIR_Attr;
	u8 LDIR_Type;
	u8 LDIR_Chksum;
	u16 LDIR_Name2[6];
	u16 LDIR_FstClusLO;
	u16 LDIR_Name3[2];
}__attribute__((packed));

int main(int argc, char *argv[]) {
	assert(argc == 2);
	assert(sizeof(struct fat32hdr) == 512);
	assert(sizeof(struct fat32dent) == 32);
	assert(sizeof(struct fat32dent_long) == 32);

	setbuf(stdout, NULL);

	return 0;
}
