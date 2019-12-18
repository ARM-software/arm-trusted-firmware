/*
 * Copyright (c) 2017 - 2020, Broadcom
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <string.h>

#include <arch_helpers.h>
#include <common/debug.h>

/* MCU binary image structure: <header> <data>
 *
 * Header structure:
 * <magic-start>
 * <num-sections>
 * {<src-offset> <src-size> <dst-addr>}*
 * <magic-end>
 *
 * MCU data (<data>) consists of several sections of code/data, to be
 *             installed (copied) into MCU memories.
 * Header (<header>) gives information about sections contained in <data>.
 *
 * The installer code iterates over sections in MCU binary.
 * For each section, it copies the section into MCU memory.
 *
 * The header contains:
 *	 - <magic-start> - 32-bit magic number to mark header start
 *	 - <num-sections> - number of sections in <data>
 *	 - <num-sections> tuples. Each tuple describes a section.
 *			 A tuple contains three 32-bit words.
 *	 - <magic-end> - 32-bit magic number to mark header end
 *
 * Each section is describes by a tuple, consisting of three 32-bit words:
 *	 - offset of section within MCU binary (relative to beginning of <data>)
 *	 - section size (in bytes) in MCU binary
 *	 - target address (in MCU memory). Section is copied to this location.
 *
 * All fields are 32-bit unsigned integers in little endian format.
 * All sizes are assumed to be 32-bit aligned.
 */

#define SCP_BIN_HEADER_MAGIC_START 0xfa587D01
#define SCP_BIN_HEADER_MAGIC_END 0xf3e06a85

int download_scp_patch(void *image, unsigned int image_size)
{
	unsigned int *pheader = (unsigned int *)(image);
	unsigned int header_size;
	unsigned char *pdata;
	void *dest;
	unsigned int num_sections;
	unsigned int section_src_offset;
	unsigned int section_size;

	if (pheader && (pheader[0] != SCP_BIN_HEADER_MAGIC_START)) {
		ERROR("SCP: Could not find SCP header.\n");
		return -1;
	}

	num_sections = pheader[1];
	INFO("...Number of sections: %d\n", num_sections);
	header_size = 4 * (1 + 1 + 3 * num_sections + 1);

	if (image_size < header_size) {
		ERROR("SCP: Wrong size.\n");
		return -1;
	}

	if (*(pheader + header_size/4 - 1) != SCP_BIN_HEADER_MAGIC_END) {
		ERROR("SCP: Could not find SCP footer.\n");
		return -1;
	}

	VERBOSE("SCP image header validated successfully\n");
	pdata = (unsigned char *)pheader + header_size;

	for (pheader += 2; num_sections > 0; num_sections--) {

		section_src_offset = pheader[0];
		section_size = pheader[1];
		dest = (void *)(unsigned long)pheader[2];

		INFO("section: src:0x%x, size:%d, dst:0x%x\n",
				section_src_offset, section_size, pheader[2]);

		if ((section_src_offset + section_size) > image_size) {
			ERROR("SCP: Section points to outside of patch.\n");
			return -1;
		}

		/* copy from source to target section */
		memcpy(dest, pdata + section_src_offset, section_size);
		flush_dcache_range((uintptr_t)dest, section_size);

		/* next section */
		pheader += 3;
	}
	return 0;
}
