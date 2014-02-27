/*
 * Copyright (c) 2014, ARM Limited and Contributors. All rights reserved.
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

#include <stdint.h>

/*
 * Flags for building up memory mapping attributes.
 * These are organised so that a clear bit gives a more restrictive  mapping
 * that a set bit, that way a bitwise-and two sets of attributes will never give
 * an attribute which has greater access rights that any of the original
 * attributes.
 */
typedef enum  {
	MT_DEVICE	= 0 << 0,
	MT_MEMORY	= 1 << 0,

	MT_RO		= 0 << 1,
	MT_RW		= 1 << 1,

	MT_SECURE	= 0 << 2,
	MT_NS		= 1 << 2
} mmap_attr;

/*
 * Structure for specifying a single region of memory.
 */
typedef struct {
	unsigned long	base;
	unsigned long	size;
	mmap_attr	attr;
} mmap_region;

extern void mmap_add_region(unsigned long base, unsigned long size,
				unsigned attr);
extern void mmap_add(const mmap_region *mm);

extern void init_xlat_tables(void);

extern uint64_t l1_xlation_table[];
