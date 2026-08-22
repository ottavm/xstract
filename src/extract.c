#include "extract.h"

#include <stdio.h>
#include <stdlib.h>

static size_t extract_file(STFS_File *const file, FILE *fptr)
{
	size_t size;
	size_t written;
	int32_t blockc;
	int32_t blk_ind;
	size_t len;
	size_t err;

	size = file->size;
	blockc = file->block_count;
	blk_ind = 1;
	written = 0;

	rewind(fptr);
	for (int i = 0; blk_ind <= blockc; ++i) {
		if ((size - written) < STFS_BLOCK_LEN)
			len = (size - written);
		else
			len = STFS_BLOCK_LEN;

		err = fwrite(file->blocks[i].data, len, 1, fptr);
		if (err < 1)
			return 0;

		blk_ind++;
		written += len;
	}
	
	return written;
}

	#if defined(__linux)
int extract_package(XPKG_Handle *const package, DIR *dir)
{
	if ((package == NULL) || (dir == NULL))
		return EXIT_FAILURE;

	size_t fcount;
	size_t written;
	char *filename;
	STFS_File *file;
	int dird;
	FILE *tfptr;
	int tfd;

	dird = dirfd(dir);
	fcount = package->package.file_count;

	for (size_t i = 0; i < fcount; i++) {
		file = &package->package.files[i];
		filename = &file->name[1];
		
		tfd = openat(dird, filename, O_WRONLY | O_CREAT,
			     S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
		if (tfd == -1)
			return 1;
		tfptr = fdopen(tfd, "w");
		if (tfptr == NULL)
			return 1;

		printf("INFO: Extracting \"%s\" ... ", filename);
		written = extract_file(file, tfptr);
		if (written == 0) {
			puts("Failed.");
			fclose(tfptr);
			return 1;
		} else {
			printf("Done. ");
			printf("(Total size: %zu Bytes, written: %zu Bytes)\n", file->size, written);
		}

		fclose(tfptr);
	}

	return 0;
}
	#endif

