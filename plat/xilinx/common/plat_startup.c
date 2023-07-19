/*
 * Copyright (c) 2014-2020, Arm Limited and Contributors. All rights reserved.
 * Copyright (c) 2023, Advanced Micro Devices, Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <inttypes.h>
#include <stdint.h>

#include <arch_helpers.h>
#include <common/debug.h>
#include <plat_startup.h>


/*
 * HandoffParams
 * Parameter		bitfield	encoding
 * -----------------------------------------------------------------------------
 * Exec State		0		0 -> Aarch64, 1-> Aarch32
 * endianness		1		0 -> LE, 1 -> BE
 * secure (TZ)		2		0 -> Non secure, 1 -> secure
 * EL			3:4		00 -> EL0, 01 -> EL1, 10 -> EL2, 11 -> EL3
 * CPU#			5:6		00 -> A53_0, 01 -> A53_1, 10 -> A53_2, 11 -> A53_3
 * Reserved		7:10		Reserved
 * Cluster#		11:12		00 -> Cluster 0, 01 -> Cluster 1, 10 -> Cluster 2,
 *					11 -> Cluster (Applicable for Versal NET only).
 * Reserved		13:16		Reserved
 */

#define XBL_FLAGS_ESTATE_SHIFT		0U
#define XBL_FLAGS_ESTATE_MASK		(1U << XBL_FLAGS_ESTATE_SHIFT)
#define XBL_FLAGS_ESTATE_A64		0U
#define XBL_FLAGS_ESTATE_A32		1U

#define XBL_FLAGS_ENDIAN_SHIFT		1U
#define XBL_FLAGS_ENDIAN_MASK		(1U << XBL_FLAGS_ENDIAN_SHIFT)
#define XBL_FLAGS_ENDIAN_LE		0U
#define XBL_FLAGS_ENDIAN_BE		1U

#define XBL_FLAGS_TZ_SHIFT		2U
#define XBL_FLAGS_TZ_MASK		(1U << XBL_FLAGS_TZ_SHIFT)
#define XBL_FLAGS_NON_SECURE		0U
#define XBL_FLAGS_SECURE		1U

#define XBL_FLAGS_EL_SHIFT		3U
#define XBL_FLAGS_EL_MASK		(3U << XBL_FLAGS_EL_SHIFT)
#define XBL_FLAGS_EL0			0U
#define XBL_FLAGS_EL1			1U
#define XBL_FLAGS_EL2			2U
#define XBL_FLAGS_EL3			3U

#define XBL_FLAGS_CPU_SHIFT		5U
#define XBL_FLAGS_CPU_MASK		(3U << XBL_FLAGS_CPU_SHIFT)
#define XBL_FLAGS_A53_0		0U
#define XBL_FLAGS_A53_1		1U
#define XBL_FLAGS_A53_2		2U
#define XBL_FLAGS_A53_3		3U

#if defined(PLAT_versal_net)
#define XBL_FLAGS_CLUSTER_SHIFT		11U
#define XBL_FLAGS_CLUSTER_MASK		GENMASK(11, 12)

#define XBL_FLAGS_CLUSTER_0		0U
#endif /* PLAT_versal_net */

/**
 * get_xbl_cpu() - Get the target CPU for partition.
 * @partition: Pointer to partition struct.
 *
 * Return: XBL_FLAGS_A53_0, XBL_FLAGS_A53_1, XBL_FLAGS_A53_2 or XBL_FLAGS_A53_3.
 *
 */
static int32_t get_xbl_cpu(const struct xbl_partition *partition)
{
	uint64_t flags = partition->flags & XBL_FLAGS_CPU_MASK;

	return flags >> XBL_FLAGS_CPU_SHIFT;
}

/**
 * get_xbl_el() - Get the target exception level for partition.
 * @partition: Pointer to partition struct.
 *
 * Return: XBL_FLAGS_EL0, XBL_FLAGS_EL1, XBL_FLAGS_EL2 or XBL_FLAGS_EL3.
 *
 */
