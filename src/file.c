#include "file.h"

char*
file_solve_path(char* dst, size_t dstsize, char const* path) {
	char tmp[FILE_NPATH];

	// Check arugments
	if (!dst || !path) {
		WARN("Invalid arguments");
		return NULL;
	}

	// Solve '~'
	if (path[0] == '~') {
		char const* homepath = NULL;

#if defined(_WIN32) || defined(_WIN64)
		homepath = getenv("USERPROFILE");
#else
		homepath = getenv("HOME");
#endif

		snprintf(tmp, FILE_NPATH, "%s/%s", homepath, path+1);
	} else {
		snprintf(tmp, FILE_NPATH, "%s", path);
	}

	// Solve path
#if defined(_WIN32) || defined(_WIN64)
	char* fpart;

	if (!GetFullPathName(tmp, dstsize, dst, &fpart)) {
		WARN("Failed to solve path");
		return NULL;
	}
#else
	errno = 0;

	if (!realpath(tmp, dst)) {
		if (errno == ENOENT) {
			// Path is not exists
			snprintf(dst, dstsize, "%s", tmp);
		} else {
			WARN("Failed to realpath \"%s\"", tmp);
			return NULL;
		}
	}
#endif

	return dst;
}

char*
file_make_solve_path(char const* path) {
	// Check arguments
	if (!path) {
		WARN("Invalid arguments");
		return NULL;
	}

	// Ready
	char* dst = (char*) malloc(sizeof(char) * FILE_NPATH);
	if (!dst) {
		WARN("Failed to malloc");
		return NULL;
	}

	// Solve
	char* res = file_solve_path(dst, FILE_NPATH, path);
	if (!res) {
		WARN("Failed to solve path \"%s\"", path);
		free(dst);
		return NULL;
	}

	return res;
}

FILE*
file_open(char const* path, char const* mode) {
	char spath[FILE_NPATH];

	if (!file_solve_path(spath, sizeof spath, path)) {
		WARN("Failed to solve path \"%s\"", path);
		return NULL;
	}

	return fopen(spath, mode);
}

DIR*
file_opendir(char const* path) {
	char spath[FILE_NPATH];

	if (!file_solve_path(spath, sizeof spath, path)) {
		WARN("Failed to solve path \"%s\"", path);
		return NULL;
	}

	return opendir(spath);
}

int
file_closedir(DIR* dir) {
	return closedir(dir);
}

int
file_close(FILE* fp) {
	return fclose(fp);
}

bool
file_is_exists(char const* path) {
	char spath[FILE_NPATH];

	if (!file_solve_path(spath, sizeof spath, path)) {
		WARN("Failed to solve path \"%s\"", path);
		return false;
	}

	struct stat s;
	int res = stat(spath, &s);

	if (res == -1) {
		if (errno == ENOENT) {
			goto notfound;
		} else {
			die("stat");
		}
	}
	return true;

notfound:
	return false;  // Does not exists
}

bool
file_is_dir(char const* path) {
	char spath[FILE_NPATH];

	if (!file_solve_path(spath, sizeof spath, path)) {
		WARN("Failed to solve path \"%s\"", path);
		return NULL;
	}

	struct stat s;
	int res = stat(spath, &s);

	if (res == -1) {
		if (errno == ENOENT) {
			;
		} else {
			WARN("Failed to stat");
		}
		goto notfound;
	} else {
		if (S_ISDIR(s.st_mode)) {
			return true;
		} else {
			return false;
		}
	}

notfound:
	return false;
}

int
file_mkdir(char const* dirpath, mode_t mode) {
	char spath[FILE_NPATH];

	if (!file_solve_path(spath, sizeof spath, dirpath)) {
		WARN("Failed to solve path \"%s\"", dirpath);
		return -1;
	}

#if defined(_WIN32) || defined(_WIN64)
	return mkdir(spath);
#else
	return mkdir(spath, mode);
#endif
}

bool
file_create(char const* path) {
	FILE* fout = file_open(path, "wb");
	if (!fout) {
		return false;
	}
	file_close(fout);
	return true;
}

char*
file_read_string(FILE* fin) {
	// Check arguments
	if (!fin || feof(fin)) {
		WARN("Invalid stream");
		return NULL;
	}

	// Buffer for read stream
	Buffer* buf = buffer_new();
	if (!buf) {
		WARN("Failed to construct buffer");
		return NULL;
	}

	// Read stream
	int ch;
	for (;;) {
		ch = fgetc(fin);
		if (ch == EOF || ferror(fin)) {
			goto done;
		}
		buffer_push(buf, ch);
	}

done:
	// Done
	buffer_push(buf, '\0');
	return buffer_escape_delete(buf);
}

/*****************
* file Directory *
*****************/

struct DirectoryNode {
#if defined(_WIN32) || defined(_WIN64)
	WIN32_FIND_DATA finddata;
#else
	struct dirent* node;
#endif	
};

void
dirnode_delete(DirectoryNode* self) {
	if (self) {
		free(self);
	}
}

