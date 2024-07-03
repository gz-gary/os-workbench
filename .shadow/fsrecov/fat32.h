#include <stdint.h>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;

struct fat32hdr {
    u8  BS_jmpBoot[3];
    u8  BS_OEMName[8];
    u16 BPB_BytsPerSec;
    u8  BPB_SecPerClus;
    u16 BPB_RsvdSecCnt;
    u8  BPB_NumFATs;
    u16 BPB_RootEntCnt;
    u16 BPB_TotSec16;
    u8  BPB_Media;
    u16 BPB_FATSz16;
    u16 BPB_SecPerTrk;
    u16 BPB_NumHeads;
    u32 BPB_HiddSec;
    u32 BPB_TotSec32;
    u32 BPB_FATSz32;
    u16 BPB_ExtFlags;
    u16 BPB_FSVer;
    u32 BPB_RootClus;
    u16 BPB_FSInfo;
    u16 BPB_BkBootSec;
    u8  BPB_Reserved[12];
    u8  BS_DrvNum;
    u8  BS_Reserved1;
    u8  BS_BootSig;
    u32 BS_VolID;
    u8  BS_VolLab[11];
    u8  BS_FilSysType[8];
    u8  __padding_1[420];
    u16 Signature_word;
} __attribute__((packed));

struct fat32dent {
    u8  DIR_Name[11];
    u8  DIR_Attr;
    u8  DIR_NTRes;
    u8  DIR_CrtTimeTenth;
    u16 DIR_CrtTime;
    u16 DIR_CrtDate;
    u16 DIR_LastAccDate;
    u16 DIR_FstClusHI;
    u16 DIR_WrtTime;
    u16 DIR_WrtDate;
    u16 DIR_FstClusLO;
    u32 DIR_FileSize;
} __attribute__((packed));

struct fat32ldent {
    u8  LDIR_Ord;
    u16 LDIR_Name1[5];
    u8  LDIR_Attr;
    u8  LDIR_Type;
    u8  LDIR_Chksum;
    u16 LDIR_Name2[6];
    u16 LDIR_FstClusLO;
    u16 LDIR_Name3[2];
} __attribute__((packed));

typedef enum {
    CLUS_DENT = 0,
    CLUS_BMPHDR,
    CLUS_BMPDATA,
    CLUS_OTHERS
} clus_type_t;
/*char idstr[4][20] = {
    "CLUS_DENT",
    "CLUS_BMPHDR",
    "CLUS_BMPDATA",
    "CLUS_OTHERS"
};*/

#define CLUS_INVALID   0xffffff7

#define ATTR_READ_ONLY 0x01
#define ATTR_HIDDEN    0x02
#define ATTR_SYSTEM    0x04
#define ATTR_VOLUME_ID 0x08
#define ATTR_DIRECTORY 0x10
#define ATTR_ARCHIVE   0x20

#define ATTR_LONG_NAME (ATTR_READ_ONLY | ATTR_HIDDEN | ATTR_SYSTEM | ATTR_VOLUME_ID)
#define ATTR_LONG_NAME_MASK (ATTR_READ_ONLY | ATTR_HIDDEN | ATTR_SYSTEM | ATTR_VOLUME_ID | ATTR_DIRECTORY | ATTR_ARCHIVE)

#define MAX_CLUS 20000
#define LAST_LONG_ENTRY 0x40

struct bmp_pixel {
    u8 b, g, r;
} __attribute__((packed));

struct bmp_hdr_t {
    u16   BMP_Magic;
    u32   BMP_FileSz;
    u16   BMP_Rsvd1;
    u16   BMP_Rsvd2;
    u32   BMP_DataOff;
    u32   BMP_DIBHdrSz;
    u32   BMP_Width;
    u32   BMP_Height;
    u16   BMP_Planes;
    u16   BMP_BitsPerPixel;
    u32   BMP_Compress;
    u32   BMP_ImgSz;
    u32   BMP_XResol;
    u32   BMP_YResol;
    u32   BMP_Cols;
    u32   BMP_ImportantCols;
    struct bmp_pixel pixels[0];
} __attribute__((packed));
