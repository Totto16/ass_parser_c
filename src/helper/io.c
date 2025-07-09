

#include "./io.h"

#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#undef _POSIX_C_SOURCE

#include <errno.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

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

[[nodiscard]] static SizedPtr read_entire_file_raw(FILE* file) {

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
	size_t actual_read = fread(data, 1, fsize, file);

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

[[nodiscard]] SizedPtr read_entire_file(const char* file_name) {

	FILE* file = fopen(file_name, "rb");

	return read_entire_file_raw(file);
}

#define CHUNK_SIZE 512

[[nodiscard]] static SizedPtr read_string_raw(int fd) {

	char* start_buffer = (char*)malloc(CHUNK_SIZE);

	if(!start_buffer) {
		return ptr_error("allocation error");
	}

	SizedPtr result = { .data = start_buffer, .len = 0 };

	while(true) {
		// read bytes, save the amount of read bytes, and then test for various scenarios
		ssize_t read_bytes = read(fd, (uint8_t*)result.data + result.len, CHUNK_SIZE);

		if(read_bytes == -1) {
			return ptr_error("read error");
		}

		if(read_bytes == 0) {
			break;
		}

		result.len = result.len + read_bytes;

		if(read_bytes == CHUNK_SIZE) {

			char* new_buffer = (char*)realloc(result.data, result.len + CHUNK_SIZE);

			if(!new_buffer) {
				return ptr_error("realloc error");
			}

			result.data = new_buffer;

			continue;
		}

		// the message was shorter and could fit in the existing buffer!
		break;
	}

	// malloced, null terminated an probably "huge"
	return result;
}

[[nodiscard]] SizedPtr read_entire_stdin(void) {

	return read_string_raw(STDIN_FILENO);
}
