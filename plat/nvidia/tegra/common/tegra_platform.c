/*
 * Copyright (c) 2016-2020, ARM Limited and Contributors. All rights reserved.
 * Copyright (c) 2020, NVIDIA Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch_helpers.h>
#include <assert.h>
#include <lib/mmio.h>
#include <lib/smccc.h>
#include <services/arm_arch_svc.h>
#include <tegra_def.h>
#include <tegra_platform.h>
#include <tegra_private.h>

/*******************************************************************************
 * Tegra platforms
 ******************************************************************************/
typedef enum tegra_platform {
	TEGRA_PLATFORM_SILICON = 0U,
	TEGRA_PLATFORM_QT,
	TEGRA_PLATFORM_FPGA,
	TEGRA_PLATFORM_EMULATION,
	TEGRA_PLATFORM_LINSIM,
	TEGRA_PLATFORM_UNIT_FPGA,
	TEGRA_PLATFORM_VIRT_DEV_KIT,
	TEGRA_PLATFORM_MAX,
} tegra_platform_t;

/*******************************************************************************
 * Tegra macros defining all the SoC minor versions
 ******************************************************************************/
#define TEGRA_MINOR_QT			U(0)
#define TEGRA_MINOR_FPGA		U(1)
#define TEGRA_MINOR_ASIM_QT		U(2)
#define TEGRA_MINOR_ASIM_LINSIM		U(3)
#define TEGRA_MINOR_DSIM_ASIM_LINSIM	U(4)
#define TEGRA_MINOR_UNIT_FPGA		U(5)
#define TEGRA_MINOR_VIRT_DEV_KIT	U(6)

/*******************************************************************************
 * Tegra macros defining all the SoC pre_si_platform
 ******************************************************************************/
#define TEGRA_PRE_SI_QT			U(1)
#define TEGRA_PRE_SI_FPGA		U(2)
#define TEGRA_PRE_SI_UNIT_FPGA		U(3)
#define TEGRA_PRE_SI_ASIM_QT		U(4)
#define TEGRA_PRE_SI_ASIM_LINSIM	U(5)
#define TEGRA_PRE_SI_DSIM_ASIM_LINSIM	U(6)
#define TEGRA_PRE_SI_VDK		U(8)

/*
 * Read the chip ID value
 */
static uint32_t tegra_get_chipid(void)
{
	return mmio_read_32(TEGRA_MISC_BASE + HARDWARE_REVISION_OFFSET);
}

/*
 * Read the chip's major version from chip ID value
 */
uint32_t tegra_get_chipid_major(void)
{
	return (tegra_get_chipid() >> MAJOR_VERSION_SHIFT) & MAJOR_VERSION_MASK;
}

/*
 * Read the chip's minor version from the chip ID value
 */
uint32_t tegra_get_chipid_minor(void)
{
	return (tegra_get_chipid() >> MINOR_VERSION_SHIFT) & MINOR_VERSION_MASK;
}

/*
 * Read the chip's pre_si_platform valus from the chip ID value
 */
static uint32_t tegra_get_chipid_pre_si_platform(void)
{
	return (tegra_get_chipid() >> PRE_SI_PLATFORM_SHIFT) & PRE_SI_PLATFORM_MASK;
}

bool tegra_chipid_is_t132(void)
{
	uint32_t chip_id = ((tegra_get_chipid() >> CHIP_ID_SHIFT) & CHIP_ID_MASK);

	return (chip_id == TEGRA_CHIPID_TEGRA13);
}

bool tegra_chipid_is_t186(void)
{
	uint32_t chip_id = (tegra_get_chipid() >> CHIP_ID_SHIFT) & CHIP_ID_MASK;

	return (chip_id == TEGRA_CHIPID_TEGRA18);
}

bool tegra_chipid_is_t210(void)
{
	uint32_t chip_id = (tegra_get_chipid() >> CHIP_ID_SHIFT) & CHIP_ID_MASK;

	return (chip_id == TEGRA_CHIPID_TEGRA21);
}

bool tegra_chipid_is_t210_b01(void)
{
	return (tegra_chipid_is_t210() && (tegra_get_chipid_major() == 0x2U));
}

bool tegra_chipid_is_t194(void)
{
	uint32_t chip_id = (tegra_get_chipid() >> CHIP_ID_SHIFT) & CHIP_ID_MASK;

	return (chip_id == TEGRA_CHIPID_TEGRA19);
}

/*
 * Read the chip ID value and derive the platform
 */