static int32_t get_xbl_el(const struct xbl_partition *partition)
{
	uint64_t flags = partition->flags & XBL_FLAGS_EL_MASK;

	return flags >> XBL_FLAGS_EL_SHIFT;
}

/**
 * get_xbl_ss() - Get the target security state for partition.
 * @partition: Pointer to partition struct.
 *
 * Return: XBL_FLAGS_NON_SECURE or XBL_FLAGS_SECURE.
 *
 */
static int32_t get_xbl_ss(const struct xbl_partition *partition)
{
	uint64_t flags = partition->flags & XBL_FLAGS_TZ_MASK;

	return flags >> XBL_FLAGS_TZ_SHIFT;
}

/**
 * get_xbl_endian() - Get the target endianness for partition.
 * @partition: Pointer to partition struct.
 *
 * Return: SPSR_E_LITTLE or SPSR_E_BIG.
 *
 */
static int32_t get_xbl_endian(const struct xbl_partition *partition)
{
	uint64_t flags = partition->flags & XBL_FLAGS_ENDIAN_MASK;

	flags >>= XBL_FLAGS_ENDIAN_SHIFT;

	if (flags == XBL_FLAGS_ENDIAN_BE) {
		return SPSR_E_BIG;
	} else {
		return SPSR_E_LITTLE;
	}
}

/**
 * get_xbl_estate() - Get the target execution state for partition.
 * @partition: Pointer to partition struct.
 *
 * Return: XBL_FLAGS_ESTATE_A32 or XBL_FLAGS_ESTATE_A64.
 *
 */
static int32_t get_xbl_estate(const struct xbl_partition *partition)
{
	uint64_t flags = partition->flags & XBL_FLAGS_ESTATE_MASK;

	return flags >> XBL_FLAGS_ESTATE_SHIFT;
}

#if defined(PLAT_versal_net)
/**
 * get_xbl_cluster - Get the cluster number
 * @partition: pointer to the partition structure.
 *
 * Return: cluster number for the partition.
 */
static int32_t get_xbl_cluster(const struct xbl_partition *partition)
{
	uint64_t flags = partition->flags & XBL_FLAGS_CLUSTER_MASK;

	return (int32_t)(flags >> XBL_FLAGS_CLUSTER_SHIFT);
}
#endif /* PLAT_versal_net */

/**
 * xbl_handover() - Populates the bl32 and bl33 image info structures.
 * @bl32: BL32 image info structure.
 * @bl33: BL33 image info structure.
 * @handoff_addr: TF-A handoff address.
 *
 * Process the handoff parameters from the XBL and populate the BL32 and BL33
 * image info structures accordingly.
 *
 * Return: Return the status of the handoff. The value will be from the
 *         xbl_handoff enum.
 *
 */
