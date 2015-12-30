/*
 * Copyright (c) 2015-2016, ARM Limited and Contributors. All rights reserved.
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

#ifndef __MEMCTRLV2_H__
#define __MEMCTRLV2_H__

#include <mmio.h>
#include <tegra_def.h>

/*******************************************************************************
 * StreamID to indicate no SMMU translations (requests to be steered on the
 * SMMU bypass path)
 ******************************************************************************/
#define MC_STREAM_ID_MAX			0x7F

/*******************************************************************************
 * Stream ID Override Config registers
 ******************************************************************************/
#define MC_STREAMID_OVERRIDE_CFG_PTCR		0x0
#define MC_STREAMID_OVERRIDE_CFG_AFIR		0x70
#define MC_STREAMID_OVERRIDE_CFG_HDAR		0xA8
#define MC_STREAMID_OVERRIDE_CFG_HOST1XDMAR	0xB0
#define MC_STREAMID_OVERRIDE_CFG_NVENCSRD	0xE0
#define MC_STREAMID_OVERRIDE_CFG_SATAR		0xF8
#define MC_STREAMID_OVERRIDE_CFG_MPCORER	0x138
#define MC_STREAMID_OVERRIDE_CFG_NVENCSWR	0x158
#define MC_STREAMID_OVERRIDE_CFG_AFIW		0x188
#define MC_STREAMID_OVERRIDE_CFG_SATAW		0x1E8
#define MC_STREAMID_OVERRIDE_CFG_MPCOREW	0x1C8
#define MC_STREAMID_OVERRIDE_CFG_SATAW		0x1E8
#define MC_STREAMID_OVERRIDE_CFG_HDAW		0x1A8
#define MC_STREAMID_OVERRIDE_CFG_ISPRA		0x220
#define MC_STREAMID_OVERRIDE_CFG_ISPWA		0x230
#define MC_STREAMID_OVERRIDE_CFG_ISPWB		0x238
#define MC_STREAMID_OVERRIDE_CFG_XUSB_HOSTR	0x250
#define MC_STREAMID_OVERRIDE_CFG_XUSB_HOSTW	0x258
#define MC_STREAMID_OVERRIDE_CFG_XUSB_DEVR	0x260
#define MC_STREAMID_OVERRIDE_CFG_XUSB_DEVW	0x268
#define MC_STREAMID_OVERRIDE_CFG_TSECSRD	0x2A0
#define MC_STREAMID_OVERRIDE_CFG_TSECSWR	0x2A8
#define MC_STREAMID_OVERRIDE_CFG_GPUSRD		0x2C0
#define MC_STREAMID_OVERRIDE_CFG_GPUSWR		0x2C8
#define MC_STREAMID_OVERRIDE_CFG_SDMMCRA	0x300
#define MC_STREAMID_OVERRIDE_CFG_SDMMCRAA	0x308
#define MC_STREAMID_OVERRIDE_CFG_SDMMCR		0x310
#define MC_STREAMID_OVERRIDE_CFG_SDMMCRAB	0x318
#define MC_STREAMID_OVERRIDE_CFG_SDMMCWA	0x320
#define MC_STREAMID_OVERRIDE_CFG_SDMMCWAA	0x328
#define MC_STREAMID_OVERRIDE_CFG_SDMMCW		0x330
#define MC_STREAMID_OVERRIDE_CFG_SDMMCWAB	0x338
#define MC_STREAMID_OVERRIDE_CFG_VICSRD		0x360
#define MC_STREAMID_OVERRIDE_CFG_VICSWR		0x368
#define MC_STREAMID_OVERRIDE_CFG_VIW		0x390
#define MC_STREAMID_OVERRIDE_CFG_NVDECSRD	0x3C0
#define MC_STREAMID_OVERRIDE_CFG_NVDECSWR	0x3C8
#define MC_STREAMID_OVERRIDE_CFG_APER		0x3D0
#define MC_STREAMID_OVERRIDE_CFG_APEW		0x3D8
#define MC_STREAMID_OVERRIDE_CFG_NVJPGSRD	0x3F0
#define MC_STREAMID_OVERRIDE_CFG_NVJPGSWR	0x3F8
#define MC_STREAMID_OVERRIDE_CFG_SESRD		0x400
#define MC_STREAMID_OVERRIDE_CFG_SESWR		0x408
#define MC_STREAMID_OVERRIDE_CFG_ETRR		0x420
#define MC_STREAMID_OVERRIDE_CFG_ETRW		0x428
#define MC_STREAMID_OVERRIDE_CFG_TSECSRDB	0x430
#define MC_STREAMID_OVERRIDE_CFG_TSECSWRB	0x438
#define MC_STREAMID_OVERRIDE_CFG_GPUSRD2	0x440
#define MC_STREAMID_OVERRIDE_CFG_GPUSWR2	0x448
#define MC_STREAMID_OVERRIDE_CFG_AXISR		0x460
#define MC_STREAMID_OVERRIDE_CFG_AXISW		0x468
#define MC_STREAMID_OVERRIDE_CFG_EQOSR		0x470
#define MC_STREAMID_OVERRIDE_CFG_EQOSW		0x478
#define MC_STREAMID_OVERRIDE_CFG_UFSHCR		0x480
#define MC_STREAMID_OVERRIDE_CFG_UFSHCW		0x488
#define MC_STREAMID_OVERRIDE_CFG_NVDISPLAYR	0x490
#define MC_STREAMID_OVERRIDE_CFG_BPMPR		0x498
#define MC_STREAMID_OVERRIDE_CFG_BPMPW		0x4A0
#define MC_STREAMID_OVERRIDE_CFG_BPMPDMAR	0x4A8
#define MC_STREAMID_OVERRIDE_CFG_BPMPDMAW	0x4B0
#define MC_STREAMID_OVERRIDE_CFG_AONR		0x4B8
#define MC_STREAMID_OVERRIDE_CFG_AONW		0x4C0
#define MC_STREAMID_OVERRIDE_CFG_AONDMAR	0x4C8
#define MC_STREAMID_OVERRIDE_CFG_AONDMAW	0x4D0
#define MC_STREAMID_OVERRIDE_CFG_SCER		0x4D8
#define MC_STREAMID_OVERRIDE_CFG_SCEW		0x4E0
#define MC_STREAMID_OVERRIDE_CFG_SCEDMAR	0x4E8
#define MC_STREAMID_OVERRIDE_CFG_SCEDMAW	0x4F0
#define MC_STREAMID_OVERRIDE_CFG_APEDMAR	0x4F8
#define MC_STREAMID_OVERRIDE_CFG_APEDMAW	0x500
#define MC_STREAMID_OVERRIDE_CFG_NVDISPLAYR1	0x508
#define MC_STREAMID_OVERRIDE_CFG_VICSRD1	0x510
#define MC_STREAMID_OVERRIDE_CFG_NVDECSRD1	0x518

