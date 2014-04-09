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

#include <arch.h>
#include <assert.h>

/*******************************************************************************
 * Helper to create a level 1/2 table descriptor which points to a level 2/3
 * table.
 ******************************************************************************/
unsigned long create_table_desc(unsigned long *next_table_ptr)
{
	unsigned long desc = (unsigned long) next_table_ptr;

	/* Clear the last 12 bits */
	desc >>= FOUR_KB_SHIFT;
	desc <<= FOUR_KB_SHIFT;

	desc |= TABLE_DESC;

	return desc;
}

/*******************************************************************************
 * Helper to create a level 1/2/3 block descriptor which maps the va to addr
 ******************************************************************************/
unsigned long create_block_desc(unsigned long desc,
				unsigned long addr,
				unsigned int level)
{
	switch (level) {
	case LEVEL1:
		desc |= (addr << FIRST_LEVEL_DESC_N) | BLOCK_DESC;
		break;
	case LEVEL2:
		desc |= (addr << SECOND_LEVEL_DESC_N) | BLOCK_DESC;
		break;
	case LEVEL3:
		desc |= (addr << THIRD_LEVEL_DESC_N) | TABLE_DESC;
		break;
	default:
		assert(0);
	}

	return desc;
}

/*******************************************************************************
 * Helper to create a level 1/2/3 block descriptor which maps the va to output_
 * addr with Device nGnRE attributes.
 ******************************************************************************/
unsigned long create_device_block(unsigned long output_addr,
				  unsigned int level,
				  unsigned int ns)
{
	unsigned long upper_attrs, lower_attrs, desc;

	lower_attrs = LOWER_ATTRS(ACCESS_FLAG | OSH | AP_RW);
	lower_attrs |= LOWER_ATTRS(ns | ATTR_DEVICE_INDEX);
	upper_attrs = UPPER_ATTRS(XN);
	desc = upper_attrs | lower_attrs;

	return create_block_desc(desc, output_addr, level);
}

/*******************************************************************************
 * Helper to create a level 1/2/3 block descriptor which maps the va to output_
 * addr with inner-shareable normal wbwa read-only memory attributes.
 ******************************************************************************/
unsigned long create_romem_block(unsigned long output_addr,
				 unsigned int level,
				 unsigned int ns)
{
	unsigned long upper_attrs, lower_attrs, desc;

	lower_attrs = LOWER_ATTRS(ACCESS_FLAG | ISH | AP_RO);
	lower_attrs |= LOWER_ATTRS(ns | ATTR_IWBWA_OWBWA_NTR_INDEX);
	upper_attrs = UPPER_ATTRS(0ull);
	desc = upper_attrs | lower_attrs;

	return create_block_desc(desc, output_addr, level);
}

/*******************************************************************************
 * Helper to create a level 1/2/3 block descriptor which maps the va to output_
 * addr with inner-shareable normal wbwa read-write memory attributes.
 ******************************************************************************/
unsigned long create_rwmem_block(unsigned long output_addr,
				 unsigned int level,
				 unsigned int ns)
{
	unsigned long upper_attrs, lower_attrs, desc;

	lower_attrs = LOWER_ATTRS(ACCESS_FLAG | ISH | AP_RW);
	lower_attrs |= LOWER_ATTRS(ns | ATTR_IWBWA_OWBWA_NTR_INDEX);
	upper_attrs = UPPER_ATTRS(XN);
	desc = upper_attrs | lower_attrs;

	return create_block_desc(desc, output_addr, level);
}
