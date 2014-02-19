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

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include <arch_helpers.h>
#include <console.h>
#include <platform.h>
#include <semihosting.h>
#include <bl_common.h>
#include "io_storage.h"
#include "debug.h"

unsigned long page_align(unsigned long value, unsigned dir)
{
	unsigned long page_size = 1 << FOUR_KB_SHIFT;

	/* Round up the limit to the next page boundary */
	if (value & (page_size - 1)) {
		value &= ~(page_size - 1);
		if (dir == UP)
			value += page_size;
	}

	return value;
}

static inline unsigned int is_page_aligned (unsigned long addr) {
	const unsigned long page_size = 1 << FOUR_KB_SHIFT;

	return (addr & (page_size - 1)) == 0;
}

void change_security_state(unsigned int target_security_state)
{
	unsigned long scr = read_scr();

	if (target_security_state == SECURE)
		scr &= ~SCR_NS_BIT;
	else if (target_security_state == NON_SECURE)
		scr |= SCR_NS_BIT;
	else
		assert(0);

	write_scr(scr);
}

void __dead2 drop_el(aapcs64_params *args,
		     unsigned long spsr,
		     unsigned long entrypoint)
{
	write_spsr(spsr);
	write_elr(entrypoint);
	eret(args->arg0,
	     args->arg1,
	     args->arg2,
	     args->arg3,
	     args->arg4,
	     args->arg5,
	     args->arg6,
	     args->arg7);
}

void __dead2 raise_el(aapcs64_params *args)
{
	smc(args->arg0,
	    args->arg1,
	    args->arg2,
	    args->arg3,
	    args->arg4,
	    args->arg5,
	    args->arg6,
	    args->arg7);
}

/*
 * TODO: If we are not EL3 then currently we only issue an SMC.
 * Add support for dropping into EL0 etc. Consider adding support
 * for switching from S-EL1 to S-EL0/1 etc.
 */
void __dead2 change_el(el_change_info *info)
{
	unsigned long current_el = read_current_el();

	if (GET_EL(current_el) == MODE_EL3) {
		/*
		 * We can go anywhere from EL3. So find where.
		 * TODO: Lots to do if we are going non-secure.
		 * Flip the NS bit. Restore NS registers etc.
		 * Just doing the bare minimal for now.
		 */

		if (info->security_state == NON_SECURE)
			change_security_state(info->security_state);

		drop_el(&info->args, info->spsr, info->entrypoint);
	} else
		raise_el(&info->args);
}

/* TODO: add a parameter for DAIF. not needed right now */
unsigned long make_spsr(unsigned long target_el,
			unsigned long target_sp,
			unsigned long target_rw)
{
	unsigned long spsr;

	/* Disable all exceptions & setup the EL */
	spsr = (DAIF_FIQ_BIT | DAIF_IRQ_BIT | DAIF_ABT_BIT | DAIF_DBG_BIT)
		<< PSR_DAIF_SHIFT;
	spsr |= PSR_MODE(target_rw, target_el, target_sp);

	return spsr;
}

/*******************************************************************************
 * The next two functions are the weak definitions. Platform specific
 * code can override them if it wishes to.
 ******************************************************************************/

/*******************************************************************************
 * Function that takes a memory layout into which BL31 has been either top or
 * bottom loaded. Using this information, it populates bl31_mem_layout to tell
 * BL31 how much memory it has access to and how much is available for use. It
 * does not need the address where BL31 has been loaded as BL31 will reclaim
 * all the memory used by BL2.
 * TODO: Revisit if this and init_bl2_mem_layout can be replaced by a single
 * routine.
 ******************************************************************************/
