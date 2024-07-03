#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "fat32.h"

#define ATTR_LONG_NAME (ATTR_READ_ONLY | ATTR_HIDDEN | ATTR_SYSTEM | ATTR_VOLUME_ID)

struct fat32dent_long {
	u8 LDIR_Ord;
	u16 LDIR_Name1[5];
	u8 LDIR_Attr;
	u8 LDIR_Type;
	u8 LDIR_Chksum;
	u16 LDIR_Name2[6];
	u16 LDIR_FstClusLO;
	u16 LDIR_Name3[2];
} __attribute__((packed));

struct fat32hdr *hdr;

void* mmap_disk(const char *filename);
void dump_bmp();

int main(int argc, char *argv[]) {
	assert(argc == 2);
	assert(sizeof(struct fat32hdr) == 512);
	assert(sizeof(struct fat32dent) == 32);
	assert(sizeof(struct fat32dent_long) == 32);
	setbuf(stdout, NULL);

	hdr = mmap_disk(argv[1]);
	assert(hdr->BPB_BytsPerSec == 512);
	assert(hdr->BPB_SecPerClus == 8);

	dump_bmp();

	munmap(hdr, hdr->BPB_TotSec32 * hdr->BPB_BytsPerSec);
	return 0;
}

void *mmap_disk(const char *fname) {
    int fd = open(fname, O_RDWR);

    if (fd < 0) {
        goto release;
    }

    off_t size = lseek(fd, 0, SEEK_END);
    if (size < 0) {
        goto release;
    }

    struct fat32hdr *hdr = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
    if (hdr == MAP_FAILED) {
        goto release;
    }

    close(fd);

    assert(hdr->Signature_word == 0xaa55); // this is an MBR
    assert(hdr->BPB_TotSec32 * hdr->BPB_BytsPerSec == size);

    printf("%s: DOS/MBR boot sector, ", fname);
    printf("OEM-ID \"%s\", ", hdr->BS_OEMName);
    printf("sectors/cluster %d, ", hdr->BPB_SecPerClus);
    printf("sectors %d, ", hdr->BPB_TotSec32);
    printf("sectors %d, ", hdr->BPB_TotSec32);
    printf("sectors/FAT %d, ", hdr->BPB_FATSz32);
    printf("serial number 0x%x\n", hdr->BS_VolID);
    return hdr;

release:
    perror("map disk");
    if (fd > 0) {
        close(fd);
    }
    exit(1);
}

void dump_bmp() {
}
