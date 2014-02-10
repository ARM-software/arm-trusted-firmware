/*
 * Copyright (c) 2013-2014, ARM Limited and Contributors. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * Neither the name of ARM nor the names of its contributors may be used
 * to endorse or promote products derived from this software without specific
 * prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <assert.h>
#include <string.h>
#include <errno.h>
#include <semihosting.h>

#ifndef SEMIHOSTING_SUPPORTED
#define SEMIHOSTING_SUPPORTED  1
#endif

extern int semihosting_call(unsigned int operation,
			    void *system_block_address);

typedef struct {
	const char *file_name;
	unsigned int mode;
	unsigned int name_length;
} smh_file_open_block;

typedef struct {
	int handle;
	void *buffer;
	unsigned int length;
} smh_file_read_write_block;

typedef struct {
	int handle;
	unsigned int location;
} smh_file_seek_block;

typedef struct {
	char *command_line;
	unsigned int command_length;
} smh_system_block;

int semihosting_connection_supported(void)
{
	return SEMIHOSTING_SUPPORTED;
}

int semihosting_file_open(const char *file_name, unsigned int mode)
{
	smh_file_open_block open_block;

	open_block.file_name = file_name;
	open_block.mode = mode;
	open_block.name_length = strlen(file_name);

	return semihosting_call(SEMIHOSTING_SYS_OPEN,
				(void *) &open_block);
}

int semihosting_file_seek(int file_handle, unsigned int offset)
{
	smh_file_seek_block seek_block;
	int result;

	seek_block.handle = file_handle;
	seek_block.location = offset;

	result = semihosting_call(SEMIHOSTING_SYS_SEEK,
				  (void *) &seek_block);

	if (result)
		result = semihosting_call(SEMIHOSTING_SYS_ERRNO, 0);

	return result;
}

int semihosting_file_read(int file_handle, int *length, void *buffer)
{
	smh_file_read_write_block read_block;
	int result = -EINVAL;

	if ((length == NULL) || (buffer == NULL))
		return result;

	read_block.handle = file_handle;
	read_block.buffer = buffer;
	read_block.length = *length;

	result = semihosting_call(SEMIHOSTING_SYS_READ,
				  (void *) &read_block);

	if (result == *length) {
		return -EINVAL;
	} else if (result < *length) {
		*length -= result;
		return 0;
	} else
		return result;
}

int semihosting_file_write(int file_handle, int *length, const void *buffer)
{
	smh_file_read_write_block write_block;

	if ((length == NULL) || (buffer == NULL))
		return -EINVAL;

	write_block.handle = file_handle;
	write_block.buffer = (void *)buffer;
	write_block.length = *length;

	*length = semihosting_call(SEMIHOSTING_SYS_WRITE,
				   (void *) &write_block);

	return *length;
}

int semihosting_file_close(int file_handle)
{
	return semihosting_call(SEMIHOSTING_SYS_CLOSE,
				(void *) &file_handle);
}

int semihosting_file_length(int file_handle)
{
	return semihosting_call(SEMIHOSTING_SYS_FLEN,
				(void *) &file_handle);
}

char semihosting_read_char(void)
{
	return semihosting_call(SEMIHOSTING_SYS_READC, NULL);
}

void semihosting_write_char(char character)
{
	semihosting_call(SEMIHOSTING_SYS_WRITEC, (void *) &character);
}

void semihosting_write_string(char *string)
{
	semihosting_call(SEMIHOSTING_SYS_WRITE0, (void *) string);
}

int semihosting_system(char *command_line)
{
	smh_system_block system_block;

	system_block.command_line = command_line;
	system_block.command_length = strlen(command_line);

	return semihosting_call(SEMIHOSTING_SYS_SYSTEM,
				(void *) &system_block);
}

int semihosting_get_flen(const char *file_name)
{
	int file_handle, length;

	assert(semihosting_connection_supported());

	file_handle = semihosting_file_open(file_name, FOPEN_MODE_RB);
	if (file_handle == -1)
		return file_handle;

	/* Find the length of the file */
	length = semihosting_file_length(file_handle);

	return semihosting_file_close(file_handle) ? -1 : length;
}

int semihosting_download_file(const char *file_name,
			      int buf_size,
			      void *buf)
{
	int ret = -EINVAL, file_handle, length;

	/* Null pointer check */
	if (!buf)
		return ret;

	assert(semihosting_connection_supported());

	file_handle = semihosting_file_open(file_name, FOPEN_MODE_RB);
	if (file_handle == -1)
		return ret;

	/* Find the actual length of the file */
	length = semihosting_file_length(file_handle);
	if (length == -1)
		goto semihosting_fail;

	/* Signal error if we do not have enough space for the file */
	if (length > buf_size)
		goto semihosting_fail;

	/*
	 * A successful read will return 0 in which case we pass back
	 * the actual number of bytes read. Else we pass a negative
	 * value indicating an error.
	 */
	ret = semihosting_file_read(file_handle, &length, buf);
	if (ret)
		goto semihosting_fail;
	else
		ret = length;

semihosting_fail:
	semihosting_file_close(file_handle);
	return ret;
}
