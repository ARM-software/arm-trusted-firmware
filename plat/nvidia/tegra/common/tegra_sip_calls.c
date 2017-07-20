/*
 * Copyright (c) 2015-2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <arch_helpers.h>
#include <assert.h>
#include <bl_common.h>
#include <debug.h>
#include <errno.h>
#include <memctrl.h>
#include <mmio.h>
#include <runtime_svc.h>
#include <tegra_platform.h>
#include <tegra_private.h>

/*******************************************************************************
 * Common Tegra SiP SMCs
 ******************************************************************************/
#define TEGRA_SIP_NEW_VIDEOMEM_REGION		0x82000003
#define TEGRA_SIP_FIQ_NS_ENTRYPOINT		0x82000005
#define TEGRA_SIP_FIQ_NS_GET_CONTEXT		0x82000006
#define TEGRA_SIP_ENABLE_FAKE_SYSTEM_SUSPEND	0xC2000007

/*******************************************************************************
 * Fake system suspend mode control var
 ******************************************************************************/
extern uint8_t tegra_fake_system_suspend;


/*******************************************************************************
 * SoC specific SiP handler
 ******************************************************************************/
#pragma weak plat_sip_handler
int plat_sip_handler(uint32_t smc_fid,
		     uint64_t x1,
		     uint64_t x2,
		     uint64_t x3,
		     uint64_t x4,
		     void *cookie,
		     void *handle,
		     uint64_t flags)
{
	return -ENOTSUP;
}

/*******************************************************************************
 * This function is responsible for handling all SiP calls
 ******************************************************************************/
uint64_t tegra_sip_handler(uint32_t smc_fid,
			   uint64_t x1,
			   uint64_t x2,
			   uint64_t x3,
			   uint64_t x4,
			   void *cookie,
			   void *handle,
			   uint64_t flags)
{
	uint32_t regval;
	int err;

	/* Check if this is a SoC specific SiP */
	err = plat_sip_handler(smc_fid, x1, x2, x3, x4, cookie, handle, flags);
	if (err == 0)
		SMC_RET1(handle, (uint64_t)err);

	switch (smc_fid) {

	case TEGRA_SIP_NEW_VIDEOMEM_REGION:

		/* clean up the high bits */
		x2 = (uint32_t)x2;

		/*
		 * Check if Video Memory overlaps TZDRAM (contains bl31/bl32)
		 * or falls outside of the valid DRAM range
		 */
		err = bl31_check_ns_address(x1, x2);
		if (err)
			SMC_RET1(handle, err);

		/*
		 * Check if Video Memory is aligned to 1MB.
		 */
		if ((x1 & 0xFFFFF) || (x2 & 0xFFFFF)) {
			ERROR("Unaligned Video Memory base address!\n");
			SMC_RET1(handle, -ENOTSUP);
		}

		/*
		 * The GPU is the user of the Video Memory region. In order to
		 * transition to the new memory region smoothly, we program the
		 * new base/size ONLY if the GPU is in reset mode.
		 */
		regval = mmio_read_32(TEGRA_CAR_RESET_BASE +
				      TEGRA_GPU_RESET_REG_OFFSET);
		if ((regval & GPU_RESET_BIT) == 0U) {
			ERROR("GPU not in reset! Video Memory setup failed\n");
			SMC_RET1(handle, -ENOTSUP);
		}

		/* new video memory carveout settings */
		tegra_memctrl_videomem_setup(x1, x2);

		SMC_RET1(handle, 0);
		break;

	/*
	 * The NS world registers the address of its handler to be
	 * used for processing the FIQ. This is normally used by the
	 * NS FIQ debugger driver to detect system hangs by programming
	 * a watchdog timer to fire a FIQ interrupt.
	 */
	case TEGRA_SIP_FIQ_NS_ENTRYPOINT:

		if (!x1)
			SMC_RET1(handle, SMC_UNK);

		/*
		 * TODO: Check if x1 contains a valid DRAM address
		 */

		/* store the NS world's entrypoint */
		tegra_fiq_set_ns_entrypoint(x1);

		SMC_RET1(handle, 0);
		break;

	/*
	 * The NS world's FIQ handler issues this SMC to get the NS EL1/EL0
	 * CPU context when the FIQ interrupt was triggered. This allows the
	 * NS world to understand the CPU state when the watchdog interrupt
	 * triggered.
	 */
	case TEGRA_SIP_FIQ_NS_GET_CONTEXT:

		/* retrieve context registers when FIQ triggered */
		tegra_fiq_get_intr_context();

		SMC_RET0(handle);
		break;

	case TEGRA_SIP_ENABLE_FAKE_SYSTEM_SUSPEND:
		/*
		 * System suspend fake mode is set if we are on VDK and we make
		 * a debug SIP call. This mode ensures that we excercise debug
		 * path instead of the regular code path to suit the pre-silicon
		 * platform needs. These include replacing the call to WFI by
		 * a warm reset request.
		 */
		if (tegra_platform_is_emulation() != 0U) {

			tegra_fake_system_suspend = 1;
			SMC_RET1(handle, 0);
		}

		/*
		 * We return to the external world as if this SIP is not
		 * implemented in case, we are not running on VDK.
		 */
		break;

	default:
		ERROR("%s: unhandled SMC (0x%x)\n", __func__, smc_fid);
		break;
	}

	SMC_RET1(handle, SMC_UNK);
}

/* Define a runtime service descriptor for fast SMC calls */
DECLARE_RT_SVC(
	tegra_sip_fast,

	OEN_SIP_START,
	OEN_SIP_END,
	SMC_TYPE_FAST,
	NULL,
	tegra_sip_handler
);
