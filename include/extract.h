#ifndef EXTRACT_H
#define EXTRACT_H

#include "xstfs.h"

	#if defined(__linux)
#include <dirent.h>
#include <unistd.h>
#include <linux/limits.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
	#endif

	#if defined(__linux)
int extract_package(XPKG_Handle *const package, DIR *dir);
	#endif
#endif /* EXTRACT_H */

