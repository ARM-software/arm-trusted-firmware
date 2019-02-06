/*
 * Copyright (c) 2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/bl_common.h>

#include <smmu.h>
#include <tegra_def.h>
#include <tegra_mc_def.h>

#define MAX_NUM_SMMU_DEVICES	U(1)

/*******************************************************************************
 * Array to hold SMMU context for Tegra186
 ******************************************************************************/
static __attribute__((aligned(16))) smmu_regs_t tegra186_smmu_context[] = {
	_START_OF_TABLE_,
	mc_make_sid_security_cfg(SCEW),
	mc_make_sid_security_cfg(AFIR),
	mc_make_sid_security_cfg(NVDISPLAYR1),
	mc_make_sid_security_cfg(XUSB_DEVR),
	mc_make_sid_security_cfg(VICSRD1),
	mc_make_sid_security_cfg(NVENCSWR),
	mc_make_sid_security_cfg(TSECSRDB),
	mc_make_sid_security_cfg(AXISW),
	mc_make_sid_security_cfg(SDMMCWAB),
	mc_make_sid_security_cfg(AONDMAW),
	mc_make_sid_security_cfg(GPUSWR2),
	mc_make_sid_security_cfg(SATAW),
	mc_make_sid_security_cfg(UFSHCW),
	mc_make_sid_security_cfg(AFIW),
	mc_make_sid_security_cfg(SDMMCR),
	mc_make_sid_security_cfg(SCEDMAW),
	mc_make_sid_security_cfg(UFSHCR),
	mc_make_sid_security_cfg(SDMMCWAA),
	mc_make_sid_security_cfg(APEDMAW),
	mc_make_sid_security_cfg(SESWR),
	mc_make_sid_security_cfg(MPCORER),
	mc_make_sid_security_cfg(PTCR),
	mc_make_sid_security_cfg(BPMPW),
	mc_make_sid_security_cfg(ETRW),
	mc_make_sid_security_cfg(GPUSRD),
	mc_make_sid_security_cfg(VICSWR),
	mc_make_sid_security_cfg(SCEDMAR),
	mc_make_sid_security_cfg(HDAW),
	mc_make_sid_security_cfg(ISPWA),
	mc_make_sid_security_cfg(EQOSW),
	mc_make_sid_security_cfg(XUSB_HOSTW),
	mc_make_sid_security_cfg(TSECSWR),
	mc_make_sid_security_cfg(SDMMCRAA),
	mc_make_sid_security_cfg(APER),
	mc_make_sid_security_cfg(VIW),
	mc_make_sid_security_cfg(APEW),
	mc_make_sid_security_cfg(AXISR),
	mc_make_sid_security_cfg(SDMMCW),
	mc_make_sid_security_cfg(BPMPDMAW),
	mc_make_sid_security_cfg(ISPRA),
	mc_make_sid_security_cfg(NVDECSWR),
	mc_make_sid_security_cfg(XUSB_DEVW),
	mc_make_sid_security_cfg(NVDECSRD),
	mc_make_sid_security_cfg(MPCOREW),
	mc_make_sid_security_cfg(NVDISPLAYR),
	mc_make_sid_security_cfg(BPMPDMAR),
	mc_make_sid_security_cfg(NVJPGSWR),
	mc_make_sid_security_cfg(NVDECSRD1),
	mc_make_sid_security_cfg(TSECSRD),
	mc_make_sid_security_cfg(NVJPGSRD),
	mc_make_sid_security_cfg(SDMMCWA),
	mc_make_sid_security_cfg(SCER),
	mc_make_sid_security_cfg(XUSB_HOSTR),
	mc_make_sid_security_cfg(VICSRD),
	mc_make_sid_security_cfg(AONDMAR),
	mc_make_sid_security_cfg(AONW),
	mc_make_sid_security_cfg(SDMMCRA),
	mc_make_sid_security_cfg(HOST1XDMAR),
	mc_make_sid_security_cfg(EQOSR),
	mc_make_sid_security_cfg(SATAR),
	mc_make_sid_security_cfg(BPMPR),
	mc_make_sid_security_cfg(HDAR),
	mc_make_sid_security_cfg(SDMMCRAB),
	mc_make_sid_security_cfg(ETRR),
	mc_make_sid_security_cfg(AONR),
	mc_make_sid_security_cfg(APEDMAR),
	mc_make_sid_security_cfg(SESRD),
	mc_make_sid_security_cfg(NVENCSRD),
	mc_make_sid_security_cfg(GPUSWR),
	mc_make_sid_security_cfg(TSECSWRB),
	mc_make_sid_security_cfg(ISPWB),
	mc_make_sid_security_cfg(GPUSRD2),
	mc_make_sid_override_cfg(APER),
	mc_make_sid_override_cfg(VICSRD),
	mc_make_sid_override_cfg(NVENCSRD),
	mc_make_sid_override_cfg(NVJPGSWR),
	mc_make_sid_override_cfg(AONW),
	mc_make_sid_override_cfg(BPMPR),
	mc_make_sid_override_cfg(BPMPW),
	mc_make_sid_override_cfg(HDAW),
	mc_make_sid_override_cfg(NVDISPLAYR1),
	mc_make_sid_override_cfg(APEDMAR),
	mc_make_sid_override_cfg(AFIR),
	mc_make_sid_override_cfg(AXISR),
	mc_make_sid_override_cfg(VICSRD1),
	mc_make_sid_override_cfg(TSECSRD),
	mc_make_sid_override_cfg(BPMPDMAW),
	mc_make_sid_override_cfg(MPCOREW),
	mc_make_sid_override_cfg(XUSB_HOSTR),
	mc_make_sid_override_cfg(GPUSWR),
	mc_make_sid_override_cfg(XUSB_DEVR),
	mc_make_sid_override_cfg(UFSHCW),
	mc_make_sid_override_cfg(XUSB_HOSTW),
	mc_make_sid_override_cfg(SDMMCWAB),
	mc_make_sid_override_cfg(SATAW),
	mc_make_sid_override_cfg(SCEDMAR),
	mc_make_sid_override_cfg(HOST1XDMAR),
	mc_make_sid_override_cfg(SDMMCWA),
	mc_make_sid_override_cfg(APEDMAW),
	mc_make_sid_override_cfg(SESWR),
	mc_make_sid_override_cfg(AXISW),
	mc_make_sid_override_cfg(AONDMAW),
	mc_make_sid_override_cfg(TSECSWRB),
	mc_make_sid_override_cfg(MPCORER),
	mc_make_sid_override_cfg(ISPWB),
	mc_make_sid_override_cfg(AONR),
	mc_make_sid_override_cfg(BPMPDMAR),
	mc_make_sid_override_cfg(HDAR),
	mc_make_sid_override_cfg(SDMMCRA),
	mc_make_sid_override_cfg(ETRW),
	mc_make_sid_override_cfg(GPUSWR2),
	mc_make_sid_override_cfg(EQOSR),
	mc_make_sid_override_cfg(TSECSWR),
	mc_make_sid_override_cfg(ETRR),
	mc_make_sid_override_cfg(NVDECSRD),
	mc_make_sid_override_cfg(TSECSRDB),
	mc_make_sid_override_cfg(SDMMCRAA),
	mc_make_sid_override_cfg(NVDECSRD1),
	mc_make_sid_override_cfg(SDMMCR),
	mc_make_sid_override_cfg(NVJPGSRD),
	mc_make_sid_override_cfg(SCEDMAW),
	mc_make_sid_override_cfg(SDMMCWAA),
	mc_make_sid_override_cfg(APEW),
	mc_make_sid_override_cfg(AONDMAR),
	mc_make_sid_override_cfg(PTCR),
	mc_make_sid_override_cfg(SCER),
	mc_make_sid_override_cfg(ISPRA),
	mc_make_sid_override_cfg(ISPWA),
	mc_make_sid_override_cfg(VICSWR),
	mc_make_sid_override_cfg(SESRD),
	mc_make_sid_override_cfg(SDMMCW),
	mc_make_sid_override_cfg(SDMMCRAB),
	mc_make_sid_override_cfg(EQOSW),
	mc_make_sid_override_cfg(GPUSRD2),
	mc_make_sid_override_cfg(SCEW),
	mc_make_sid_override_cfg(GPUSRD),
	mc_make_sid_override_cfg(NVDECSWR),
	mc_make_sid_override_cfg(XUSB_DEVW),
	mc_make_sid_override_cfg(SATAR),
	mc_make_sid_override_cfg(NVDISPLAYR),
	mc_make_sid_override_cfg(VIW),
	mc_make_sid_override_cfg(UFSHCR),
	mc_make_sid_override_cfg(NVENCSWR),
	mc_make_sid_override_cfg(AFIW),
	smmu_make_cfg(TEGRA_SMMU0_BASE),
	smmu_bypass_cfg,	/* TBU settings */
	_END_OF_TABLE_,
};

/*******************************************************************************
 * Handler to return the pointer to the SMMU's context struct
 ******************************************************************************/
smmu_regs_t *plat_get_smmu_ctx(void)
{
	/* index of _END_OF_TABLE_ */
	tegra186_smmu_context[0].val = (uint32_t)(ARRAY_SIZE(tegra186_smmu_context)) - 1U;

	return tegra186_smmu_context;
}

/*******************************************************************************
 * Handler to return the support SMMU devices number
 ******************************************************************************/
uint32_t plat_get_num_smmu_devices(void)
{
	return MAX_NUM_SMMU_DEVICES;
}
