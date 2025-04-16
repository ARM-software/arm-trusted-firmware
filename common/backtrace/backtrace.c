/*
 * Copyright (c) 2018-2025, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>

#include <arch_features.h>
#include <arch_helpers.h>
#include <common/debug.h>
#include <drivers/console.h>

/* Maximum number of entries in the backtrace to display */
#define UNWIND_LIMIT	20U

/*
 * If -fno-omit-frame-pointer is used:
 *
 * - AArch64: The AAPCS defines the format of the frame records and mandates the
 *   usage of r29 as frame pointer.
 *
 * - AArch32: The format of the frame records is not defined in the AAPCS.
 *   However, at least GCC and Clang use the same format. When they are forced
 *   to only generate A32 code (with -marm), they use r11 as frame pointer and a
 *   similar format as in AArch64. If interworking with T32 is enabled, the
 *   frame pointer is r7 and the format is  different. This is not supported by
 *   this implementation of backtrace, so it is needed to use -marm.
 */

/* Frame records form a linked list in the stack */
struct frame_record {
	/* Previous frame record in the list */
	struct frame_record *parent;
	/* Return address of the function at this level */
	uintptr_t return_addr;
};

static inline uintptr_t extract_address(uintptr_t address)
{
	uintptr_t ret = address;

	/*
	 * When pointer authentication is enabled, the LR value saved on the
	 * stack contains a PAC. It must be stripped to retrieve the return
	 * address.
	 */
	if (is_feat_pauth_supported()) {
		ret = xpaci(address);
	}

	return ret;
}

/*
 * Returns true if the address points to a virtual address that can be read at
 * the current EL, false otherwise.
 */
#ifdef __aarch64__
static bool is_address_readable(uintptr_t address)
{
	unsigned int el = get_current_el();
	uintptr_t addr = extract_address(address);

	if (el == 3U) {
		ats1e3r(addr);
	} else if (el == 2U) {
		ats1e2r(addr);
	} else {
		AT(ats1e1r, addr);
	}

	isb();

	/* If PAR.F == 1 the address translation was aborted.  */
	if ((read_par_el1() & PAR_F_MASK) != 0U)
		return false;

	return true;
}
#else /* !__aarch64__ */
static bool is_address_readable(uintptr_t addr)
{
	unsigned int el = get_current_el();

	if (el == 3U) {
		write_ats1cpr(addr);
	} else if (el == 2U) {
		write_ats1hr(addr);
	} else {
		write_ats1cpr(addr);
	}

	isb();

	/* If PAR.F == 1 the address translation was aborted.  */
	if ((read64_par() & PAR_F_MASK) != 0U)
		return false;

	return true;
}
#endif /* __aarch64__ */

/*
 * Returns true if all the bytes in a given object are in mapped memory and an
 * LDR using this pointer would succeed, false otherwise.
 */
static bool is_valid_object(uintptr_t addr, size_t size)
{
	assert(size > 0U);

	if (addr == 0U)
		return false;

	/* Detect overflows */
	if ((addr + size) < addr)
		return false;

	/* A pointer not aligned properly could trigger an alignment fault. */
	if ((addr & (sizeof(uintptr_t) - 1U)) != 0U)
		return false;

	/* Check that all the object is readable */
	for (size_t i = 0; i < size; i++) {
		if (!is_address_readable(addr + i))
			return false;
	}

	return true;
}

/*
 * Returns true if the specified address is correctly aligned and points to a
 * valid memory region.
 */
static bool is_valid_jump_address(uintptr_t addr)
{
	if (addr == 0U)
		return false;

	/* Check alignment. Both A64 and A32 use 32-bit opcodes */
	if ((addr & (sizeof(uint32_t) - 1U)) != 0U)
		return false;

	if (!is_address_readable(addr))
		return false;

	return true;
}

/*
 * Returns true if the pointer points at a valid frame record, false otherwise.
 */
static bool is_valid_frame_record(struct frame_record *fr)
{
	return is_valid_object((uintptr_t)fr, sizeof(struct frame_record));
}

/*
 * Adjust the frame-pointer-register value by 4 bytes on AArch32 to have the
 * same layout as AArch64.
 */
static struct frame_record *adjust_frame_record(struct frame_record *fr)
{
#ifdef __aarch64__
	return fr;
#else
	return (struct frame_record *)((uintptr_t)fr - 4U);
#endif
}

static void unwind_stack(struct frame_record *fr, uintptr_t current_pc,
			 uintptr_t link_register)
{
	uintptr_t call_site;
	static const char *backtrace_str = "%u: %s: 0x%lx\n";
	const char *el_str = get_el_str(get_current_el());

	if (!is_valid_frame_record(fr)) {
		printf("ERROR: Corrupted frame pointer (frame record address = %p)\n",
		       fr);
		return;
	}

	call_site = extract_address(fr->return_addr);
	if (call_site != link_register) {
		printf("ERROR: Corrupted stack (frame record address = %p)\n",
		       fr);
		return;
	}

	/* The level 0 of the backtrace is the current backtrace function */
	printf(backtrace_str, 0U, el_str, current_pc);

	/*
	 * The last frame record pointer in the linked list at the beginning of
	 * the stack should be NULL unless stack is corrupted.
	 */
	for (unsigned int i = 1U; i < UNWIND_LIMIT; i++) {
		/* If an invalid frame record is found, exit. */
		if (!is_valid_frame_record(fr))
			return;
		/*
		 * A32 and A64 are fixed length so the address from where the
		 * call was made is the instruction before the return address,
		 * which is always 4 bytes before it.
		 */

		call_site = extract_address(fr->return_addr) - 4U;

		/*
		 * If the address is invalid it means that the frame record is
		 * probably corrupted.
		 */
		if (!is_valid_jump_address(call_site))
			return;

		printf(backtrace_str, i, el_str, call_site);

		fr = adjust_frame_record(fr->parent);
	}

	printf("ERROR: Max backtrace depth reached\n");
}

/*
 * Display a backtrace. The cookie string parameter is displayed along the
 * trace to help filter the log messages.
 *
 * Many things can prevent displaying the expected backtrace. For example,
 * compiler optimizations can use a branch instead of branch with link when it
 * detects a tail call. The backtrace level for this caller will not be
 * displayed, as it does not appear in the call stack anymore. Also, assembly
 * functions will not be displayed unless they setup AAPCS compliant frame
 * records on AArch64 and compliant with GCC-specific frame record format on
 * AArch32.
 *
 * Usage of the trace: addr2line can be used to map the addresses to function
 * and source code location when given the ELF file compiled with debug
 * information. The "-i" flag is highly recommended to improve display of
 * inlined function. The *.dump files generated when building each image can
 * also be used.
 *
 * WARNING: In case of corrupted stack, this function could display security
 * sensitive information past the beginning of the stack so it must not be used
 * in production build. This function is only compiled in when ENABLE_BACKTRACE
 * is set to 1.
 */
void backtrace(const char *cookie)
{
	uintptr_t return_address = (uintptr_t)__builtin_return_address(0U);
	struct frame_record *fr = __builtin_frame_address(0U);

	/* Printing the backtrace may crash the system, flush before starting */
	console_flush();

	fr = adjust_frame_record(fr);

	printf("BACKTRACE: START: %s\n", cookie);

	unwind_stack(fr, (uintptr_t)&backtrace, return_address);

	printf("BACKTRACE: END: %s\n", cookie);
}