void init_bl31_mem_layout(const meminfo *bl2_mem_layout,
			  meminfo *bl31_mem_layout,
			  unsigned int load_type)
{
	if (load_type == BOT_LOAD) {
		/*
		 * ------------                             ^
		 * |   BL2    |                             |
		 * |----------|                 ^           |  BL2
		 * |          |                 | BL2 free  |  total
		 * |          |                 |   size    |  size
		 * |----------| BL2 free base   v           |
		 * |   BL31   |                             |
		 * ------------ BL2 total base              v
		 */
		unsigned long bl31_size;

		bl31_mem_layout->free_base = bl2_mem_layout->free_base;

		bl31_size = bl2_mem_layout->free_base - bl2_mem_layout->total_base;
		bl31_mem_layout->free_size = bl2_mem_layout->total_size - bl31_size;
	} else {
		/*
		 * ------------                             ^
		 * |   BL31   |                             |
		 * |----------|                 ^           |  BL2
		 * |          |                 | BL2 free  |  total
		 * |          |                 |   size    |  size
		 * |----------| BL2 free base   v           |
		 * |   BL2    |                             |
		 * ------------ BL2 total base              v
		 */
		unsigned long bl2_size;

		bl31_mem_layout->free_base = bl2_mem_layout->total_base;

		bl2_size = bl2_mem_layout->free_base - bl2_mem_layout->total_base;
		bl31_mem_layout->free_size = bl2_mem_layout->free_size + bl2_size;
	}

	bl31_mem_layout->total_base = bl2_mem_layout->total_base;
	bl31_mem_layout->total_size = bl2_mem_layout->total_size;
	bl31_mem_layout->attr = load_type;

	flush_dcache_range((unsigned long) bl31_mem_layout, sizeof(meminfo));
	return;
}

/*******************************************************************************
 * Function that takes a memory layout into which BL2 has been either top or
 * bottom loaded along with the address where BL2 has been loaded in it. Using
 * this information, it populates bl2_mem_layout to tell BL2 how much memory
 * it has access to and how much is available for use.
 ******************************************************************************/
void init_bl2_mem_layout(meminfo *bl1_mem_layout,
			 meminfo *bl2_mem_layout,
			 unsigned int load_type,
			 unsigned long bl2_base)
{
	unsigned tmp;

	if (load_type == BOT_LOAD) {
		bl2_mem_layout->total_base = bl2_base;
		tmp = bl1_mem_layout->free_base - bl2_base;
		bl2_mem_layout->total_size = bl1_mem_layout->free_size + tmp;

	} else {
		bl2_mem_layout->total_base = bl1_mem_layout->free_base;
		tmp = bl1_mem_layout->total_base + bl1_mem_layout->total_size;
		bl2_mem_layout->total_size = tmp - bl1_mem_layout->free_base;
	}

	bl2_mem_layout->free_base = bl1_mem_layout->free_base;
	bl2_mem_layout->free_size = bl1_mem_layout->free_size;
	bl2_mem_layout->attr = load_type;

	flush_dcache_range((unsigned long) bl2_mem_layout, sizeof(meminfo));
	return;
}

static void dump_load_info(unsigned long image_load_addr,
			   unsigned long image_size,
			   const meminfo *mem_layout)
{
#if DEBUG
	printf("Trying to load image at address 0x%lx, size = 0x%lx\r\n",
		image_load_addr, image_size);
	printf("Current memory layout:\r\n");
	printf("  total region = [0x%lx, 0x%lx]\r\n", mem_layout->total_base,
			mem_layout->total_base + mem_layout->total_size);
	printf("  free region = [0x%lx, 0x%lx]\r\n", mem_layout->free_base,
			mem_layout->free_base + mem_layout->free_size);
#endif
}

/*******************************************************************************
 * Generic function to load an image into the trusted RAM,
 * given a name, extents of free memory & whether the image should be loaded at
 * the bottom or top of the free memory. It updates the memory layout if the
 * load is successful.
 ******************************************************************************/
