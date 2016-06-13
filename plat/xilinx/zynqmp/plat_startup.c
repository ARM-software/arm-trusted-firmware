/*
 * Copyright (c) 2014-2016, ARM Limited and Contributors. All rights reserved.
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

#include <arch_helpers.h>
#include <assert.h>
#include <debug.h>
#include <mmio.h>
#include "zynqmp_def.h"

/*
 * ATFHandoffParams
 * Parameter		bitfield	encoding
 * -----------------------------------------------------------------------------
 * Exec State		0		0 -> Aarch64, 1-> Aarch32
 * endianness		1		0 -> LE, 1 -> BE
 * secure (TZ)		2		0 -> Non secure, 1 -> secure
 * EL			3:4		00 -> EL0, 01 -> EL1, 10 -> EL2, 11 -> EL3
 * CPU#			5:6		00 -> A53_0, 01 -> A53_1, 10 -> A53_2, 11 -> A53_3
 */

#define FSBL_FLAGS_ESTATE_SHIFT		0
#define FSBL_FLAGS_ESTATE_MASK		(1 << FSBL_FLAGS_ESTATE_SHIFT)
#define FSBL_FLAGS_ESTATE_A64		0
#define FSBL_FLAGS_ESTATE_A32		1

#define FSBL_FLAGS_ENDIAN_SHIFT		1
#define FSBL_FLAGS_ENDIAN_MASK		(1 << FSBL_FLAGS_ENDIAN_SHIFT)
#define FSBL_FLAGS_ENDIAN_LE		0
#define FSBL_FLAGS_ENDIAN_BE		1

#define FSBL_FLAGS_TZ_SHIFT		2
#define FSBL_FLAGS_TZ_MASK		(1 << FSBL_FLAGS_TZ_SHIFT)
#define FSBL_FLAGS_NON_SECURE		0
#define FSBL_FLAGS_SECURE		1

#define FSBL_FLAGS_EL_SHIFT		3
#define FSBL_FLAGS_EL_MASK		(3 << FSBL_FLAGS_EL_SHIFT)
#define FSBL_FLAGS_EL0			0
#define FSBL_FLAGS_EL1			1
#define FSBL_FLAGS_EL2			2
#define FSBL_FLAGS_EL3			3

#define FSBL_FLAGS_CPU_SHIFT		5
#define FSBL_FLAGS_CPU_MASK		(3 << FSBL_FLAGS_CPU_SHIFT)
#define FSBL_FLAGS_A53_0		0
#define FSBL_FLAGS_A53_1		1
#define FSBL_FLAGS_A53_2		2
#define FSBL_FLAGS_A53_3		3

#define FSBL_MAX_PARTITIONS		8

/* Structure corresponding to each partition entry */
struct xfsbl_partition {
	uint64_t entry_point;
	uint64_t flags;
};

/* Structure for handoff parameters to ARM Trusted Firmware (ATF) */
struct xfsbl_atf_handoff_params {
	uint8_t magic[4];
	uint32_t num_entries;
	struct xfsbl_partition partition[FSBL_MAX_PARTITIONS];
};

/**
 * @partition: Pointer to partition struct
 *
 * Get the target CPU for @partition.
 *
 * Return: FSBL_FLAGS_A53_0, FSBL_FLAGS_A53_1, FSBL_FLAGS_A53_2 or FSBL_FLAGS_A53_3
 */
static int get_fsbl_cpu(const struct xfsbl_partition *partition)
{
	uint64_t flags = partition->flags & FSBL_FLAGS_CPU_MASK;

	return flags >> FSBL_FLAGS_CPU_SHIFT;
}

/**
 * @partition: Pointer to partition struct
 *
 * Get the target exception level for @partition.
 *
 * Return: FSBL_FLAGS_EL0, FSBL_FLAGS_EL1, FSBL_FLAGS_EL2 or FSBL_FLAGS_EL3
 */
static int get_fsbl_el(const struct xfsbl_partition *partition)
{
	uint64_t flags = partition->flags & FSBL_FLAGS_EL_MASK;

	return flags >> FSBL_FLAGS_EL_SHIFT;
}

/**
 * @partition: Pointer to partition struct
 *
 * Get the target security state for @partition.
 *
 * Return: FSBL_FLAGS_NON_SECURE or FSBL_FLAGS_SECURE
 */
static int get_fsbl_ss(const struct xfsbl_partition *partition)
{
	uint64_t flags = partition->flags & FSBL_FLAGS_TZ_MASK;

	return flags >> FSBL_FLAGS_TZ_SHIFT;
}

/**
 * @partition: Pointer to partition struct
 *
 * Get the target endianness for @partition.
 *
 * Return: SPSR_E_LITTLE or SPSR_E_BIG
 */
static int get_fsbl_endian(const struct xfsbl_partition *partition)
{
	uint64_t flags = partition->flags & FSBL_FLAGS_ENDIAN_MASK;

	flags >>= FSBL_FLAGS_ENDIAN_SHIFT;

	if (flags == FSBL_FLAGS_ENDIAN_BE)
		return SPSR_E_BIG;
	else
		return SPSR_E_LITTLE;
}

/**
 * @partition: Pointer to partition struct
 *
 * Get the target execution state for @partition.
 *
 * Return: FSBL_FLAGS_ESTATE_A32 or FSBL_FLAGS_ESTATE_A64
 */
