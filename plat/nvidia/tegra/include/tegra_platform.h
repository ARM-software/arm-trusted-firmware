/*
 * Copyright (c) 2017-2018, ARM Limited and Contributors. All rights reserved.
 * Copyright (c) 2020, NVIDIA Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef TEGRA_PLATFORM_H
#define TEGRA_PLATFORM_H

#include <cdefs.h>
#include <lib/utils_def.h>
#include <stdbool.h>

/*******************************************************************************
 * Tegra major, minor version helper macros
 ******************************************************************************/
#define MAJOR_VERSION_SHIFT		U(0x4)
#define MAJOR_VERSION_MASK		U(0xF)
#define MINOR_VERSION_SHIFT		U(0x10)
#define MINOR_VERSION_MASK		U(0xF)
#define CHIP_ID_SHIFT			U(8)
#define CHIP_ID_MASK			U(0xFF)
#define PRE_SI_PLATFORM_SHIFT		U(0x14)
#define PRE_SI_PLATFORM_MASK		U(0xF)

/*******************************************************************************
 * Tegra chip ID values
 ******************************************************************************/
#define TEGRA_CHIPID_TEGRA13		U(0x13)
#define TEGRA_CHIPID_TEGRA21		U(0x21)
#define TEGRA_CHIPID_TEGRA18		U(0x18)
#define TEGRA_CHIPID_TEGRA19		U(0x19)

/*******************************************************************************
 * JEDEC Standard Manufacturer's Identification Code and Bank ID
 ******************************************************************************/
#define JEDEC_NVIDIA_MFID		U(0x6B)
#define JEDEC_NVIDIA_BKID		U(3)

#ifndef __ASSEMBLER__

/*
 * Tegra chip ID major/minor identifiers
 */
uint32_t tegra_get_chipid_major(void);
uint32_t tegra_get_chipid_minor(void);

/*
 * Tegra chip ID identifiers
 */
bool tegra_chipid_is_t132(void);
bool tegra_chipid_is_t186(void);
bool tegra_chipid_is_t210(void);
bool tegra_chipid_is_t210_b01(void);
bool tegra_chipid_is_t194(void);

/*
 * Tegra platform identifiers
 */
bool tegra_platform_is_silicon(void);
bool tegra_platform_is_qt(void);
bool tegra_platform_is_emulation(void);
bool tegra_platform_is_linsim(void);
bool tegra_platform_is_fpga(void);
bool tegra_platform_is_unit_fpga(void);
bool tegra_platform_is_virt_dev_kit(void);

#endif /* __ASSEMBLER__ */

#endif /* TEGRA_PLATFORM_H */
