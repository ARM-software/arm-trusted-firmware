/*
 * Copyright (c) 2020, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <errno.h>

#include <common/fdt_fixup.h>
#include <common/fdt_wrappers.h>
#include <drivers/arm/gicv3.h>
#include <drivers/delay_timer.h>
#include <drivers/generic_delay_timer.h>
#include <lib/extensions/spe.h>
#include <lib/mmio.h>
#include <libfdt.h>

#include "fpga_private.h"
#include <plat/common/platform.h>
#include <platform_def.h>

static entry_point_info_t bl33_image_ep_info;
static unsigned int system_freq;
volatile uint32_t secondary_core_spinlock;

uintptr_t plat_get_ns_image_entrypoint(void)
{
#ifdef PRELOADED_BL33_BASE
	return PRELOADED_BL33_BASE;
#else
	return 0ULL;
#endif
}

uint32_t fpga_get_spsr_for_bl33_entry(void)
{
	return SPSR_64(MODE_EL2, MODE_SP_ELX, DISABLE_ALL_EXCEPTIONS);
}

void bl31_early_platform_setup2(u_register_t arg0, u_register_t arg1,
				u_register_t arg2, u_register_t arg3)
{
	/* Add this core to the VALID mpids list */
	fpga_valid_mpids[plat_my_core_pos()] = VALID_MPID;

	/*
	 * Notify the secondary CPUs that the C runtime is ready
	 * so they can announce themselves.
	 */
	secondary_core_spinlock = C_RUNTIME_READY_KEY;
	dsbish();
	sev();

	fpga_console_init();

	bl33_image_ep_info.pc = plat_get_ns_image_entrypoint();
	bl33_image_ep_info.spsr = fpga_get_spsr_for_bl33_entry();
	SET_SECURITY_STATE(bl33_image_ep_info.h.attr, NON_SECURE);

	/* Set x0-x3 for the primary CPU as expected by the kernel */
	bl33_image_ep_info.args.arg0 = (u_register_t)FPGA_PRELOADED_DTB_BASE;
	bl33_image_ep_info.args.arg1 = 0U;
	bl33_image_ep_info.args.arg2 = 0U;
	bl33_image_ep_info.args.arg3 = 0U;
}

void bl31_plat_arch_setup(void)
{
}

void bl31_platform_setup(void)
{
	/* Write frequency to CNTCRL and initialize timer */
	generic_delay_timer_init();

	/*
	 * Before doing anything else, wait for some time to ensure that
	 * the secondary CPUs have populated the fpga_valid_mpids array.
	 * As the number of secondary cores is unknown and can even be 0,
	 * it is not possible to rely on any signal from them, so use a
	 * delay instead.
	 */
	mdelay(5);

	/*
	 * On the event of a cold reset issued by, for instance, a reset pin
	 * assertion, we cannot guarantee memory to be initialized to zero.
	 * In such scenario, if the secondary cores reached
	 * plat_secondary_cold_boot_setup before the primary one initialized
	 * .BSS, we could end up having a race condition if the spinlock
	 * was not cleared before.
	 *
	 * Similarly, if there were a reset before the spinlock had been
	 * cleared, the secondary cores would find the lock opened before
	 * .BSS is cleared, causing another race condition.
	 *
	 * So clean the spinlock as soon as we think it is safe to reduce the
	 * chances of any race condition on a reset.
	 */
	secondary_core_spinlock = 0UL;

	/* Initialize the GIC driver, cpu and distributor interfaces */
	plat_fpga_gic_init();
}

entry_point_info_t *bl31_plat_get_next_image_ep_info(uint32_t type)
{
	entry_point_info_t *next_image_info;
	next_image_info = &bl33_image_ep_info;

	/* Only expecting BL33: the kernel will run in EL2NS */
	assert(type == NON_SECURE);

	/* None of the images can have 0x0 as the entrypoint */
	if (next_image_info->pc) {
		return next_image_info;
	} else {
		return NULL;
	}
}

/*
 * Even though we sell the FPGA UART as an SBSA variant, it is actually
 * a full fledged PL011. So the baudrate divider registers exist.
 */
#ifndef UARTIBRD
#define UARTIBRD	0x024
#define UARTFBRD	0x028
#endif

