#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "fat32.h"
#define MAX_CLUS 20000

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
u32 bytes_per_clus;
u8 before_data_sec;
u8 *clus_begin;
u8 *clus_end;

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

    return hdr;

release:
    perror("map disk");
    if (fd > 0) {
        close(fd);
    }
    exit(1);
}

typedef enum {
	CLUS_DENT = 0,
	CLUS_BMPHDR,
	CLUS_BMPDATA,
	CLUS_OTHERS
} clus_type_t;
char idstr[4][20] = {
	"CLUS_DENT",
	"CLUS_BMPHDR",
	"CLUS_BMPDATA",
	"CLUS_OTHERS"
};

clus_type_t clus_type[MAX_CLUS];

clus_type_t probe_clus_type(u8 *clus) {
	if (clus[0] == 'B' && clus[1] == 'M') return CLUS_BMPHDR;
	int cnt_bmp = 0, cnt_zero = 0;

	for (int i = 0; i < bytes_per_clus - 2; ++i) {
		if ((clus[i] == 'B' && clus[i + 1] == 'M' && clus[i + 2] == 'P') ||
			(clus[i] == 'b' && clus[i + 1] == 'm' && clus[i + 2] == 'p')) {
			++cnt_bmp;
		}
	}

	for (int i = 0; i < bytes_per_clus; ++i) if (clus[i] == 0) ++cnt_zero;

	if (cnt_bmp >= 3) return CLUS_DENT;
	if (cnt_zero == bytes_per_clus) return CLUS_OTHERS;
	return CLUS_BMPDATA;
}

void dump_bmp() {
	bytes_per_clus   = hdr->BPB_SecPerClus * hdr->BPB_BytsPerSec;
	before_data_sec  = hdr->BPB_RsvdSecCnt + ((hdr->BPB_NumFATs) * (hdr->BPB_FATSz32));
	clus_begin       = (u8 *)hdr + before_data_sec * hdr->BPB_BytsPerSec;
	clus_end         = (u8 *)hdr + hdr->BPB_TotSec32 * hdr->BPB_BytsPerSec;
	printf("%u\n", before_data_sec);
	printf("%u\n", hdr->BPB_RsvdSecCnt);
	printf("%u\n", hdr->BPB_NumFATs);
	printf("%u\n", hdr->BPB_FATSz32);
	printf("%d\n", (int)((void *)clus_begin - (void *)hdr));
	u32 clus_id = 2;
	for (u8 *clus = clus_begin; clus < clus_end; clus += bytes_per_clus) {
		clus_type[clus_id] = probe_clus_type(clus);
		// printf("%s ", idstr[clus_type[clus_id]]);
		++clus_id;
	}
	// printf("%u\n", hdr->BPB_TotSec32 / hdr->BPB_SecPerClus);
}
