/*
 * Copyright (c) 2017, ARM Limited and Contributors. All rights reserved.
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
#include <mmio.h>
#include <tegra_def.h>
#include <tegra_platform.h>
#include <tegra_private.h>

/*******************************************************************************
 * Tegra platforms
 ******************************************************************************/
typedef enum tegra_platform {
	TEGRA_PLATFORM_SILICON = 0,
	TEGRA_PLATFORM_QT,
	TEGRA_PLATFORM_FPGA,
	TEGRA_PLATFORM_EMULATION,
	TEGRA_PLATFORM_MAX,
} tegra_platform_t;

/*******************************************************************************
 * Tegra macros defining all the SoC minor versions
 ******************************************************************************/
#define TEGRA_MINOR_QT			0
#define TEGRA_MINOR_FPGA		1
#define TEGRA_MINOR_EMULATION_MIN	2
#define TEGRA_MINOR_EMULATION_MAX	10

/*******************************************************************************
 * Tegra major, minor version helper macros
 ******************************************************************************/
#define MAJOR_VERSION_SHIFT		0x4
#define MAJOR_VERSION_MASK		0xF
#define MINOR_VERSION_SHIFT		0x10
#define MINOR_VERSION_MASK		0xF
#define CHIP_ID_SHIFT			8
#define CHIP_ID_MASK			0xFF

/*******************************************************************************
 * Tegra chip ID values
 ******************************************************************************/
typedef enum tegra_chipid {
	TEGRA_CHIPID_TEGRA13 = 0x13,
	TEGRA_CHIPID_TEGRA21 = 0x21,
} tegra_chipid_t;

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

uint8_t tegra_chipid_is_t132(void)
{
	uint32_t chip_id = (tegra_get_chipid() >> CHIP_ID_SHIFT) & CHIP_ID_MASK;

	return (chip_id == TEGRA_CHIPID_TEGRA13);
}

uint8_t tegra_chipid_is_t210(void)
{
	uint32_t chip_id = (tegra_get_chipid() >> CHIP_ID_SHIFT) & CHIP_ID_MASK;

	return (chip_id == TEGRA_CHIPID_TEGRA21);
}

/*
 * Read the chip ID value and derive the platform
 */
static tegra_platform_t tegra_get_platform(void)
{
	uint32_t major = tegra_get_chipid_major();
	uint32_t minor = tegra_get_chipid_minor();

	/* Actual silicon platforms have a non-zero major version */
	if (major > 0)
		return TEGRA_PLATFORM_SILICON;

	/*
	 * The minor version number is used by simulation platforms
	 */

	/*
	 * Cadence's QuickTurn emulation system is a Solaris-based
	 * chip emulation system
	 */
	if (minor == TEGRA_MINOR_QT)
		return TEGRA_PLATFORM_QT;

	/*
	 * FPGAs are used during early software/hardware development
	 */
	if (minor == TEGRA_MINOR_FPGA)
		return TEGRA_PLATFORM_FPGA;

	/* Minor version reserved for other emulation platforms */
	if ((minor > TEGRA_MINOR_FPGA) && (minor <= TEGRA_MINOR_EMULATION_MAX))
		return TEGRA_PLATFORM_EMULATION;

	/* unsupported platform */
	return TEGRA_PLATFORM_MAX;
}

uint8_t tegra_platform_is_silicon(void)
{
	return (tegra_get_platform() == TEGRA_PLATFORM_SILICON);
}

uint8_t tegra_platform_is_qt(void)
{
	return (tegra_get_platform() == TEGRA_PLATFORM_QT);
}

uint8_t tegra_platform_is_fpga(void)
{
	return (tegra_get_platform() == TEGRA_PLATFORM_FPGA);
}

uint8_t tegra_platform_is_emulation(void)
{
	return (tegra_get_platform() == TEGRA_PLATFORM_EMULATION);
}