/*******************************************************************************
 * Stream ID Security Config registers
 ******************************************************************************/
#define MC_STREAMID_SECURITY_CFG_PTCR		0x4
#define MC_STREAMID_SECURITY_CFG_AFIR		0x74
#define MC_STREAMID_SECURITY_CFG_HDAR		0xAC
#define MC_STREAMID_SECURITY_CFG_HOST1XDMAR	0xB4
#define MC_STREAMID_SECURITY_CFG_NVENCSRD	0xE4
#define MC_STREAMID_SECURITY_CFG_SATAR		0xFC
#define MC_STREAMID_SECURITY_CFG_HDAW		0x1AC
#define MC_STREAMID_SECURITY_CFG_MPCORER	0x13C
#define MC_STREAMID_SECURITY_CFG_NVENCSWR	0x15C
#define MC_STREAMID_SECURITY_CFG_AFIW		0x18C
#define MC_STREAMID_SECURITY_CFG_MPCOREW	0x1CC
#define MC_STREAMID_SECURITY_CFG_SATAW		0x1EC
#define MC_STREAMID_SECURITY_CFG_ISPRA		0x224
#define MC_STREAMID_SECURITY_CFG_ISPWA		0x234
#define MC_STREAMID_SECURITY_CFG_ISPWB		0x23C
#define MC_STREAMID_SECURITY_CFG_XUSB_HOSTR	0x254
#define MC_STREAMID_SECURITY_CFG_XUSB_HOSTW	0x25C
#define MC_STREAMID_SECURITY_CFG_XUSB_DEVR	0x264
#define MC_STREAMID_SECURITY_CFG_XUSB_DEVW	0x26C
#define MC_STREAMID_SECURITY_CFG_TSECSRD	0x2A4
#define MC_STREAMID_SECURITY_CFG_TSECSWR	0x2AC
#define MC_STREAMID_SECURITY_CFG_GPUSRD		0x2C4
#define MC_STREAMID_SECURITY_CFG_GPUSWR		0x2CC
#define MC_STREAMID_SECURITY_CFG_SDMMCRA	0x304
#define MC_STREAMID_SECURITY_CFG_SDMMCRAA	0x30C
#define MC_STREAMID_SECURITY_CFG_SDMMCR		0x314
#define MC_STREAMID_SECURITY_CFG_SDMMCRAB	0x31C
#define MC_STREAMID_SECURITY_CFG_SDMMCWA	0x324
#define MC_STREAMID_SECURITY_CFG_SDMMCWAA	0x32C
#define MC_STREAMID_SECURITY_CFG_SDMMCW		0x334
#define MC_STREAMID_SECURITY_CFG_SDMMCWAB	0x33C
#define MC_STREAMID_SECURITY_CFG_VICSRD		0x364
#define MC_STREAMID_SECURITY_CFG_VICSWR		0x36C
#define MC_STREAMID_SECURITY_CFG_VIW		0x394
#define MC_STREAMID_SECURITY_CFG_NVDECSRD	0x3C4
#define MC_STREAMID_SECURITY_CFG_NVDECSWR	0x3CC
#define MC_STREAMID_SECURITY_CFG_APER		0x3D4
#define MC_STREAMID_SECURITY_CFG_APEW		0x3DC
#define MC_STREAMID_SECURITY_CFG_NVJPGSRD	0x3F4
#define MC_STREAMID_SECURITY_CFG_NVJPGSWR	0x3FC
#define MC_STREAMID_SECURITY_CFG_SESRD		0x404
#define MC_STREAMID_SECURITY_CFG_SESWR		0x40C
#define MC_STREAMID_SECURITY_CFG_ETRR		0x424
#define MC_STREAMID_SECURITY_CFG_ETRW		0x42C
#define MC_STREAMID_SECURITY_CFG_TSECSRDB	0x434
#define MC_STREAMID_SECURITY_CFG_TSECSWRB	0x43C
#define MC_STREAMID_SECURITY_CFG_GPUSRD2	0x444
#define MC_STREAMID_SECURITY_CFG_GPUSWR2	0x44C
#define MC_STREAMID_SECURITY_CFG_AXISR		0x464
#define MC_STREAMID_SECURITY_CFG_AXISW		0x46C
#define MC_STREAMID_SECURITY_CFG_EQOSR		0x474
#define MC_STREAMID_SECURITY_CFG_EQOSW		0x47C
#define MC_STREAMID_SECURITY_CFG_UFSHCR		0x484
#define MC_STREAMID_SECURITY_CFG_UFSHCW		0x48C
#define MC_STREAMID_SECURITY_CFG_NVDISPLAYR	0x494
#define MC_STREAMID_SECURITY_CFG_BPMPR		0x49C
#define MC_STREAMID_SECURITY_CFG_BPMPW		0x4A4
#define MC_STREAMID_SECURITY_CFG_BPMPDMAR	0x4AC
#define MC_STREAMID_SECURITY_CFG_BPMPDMAW	0x4B4
#define MC_STREAMID_SECURITY_CFG_AONR		0x4BC
#define MC_STREAMID_SECURITY_CFG_AONW		0x4C4
#define MC_STREAMID_SECURITY_CFG_AONDMAR	0x4CC
#define MC_STREAMID_SECURITY_CFG_AONDMAW	0x4D4
#define MC_STREAMID_SECURITY_CFG_SCER		0x4DC
#define MC_STREAMID_SECURITY_CFG_SCEW		0x4E4
#define MC_STREAMID_SECURITY_CFG_SCEDMAR	0x4EC
#define MC_STREAMID_SECURITY_CFG_SCEDMAW	0x4F4
#define MC_STREAMID_SECURITY_CFG_APEDMAR	0x4FC
#define MC_STREAMID_SECURITY_CFG_APEDMAW	0x504
#define MC_STREAMID_SECURITY_CFG_NVDISPLAYR1	0x50C
#define MC_STREAMID_SECURITY_CFG_VICSRD1	0x514
#define MC_STREAMID_SECURITY_CFG_NVDECSRD1	0x51C