unsigned long load_image(meminfo *mem_layout,
			 const char *image_name,
			 unsigned int load_type,
			 unsigned long fixed_addr)
{
	io_dev_handle dev_handle;
	io_handle image_handle;
	void *image_spec;
	unsigned long temp_image_base = 0;
	unsigned long image_base = 0;
	long offset = 0;
	size_t image_size = 0;
	size_t bytes_read = 0;
	int io_result = IO_FAIL;

	assert(mem_layout != NULL);
	assert(image_name != NULL);

	/* Obtain a reference to the image by querying the platform layer */
	io_result = plat_get_image_source(image_name, &dev_handle, &image_spec);
	if (io_result != IO_SUCCESS) {
		ERROR("Failed to obtain reference to image '%s' (%i)\n",
			image_name, io_result);
		return 0;
	}

	/* Attempt to access the image */
	io_result = io_open(dev_handle, image_spec, &image_handle);
	if (io_result != IO_SUCCESS) {
		ERROR("Failed to access image '%s' (%i)\n",
			image_name, io_result);
		return 0;
	}

	/* Find the size of the image */
	io_result = io_size(image_handle, &image_size);
	if ((io_result != IO_SUCCESS) || (image_size == 0)) {
		ERROR("Failed to determine the size of the image '%s' file (%i)\n",
			image_name, io_result);
		goto fail;
	}

	/* See if we have enough space */
	if (image_size > mem_layout->free_size) {
		ERROR("ERROR: Cannot load '%s' file: Not enough space.\n",
			image_name);
		dump_load_info(0, image_size, mem_layout);
		goto fail;
	}

	switch (load_type) {

	case TOP_LOAD:

	  /* Load the image in the top of free memory */
	  temp_image_base = mem_layout->free_base + mem_layout->free_size;
	  temp_image_base -= image_size;

	  /* Page align base address and check whether the image still fits */
	  image_base = page_align(temp_image_base, DOWN);
	  assert(image_base <= temp_image_base);

	  if (image_base < mem_layout->free_base) {
		ERROR("Cannot load '%s' file: Not enough space.\n",
			image_name);
		dump_load_info(image_base, image_size, mem_layout);
		goto fail;
	  }

	  /* Calculate the amount of extra memory used due to alignment */
	  offset = temp_image_base - image_base;

	  break;

	case BOT_LOAD:

	  /* Load the BL2 image in the bottom of free memory */
	  temp_image_base = mem_layout->free_base;
	  image_base = page_align(temp_image_base, UP);
	  assert(image_base >= temp_image_base);

	  /* Page align base address and check whether the image still fits */
	  if (image_base + image_size >
	      mem_layout->free_base + mem_layout->free_size) {
		  ERROR("Cannot load '%s' file: Not enough space.\n",
			  image_name);
		  dump_load_info(image_base, image_size, mem_layout);
		  goto fail;
	  }

	  /* Calculate the amount of extra memory used due to alignment */
	  offset = image_base - temp_image_base;

	  break;

	default:
	  assert(0);

	}

	/*
	 * Some images must be loaded at a fixed address, not a dynamic one.
	 *
	 * This has been implemented as a hack on top of the existing dynamic
	 * loading mechanism, for the time being.  If the 'fixed_addr' function
	 * argument is different from zero, then it will force the load address.
	 * So we still have this principle of top/bottom loading but the code
	 * determining the load address is bypassed and the load address is
	 * forced to the fixed one.
	 *
	 * This can result in quite a lot of wasted space because we still use
	 * 1 sole meminfo structure to represent the extents of free memory,
	 * where we should use some sort of linked list.
	 *
	 * E.g. we want to load BL2 at address 0x04020000, the resulting memory
	 *      layout should look as follows:
	 * ------------ 0x04040000
	 * |          |  <- Free space (1)
	 * |----------|
	 * |   BL2    |
	 * |----------| 0x04020000
	 * |          |  <- Free space (2)
	 * |----------|
	 * |   BL1    |
	 * ------------ 0x04000000
	 *
	 * But in the current hacky implementation, we'll need to specify
	 * whether BL2 is loaded at the top or bottom of the free memory.
	 * E.g. if BL2 is considered as top-loaded, the meminfo structure
	 * will give the following view of the memory, hiding the chunk of
	 * free memory above BL2:
	 * ------------ 0x04040000
	 * |          |
	 * |          |
	 * |   BL2    |
	 * |----------| 0x04020000
	 * |          |  <- Free space (2)
	 * |----------|
	 * |   BL1    |
	 * ------------ 0x04000000
	 */
	if (fixed_addr != 0) {
		/* Load the image at the given address. */
		image_base = fixed_addr;

		/* Check whether the image fits. */
		if ((image_base < mem_layout->free_base) ||
		    (image_base + image_size >
		       mem_layout->free_base + mem_layout->free_size)) {
			ERROR("Cannot load '%s' file: Not enough space.\n",
				image_name);
			dump_load_info(image_base, image_size, mem_layout);
			goto fail;
		}

		/* Check whether the fixed load address is page-aligned. */
		if (!is_page_aligned(image_base)) {
			ERROR("Cannot load '%s' file at unaligned address 0x%lx\n",
				image_name, fixed_addr);
			goto fail;
		}

		/*
		 * Calculate the amount of extra memory used due to fixed
		 * loading.
		 */
		if (load_type == TOP_LOAD) {
			unsigned long max_addr, space_used;
			/*
			 * ------------ max_addr
			 * | /wasted/ |                 | offset
			 * |..........|..............................
			 * |  image   |                 | image_flen
			 * |----------| fixed_addr
			 * |          |
			 * |          |
			 * ------------ total_base
			 */
			max_addr = mem_layout->total_base + mem_layout->total_size;
			/*
			 * Compute the amount of memory used by the image.
			 * Corresponds to all space above the image load
			 * address.
			 */
			space_used = max_addr - fixed_addr;
			/*
			 * Calculate the amount of wasted memory within the
			 * amount of memory used by the image.
			 */
			offset = space_used - image_size;
		} else /* BOT_LOAD */
			/*
			 * ------------
			 * |          |
			 * |          |
			 * |----------|
			 * |  image   |
			 * |..........| fixed_addr
			 * | /wasted/ |                 | offset
			 * ------------ total_base
			 */
			offset = fixed_addr - mem_layout->total_base;
	}

	/* We have enough space so load the image now */
	/* TODO: Consider whether to try to recover/retry a partially successful read */
	io_result = io_read(image_handle, (void *)image_base, image_size, &bytes_read);
	if ((io_result != IO_SUCCESS) || (bytes_read < image_size)) {
		ERROR("Failed to load '%s' file (%i)\n", image_name, io_result);
		goto fail;
	}

	/*
	 * File has been successfully loaded. Update the free memory
	 * data structure & flush the contents of the TZRAM so that
	 * the next EL can see it.
	 */
	/* Update the memory contents */
	flush_dcache_range(image_base, image_size);

	mem_layout->free_size -= image_size + offset;

	/* Update the base of free memory since its moved up */
	if (load_type == BOT_LOAD)
		mem_layout->free_base += offset + image_size;

exit:
	io_result = io_close(image_handle);
	/* Ignore improbable/unrecoverable error in 'close' */

	/* TODO: Consider maintaining open device connection from this bootloader stage */
	io_result = io_dev_close(dev_handle);
	/* Ignore improbable/unrecoverable error in 'dev_close' */

	return image_base;

fail:	image_base = 0;
	goto exit;
}

