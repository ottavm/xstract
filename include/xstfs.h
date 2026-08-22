#ifndef XSTFS_H
#define XSTFS_H

/* - About this project -
 *	*: This header-only library exposes both high-level(ish) and low-level 
 *	C structs for managing Xbox 360 Secure Transacted File Systems
 *	packages. STFS is used by, for example, a savegame file.
 *		
 *	*: Low-level structs always begins with a lowercase x <'x'>. These
 *	structs represents, with precision, how STFS files are contituted
 *	in their raw form.
 *
 *	*: Xbox 360 is big-endian. Keep this in mind. (note for myself)
 *
 *	*: High-level structs otherwise, represents STFS files in a more 
 *	general way. More like representint the actual content of a package
 *	than the package's guts.
 *
 *	?: Why?
 *		I didn't find a Linux-compatible tool for extracting my
 *		Minecraft savegame's level.dat, so I'm supposed to build
 *		it by myself, right?
 *
 *	NOTE: I'm not an Xbox 360 hacker, modding enthusiast nor engaged in any
 *	way with the X360's reverse engineering community. So, expect a lot of 
 *	mistakes with the terms!
 *
 * - Development notes:
 * TODO: Support Profile Embedded Content (PEC)
 *
 * --
 * NOTE 24-Jul-26: I'm quite concerned about the memory usage.
 * The high-level representation of STFS's embedded files (SEFiles) needs
 * a contiguous byte array to handle the contents of the underlying STFS file
 * in a nice and sane way. Since the STFS's embedded files are not represented
 * in a [header:content] layout, but rather in a 
 * [[file table]:[sparse and non-consecutive blocks]] way, requiring the
 * duplication of the file's content.
 *
 *	SEFile representation (pseudocode)
 *	-----------
 *	name: "foo_dir",
 *	type: dir,
 *	size: 0 (its a directory),
 *	content: [bytes...]
 *	-----------
 *	name: 'foo.bin',
 *	type: file,
 *	size: 10,
 *	content: [bytes...] -> 'helloworld'
 *	-----------
 *	Repeat the structure defined above.
 *	[...]
 *
 *	STFS Embedded files representation (pseudocode)
 *	----------- (File Table list defined somewhere) -----------
 *	[
 *	  name: 'foo.bin',
 *	  type: file,
 *	  block count: 10,
 *	  starting block number: 1
 *	],
 *	[
 *	  name: 'bar.bin',
 *	  type: file,
 *	  allocated block count: 20,
 *	  starting block number: 11,
 *	]
 *	---------- (Actual files content stored somewhere) ----------
 *	block1,
 *	block2,
 *	hash,
 *	block3
 *	...
 *	----------
 *
 *	As you can notice, my representation needs the file content
 *	to be a contiguous byte array. That characteristic, exposes
 *	it's flaw: the need to duplicate the already memory-mapped blocks
 *	and reorganize it in a sequence (and adjusting their size too, since
 *	the blocks are 4096 Bytes large, regardless of the it's content size).
 *
 *	The only solution that I can think of is by representing the file's 
 *	content as a linked-list of blocks, and "dynamic" adjusting the size
 *	of the last block as needed.
 *	
 *	--------- end of the 24-Jul-26 NOTE.
 */

/* --- LICENSE ---
 * BSD 3-Clause License
 * 
 * Copyright 2026 Murilo Ottávio A. Branco Reis
 * 
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 * 
 * 1. Redistributions of source code must retain the above copyright notice,
 *	  this list of conditions and the following disclaimer.
 * 
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *	  this list of conditions and the following disclaimer in the documentation
 *	  and/or other materials provided with the distribution.
 * 
 * 3. Neither the name of the copyright holder nor the names of its contributors
 *	  may be used to endorse or promote products derived from this software
 *	  without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS “AS IS”
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR 
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR 
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, 
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, 
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; 
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, 
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR 
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF 
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * */


#include <stdint.h>
#include <stddef.h>

	#if defined(__linux__)
#include <linux/limits.h>
#define XSTFS_PATH_MAX PATH_MAX
#define PATH_SEPARATOR "/"
	#elif defined(_WIN32)
#define PATHPATH_SEPARATOR "\\"
#define XSTFSXSTFS_PATH_MAX 260
	#else
#define XSTFS_PATH_MAX 260
	#endif

#if !defined(__BYTE_ORDER__) || (__BYTE_ORDER__ != __ORDER_LITTLE_ENDIAN__)
	#error "XSTFS_H does not fully supports non little-endian byte orders."
#endif

#define STFS_BLOCK_LEN 0x1000 /* 4096 */