/*******************************************************************************
 * Memory Controller SMMU Bypass config register
 ******************************************************************************/
#define MC_SMMU_BYPASS_CONFIG			0x1820
#define MC_SMMU_BYPASS_CTRL_MASK		0x3
#define MC_SMMU_BYPASS_CTRL_SHIFT		0
#define MC_SMMU_CTRL_TBU_BYPASS_ALL		(0 << MC_SMMU_BYPASS_CTRL_SHIFT)
#define MC_SMMU_CTRL_TBU_RSVD			(1 << MC_SMMU_BYPASS_CTRL_SHIFT)
#define MC_SMMU_CTRL_TBU_BYPASS_SPL_STREAMID	(2 << MC_SMMU_BYPASS_CTRL_SHIFT)
#define MC_SMMU_CTRL_TBU_BYPASS_NONE		(3 << MC_SMMU_BYPASS_CTRL_SHIFT)
#define MC_SMMU_BYPASS_CONFIG_WRITE_ACCESS_BIT	(1 << 31)
#define MC_SMMU_BYPASS_CONFIG_SETTINGS		(MC_SMMU_BYPASS_CONFIG_WRITE_ACCESS_BIT | \
						 MC_SMMU_CTRL_TBU_BYPASS_SPL_STREAMID)

/*******************************************************************************
 * Memory Controller SMMU Global Secure Aux. Configuration Register
 ******************************************************************************/
