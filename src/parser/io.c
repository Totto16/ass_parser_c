

#include "./io.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

[[nodiscard]] bool is_file_a_directory(FILE* file) {
	if(!file) {
		return false;
	}

	int file_descriptor = fileno(file);
	if(file_descriptor < 0) {
		return false;
	}

	struct stat stat_struct = {};
	if(fstat(file_descriptor, &stat_struct) < 0) {
		return false;
	}

	return S_ISDIR(stat_struct.st_mode);
}

[[nodiscard]] SizedPtr read_entire_file(char* file_name) {

	FILE* file = fopen(file_name, "rb");

	if(file == NULL) {
		if(errno == EACCES) {
			return ptr_error("no permissions for file");
		} else if(errno == ENOENT) {
			return ptr_error("no such file");
		} else {
			return ptr_error("unknown file error");
		}
	}

	if(is_file_a_directory(file)) {
		return ptr_error("can't open a directory");
	}

	int res = fseek(file, 0, SEEK_END);
	if(res != 0) {
		return ptr_error("fseek error");
	}
	long fsize = ftell(file);
	if(fsize < 0) {
		return ptr_error("ftell error");
	}

	res = fseek(file, 0, SEEK_SET);
	if(res != 0) {
		return ptr_error("rewind error");
	}

	void* data = malloc(fsize);
	if(data == NULL) {
		return ptr_error("allocation error");
	}
	size_t actual_read = fread(data, fsize, 1, file);

	if((size_t)fsize != actual_read) {
		free(data);
		return ptr_error("read error");
	}

	res = fclose(file);

	if(res != 0) {
		free(data);
		return ptr_error("fclose error");
	}

	return (SizedPtr){ .data = data, .len = (size_t)fsize };
}