/* Content type constants */
#define XC_SAVEGAME         0x0000001
#define XC_MARKETPLACE      0x0000002
#define XC_PUBLISHER        0x0000003
#define XC_X360_TITLE       0x0001000
#define XC_IPTV_PB          0x0002000
#define XC_INSTALLED_GAME   0x0004000
#define XC_XOG_GAME         0x0005000
#define XC_XBOX_TTLE        0x0005000
#define XC_GOD              0x0007000
#define XC_AVAT_ITEM        0x0009000
#define XC_PROFILE          0x0010000
#define XC_GAMER_PIC        0x0020000
#define XC_THEME            0x0030000
#define XC_CACHE_FILE       0x0040000
#define XC_STORAGE_DONWLOAD 0x0050000
#define XC_XBOX_SAVED_GAME  0x0060000
#define XC_XBOX_DOWNLOAD    0x0070000
#define XC_XBOX_GAME_DEMO   0x0080000
#define XC_VIDEO            0x0090000
#define XC_GAME_TITLE       0x00A0000
#define XC_INSTALLER        0x00B0000
#define XC_GAME_TRAILER     0x00C0000
#define XC_ARCADE_TITLE     0x00D0000
#define XC_XNA              0x00E0000
#define XC_LICENSE_STORE    0x00F0000
#define XC_MOVIE            0x0100000
#define XC_TV               0x0200000
#define XC_MUSIC_VIDEO      0x0300000
#define XC_GAME_VIDEO       0x0400000
#define XC_PODCAST_VIDEO    0x0500000
#define XC_VIRAL_VIDEO      0x0600000
#define XC_COMMUNITY_VIDEO  0x2000000

#define ERR_NULL_PTR        "Null pointer."
#define ERR_NOT_ERR         "Success."
#define ERR_GENERIC         "Unknown error."
#define ERR_CORRUPT_AR      "Corrupted or invalid archive."
#define ERR_INVALID_VAL     "Invalid value."
#define ERR_NOT_IMPLEMENTED "Feature requested is not implemented yet."

/* Magics */
#define MAGIC_CON  "CON "
#define MAGIC_PIRS "PIRS"
#define MAGIC_LIVE "LIVE"
#define MAGIC_LEN  4

/* Transfer bit flags */
enum {
	F_DEEP_LINK_SUPPORTED     = (1 << 2),
	F_DISABLE_NETWORK_STORAGE = (1 << 3),
	F_KINECT_ENABLED          = (1 << 4),
	F_MVONLY_TRANSFER         = (1 << 5),
	F_DEV_ID_TRANSFER         = (1 << 6),
	F_PROF_ID_TRANSFER        = (1 << 7)
};

/* Volume descriptor type. 0 = STFS; 1 = SVOD */
enum {
	STFS,
	SVOD
};

/* Package Type */
enum {
	CON = 1,
	PIRS,
	LIVE
};

enum xstfs_errors {
	XSTFS_NULL_PTR = -1,
	XSTFS_SUCCESS,
	XSTFS_GENERIC_ERR,
	XSTFS_CORRUPTED_AR,
	XSTFS_INVALID_VAL,
	XSTFS_NOT_IMPLEMENTED,
};

enum {
	FILE_REG, /* Regular file. */
	FILE_DIR  /* Directory. */
};

/* big-endian representation of a 24-bit signed integer. */
typedef struct {
	uint8_t val[3];
} __attribute__((packed)) xint24be_t;

/* big-endian representation of a 24-bit unsigned integer. */
typedef struct {
	uint8_t val[3];
} __attribute__((packed)) xuint24be_t;

/* little-endian representation of a 24-bit signed integer. */
typedef struct {
	uint8_t val[3];
} __attribute__((packed)) xint24le_t;

/* little-endian representation of a 24-bit unsigned integer. */
typedef struct {
	uint8_t val[3];
} __attribute__((packed)) xuint24le_t;

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 * Low-level representation of STFS headers. 
 * These structs will be converted to a high level representation
 * later on.
 * NOTE: Some integers with more than one byte of lenght need to be converted
 * to the host PC endianness BEFORE any bitwise operations.
 * */

/*
 * Data structure of a STFS Volume descriptor and it's fields. 
 * From Free60.org, 2026. <https://free60.org/System-Software/Formats/STFS/>
 * 
 * NAME			| SIZE (BYTES) | TYPE		| DESCRIPTION
 * vd_size		| 1			   | uint8_t	| Volume descriptor size. Usually 0x24.
 * reserved		| 1			   | uint8_t	| Reserved.
 * blk_sp		| 1			   | uint8_t	| Block separation.
 * ft_blkc		| 2			   | int16_t LE | Number of blocks allocated to the file table.
 * ft_blkn		| 3			   | int24_t BE | Starting block of the file table.
 * thash		| 0x14		   | uint8_t[]	| Top hash table hash.
 * alloc_blkc	| 4			   | int32_t BE | Number of allocated blocks.
 * unalloc_blkc | 4			   | int32_t BE | Number of unallocated blocks.
 *
 * NOTE 1: To obtain the total number of blocks in a package,
 * you must add the latter two fields (`alloc_blkc` and `unalloc_blkc`)
 * together.
 * */
struct xstfs_vd {
	uint8_t vd_size;
	uint8_t reserved;
	uint8_t blk_sp;
	int16_t ft_blkc; /* little-endian */
	xint24be_t ft_blkn;
	uint8_t thash[0x14];
	int32_t alloc_blkc;
	int32_t unalloc_blkc;
} __attribute__((packed));