/* Round an integer to the closest multiple of a value. */
static unsigned int round_multiple(unsigned int x, unsigned int multiple)
{
	if (multiple < 2) {
		return x;
	}

	return ((x + (multiple / 2 - 1)) / multiple) * multiple;
}

#define PL011_FRAC_SHIFT	6
#define FPGA_DEFAULT_BAUDRATE	38400
#define PL011_OVERSAMPLING	16
static unsigned int pl011_freq_from_divider(unsigned int divider)
{
	unsigned int freq;

	freq = divider * FPGA_DEFAULT_BAUDRATE * PL011_OVERSAMPLING;

	return freq >> PL011_FRAC_SHIFT;
}

/*
 * The FPGAs run most peripherals from one main clock, among them the CPUs,
 * the arch timer, and the UART baud base clock.
 * The SCP knows this frequency and programs the UART clock divider for a
 * 38400 bps baudrate. Recalculate the base input clock from there.
 */
static unsigned int fpga_get_system_frequency(void)
{
	const void *fdt = (void *)(uintptr_t)FPGA_PRELOADED_DTB_BASE;
	int node, err;

	/*
	 * If the arch timer DT node has an explicit clock-frequency property
	 * set, use that, to allow people overriding auto-detection.
	 */
	node = fdt_node_offset_by_compatible(fdt, 0, "arm,armv8-timer");
	if (node >= 0) {
		uint32_t freq;

		err = fdt_read_uint32(fdt, node, "clock-frequency", &freq);
		if (err >= 0) {
			return freq;
		}
	}

	node = fdt_node_offset_by_compatible(fdt, 0, "arm,pl011");
	if (node >= 0) {
		uintptr_t pl011_base;
		unsigned int divider;

		err = fdt_get_reg_props_by_index(fdt, node, 0,
						 &pl011_base, NULL);
		if (err >= 0) {
			divider = mmio_read_32(pl011_base + UARTIBRD);
			divider <<= PL011_FRAC_SHIFT;
			divider += mmio_read_32(pl011_base + UARTFBRD);

			/*
			 * The result won't be exact, due to rounding errors,
			 * but the input frequency was a multiple of 250 KHz.
			 */
			return round_multiple(pl011_freq_from_divider(divider),
					      250000);
		} else {
			WARN("Cannot read PL011 MMIO base\n");
		}
	} else {
		WARN("No PL011 DT node\n");
	}

	/* No PL011 DT node or calculation failed. */
	return FPGA_DEFAULT_TIMER_FREQUENCY;
}

unsigned int plat_get_syscnt_freq2(void)
{
	if (system_freq == 0U) {
		system_freq = fpga_get_system_frequency();
	}

	return system_freq;
}

static void fpga_dtb_update_clock(void *fdt, unsigned int freq)
{
	uint32_t freq_dtb = fdt32_to_cpu(freq);
	uint32_t phandle;
	int node, err;

	node = fdt_node_offset_by_compatible(fdt, 0, "arm,pl011");
	if (node < 0) {
		WARN("%s(): No PL011 DT node found\n", __func__);

		return;
	}

	err = fdt_read_uint32(fdt, node, "clocks", &phandle);
	if (err != 0) {
		WARN("Cannot find clocks property\n");

		return;
	}

	node = fdt_node_offset_by_phandle(fdt, phandle);
	if (node < 0) {
		WARN("Cannot get phandle\n");

		return;
	}

	err = fdt_setprop_inplace(fdt, node,
				  "clock-frequency",
				  &freq_dtb,
				  sizeof(freq_dtb));
	if (err < 0) {
		WARN("Could not update DT baud clock frequency\n");

		return;
	}
}

#define CMDLINE_SIGNATURE	"CMD:"

