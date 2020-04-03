/*
 * Copyright (c) 2016 - 2020, Broadcom
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/debug.h>
#include <drivers/arm/tzc400.h>
#include <lib/mmio.h>

#include <cmn_sec.h>
#include <platform_def.h>

/*
 * Trust Zone controllers
 */
#define TZC400_FS_SRAM_ROOT	0x66d84000

/*
 * TZPC Master configure registers
 */

/* TZPC_TZPCDECPROT0set */
#define TZPC0_MASTER_NS_BASE		0x68b40804
#define TZPC0_SATA3_BIT			5
#define TZPC0_SATA2_BIT			4
#define TZPC0_SATA1_BIT			3
#define TZPC0_SATA0_BIT			2
#define TZPC0_USB3H1_BIT		1
#define TZPC0_USB3H0_BIT		0
#define TZPC0_MASTER_SEC_DEFAULT	0

/* TZPC_TZPCDECPROT1set */
#define TZPC1_MASTER_NS_BASE		0x68b40810
#define TZPC1_SDIO1_BIT			6
#define TZPC1_SDIO0_BIT			5
#define TZPC1_AUDIO0_BIT		4
#define TZPC1_USB2D_BIT			3
#define TZPC1_USB2H1_BIT		2
#define TZPC1_USB2H0_BIT		1
#define TZPC1_AMAC0_BIT			0
#define TZPC1_MASTER_SEC_DEFAULT	0


struct tz_sec_desc {
	uintptr_t addr;
	uint32_t val;
};

static const struct tz_sec_desc tz_master_defaults[] = {
{ TZPC0_MASTER_NS_BASE, TZPC0_MASTER_SEC_DEFAULT },
{ TZPC1_MASTER_NS_BASE, TZPC1_MASTER_SEC_DEFAULT }
};

/*
 * Initialize the TrustZone Controller for SRAM partitioning.
 */
static void bcm_tzc_setup(void)
{
	VERBOSE("Configuring SRAM TrustZone Controller\n");

	/* Init the TZASC controller */
	tzc400_init(TZC400_FS_SRAM_ROOT);

	/*
	 * Close the entire SRAM space
	 * Region 0 covers the entire SRAM space
	 * None of the NS device can access it.
	 */
	tzc400_configure_region0(TZC_REGION_S_RDWR, 0);

	/* Do raise an exception if a NS device tries to access secure memory */
	tzc400_set_action(TZC_ACTION_ERR);
}

/*
 * Configure TZ Master as NS_MASTER or SECURE_MASTER
 * To set a Master to non-secure, use *_SET registers
 * To set a Master to secure, use *_CLR registers (set + 0x4 address)
 */
static void tz_master_set(uint32_t base, uint32_t value, uint32_t ns)
{
	if (ns == SECURE_MASTER) {
		mmio_write_32(base + 4, value);
	} else {
		mmio_write_32(base, value);
	}
}

/*
 * Initialize the secure environment for sdio.
 */
void plat_tz_sdio_ns_master_set(uint32_t ns)
{
	tz_master_set(TZPC1_MASTER_NS_BASE,
			1 << TZPC1_SDIO0_BIT,
			ns);
}

/*
 * Initialize the secure environment for usb.
 */
void plat_tz_usb_ns_master_set(uint32_t ns)
{
	tz_master_set(TZPC1_MASTER_NS_BASE,
			1 << TZPC1_USB2H0_BIT,
			ns);
}

/*
 * Set masters to default configuration.
 *
 * DMA security settings are programmed into the PL-330 controller and
 * are not set by iProc TZPC registers.
 * DMA always comes up as secure master (*NS bit is 0).
 *
 * Because the default reset values of TZPC are 0 (== Secure),
 * ARM Verilog code makes all masters, including PCIe, come up as
 * secure.
 * However, SOTP has a bit called SOTP_ALLMASTER_NS that overrides
 * TZPC and makes all masters non-secure for AB devices.
 *
 * Hence we first set all the TZPC bits to program all masters,
 * including PCIe, as non-secure, then set the CLEAR_ALLMASTER_NS bit
 * so that the SOTP_ALLMASTER_NS cannot override TZPC.
 * now security settings for each masters come from TZPC
 * (which makes all masters other than DMA as non-secure).
 *
 * During the boot, all masters other than DMA Ctrlr + list
 * are non-secure in an AB Prod/AB Dev/AB Pending device.
 *
 */
void plat_tz_master_default_cfg(void)
{
	int i;

	/* Configure default secure and non-secure TZ Masters */
	for (i = 0; i < ARRAY_SIZE(tz_master_defaults); i++) {
		tz_master_set(tz_master_defaults[i].addr,
			      tz_master_defaults[i].val,
			      SECURE_MASTER);
		tz_master_set(tz_master_defaults[i].addr,
			      ~tz_master_defaults[i].val,
			      NS_MASTER);
	}

	/* Clear all master NS */
	mmio_setbits_32(SOTP_CHIP_CTRL,
			1 << SOTP_CLEAR_SYSCTRL_ALL_MASTER_NS);

	/* Initialize TZ controller and Set SRAM to secure */
	bcm_tzc_setup();
}