/* 
 * Data structure of a SVOD Volume descriptor and it's fields.
 * From Free60.org, 2026. <https://free60.org/System-Software/Formats/STFS/>
 * 
 * NAME	       | SIZE (BYTES) | TYPE        | DESCRIPTION
 * vd_size     | 1            | uint8_t     | Volume descriptor size.
 * blkcachec   | 1            | uint8_t     | Block cache element count.
 * wkthrdproc  | 1            | uint8_t     | Worker thread processor.
 * wkthrdprio  | 1            | uint8_t     | Worker thread priority.
 * hash        | 0x14         | uint8_t     | Hash.
 * devfeatures | 1            | uint8_t     | Device features.
 * dtblkc      | 3            | uint24_t BE | Data block count.
 * dtblkoff    | 3            | uint24_t BE | Data block offset.
 * pad         | 5            | uint8_t[]   | paddind/reserved.
 * */
struct xsvod_vd {
	uint8_t vd_size;
	uint8_t blkcachec;
	uint8_t wkthrdproc;
	uint8_t wkthrdprio;
	uint8_t hash[0x14];
	uint8_t devfeatures;
	xuint24be_t dtblkc;
	xuint24be_t dtblkoff;
	uint8_t pad[0x5];
} __attribute__((packed));

/* Package signatures and it's fields. 
 * From Free60.org, 2026. <https://free60.org/System-Software/Formats/STFS/> */

/* Console Signed signature. Often found in savegames and other console-made
 * packages.
 * 
 * NAME | SIZE (BYTES)  | TYPE	    | DESCRIPTION
 * crtf_size    | 0x2   | uint8_t[] | Public Key Certificate size.
 * owner_id     | 0x5   | uint8_t[] | Certificate Owner Console ID.
 * partnumber   | 0x14  | uint8_t[] | Certificate Owner Console part number.
 * console_type | 1     | uint8_t   | Certificate Owner Console type. 
 *                                        (1 for devkit, 2 for retail.)
 * crtf_gendate | 0x8   | uint8_t[] | Certificate Date of Generation
 * pub_modulus  | 0x80  | uint8_t[] | Public modulus.
 * crtf_sig     | 0x100 | uint8_t[] | Certificate Signatures.
 * sig          | 0x80  | uint8_t[] | Signature.
 * */
struct xsig_con {
	uint8_t crtf_size[0x2];
	uint8_t owner_id[0x5];
	uint8_t partnumber[0x14];
	uint8_t console_type;
	uint8_t crtf_gendate[0x8];
	uint8_t pub_exponent[0x4];
	uint8_t pub_modulus[0x80];
	uint8_t crtf_sig[0x100];
	uint8_t sig[0x80];
} __attribute__((packed));

/* Remotely Signed signature. Often found in LIVE packages and PIRS packages.
 *		From Free60.org, 2026. <https://free60.org/System-Software/Formats/STFS/>
 *
 * NAME    | SIZE (BYTES) | TYPE      | DESCRIPTION
 * pkg_sig | 0x100        | uint8_t[] | Package Signature.
 * pad	   | 0x128        | uint8_t[] | Padding.
 * */
struct xsig_live {
	uint8_t pkg_sig[0x100];
	uint8_t pad[0x128];
}__attribute__((packed));

struct xsig_pirs {
	uint8_t pkg_sig[0x100];
	uint8_t pad[0x128];
} __attribute__((packed));

/* STFS Package header. 
 * From Free60.org, 2026. <https://free60.org/System-Software/Formats/STFS/>
 *
 * NAME	           | SIZE (BYTES)  | TYPE      | DESCRIPTION
 * magic           | 0x4           | uint8_t[] | Package's magic. Either "CON ",
 *                                                   "LIVE", or "PIRS".
 *anonymous union *| sizeof(void*) | union     | Anonymous union containing a pointer to a signature.
 * */
struct xstfs_header {
	char magic[MAGIC_LEN];
	union {
		struct xsig_con con;
		struct xsig_live live;
		struct xsig_pirs pirs;
	} __attribute__((packed));
} __attribute__((packed));

struct xlicense_data {
	int64_t l_id;
	int32_t l_bits;
	int32_t l_flags;
} __attribute__((packed));

struct xmetadata_v1 {
	uint8_t pad[0x4C];
	uint8_t device_id[0x14];
	uint8_t utf8_displayname[0x900];
	uint8_t utf8_displaydesc[0x900];
	uint8_t utf8_pubname[0x80];
	uint8_t utf8_titlename[0x80];
	uint8_t trans_flags;
	int32_t t_img_size;
	int32_t ttitle_img_size;
	uint8_t thumbnail[0x4000];
	uint8_t tthumbnail[0x4000];
} __attribute__((packed));

struct xmetadata_v2 {
	uint8_t series_id[0x10];
	uint8_t season_id[0x10];
	int16_t season_number;
	int16_t episode_number;
	uint8_t pad[0x28];
	uint8_t thumbnail_img[0x3D00];
	uint8_t add_displ_names[0x300];
	uint8_t titl_thumbnail_img[0x3D00];
	uint8_t add_displ_desc[0x300];
} __attribute__((packed));