static int fpga_dtb_set_commandline(void *fdt, const char *cmdline)
{
	int chosen;
	const char *eol;
	char nul = 0;
	int slen, err;

	chosen = fdt_add_subnode(fdt, 0, "chosen");
	if (chosen == -FDT_ERR_EXISTS) {
		chosen = fdt_path_offset(fdt, "/chosen");
	}

	if (chosen < 0) {
		return chosen;
	}

	/*
	 * There is most likely an EOL at the end of the
	 * command line, make sure we terminate the line there.
	 * We can't replace the EOL with a NUL byte in the
	 * source, as this is in read-only memory. So we first
	 * create the property without any termination, then
	 * append a single NUL byte.
	 */
	eol = strchr(cmdline, '\n');
	if (eol == NULL) {
		eol = strchr(cmdline, 0);
	}
	/* Skip the signature and omit the EOL/NUL byte. */
	slen = eol - (cmdline + strlen(CMDLINE_SIGNATURE));
	/*
	 * Let's limit the size of the property, just in case
	 * we find the signature by accident. The Linux kernel
	 * limits to 4096 characters at most (in fact 2048 for
	 * arm64), so that sounds like a reasonable number.
	 */
	if (slen > 4095) {
		slen = 4095;
	}

	err = fdt_setprop(fdt, chosen, "bootargs",
			  cmdline + strlen(CMDLINE_SIGNATURE), slen);
	if (err != 0) {
		return err;
	}

	return fdt_appendprop(fdt, chosen, "bootargs", &nul, 1);
}

static void fpga_prepare_dtb(void)
{
	void *fdt = (void *)(uintptr_t)FPGA_PRELOADED_DTB_BASE;
	const char *cmdline = (void *)(uintptr_t)FPGA_PRELOADED_CMD_LINE;
	int err;

	err = fdt_open_into(fdt, fdt, FPGA_MAX_DTB_SIZE);
	if (err < 0) {
		ERROR("cannot open devicetree at %p: %d\n", fdt, err);
		panic();
	}

	/* Reserve memory used by Trusted Firmware. */
	if (fdt_add_reserved_memory(fdt, "tf-a@80000000", BL31_BASE,
				    BL31_LIMIT - BL31_BASE)) {
		WARN("Failed to add reserved memory node to DT\n");
	}

	/* Check for the command line signature. */
	if (!strncmp(cmdline, CMDLINE_SIGNATURE, strlen(CMDLINE_SIGNATURE))) {
		err = fpga_dtb_set_commandline(fdt, cmdline);
		if (err == 0) {
			INFO("using command line at 0x%x\n",
			     FPGA_PRELOADED_CMD_LINE);
		} else {
			ERROR("failed to put command line into DTB: %d\n", err);
		}
	}

	if (err < 0) {
		ERROR("Error %d extending Device Tree\n", err);
		panic();
	}

	err = fdt_add_cpus_node(fdt, FPGA_MAX_PE_PER_CPU,
				FPGA_MAX_CPUS_PER_CLUSTER,
				FPGA_MAX_CLUSTER_COUNT);

	if (err == -EEXIST) {
		WARN("Not overwriting already existing /cpus node in DTB\n");
	} else {
		if (err < 0) {
			ERROR("Error %d creating the /cpus DT node\n", err);
			panic();
		} else {
			unsigned int nr_cores = fpga_get_nr_gic_cores();

			INFO("Adjusting GICR DT region to cover %u cores\n",
			      nr_cores);
			err = fdt_adjust_gic_redist(fdt, nr_cores,
						    fpga_get_redist_base(),
						    fpga_get_redist_size());
			if (err < 0) {
				ERROR("Error %d fixing up GIC DT node\n", err);
			}
		}
	}

	fpga_dtb_update_clock(fdt, system_freq);

	/* Check whether we support the SPE PMU. Remove the DT node if not. */
	if (!spe_supported()) {
		int node = fdt_node_offset_by_compatible(fdt, 0,
				     "arm,statistical-profiling-extension-v1");

		if (node >= 0) {
			fdt_del_node(fdt, node);
		}
	}

	/* Check whether we have an ITS. Remove the DT node if not. */
	if (!fpga_has_its()) {
		int node = fdt_node_offset_by_compatible(fdt, 0,
							 "arm,gic-v3-its");

		if (node >= 0) {
			fdt_del_node(fdt, node);
		}
	}

	err = fdt_pack(fdt);
	if (err < 0) {
		ERROR("Failed to pack Device Tree at %p: error %d\n", fdt, err);
	}

	clean_dcache_range((uintptr_t)fdt, fdt_blob_size(fdt));
}

void bl31_plat_runtime_setup(void)
{
	fpga_prepare_dtb();
}

void bl31_plat_enable_mmu(uint32_t flags)
{
	/* TODO: determine if MMU needs to be enabled */
}