#define ARM_SMMU_GSR0_SECURE_ACR		0x10
#define ARM_SMMU_GSR0_PGSIZE_SHIFT		16
#define ARM_SMMU_GSR0_PGSIZE_4K			(0 << ARM_SMMU_GSR0_PGSIZE_SHIFT)
#define ARM_SMMU_GSR0_PGSIZE_64K		(1 << ARM_SMMU_GSR0_PGSIZE_SHIFT)

/*******************************************************************************
 * Structure to hold the Stream ID to use to override client inputs
 ******************************************************************************/
typedef struct mc_streamid_override_cfg {
	uint32_t offset;
	uint8_t stream_id;
} mc_streamid_override_cfg_t;

/*******************************************************************************
 * Structure to hold the Stream ID Security Configuration settings
 ******************************************************************************/
typedef struct mc_streamid_security_cfg {
	char *name;
	uint32_t offset;
	int override_enable;
	int override_client_inputs;
	int override_client_ns_flag;
} mc_streamid_security_cfg_t;

#define OVERRIDE_DISABLE			1
#define OVERRIDE_ENABLE				0
#define CLIENT_FLAG_SECURE			0
#define CLIENT_FLAG_NON_SECURE			1
#define CLIENT_INPUTS_OVERRIDE			1
#define CLIENT_INPUTS_NO_OVERRIDE		0

#define mc_make_sec_cfg(off, ns, ovrrd, access) \
		{ \
			.name = # off, \
			.offset = MC_STREAMID_SECURITY_CFG_ ## off, \
			.override_client_ns_flag = CLIENT_FLAG_ ## ns, \
			.override_client_inputs = CLIENT_INPUTS_ ## ovrrd, \
			.override_enable = OVERRIDE_ ## access \
		}

/*******************************************************************************
 * TZDRAM carveout configuration registers
 ******************************************************************************/
#define MC_SECURITY_CFG0_0			0x70
#define MC_SECURITY_CFG1_0			0x74
#define MC_SECURITY_CFG3_0			0x9BC

/*******************************************************************************
 * Video Memory carveout configuration registers
 ******************************************************************************/
#define MC_VIDEO_PROTECT_BASE_HI		0x978
#define MC_VIDEO_PROTECT_BASE_LO		0x648
#define MC_VIDEO_PROTECT_SIZE_MB		0x64c

/*******************************************************************************
 * TZRAM carveout configuration registers
 ******************************************************************************/
#define MC_TZRAM_BASE				0x1850
#define MC_TZRAM_END				0x1854
#define MC_TZRAM_HI_ADDR_BITS			0x1588
 #define TZRAM_ADDR_HI_BITS_MASK		0x3
 #define TZRAM_END_HI_BITS_SHIFT		8
#define MC_TZRAM_REG_CTRL			0x185c
 #define DISABLE_TZRAM_ACCESS			1

static inline uint32_t tegra_mc_read_32(uint32_t off)
{
	return mmio_read_32(TEGRA_MC_BASE + off);
}

static inline void tegra_mc_write_32(uint32_t off, uint32_t val)
{
	mmio_write_32(TEGRA_MC_BASE + off, val);
}

static inline uint32_t tegra_mc_streamid_read_32(uint32_t off)
{
	return mmio_read_32(TEGRA_MC_STREAMID_BASE + off);
}

static inline void tegra_mc_streamid_write_32(uint32_t off, uint32_t val)
{
	mmio_write_32(TEGRA_MC_STREAMID_BASE + off, val);
}

static inline uint32_t tegra_smmu_read_32(uint32_t off)
{
	return mmio_read_32(TEGRA_SMMU_BASE + off);
}

static inline void tegra_smmu_write_32(uint32_t off, uint32_t val)
{
	mmio_write_32(TEGRA_SMMU_BASE + off, val);
}

#endif /* __MEMCTRLV2_H__ */