struct xmetadata {
	struct xlicense_data l_entries[0x10];
	uint8_t hdr_sha1[0x14];
	uint32_t hdr_size;
	int32_t cnt_type;
	int32_t met_version;
	int64_t cnt_size;
	uint32_t media_id;
	int32_t version;
	int32_t base_version;
	uint32_t title_id;
	uint8_t platform;
	uint8_t exe_type;
	uint8_t disc_number;
	uint8_t disc_in_set;
	uint32_t savegame_id;
	uint8_t cnsl_id[0x5];
	uint8_t prof_id[0x8];
	union {
		struct xstfs_vd stfs;
		struct xsvod_vd svod;
	} __attribute__((packed)) vd_desc;
	int32_t dataf_count;
	int64_t dataf_csize;
	uint32_t vd_type;
	uint32_t reserved;
	union {
		struct xmetadata_v1 v1;
		struct xmetadata_v2 v2;
	} __attribute__((packed));
} __attribute__((packed));

struct xstfs_file {
	/* File name, null-padded. */
	char name[0x28];
	/* File name, plus flags
	 * The first 6 bits (0-5) are the lenght of the file name.
	 * Bit 6 and 7 are bitflags. If the 6th bit is set, it means that
	 * all of the blocks in the file are consecutive. Bit 7 indicates
	 * that the file is a directory.
	 * */
	uint8_t fn_flags;

	xint24le_t alloc_blkc;	/* little-endian */
	 /* For some reason, there is a copy of the allocated block count. */
	xint24le_t alloc_blkc2; /* little-endian */
	/* Number of the first block allocated for the file. */
	xint24le_t first_blkn;
	int16_t path_ind;	/* big-endian */
	uint32_t file_size; /* big-endian */
	int32_t udts; /* Update date/time stamp of file */
	int32_t adts; /* Access date/time stamp of file */
} __attribute__((packed));

/* High-level datatypes and structs */

struct stfs_vd {
	uint8_t vd_size;
	uint8_t reserved;
	uint8_t blk_sp;
	int16_t ft_blkc;
	int32_t ft_blkn;
	uint8_t thash[0x14];
	int32_t alloc_blkc;
	int32_t unalloc_blkc;
};

struct svod_vd {
	uint8_t vd_size;
	uint8_t blkcachec;
	uint8_t wkthrdproc;
	uint8_t wkthrdprio;
	uint8_t hash[0x14];
	uint8_t devfeatures;
	uint32_t dtblkc;
	uint32_t dtblkoff;
};

struct stfs_metadata {
	struct xlicense_data l_entries[0x10];
	uint8_t hdr_sha1[0x14];
	uint32_t hdr_size;
	int32_t cnt_type;
	int32_t met_version;
	int64_t cnt_size;
	uint32_t media_id;
	int32_t version;
	int32_t base_version;
	uint32_t title_id;
	uint8_t platform;
	uint8_t exe_type;
	uint8_t disc_number;
	uint8_t disc_in_set;
	uint32_t savegame_id;
	uint8_t cnsl_id[0x5];
	uint8_t prof_id[0x8];
	union {
		struct stfs_vd stfs;
		struct svod_vd svod;
	} vd_desc;
	int32_t dataf_count;
	int64_t dataf_csize;
	uint32_t vd_type;
	uint32_t reserved;
	union {
		struct xmetadata_v1 v1;
		struct xmetadata_v2 v2;
	};
};

struct stfs_file {
	/* File name, null-padded. */
	char name[0x28];
	/* File name, plus flags
	 * The first 6 bits (0-5) are the lenght of the file name.
	 * Bit 6 and 7 are bitflags. If the 6th bit is set, it means that
	 * all of the blocks in the file are consecutive. Bit 7 indicates
	 * that the file is a directory.
	 * */
	uint8_t fn_flags;

	int32_t alloc_blkc;  /* little-endian */
	/* Number of the first block allocated for the file. */
	int32_t first_blkn;
	int16_t path_ind;	/* big-endian */
	uint32_t file_size; /* big-endian */
	int32_t udts; /* Update date/time stamp of file */
	int32_t adts; /* Access date/time stamp of file */
};

struct xstfs_desc {
	struct xstfs_header *header;
	struct xmetadata *metadata;
	struct xstfs_file *file_table;
	struct xlicense_data *licenses;
};

typedef struct stfs_block {
	uint8_t *data;
} STFS_Block;

typedef struct {
	char name[XSTFS_PATH_MAX];
	uint32_t upd_utime;
	uint32_t acs_utime;
	STFS_Block *blocks;
	size_t block_count;
	size_t size;
} STFS_File;

struct xpackage {
	struct xstfs_desc raw;
	struct stfs_metadata metadata;
	struct xstfs_header header;
	struct stfs_file *embedded_files;
	STFS_File *files;
	size_t file_count;
	int type;
};

typedef struct xpackage_handle {
	uint8_t* content;
	size_t size;

	struct xpackage package;
} XPKG_Handle;

extern int xstfs_error_ind;

/* Helper functions */
/* This function converts the block-relative position of `blockn` to it's 'raw'
 * position in a file. */
