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

#ifndef __SEMIHOSTING_H__
#define __SEMIHOSTING_H__

#include <stdint.h>
#include <stdio.h> /* For ssize_t */


#define SEMIHOSTING_SYS_OPEN            0x01
#define SEMIHOSTING_SYS_CLOSE           0x02
#define SEMIHOSTING_SYS_WRITE0          0x04
#define SEMIHOSTING_SYS_WRITEC          0x03
#define SEMIHOSTING_SYS_WRITE           0x05
#define SEMIHOSTING_SYS_READ            0x06
#define SEMIHOSTING_SYS_READC           0x07
#define SEMIHOSTING_SYS_SEEK            0x0A
#define SEMIHOSTING_SYS_FLEN            0x0C
#define SEMIHOSTING_SYS_REMOVE          0x0E
#define SEMIHOSTING_SYS_SYSTEM          0x12
#define SEMIHOSTING_SYS_ERRNO           0x13

#define FOPEN_MODE_R			0x0
#define FOPEN_MODE_RB			0x1
#define FOPEN_MODE_RPLUS		0x2
#define FOPEN_MODE_RPLUSB		0x3
#define FOPEN_MODE_W			0x4
#define FOPEN_MODE_WB			0x5
#define FOPEN_MODE_WPLUS		0x6
#define FOPEN_MODE_WPLUSB		0x7
#define FOPEN_MODE_A			0x8
#define FOPEN_MODE_AB			0x9
#define FOPEN_MODE_APLUS		0xa
#define FOPEN_MODE_APLUSB		0xb

long semihosting_connection_supported(void);
long semihosting_file_open(const char *file_name, size_t mode);
long semihosting_file_seek(long file_handle, ssize_t offset);
long semihosting_file_read(long file_handle, size_t *length, uintptr_t buffer);
long semihosting_file_write(long file_handle,
			    size_t *length,
			    const uintptr_t buffer);
long semihosting_file_close(long file_handle);
long semihosting_file_length(long file_handle);
long semihosting_system(char *command_line);
long semihosting_get_flen(const char *file_name);
long semihosting_download_file(const char *file_name,
			       size_t buf_size,
			       uintptr_t buf);
void semihosting_write_char(char character);
void semihosting_write_string(char *string);
char semihosting_read_char(void);

#endif /* __SEMIHOSTING_H__ */