DirectoryNode*
dirnode_new(void) {
	DirectoryNode* self = (DirectoryNode*) calloc(1, sizeof(DirectoryNode));
	if (!self) {
		WARN("Failed to construct DirectoryNode");
		return NULL;
	}
	return self;
}

char const*
dirnode_name(DirectoryNode const* self) {
#if defined(_WIN32) || defined(_WIN64)
	return self->finddata.cFileName;
#else
	return self->node->d_name;
#endif
}

struct Directory {
#if defined(_WIN32) || defined(_WIN64)
	HANDLE directory;
	char dirpath[FILE_NPATH];
#else
	DIR* directory;
#endif
};

void
dir_close(Directory* self) {
	if (self) {
#if defined(_WIN32) || defined(_WIN64)
		if (self->directory) {
			if (!FindClose(self->directory)) {
				WARN("Failed to close directory");
			}
			self->directory = NULL;
		}

#else
		if (closedir(self->directory) != 0) {
			WARN("Failed to close directory");
		}
#endif	
		
		free(self);
	}
}

Directory*
dir_open(char const* path) {
	Directory* self = (Directory*) calloc(1, sizeof(Directory));
	if (!self) {
		WARN("Failed to construct Directory");
		return NULL;
	}

#if defined(_WIN32) || defined(_WIN64)
	self->directory = NULL;
	snprintf(self->dirpath, sizeof(self->dirpath), "%s/*", path);

#else
	if (!(self->directory = opendir(path))) {
		WARN("Failed to open directory \"%s\"", path);
		free(self);
		return NULL;
	}

#endif
	return self;
}

DirectoryNode*
dir_read_node(Directory* self) {
	DirectoryNode* node = dirnode_new();
	if (!node) {
		WARN("Failed to construct DirectoryNode");
		return NULL;
	}

#if defined(_WIN32) || defined(_WIN64)
	if (!self->directory) {
		if ((self->directory = FindFirstFile(self->dirpath, &node->finddata)) == INVALID_HANDLE_VALUE) {
			WARN("Failed to open directory \"%s\"", self->dirpath);
			dirnode_delete(node);
			return NULL;

		}

	} else {
		if (!FindNextFile(self->directory, &node->finddata)) {
			dirnode_delete(node);
			return NULL; // Done to find
		}
	}

#else
	errno = 0;
	if (!(node->node = readdir(self->directory))) {
		if (errno != 0) {
			dirnode_delete(node);
			WARN("Failed to readdir");
			return NULL;
		} else {
			// Done to readdir
			dirnode_delete(node);
			return NULL;
		}
	}

#endif

	return node;
}

/************
* file test *
************/

#if defined(TEST_FILE)
int
test_mkdir(int argc, char* argv[]) {
	if (argc < 2) {
		die("need path");
	}

	char const* path = argv[1];

	if (file_is_exists(path)) {
		printf("is exists [%s]\n", path);
	} else {
		printf("is not exists [%s]\n", path);
		file_mkdir(path, S_IRUSR | S_IWUSR | S_IXUSR);
	}

	return 0;
}

char*
solve_path(char* dst, size_t dstsize, char const* path) {
#if defined(_WIN32) || defined(_WIN64)
	char* fpart;

	if (!GetFullPathName(path, dstsize, dst, &fpart)) {
		WARN("Failed to solve path");
		return NULL;
	}
#endif
	return dst;
}

int
test_solve_path(int argc, char* argv[]) {
	if (argc < 2) {
		die("need path");
	}

	char dst[FILE_NPATH];
	printf("[%s] -> \n[%s]\n", argv[1], file_solve_path(dst, sizeof dst, argv[1]));

	return 0;
}

int
test_directory(int argc, char* argv[]) {
#if defined(_WIN32) || defined(_WIN64)
	char const* dirpath = "C:/Windows/Temp";
	
	if (argc >= 2) {
		dirpath = argv[1];
	}

	Directory* dir = dir_open(dirpath);
	if (!dir) {
		WARN("Failed to open dir \"%s\"", dirpath);
		return 1;
	}

	for (DirectoryNode* node; (node = dir_read_node(dir)); ) {
		fprintf(stderr, "name[%s]\n", dirnode_name(node));
		dirnode_delete(node);
	} 

	fflush(stderr);
	dir_close(dir);
	return 0;

#else
	char const* dirpath = "/tmp";
	
	if (argc >= 2) {
		dirpath = argv[1];
	}

	Directory* dir = dir_open(dirpath);
	if (!dir) {
		WARN("Failed to open dir \"%s\"", dirpath);
		return 1;
	}

	for (DirectoryNode* node; (node = dir_read_node(dir)); ) {
		printf("name[%s]\n", dirnode_name(node));
		dirnode_delete(node);
	} 

	dir_close(dir);
#endif
	return 0;
}

int
main(int argc, char* argv[]) {
	// return test_solve_path(argc, argv);
	int ret = test_directory(argc, argv);
	if (ret != 0) {
		caperr_display(stderr);
	}

	return ret;
}
#endif