int32_t xblk_to_offset(int32_t hdr_size, int32_t blockn);
/* This function returns the actual block that `blockn` is refering to. */
int32_t xget_physical_blk(struct xpackage *pkg, int32_t blockn);
size_t xget_pblk_offset(struct xpackage *const pkg, int32_t blkn);
int ismemblk0(void *addr, size_t len);
size_t xstfs_build_pathtree(struct stfs_file *file_table, 
			    struct stfs_file *file,
			    char **pathbuf);

/* Endianess conversion */
int svod_vd2le(struct xsvod_vd *be, struct svod_vd *dest);
int stfs_vd2le(struct xstfs_vd *be, struct stfs_vd *dest);
int xmetadata2le(struct xmetadata *be, struct stfs_metadata *dest);
int xefile2le(struct xstfs_file *be, struct stfs_file *dest);
int32_t xint24le_to_int32le(xint24le_t n);
int32_t xint24be_to_int32le(xint24be_t n);
uint32_t xuint24le_to_uint32le(xuint24le_t n);
uint32_t xuint24be_to_uint32le(xuint24be_t n);

/* Error related functions */
static inline char* xstfs_strerror(int xstfs_errno)
{
	switch (xstfs_errno) {
	case XSTFS_NULL_PTR:
		return ERR_NULL_PTR;
		break;
	case XSTFS_INVALID_VAL:
		return ERR_INVALID_VAL;
		break;
	case XSTFS_CORRUPTED_AR:
		return ERR_CORRUPT_AR;
		break;
	case XSTFS_GENERIC_ERR:
		return ERR_GENERIC;
		break;
	case XSTFS_SUCCESS:
		return ERR_NOT_ERR;
		break;
	case XSTFS_NOT_IMPLEMENTED:
		return ERR_NOT_IMPLEMENTED;
		break;
	default:
		return ERR_GENERIC;
		break;
	}
}

static inline void xstfs_seterr(int errcode)
{
	xstfs_error_ind = errcode;
}

static inline int xstfs_checkerr()
{
	return (xstfs_error_ind != XSTFS_SUCCESS);
}

static inline int xstfs_geterr()
{
	return xstfs_error_ind;
}

static inline char* xstfs_geterrmsg(void)
{
	return xstfs_strerror(xstfs_error_ind);
}

/* File I/O Functions. Platform-specific code. */
XPKG_Handle* xstfs_open(const char *handle);
void xstfs_close(XPKG_Handle *handle);

/* File parsing functions. */
int xstfs_parse(XPKG_Handle *handle);
int xstfs_parse_stfs(XPKG_Handle *handle);
int xstfs_parse_files(XPKG_Handle *handle);


#ifdef XSTFS_IMPLEMENTATION

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

	#ifdef __linux__
#include <time.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <endian.h>
	#else
#error "OS not supported."
	#endif

int xstfs_error_ind = 0;

int32_t xblk_to_offset(int32_t hdr_size, int32_t blockn)
{
	int32_t offset = 0;

	/* Check if the block is inside the maximum logical limit for this
	 * filesystem
	 * */
	if (blockn > 0xFFFFFF)
		return -1;

	/* Rounds the file's header size up to the nearest 4096 bytes
	 * (4KB boundary), then multiply the block number by 4096 using a
	 * bitwise left shift. Finally, it adds both values together.
	 * */
	offset = (((hdr_size + 0xFFF) & 0xF000) + (blockn << 12));
	return offset;
}

int32_t xget_physical_blk(struct xpackage *pkg, int32_t blockn)
{
	int32_t base;
	int32_t blk_shift;
	int32_t ret;

	switch (pkg->type) {
	case SVOD:
		blk_shift = 1;
		break;
	case STFS:
		if (((pkg->metadata.hdr_size + 0xFFF) & ~0xFFF) == 0xB000) {
			blk_shift = 1;
		} else if ((pkg->metadata.vd_desc.stfs.blk_sp & 1) == 1) {
			blk_shift = 0;
		} else {
			blk_shift = 1;
		}
		break;
	default:
		blk_shift = 1;
		break;
	}

	base = ((blockn + 0xAA) / 0xAA);
	if (pkg->type == CON) {
		base = (base << blk_shift);
	}
	ret = (base + blockn);

	if (blockn >= 0xAA) {
		base = ((blockn + 0x70E4) / 0x70E4);
		if (pkg->type == CON)
			base = (base << blk_shift);
		ret += base;

		if (blockn >= 0x70E4) {
			base = ((blockn + 0x4AF768) / 0x4AF768);
			if (pkg->type == CON)
				base = (base << blk_shift);
			
			ret += base;
		}
	}

	return ret;
}

size_t xget_pblk_offset(struct xpackage *const pkg, int32_t blkn)
{
	int32_t block;
	size_t offset;

	block = xget_physical_blk(pkg, blkn);
	offset = xblk_to_offset(pkg->metadata.hdr_size, block);

	return offset;
}

inline int32_t xint24le_to_int32le(xint24le_t n)
{
	const uint8_t *bytes = n.val;

	uint32_t target =
		((uint32_t)bytes[0]) |
		((uint32_t)bytes[1] << 8) |
		((uint32_t)bytes[2] << 16);

	return ((int32_t)(target << 8)) >> 8;
}

