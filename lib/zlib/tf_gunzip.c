/*
 * Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <errno.h>
#include <string.h>

#include <common/debug.h>
#include <lib/utils.h>
#include <tf_gunzip.h>

#include "zutil.h"

/*
 * memory allocated by malloc() is supposed to be aligned for any built-in type
 */
#define ZALLOC_ALIGNMENT	sizeof(void *)

static uintptr_t zalloc_start;
static uintptr_t zalloc_end;
static uintptr_t zalloc_current;

static void * ZLIB_INTERNAL zcalloc(void *opaque, unsigned int items,
				    unsigned int size)
{
	uintptr_t p, p_end;

	size *= items;

	p = round_up(zalloc_current, ZALLOC_ALIGNMENT);
	p_end = p + size;

	if (p_end > zalloc_end)
		return NULL;

	memset((void *)p, 0, size);

	zalloc_current = p_end;

	return (void *)p;
}

static void ZLIB_INTERNAL zfree(void *opaque, void *ptr)
{
}

/*
 * gunzip - decompress gzip data
 * @in_buf: source of compressed input. Upon exit, the end of input.
 * @in_len: length of in_buf
 * @out_buf: destination of decompressed output. Upon exit, the end of output.
 * @out_len: length of out_buf
 * @work_buf: workspace
 * @work_len: length of workspace
 */
int gunzip(uintptr_t *in_buf, size_t in_len, uintptr_t *out_buf,
	   size_t out_len, uintptr_t work_buf, size_t work_len)
{
	z_stream stream;
	int zret, ret;

	zalloc_start = work_buf;
	zalloc_end = work_buf + work_len;
	zalloc_current = zalloc_start;

	stream.next_in = (typeof(stream.next_in))*in_buf;
	stream.avail_in = in_len;
	stream.next_out = (typeof(stream.next_out))*out_buf;
	stream.avail_out = out_len;
	stream.zalloc = zcalloc;
	stream.zfree = zfree;
	stream.opaque = (voidpf)0;

	zret = inflateInit(&stream);
	if (zret != Z_OK) {
		ERROR("zlib: inflate init failed (ret = %d)\n", zret);
		return (zret == Z_MEM_ERROR) ? -ENOMEM : -EIO;
	}

	zret = inflate(&stream, Z_NO_FLUSH);
	if (zret == Z_STREAM_END) {
		ret = 0;
	} else {
		if (stream.msg)
			ERROR("%s\n", stream.msg);
		ERROR("zlib: inflate failed (ret = %d)\n", zret);
		ret = (zret == Z_MEM_ERROR) ? -ENOMEM : -EIO;
	}

	VERBOSE("zlib: %lu byte input\n", stream.total_in);
	VERBOSE("zlib: %lu byte output\n", stream.total_out);

	*in_buf = (uintptr_t)stream.next_in;
	*out_buf = (uintptr_t)stream.next_out;

	inflateEnd(&stream);

	return ret;
}