enum xbl_handoff xbl_handover(entry_point_info_t *bl32,
					entry_point_info_t *bl33,
					uint64_t handoff_addr)
{
	const struct xbl_handoff_params *HandoffParams;

	if (!handoff_addr) {
		WARN("BL31: No handoff structure passed\n");
		return XBL_HANDOFF_NO_STRUCT;
	}

	HandoffParams = (struct xbl_handoff_params *)handoff_addr;
	if ((HandoffParams->magic[0] != 'X') ||
	    (HandoffParams->magic[1] != 'L') ||
	    (HandoffParams->magic[2] != 'N') ||
	    (HandoffParams->magic[3] != 'X')) {
		ERROR("BL31: invalid handoff structure at %" PRIx64 "\n", handoff_addr);
		return XBL_HANDOFF_INVAL_STRUCT;
	}

	VERBOSE("BL31: TF-A handoff params at:0x%" PRIx64 ", entries:%u\n",
		handoff_addr, HandoffParams->num_entries);
	if (HandoffParams->num_entries > XBL_MAX_PARTITIONS) {
		ERROR("BL31: TF-A handoff params: too many partitions (%u/%u)\n",
		      HandoffParams->num_entries, XBL_MAX_PARTITIONS);
		return XBL_HANDOFF_TOO_MANY_PARTS;
	}

	/*
	 * we loop over all passed entries but only populate two image structs
	 * (bl32, bl33). I.e. the last applicable images in the handoff
	 * structure will be used for the hand off
	 */
	for (size_t i = 0; i < HandoffParams->num_entries; i++) {
		entry_point_info_t *image;
		int32_t target_estate, target_secure, target_cpu;
		uint32_t target_endianness, target_el;

		VERBOSE("BL31: %zd: entry:0x%" PRIx64 ", flags:0x%" PRIx64 "\n", i,
			HandoffParams->partition[i].entry_point,
			HandoffParams->partition[i].flags);

#if defined(PLAT_versal_net)
		uint32_t target_cluster;

		target_cluster = get_xbl_cluster(&HandoffParams->partition[i]);
		if (target_cluster != XBL_FLAGS_CLUSTER_0) {
			WARN("BL31: invalid target Cluster (%i)\n",
			     target_cluster);
			continue;
		}
#endif /* PLAT_versal_net */

		target_cpu = get_xbl_cpu(&HandoffParams->partition[i]);
		if (target_cpu != XBL_FLAGS_A53_0) {
			WARN("BL31: invalid target CPU (%i)\n", target_cpu);
			continue;
		}

		target_el = get_xbl_el(&HandoffParams->partition[i]);
		if ((target_el == XBL_FLAGS_EL3) ||
		    (target_el == XBL_FLAGS_EL0)) {
			WARN("BL31: invalid target exception level(%i)\n",
			     target_el);
			continue;
		}

		target_secure = get_xbl_ss(&HandoffParams->partition[i]);
		if (target_secure == XBL_FLAGS_SECURE &&
		    target_el == XBL_FLAGS_EL2) {
			WARN("BL31: invalid security state (%i) for exception level (%i)\n",
			     target_secure, target_el);
			continue;
		}

		target_estate = get_xbl_estate(&HandoffParams->partition[i]);
		target_endianness = get_xbl_endian(&HandoffParams->partition[i]);

		if (target_secure == XBL_FLAGS_SECURE) {
			image = bl32;

			if (target_estate == XBL_FLAGS_ESTATE_A32) {
				bl32->spsr = SPSR_MODE32(MODE32_svc, SPSR_T_ARM,
							 target_endianness,
							 DISABLE_ALL_EXCEPTIONS);
			} else {
				bl32->spsr = SPSR_64(MODE_EL1, MODE_SP_ELX,
						     DISABLE_ALL_EXCEPTIONS);
			}
		} else {
			image = bl33;

			if (target_estate == XBL_FLAGS_ESTATE_A32) {
				if (target_el == XBL_FLAGS_EL2) {
					target_el = MODE32_hyp;
				} else {
					target_el = MODE32_sys;
				}

				bl33->spsr = SPSR_MODE32(target_el, SPSR_T_ARM,
							 target_endianness,
							 DISABLE_ALL_EXCEPTIONS);
			} else {
				if (target_el == XBL_FLAGS_EL2) {
					target_el = MODE_EL2;
				} else {
					target_el = MODE_EL1;
				}

				bl33->spsr = SPSR_64(target_el, MODE_SP_ELX,
						     DISABLE_ALL_EXCEPTIONS);
			}
		}

		VERBOSE("Setting up %s entry point to:%" PRIx64 ", el:%x\n",
			target_secure == XBL_FLAGS_SECURE ? "BL32" : "BL33",
			HandoffParams->partition[i].entry_point,
			target_el);
		image->pc = HandoffParams->partition[i].entry_point;

		if (target_endianness == SPSR_E_BIG) {
			EP_SET_EE(image->h.attr, EP_EE_BIG);
		} else {
			EP_SET_EE(image->h.attr, EP_EE_LITTLE);
		}
	}

	return XBL_HANDOFF_SUCCESS;
}