inline int32_t xint24be_to_int32le(xint24be_t n)
{
	const uint8_t *bytes = n.val;

	uint32_t target =
		((uint32_t)bytes[2]) |
		((uint32_t)bytes[1] << 8) |
		((uint32_t)bytes[0] << 16);

	return ((int32_t)(target << 8)) >> 8;
}

inline uint32_t xuint24le_to_uint32le(xuint24le_t n)
{
	const uint8_t *bytes = n.val;

	return
		((uint32_t)bytes[0]) |
		((uint32_t)bytes[1] << 8) |
		((uint32_t)bytes[2] << 16);
}

inline uint32_t xuint24be_to_uint32le(xuint24be_t n)
{
	const uint8_t *bytes = n.val;

	return
		((uint32_t)bytes[2]) |
		((uint32_t)bytes[1] << 8) |
		((uint32_t)bytes[0] << 16);
}

int svod_vd2le(struct xsvod_vd *be, struct svod_vd *dest)
{
	memcpy(dest, be, sizeof(*dest));
	
	dest->dtblkc = xuint24be_to_uint32le(be->dtblkc);
	dest->dtblkoff = xuint24be_to_uint32le(be->dtblkoff);

	return 0;
}

int stfs_vd2le(struct xstfs_vd *be, struct stfs_vd *dest)
{
	memcpy(dest, be, sizeof(*dest));

	dest->alloc_blkc = be32toh(be->alloc_blkc);
	dest->unalloc_blkc = be32toh(be->unalloc_blkc);
	dest->ft_blkn = xint24be_to_int32le(be->ft_blkn);
	dest->ft_blkc = be16toh(be->ft_blkc);
	
	return 0;
}

int xmetadata2le(struct xmetadata *be, struct stfs_metadata *dest)
{ 
	memcpy(dest, be, sizeof(*dest));

	dest->base_version = be32toh(be->base_version);
	dest->met_version = be32toh(be->met_version);
	dest->version = be32toh(be->version);
	dest->cnt_size = be64toh(be->cnt_size);
	dest->dataf_csize = be64toh(be->dataf_csize);
	dest->dataf_count = be32toh(be->dataf_count);
	dest->cnt_type = be32toh(be->cnt_type);
	dest->vd_type = be32toh(be->vd_type);
	dest->hdr_size = be32toh(be->hdr_size);
	dest->media_id = be32toh(be->media_id);
	dest->savegame_id = be32toh(be->savegame_id);
	dest->title_id = be32toh(be->title_id);

	dest->l_entries->l_bits = be32toh(be->l_entries->l_bits);
	dest->l_entries->l_flags = be32toh(be->l_entries->l_flags);
	dest->l_entries->l_id = be64toh(be->l_entries->l_id);

	switch (dest->vd_type) {
		case STFS:
			stfs_vd2le(&be->vd_desc.stfs, &dest->vd_desc.stfs);
			break;
		case SVOD:
			svod_vd2le(&be->vd_desc.svod, &dest->vd_desc.svod);
			break;
		default:
			break;
	}
	
	switch (dest->met_version) {
		case 1:
			dest->v1.t_img_size = be32toh(be->v1.t_img_size);
			dest->v1.ttitle_img_size = be32toh(be->v1.ttitle_img_size);
			break;
		case 2:
			dest->v2.episode_number = be16toh(be->v2.episode_number);
			dest->v2.season_number = be16toh(be->v2.season_number);
			break;
		default:
			break;
	}

	return 0;
}

int xefile2le(struct xstfs_file *be, struct stfs_file *dest)
{
	memcpy(dest, be, sizeof(*dest));

	dest->file_size = be32toh(be->file_size);
	dest->first_blkn = xint24le_to_int32le(be->first_blkn);
	dest->alloc_blkc = xint24le_to_int32le(be->alloc_blkc);
	dest->path_ind = be16toh(be->path_ind);

	return 0;
}

int ismemblk0(void *addr, size_t len)
{
	size_t sum;
	uint8_t *buffer;

	sum = 0;
	buffer = addr;

	for (size_t i = 0; i < len; ++i)
		sum += buffer[i];

	return !sum;
}

