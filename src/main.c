	#if defined(__linux)
#define _GNU_SOURCE
#define PATH_SEP "/"
#include <unistd.h>
#include <linux/limits.h>
#include <dirent.h>
	#elif defined(_WIN32)
#include <windows.h>
#include <Lmcons.h>
#define PATH_MAX MAX_PATH
	#endif

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <errno.h>

#define XSTFS_IMPLEMENTATION
#include "xstfs.h"

#include "config.h"
#include "extract.h"

enum {
	NONE,
	EXTRACT,
};

static const struct option options[] = {
	{"extract", no_argument, NULL, 'x'},
	{"pkg", required_argument, NULL, 'p'},
	{"file", required_argument, NULL, 'f'},
	{"output", required_argument, NULL, 'o'},
	{"version", no_argument, NULL, 'v'},
	{"help", no_argument, NULL, 'h'},
	{0, 0, 0, 0},
};

static void print_version()
{
	printf("xstract version %s.\n\n", VERSION);
	printf("Author: %s\n", AUTHOR);
	printf("License: %s\n", LICENSE);
	printf("Homepage: <%s>\n", HOMEPAGE_URL);
}

static void print_help(const char *const bin)
{
	printf("Usage: %s '[-p,-f,--package,--file] [PATH]' [OPTION...]\n", (bin) ? bin : "xstract");
	puts("Options:");
	puts("\t-x, --extract      : Extract the contents of package PATH to OUTPUT.");
	puts("\t-v, --version      : Shows version and copyright notice.");
	puts("\t-h, --help         : Shows this message.");
	puts("\t-o, --output [DIR] : Sets OUTPUT to directory DIR. Default value is '.'");
}

	#if defined(__linux)
static char* expanduser(void)
{
	int err;
	static char username[L_cuserid];

	memset(username, 0, sizeof(username));

	err = getlogin_r(username, sizeof(username));
	if (err != 0) {
		fprintf(stderr, "ERROR (%s): %s\n", __FUNCTION__, strerror(errno));
		exit(EXIT_FAILURE);
	}

	return username;
}
	#endif

static int parse_path(const char *const basepath, char *const path, size_t len)
{
	if (path == NULL)
		return EXIT_FAILURE;

	if (len <= 0)
		return EXIT_FAILURE;

	size_t basepath_len;
	char home_dir[PATH_MAX];
	char *user;
	size_t home_path_len;
	char c;

	memset(home_dir, 0, sizeof(home_dir));
	basepath_len = strlen(basepath);

	size_t o = 0;
	for (size_t i = 0; o < basepath_len + 1; ++i) {
		c = basepath[i];
		if (c == '~') {
			user = expanduser();

			snprintf(home_dir, sizeof(home_dir), "/home/%s", user);
			home_path_len = strlen(home_dir);

			strncpy(&path[i], home_dir, len - (sizeof(char) * home_path_len));
			i += home_path_len;
			o++;
		}

		path[i] = basepath[o++];
	}

	return 0;
}

static int extract(XPKG_Handle *package, const char *const output_dir)
{
	if (package == NULL)
		return EXIT_FAILURE;

	char output_path[PATH_MAX];
	DIR *dir;

	memset(output_path, 0, sizeof(output_path));

	parse_path(output_dir, output_path, sizeof(output_path));
	printf("INFO: Extracting to \"%s\" ...\n", output_path);

	dir = opendir(output_path);
	if (dir == NULL) {
		if (errno == ENOENT) {
			fprintf(stderr, "ERROR (%s): \"%s\": %s\n",
				__FUNCTION__, output_path, strerror(errno));
		} else {
			fprintf(stderr, "ERROR (%s): %s\n",
				__FUNCTION__, strerror(errno));
		}
		return EXIT_FAILURE;
	}

	if (extract_package(package, dir) != 0) {
		fprintf(stderr, "ERROR: %s\n", strerror(errno));
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

int main(int argc, char **argv)
{
	if (argc < 2) {
		print_help(argv[0]);
		fprintf(stderr, "ERROR: Too few arguments.\n");
		return EXIT_FAILURE;
	}

	int c;
	int status;
	int mode = NONE;
	char *pkg_path_arg;
	char package_path[PATH_MAX];
	char *output_dir;
	XPKG_Handle *package = NULL;

	while ((c = getopt_long(argc, argv, "p:f:o:vhx", options, NULL)) != -1) {
		switch (c) {
		case 'v':
			print_version();
			exit(EXIT_SUCCESS);
		case 'h':
			print_help(argv[0]);
			exit(EXIT_SUCCESS);
		case 'p':
		case 'f':
			pkg_path_arg = optarg;
			break;
		case 'x':
			mode = EXTRACT;
			break;
		case 'o':
			output_dir = optarg;
			break;
		case '?':
		default:
			exit(EXIT_FAILURE);
		}
	}

	if (pkg_path_arg == NULL) {
		print_help(argv[0]);
		fprintf(stderr, "ERROR: Please enter a package path.\n");
		exit(EXIT_FAILURE);
	}

	if (output_dir == NULL)
		output_dir = "./";

	memset(package_path, 0, sizeof(package_path));
	parse_path(pkg_path_arg, package_path, sizeof(package_path));

	errno = 0;
	package = xstfs_open(package_path);
	if (package == NULL) {
		if (errno == ENOENT) {
			fprintf(stderr, "ERROR: \"%s\": %s\n", package_path, strerror(errno));
		} else if (errno != 0) {
			fprintf(stderr, "ERROR: %s\n", strerror(errno));
		} else {
			fprintf(stderr, "XSTFS ERROR: %s (errno \"%s\")\n", xstfs_geterrmsg(),
				strerror(errno));
		}
	}

	xstfs_parse(package);

	switch (mode) {
	case EXTRACT:
		status = extract(package, output_dir);
		break;
	default:
		status = EXIT_FAILURE;
		fprintf(stderr, "Nothing to do. Run with -x to extract.\n");
		break;
	}

	xstfs_close(package);

	return status;
}
