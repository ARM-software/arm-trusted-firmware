/*
 * Copyright (c) 2013-2020, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <errno.h>
#include <string.h>

#include <lib/semihosting.h>

#ifndef SEMIHOSTING_SUPPORTED
#define SEMIHOSTING_SUPPORTED  1
#endif

long semihosting_call(unsigned long operation, uintptr_t system_block_address);

typedef struct {
	const char *file_name;
	unsigned long mode;
	size_t name_length;
} smh_file_open_block_t;

typedef struct {
	long handle;
	uintptr_t buffer;
	size_t length;
} smh_file_read_write_block_t;

typedef struct {
	long handle;
	ssize_t location;
} smh_file_seek_block_t;

typedef struct {
	char *command_line;
	size_t command_length;
} smh_system_block_t;

long semihosting_connection_supported(void)
{
	return SEMIHOSTING_SUPPORTED;
}

long semihosting_file_open(const char *file_name, size_t mode)
{
	smh_file_open_block_t open_block;

	open_block.file_name = file_name;
	open_block.mode = mode;
	open_block.name_length = strlen(file_name);

	return semihosting_call(SEMIHOSTING_SYS_OPEN, (uintptr_t)&open_block);
}

long semihosting_file_seek(long file_handle, ssize_t offset)
{
	smh_file_seek_block_t seek_block;
	long result;

	seek_block.handle = file_handle;
	seek_block.location = offset;

	result = semihosting_call(SEMIHOSTING_SYS_SEEK, (uintptr_t)&seek_block);

	if (result != 0) {
		result = semihosting_call(SEMIHOSTING_SYS_ERRNO, 0);
	}

	return result;
}

long semihosting_file_read(long file_handle, size_t *length, uintptr_t buffer)
{
	smh_file_read_write_block_t read_block;
	long result = -EINVAL;

	if ((length == NULL) || (buffer == (uintptr_t)NULL)) {
		return result;
	}

	read_block.handle = file_handle;
	read_block.buffer = buffer;
	read_block.length = *length;

	result = semihosting_call(SEMIHOSTING_SYS_READ, (uintptr_t)&read_block);

	if (result == *length) {
		return -EINVAL;
	} else if (result < *length) {
		*length -= result;
		return 0;
	} else {
		return result;
	}
}

long semihosting_file_write(long file_handle, size_t *length,
				const uintptr_t buffer)
{
	smh_file_read_write_block_t write_block;
	long result = -EINVAL;

	if ((length == NULL) || (buffer == (uintptr_t)NULL)) {
		return -EINVAL;
	}

	write_block.handle = file_handle;
	write_block.buffer = (uintptr_t)buffer; /* cast away const */
	write_block.length = *length;

	result = semihosting_call(SEMIHOSTING_SYS_WRITE,
		(uintptr_t)&write_block);

	*length = result;

	return (result == 0) ? 0 : -EINVAL;
}

long semihosting_file_close(long file_handle)
{
	return semihosting_call(SEMIHOSTING_SYS_CLOSE, (uintptr_t)&file_handle);
}

long semihosting_file_length(long file_handle)
{
	return semihosting_call(SEMIHOSTING_SYS_FLEN, (uintptr_t)&file_handle);
}

char semihosting_read_char(void)
{
	return semihosting_call(SEMIHOSTING_SYS_READC, 0);
}

void semihosting_write_char(char character)
{
	semihosting_call(SEMIHOSTING_SYS_WRITEC, (uintptr_t)&character);
}

void semihosting_write_string(char *string)
{
	semihosting_call(SEMIHOSTING_SYS_WRITE0, (uintptr_t)string);
}

long semihosting_system(char *command_line)
{
	smh_system_block_t system_block;

	system_block.command_line = command_line;
	system_block.command_length = strlen(command_line);

	return semihosting_call(SEMIHOSTING_SYS_SYSTEM,
		(uintptr_t)&system_block);
}

long semihosting_get_flen(const char *file_name)
{
	long file_handle;
	long length;

	assert(semihosting_connection_supported() != 0);

	file_handle = semihosting_file_open(file_name, FOPEN_MODE_RB);
	if (file_handle == -1) {
		return file_handle;
	}

	/* Find the length of the file */
	length = semihosting_file_length(file_handle);

	return (semihosting_file_close(file_handle) != 0) ? -1 : length;
}

long semihosting_download_file(const char *file_name,
			      size_t buf_size,
			      uintptr_t buf)
{
	long ret = -EINVAL;
	size_t length;
	long file_handle;

	/* Null pointer check */
	if (buf == 0U) {
		return ret;
	}

	assert(semihosting_connection_supported() != 0);

	file_handle = semihosting_file_open(file_name, FOPEN_MODE_RB);
	if (file_handle == -1) {
		return ret;
	}

	/* Find the actual length of the file */
	length = semihosting_file_length(file_handle);
	if (length == (size_t)(-1)) {
		goto semihosting_fail;
	}

	/* Signal error if we do not have enough space for the file */
	if (length > buf_size) {
		goto semihosting_fail;
	}

	/*
	 * A successful read will return 0 in which case we pass back
	 * the actual number of bytes read. Else we pass a negative
	 * value indicating an error.
	 */
	ret = semihosting_file_read(file_handle, &length, buf);
	if (ret != 0) {
		goto semihosting_fail;
	} else {
		ret = (long)length;
	}

semihosting_fail:
	semihosting_file_close(file_handle);
	return ret;
}

void semihosting_exit(uint32_t reason, uint32_t subcode)
{
#ifdef __aarch64__
	uint64_t parameters[] = {reason, subcode};

	(void)semihosting_call(SEMIHOSTING_SYS_EXIT, (uintptr_t)&parameters);
#else
	/* The subcode is not supported on AArch32. */
	(void)semihosting_call(SEMIHOSTING_SYS_EXIT, reason);
#endif
}