size_t xstfs_build_pathtree(struct stfs_file *file_table,
		struct stfs_file *file,
		char **pathbuf)
{
	if ((file_table == NULL) || (file == NULL)) {
		xstfs_seterr(XSTFS_NULL_PTR);
		return 0;
	}

	char *buffer;
	char *path;
	struct stfs_file *fptr;
	int current_path_ind;
	size_t path_len;
	int fn_size;

	buffer = NULL;
	path = NULL;

	/* Get the size of the file path */
	path_len = 0;
	fptr = file;
	current_path_ind = fptr->path_ind;
	do {
		fn_size = (fptr->fn_flags & 63);
		path_len += fn_size + 1;
		current_path_ind = fptr->path_ind;
		fptr = &file_table[current_path_ind];
	} while (current_path_ind != -1);

	/* Build the file path */
	buffer = malloc(sizeof(*buffer) * (path_len + 1));
	if (buffer == NULL) {
		xstfs_seterr(XSTFS_GENERIC_ERR);
		goto error;
	}

	path = malloc(sizeof(*path) * (path_len + 1));
	if (path == NULL) {
		xstfs_seterr(XSTFS_GENERIC_ERR);
		goto error;
	}

	memset(buffer, 0, sizeof(*buffer) * (path_len + 1));
	memset(path, 0, sizeof(*path) * (path_len + 1));

	fptr = file;
	current_path_ind = fptr->path_ind;
	fn_size = (fptr->fn_flags & 63) + 1;
	
	snprintf(path,
		 sizeof(char) * (path_len + 1),
		 PATH_SEPARATOR"%.*s",
		 fn_size,
		 fptr->name);

	current_path_ind = fptr->path_ind;
	fptr = &file_table[current_path_ind];

	while (current_path_ind != -1) {
		fptr = &file_table[current_path_ind];
		fn_size = (fptr->fn_flags & 63) + 1;
		snprintf(buffer,
		 sizeof(char) * (path_len + 1),
		 "%.*s"PATH_SEPARATOR"%s", 
		 fn_size, fptr->name, path);
		
		memcpy(path, buffer, sizeof(char) * (path_len + 1));
		current_path_ind = fptr->path_ind;
	}

	if (pathbuf != NULL) {
		*pathbuf = path;
		free(buffer);
	} else {
		free(path);
		free(buffer);
	}

	return path_len;

error:
	if (buffer)
		free(buffer);
	if (path)
		free(path);

	return 0;
}

/* File I/O Functions. Platform-specific code. */
	#ifdef __linux__
