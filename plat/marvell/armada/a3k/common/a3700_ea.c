/*
 * Copyright (C) 2019 Repk repk@triplefau.lt
 *
 * SPDX-License-Identifier:	BSD-3-Clause
 * https://spdx.org/licenses
 */

#include <inttypes.h>
#include <stdint.h>

#include <common/bl_common.h>
#include <common/debug.h>
#include <arch_helpers.h>
#include <plat/common/platform.h>
#include <bl31/ea_handle.h>

#define A53_SERR_INT_AXI_SLVERR_ON_EXTERNAL_ACCESS 0xbf000002

#if !ENABLE_BACKTRACE
static const char *get_el_str(unsigned int el)
{
	if (el == MODE_EL3) {
		return "EL3";
	} else if (el == MODE_EL2) {
		return "EL2";
	}
	return "S-EL1";
}
#endif /* !ENABLE_BACKTRACE */

/*
 * This source file with custom plat_ea_handler function is compiled only when
 * building TF-A with compile option HANDLE_EA_EL3_FIRST=1
 */
void plat_ea_handler(unsigned int ea_reason, uint64_t syndrome, void *cookie,
		void *handle, uint64_t flags)
{
	unsigned int level = (unsigned int)GET_EL(read_spsr_el3());

	/*
	 * Asynchronous External Abort with syndrome 0xbf000002 on Cortex A53
	 * core means SError interrupt caused by AXI SLVERR on external access.
	 *
	 * In most cases this indicates a bug in U-Boot or Linux kernel driver
	 * pci-aardvark.c which implements access to A3700 PCIe config space.
	 * Driver does not wait for PCIe PIO transfer completion and try to
	 * start a new PCIe PIO transfer while previous has not finished yet.
	 * A3700 PCIe controller in this case sends SLVERR via AXI which results
	 * in a fatal Asynchronous SError interrupt on Cortex A53 CPU.
	 *
	 * Following patches fix that bug in U-Boot and Linux kernel drivers:
	 * https://source.denx.de/u-boot/u-boot/-/commit/eccbd4ad8e4e182638eafbfb87ac139c04f24a01
	 * https://git.kernel.org/stable/c/f18139966d072dab8e4398c95ce955a9742e04f7
	 *
	 * As a hacky workaround for unpatched U-Boot and Linux kernel drivers
	 * ignore all asynchronous aborts with that syndrome value received on
	 * CPU from level lower than EL3.
	 *
	 * Because these aborts are delivered on CPU asynchronously, they are
	 * imprecise and we cannot check the real reason of abort and neither
	 * who and why sent this abort. We expect that on A3700 it is always
	 * PCIe controller.
	 *
	 * Hence ignoring all aborts with this syndrome value is just a giant
	 * hack that we need only because of bugs in old U-Boot and Linux kernel
	 * versions and because it was decided that TF-A would implement this
	 * hack for U-Boot and Linux kernel it in this way. New patched U-Boot
	 * and kernel versions do not need it anymore.
	 *
	 * Links to discussion about this workaround:
	 * https://lore.kernel.org/linux-pci/20190316161243.29517-1-repk@triplefau.lt/
	 * https://lore.kernel.org/linux-pci/971be151d24312cc533989a64bd454b4@www.loen.fr/
	 * https://review.trustedfirmware.org/c/TF-A/trusted-firmware-a/+/1541
	 */
	if (level < MODE_EL3 && ea_reason == ERROR_EA_ASYNC &&
	    syndrome == A53_SERR_INT_AXI_SLVERR_ON_EXTERNAL_ACCESS) {
		ERROR_NL();
		ERROR("Ignoring Asynchronous External Abort with"
		     " syndrome 0x%" PRIx64 " received on 0x%lx from %s\n",
		     syndrome, read_mpidr_el1(), get_el_str(level));
		ERROR("SError interrupt: AXI SLVERR on external access\n");
		ERROR("This indicates a bug in pci-aardvark.c driver\n");
		ERROR("Please update U-Boot/Linux to the latest version\n");
		ERROR_NL();
		console_flush();
		return;
	}

	plat_default_ea_handler(ea_reason, syndrome, cookie, handle, flags);
}
