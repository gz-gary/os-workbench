#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "fat32.h"
#define MAX_CLUS 20000


struct fat32hdr *hdr;
u32 bytes_per_clus;
u32 before_data_sec;
u8 *clus_begin;
u8 *clus_end;
u32 dents_per_clus;

void* mmap_disk(const char *filename);
void dump_bmp();

int main(int argc, char *argv[]) {
	assert(argc == 2);
	assert(sizeof(struct fat32hdr) == 512);
	assert(sizeof(struct fat32dent) == 32);
	assert(sizeof(struct fat32ldent) == 32);
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

int ascii_printable(const char *str) {
	int len = strlen(str);
	for (int i = 0; i < len; ++i) {
		if (str[i] != '\0' && (str[i] < ' ' || str[i] > '~')) return 0;
	}
	return 1;
}

void dump_long_file_name(struct fat32ldent *ldent, char *buf, int cnt_ldent) {
	int len = 0;
	for (int j = cnt_ldent - 1; j >= 0; --j) {
		for (int k = 1; k <= 5; ++k)
			if (ldent->LDIR_Name1[k - 1] != '\0')
				buf[len++] = ldent->LDIR_Name1[k - 1];

		for (int k = 6; k <= 11; ++k)
			if (ldent->LDIR_Name2[k - 6] != '\0')
				buf[len++] = ldent->LDIR_Name2[k - 6];

		for (int k = 12; k <= 13; ++k)
			if (ldent->LDIR_Name3[k - 12] != '\0')
				buf[len++] = ldent->LDIR_Name3[k - 12];
	}
	buf[len++] = '\0';
}

void dump_bmp() {
	bytes_per_clus   = hdr->BPB_SecPerClus * hdr->BPB_BytsPerSec;
	before_data_sec  = hdr->BPB_RsvdSecCnt + ((hdr->BPB_NumFATs) * (hdr->BPB_FATSz32));
	clus_begin       = (u8 *)hdr + before_data_sec * hdr->BPB_BytsPerSec;
	clus_end         = (u8 *)hdr + hdr->BPB_TotSec32 * hdr->BPB_BytsPerSec;
	dents_per_clus   = bytes_per_clus / sizeof(struct fat32dent);

	u32 clus_id = 0;
	for (u8 *clus = clus_begin; clus < clus_end; clus += bytes_per_clus) {
		clus_type[clus_id] = probe_clus_type(clus);
		// printf("%s ", idstr[clus_type[clus_id]]);
		++clus_id;
	}
	clus_id = 0;
	for (u8 *clus = clus_begin; clus < clus_end; clus += bytes_per_clus) {
		if (clus_type[clus_id] != CLUS_DENT) { ++clus_id; continue; }
		struct fat32dent *dent = (struct fat32dent *)clus;
		for (int i = 0; i < dents_per_clus; ++i) {
			if ((dent[i].DIR_Attr & ATTR_LONG_NAME) == ATTR_LONG_NAME) { // this entry is a 'long name directory entry' 
				struct fat32ldent *ldent = (struct fat32ldent *)&dent[i];
				if ((ldent->LDIR_Ord & LAST_LONG_ENTRY) == 0) continue;
				int cnt_ldent = ldent->LDIR_Ord ^ LAST_LONG_ENTRY;
				if ((void *)(dent + i + cnt_ldent) > (void *)clus_end) continue; // cross cluster
				printf("%d\n", cnt_ldent);
				char buf[256];
				dump_long_file_name(ldent, buf, cnt_ldent);
				printf("[Long file name]: %s\n", buf);
				i += cnt_ldent;
			} else { // this entry is a 'short name directory entry'
				if (dent[i].DIR_Name[0] == 0x00 ||
					dent[i].DIR_Name[0] == 0xE5 ||
					dent[i].DIR_Attr & ATTR_HIDDEN) {
					continue;
				}
				int len = 0;
				char buf[64];
				for (int j = 0; j < 11; ++j) {
					if (j == 8) buf[len++] = '.';
					buf[len++] = dent[i].DIR_Name[j];
				}
				buf[len++] = '\0';
				if (ascii_printable(buf)) {
					printf("%s\n", buf);
				}
			}
		}
		// printf("%s ", idstr[clus_type[clus_id]]);
		++clus_id;
	}
}