/*******************************************************************************
 * Run a loaded image from the given entry point. This could result in either
 * dropping into a lower exception level or jumping to a higher exception level.
 * The only way of doing the latter is through an SMC. In either case, setup the
 * parameters for the EL change request correctly.
 ******************************************************************************/
void __dead2 run_image(unsigned long entrypoint,
		       unsigned long spsr,
		       unsigned long target_security_state,
		       void *first_arg,
		       void *second_arg)
{
	el_change_info run_image_info;
	unsigned long current_el = read_current_el();

	/* Tell next EL what we want done */
	run_image_info.args.arg0 = RUN_IMAGE;
	run_image_info.entrypoint = entrypoint;
	run_image_info.spsr = spsr;
	run_image_info.security_state = target_security_state;

	/*
	 * If we are EL3 then only an eret can take us to the desired
	 * exception level. Else for the time being assume that we have
	 * to jump to a higher EL and issue an SMC. Contents of argY
	 * will go into the general purpose register xY e.g. arg0->x0
	 */
	if (GET_EL(current_el) == MODE_EL3) {
		run_image_info.args.arg1 = (unsigned long) first_arg;
		run_image_info.args.arg2 = (unsigned long) second_arg;
	} else {
		run_image_info.args.arg1 = entrypoint;
		run_image_info.args.arg2 = spsr;
		run_image_info.args.arg3 = (unsigned long) first_arg;
		run_image_info.args.arg4 = (unsigned long) second_arg;
	}

	change_el(&run_image_info);
}