XPKG_Handle* xstfs_open(const char *path)
{
	XPKG_Handle *handle;
	void *mmap_addr;
	int fd;
	FILE *fptr;
	struct stat st;
	struct xpackage *pkg;

	handle = NULL;
	mmap_addr = NULL;
	fd = -1;
	fptr = NULL;

	handle = malloc(sizeof(*handle));
	if (!handle) {
		xstfs_seterr(XSTFS_GENERIC_ERR);
		return NULL;
	}

	memset(handle, 0, sizeof(*handle));
	memset(&st, 0, sizeof(st));
	
	pkg = &handle->package;

	fptr = fopen(path, "r");
	if (!fptr) {
		xstfs_seterr(XSTFS_GENERIC_ERR);
		goto error;
	}

	fd = fileno(fptr);

	if (fstat(fd, &st) != 0) {
		xstfs_seterr(XSTFS_GENERIC_ERR);
		goto error;
	}

	mmap_addr = mmap(NULL, st.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
	if (mmap_addr == MAP_FAILED) {
		xstfs_seterr(XSTFS_GENERIC_ERR);
		goto error;
	}

	handle->content = mmap_addr;
	handle->size    = st.st_size;
   
	fclose(fptr);

	pkg->raw.header   = (void*) &((uint8_t*)mmap_addr)[0];
	pkg->raw.metadata = (void*) &handle->content[sizeof(struct xstfs_header)];

	return handle;

error:
	if (fptr)
		fclose(fptr);

	if (handle)
		free(handle);
	
	if (mmap_addr)
		munmap(mmap_addr, st.st_size);

	return NULL; 
}

void xstfs_close(XPKG_Handle *handle)
{
	if (!handle)
		return;
	
	struct xpackage *pkg;

	pkg = &handle->package;
	
	if (pkg->files) {
		for (size_t i = 0; i < pkg->file_count; ++i) {
			if (pkg->files[i].blocks)
				free(pkg->files[i].blocks);
		}

		free(pkg->files);
	}

	if (pkg->embedded_files)
		free(pkg->embedded_files);

	if (handle->content)
		munmap(handle->content, handle->size);

	free(handle);
}
	#endif /* __linux__ */

/* File parsing functions. */

int xstfs_parse(XPKG_Handle *handle)
{
	if (handle == NULL) {
		xstfs_seterr(XSTFS_NULL_PTR);
		return XSTFS_NULL_PTR;
	}
  
	struct xpackage *pkg;
	int vd_type;
	int err;

	err = 0;
	pkg = &handle->package;

	if (strncmp(pkg->raw.header->magic, MAGIC_CON, MAGIC_LEN) == 0) {
		pkg->type = CON;
	} else if (strncmp(pkg->raw.header->magic, MAGIC_PIRS, MAGIC_LEN) == 0) {
		pkg->type = PIRS;
	} else if (strncmp(pkg->raw.header->magic, MAGIC_LIVE, MAGIC_LEN) == 0) {
		pkg->type = LIVE;
	} else {
		xstfs_seterr(XSTFS_CORRUPTED_AR);
		return XSTFS_CORRUPTED_AR;
	}

	xmetadata2le(pkg->raw.metadata, &pkg->metadata);
	memcpy(&pkg->header, pkg->raw.header, sizeof(pkg->header));

	vd_type = pkg->metadata.vd_type;
	switch (vd_type) {
	case STFS:
		err = xstfs_parse_stfs(handle);
		if (err == XSTFS_SUCCESS) { 
			xstfs_parse_files(handle);
		} else {
			xstfs_seterr(err);
			return err;
		}
		break;
	case SVOD:
		xstfs_seterr(XSTFS_NOT_IMPLEMENTED);
		return XSTFS_NOT_IMPLEMENTED;
	default:
		xstfs_seterr(XSTFS_INVALID_VAL);
		return XSTFS_INVALID_VAL;
	}

	return XSTFS_SUCCESS;
}

int xstfs_parse_stfs(XPKG_Handle *handle)
{
	if (handle == NULL) {
		xstfs_seterr(XSTFS_NULL_PTR);
		return XSTFS_NULL_PTR;
	}

	struct xpackage *pkg;
	struct stfs_vd *vol_desc;
	struct xstfs_file *emb_file_tb;
	struct xstfs_file *xstfs_file;
	struct stfs_file *stfs_files;

	size_t file_table_offset;
	int32_t file_table_blockn;
	size_t file_count;
	int has_files;

	pkg = &handle->package;
	vol_desc = &pkg->metadata.vd_desc.stfs;
   
	has_files = (vol_desc->ft_blkc > 0);
	
	if (!has_files) {
		pkg->file_count = 0;
		pkg->embedded_files = NULL;
		goto done;
	} else { 
		goto parse_files;
	}

parse_files:
	file_table_blockn = vol_desc->ft_blkn;
	file_table_offset = xget_pblk_offset(pkg, file_table_blockn);

	emb_file_tb = (struct xstfs_file*)&handle->content[file_table_offset];
	xstfs_file = &emb_file_tb[0];
   
	file_count = 0;
	while (has_files) {
		if (ismemblk0(xstfs_file, sizeof(*xstfs_file))) {
			break;
		}
		xstfs_file++;
		file_count++;
	};
	
	if (file_count < 1) {
		has_files = 0;
		pkg->embedded_files = NULL;
		pkg->file_count = 0;
		goto done;
	} else {
		goto convert_files;
	}

convert_files:
	stfs_files = malloc(sizeof(*stfs_files) * file_count);
	if (stfs_files == NULL) {
		xstfs_seterr(XSTFS_GENERIC_ERR);
		return XSTFS_GENERIC_ERR;
	}
	
	for (size_t i = 0; i < file_count; ++i) {
		struct stfs_file *host_file;
		struct xstfs_file *embedded_file;

		host_file = &stfs_files[i];
		embedded_file = &emb_file_tb[i];

		xefile2le(embedded_file, host_file);
	}

	pkg->file_count = file_count;
	pkg->embedded_files = stfs_files;
	pkg->raw.file_table = emb_file_tb;

done:
	xstfs_seterr(XSTFS_SUCCESS);
	return XSTFS_SUCCESS;
}

int xstfs_parse_files(XPKG_Handle *handle)
{
	if (handle == NULL) {
		xstfs_seterr(XSTFS_NULL_PTR);
		return XSTFS_NULL_PTR;
	}

	if (handle->package.file_count < 1) {
		xstfs_seterr(XSTFS_SUCCESS);
		return XSTFS_SUCCESS;
	}

	struct xpackage *pkg;
	STFS_File *files;
	char *filepath_buffer;
	struct stfs_file *fptr;
	size_t path_size;
	STFS_File *file;
	STFS_Block *blk_buffer;
	size_t blk_data_offset;

	pkg = &handle->package;

	files = malloc(sizeof(*files) * pkg->file_count);
	if (files == NULL) {
		xstfs_seterr(XSTFS_GENERIC_ERR);
		return XSTFS_GENERIC_ERR;
	}

	for (size_t i = 0; i < pkg->file_count; ++i) {
		blk_buffer = NULL;
		filepath_buffer = NULL;
		path_size = 0;
		blk_data_offset = 0;
		
		fptr = &pkg->embedded_files[i];
		file = &files[i];
	
		memset(file, 0, sizeof(*file));

		filepath_buffer = file->name;
		path_size = xstfs_build_pathtree(pkg->embedded_files, fptr,
						 &filepath_buffer);
		
		if ((path_size > 0) && (filepath_buffer != NULL)) {
			memset(file->name, 0, sizeof(file->name));
			memcpy(file->name, filepath_buffer,
			       sizeof(char) * path_size);
			free(filepath_buffer);

			filepath_buffer = NULL;
			path_size = 0;
		}

		file->size = fptr->file_size;
		file->block_count = fptr->alloc_blkc;
		
		blk_buffer = calloc(file->block_count, sizeof(*blk_buffer));
		if (blk_buffer == NULL) {
			if (files != NULL) 
				free(files);
			if (filepath_buffer != NULL)
				free(filepath_buffer);
			
			xstfs_seterr(XSTFS_GENERIC_ERR);
			return XSTFS_GENERIC_ERR;
		}

		file->blocks = blk_buffer;	
		for (size_t b = 0; b < file->block_count; ++b) {
			blk_data_offset = xget_pblk_offset(pkg,
							   fptr->first_blkn + b);
			file->blocks[b].data = &handle->content[blk_data_offset];
		}

	}

	pkg->files = files;

	xstfs_seterr(XSTFS_SUCCESS);
	return XSTFS_SUCCESS;
}

#endif /* XSTFS_IMPLEMENTATION */

#endif /* XSTFS_H */