static tegra_platform_t tegra_get_platform(void)
{
	uint32_t major, minor, pre_si_platform;
	tegra_platform_t ret;

	/* get the major/minor chip ID values */
	major = tegra_get_chipid_major();
	minor = tegra_get_chipid_minor();
	pre_si_platform = tegra_get_chipid_pre_si_platform();

	if (major == 0U) {
		/*
		 * The minor version number is used by simulation platforms
		 */
		switch (minor) {
		/*
		 * Cadence's QuickTurn emulation system is a Solaris-based
		 * chip emulation system
		 */
		case TEGRA_MINOR_QT:
		case TEGRA_MINOR_ASIM_QT:
			ret = TEGRA_PLATFORM_QT;
			break;

		/*
		 * FPGAs are used during early software/hardware development
		 */
		case TEGRA_MINOR_FPGA:
			ret = TEGRA_PLATFORM_FPGA;
			break;
		/*
		 * Linsim is a reconfigurable, clock-driven, mixed RTL/cmodel
		 * simulation framework.
		 */
		case TEGRA_MINOR_ASIM_LINSIM:
		case TEGRA_MINOR_DSIM_ASIM_LINSIM:
			ret = TEGRA_PLATFORM_LINSIM;
			break;

		/*
		 * Unit FPGAs run the actual hardware block IP on the FPGA with
		 * the other parts of the system using Linsim.
		 */
		case TEGRA_MINOR_UNIT_FPGA:
			ret = TEGRA_PLATFORM_UNIT_FPGA;
			break;
		/*
		 * The Virtualizer Development Kit (VDK) is the standard chip
		 * development from Synopsis.
		 */
		case TEGRA_MINOR_VIRT_DEV_KIT:
			ret = TEGRA_PLATFORM_VIRT_DEV_KIT;
			break;

		default:
			ret = TEGRA_PLATFORM_MAX;
			break;
		}

	} else if (pre_si_platform > 0U) {

		switch (pre_si_platform) {
		/*
		 * Cadence's QuickTurn emulation system is a Solaris-based
		 * chip emulation system
		 */
		case TEGRA_PRE_SI_QT:
		case TEGRA_PRE_SI_ASIM_QT:
			ret = TEGRA_PLATFORM_QT;
			break;

		/*
		 * FPGAs are used during early software/hardware development
		 */
		case TEGRA_PRE_SI_FPGA:
			ret = TEGRA_PLATFORM_FPGA;
			break;
		/*
		 * Linsim is a reconfigurable, clock-driven, mixed RTL/cmodel
		 * simulation framework.
		 */
		case TEGRA_PRE_SI_ASIM_LINSIM:
		case TEGRA_PRE_SI_DSIM_ASIM_LINSIM:
			ret = TEGRA_PLATFORM_LINSIM;
			break;

		/*
		 * Unit FPGAs run the actual hardware block IP on the FPGA with
		 * the other parts of the system using Linsim.
		 */
		case TEGRA_PRE_SI_UNIT_FPGA:
			ret = TEGRA_PLATFORM_UNIT_FPGA;
			break;
		/*
		 * The Virtualizer Development Kit (VDK) is the standard chip
		 * development from Synopsis.
		 */
		case TEGRA_PRE_SI_VDK:
			ret = TEGRA_PLATFORM_VIRT_DEV_KIT;
			break;

		default:
			ret = TEGRA_PLATFORM_MAX;
			break;
		}

	} else {
		/* Actual silicon platforms have a non-zero major version */
		ret = TEGRA_PLATFORM_SILICON;
	}

	return ret;
}

bool tegra_platform_is_silicon(void)
{
	return ((tegra_get_platform() == TEGRA_PLATFORM_SILICON) ? true : false);
}

bool tegra_platform_is_qt(void)
{
	return ((tegra_get_platform() == TEGRA_PLATFORM_QT) ? true : false);
}

bool tegra_platform_is_linsim(void)
{
	tegra_platform_t plat = tegra_get_platform();

	return (((plat == TEGRA_PLATFORM_LINSIM) ||
	       (plat == TEGRA_PLATFORM_UNIT_FPGA)) ? true : false);
}

bool tegra_platform_is_fpga(void)
{
	return ((tegra_get_platform() == TEGRA_PLATFORM_FPGA) ? true : false);
}

bool tegra_platform_is_emulation(void)
{
	return (tegra_get_platform() == TEGRA_PLATFORM_EMULATION);
}

bool tegra_platform_is_unit_fpga(void)
{
	return ((tegra_get_platform() == TEGRA_PLATFORM_UNIT_FPGA) ? true : false);
}

bool tegra_platform_is_virt_dev_kit(void)
{
	return ((tegra_get_platform() == TEGRA_PLATFORM_VIRT_DEV_KIT) ? true : false);
}

/*
 * This function returns soc version which mainly consist of below fields
 *
 *  soc_version[30:24] = JEP-106 continuation code for the SiP
 *  soc_version[23:16] = JEP-106 identification code with parity bit for the SiP
 *  soc_version[0:15]  = chip identification
 */
int32_t plat_get_soc_version(void)
{
	uint32_t chip_id = ((tegra_get_chipid() >> CHIP_ID_SHIFT) & CHIP_ID_MASK);
	uint32_t manfid = (JEDEC_NVIDIA_BKID << 24) | (JEDEC_NVIDIA_MFID << 16);

	return (int32_t)(manfid | (chip_id & 0xFFFF));
}

/*
 * This function returns soc revision in below format
 *
 *   soc_revision[8:15] = major version number
 *   soc_revision[0:7]  = minor version number
 */
int32_t plat_get_soc_revision(void)
{
	return (int32_t)((tegra_get_chipid_major() << 8) | tegra_get_chipid_minor());
}

/*****************************************************************************
 * plat_is_smccc_feature_available() - This function checks whether SMCCC feature
 *                                  is availabile for the platform or not.
 * @fid: SMCCC function id
 *
 * Return SMC_ARCH_CALL_SUCCESS if SMCCC feature is available and
 * SMC_ARCH_CALL_NOT_SUPPORTED otherwise.
 *****************************************************************************/
int32_t plat_is_smccc_feature_available(u_register_t fid)
{
	switch (fid) {
	case SMCCC_ARCH_SOC_ID:
		return SMC_ARCH_CALL_SUCCESS;
	default:
		return SMC_ARCH_CALL_NOT_SUPPORTED;
	}
}