static int get_fsbl_estate(const struct xfsbl_partition *partition)
{
	uint64_t flags = partition->flags & FSBL_FLAGS_ESTATE_MASK;

	return flags >> FSBL_FLAGS_ESTATE_SHIFT;
}

/**
 * Populates the bl32 and bl33 image info structures
 * @bl32:	BL32 image info structure
 * @bl33:	BL33 image info structure
 *
 * Process the handoff paramters from the FSBL and populate the BL32 and BL33
 * image info structures accordingly.
 */
void fsbl_atf_handover(entry_point_info_t *bl32, entry_point_info_t *bl33)
{
	uint64_t atf_handoff_addr;
	const struct xfsbl_atf_handoff_params *ATFHandoffParams;

	atf_handoff_addr = mmio_read_32(PMU_GLOBAL_GEN_STORAGE6);
	assert((atf_handoff_addr < BL31_BASE) ||
	       (atf_handoff_addr > (uint64_t)&__BL31_END__));
	if (!atf_handoff_addr) {
		ERROR("BL31: No ATF handoff structure passed\n");
		panic();
	}

	ATFHandoffParams = (struct xfsbl_atf_handoff_params *)atf_handoff_addr;
	if ((ATFHandoffParams->magic[0] != 'X') ||
	    (ATFHandoffParams->magic[1] != 'L') ||
	    (ATFHandoffParams->magic[2] != 'N') ||
	    (ATFHandoffParams->magic[3] != 'X')) {
		ERROR("BL31: invalid ATF handoff structure at %lx\n",
		      atf_handoff_addr);
		panic();
	}

	VERBOSE("BL31: ATF handoff params at:0x%lx, entries:%u\n",
		atf_handoff_addr, ATFHandoffParams->num_entries);
	if (ATFHandoffParams->num_entries > FSBL_MAX_PARTITIONS) {
		ERROR("BL31: ATF handoff params: too many partitions (%u/%u)\n",
		      ATFHandoffParams->num_entries, FSBL_MAX_PARTITIONS);
		panic();
	}

	/*
	 * we loop over all passed entries but only populate two image structs
	 * (bl32, bl33). I.e. the last applicable images in the handoff
	 * structure will be used for the hand off
	 */
	for (size_t i = 0; i < ATFHandoffParams->num_entries; i++) {
		entry_point_info_t *image;
		int target_estate, target_secure;
		int target_cpu, target_endianness, target_el;

		VERBOSE("BL31: %zd: entry:0x%lx, flags:0x%lx\n", i,
			ATFHandoffParams->partition[i].entry_point,
			ATFHandoffParams->partition[i].flags);

		target_cpu = get_fsbl_cpu(&ATFHandoffParams->partition[i]);
		if (target_cpu != FSBL_FLAGS_A53_0) {
			WARN("BL31: invalid target CPU (%i)\n", target_cpu);
			continue;
		}

		target_el = get_fsbl_el(&ATFHandoffParams->partition[i]);
		if ((target_el == FSBL_FLAGS_EL3) ||
		    (target_el == FSBL_FLAGS_EL0)) {
			WARN("BL31: invalid exception level (%i)\n", target_el);
			continue;
		}

		target_secure = get_fsbl_ss(&ATFHandoffParams->partition[i]);
		if (target_secure == FSBL_FLAGS_SECURE &&
		    target_el == FSBL_FLAGS_EL2) {
			WARN("BL31: invalid security state (%i) for exception level (%i)\n",
			     target_secure, target_el);
			continue;
		}

		target_estate = get_fsbl_estate(&ATFHandoffParams->partition[i]);
		target_endianness = get_fsbl_endian(&ATFHandoffParams->partition[i]);

		if (target_secure == FSBL_FLAGS_SECURE) {
			image = bl32;

			if (target_estate == FSBL_FLAGS_ESTATE_A32)
				bl32->spsr = SPSR_MODE32(MODE32_svc, SPSR_T_ARM,
							 target_endianness,
							 DISABLE_ALL_EXCEPTIONS);
			else
				bl32->spsr = SPSR_64(MODE_EL1, MODE_SP_ELX,
						     DISABLE_ALL_EXCEPTIONS);
		} else {
			image = bl33;

			if (target_estate == FSBL_FLAGS_ESTATE_A32) {
				if (target_el == FSBL_FLAGS_EL2)
					target_el = MODE32_hyp;
				else
					target_el = MODE32_sys;

				bl33->spsr = SPSR_MODE32(target_el, SPSR_T_ARM,
							 target_endianness,
							 DISABLE_ALL_EXCEPTIONS);
			} else {
				if (target_el == FSBL_FLAGS_EL2)
					target_el = MODE_EL2;
				else
					target_el = MODE_EL1;

				bl33->spsr = SPSR_64(target_el, MODE_SP_ELX,
						     DISABLE_ALL_EXCEPTIONS);
			}
		}

		VERBOSE("Setting up %s entry point to:%lx, el:%x\n",
			target_secure == FSBL_FLAGS_SECURE ? "BL32" : "BL33",
			ATFHandoffParams->partition[i].entry_point,
			target_el);
		image->pc = ATFHandoffParams->partition[i].entry_point;

		if (target_endianness == SPSR_E_BIG)
			EP_SET_EE(image->h.attr, EP_EE_BIG);
		else
			EP_SET_EE(image->h.attr, EP_EE_LITTLE);
	}
}
