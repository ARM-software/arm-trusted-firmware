/*
 * Copyright (c) 2015-2020, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <stdbool.h>

#include <platform_def.h>

#include <arch_helpers.h>
#include <context.h>
#include <common/bl_common.h>
#include <common/debug.h>
#include <drivers/arm/cci.h>
#include <drivers/console.h>
#include <lib/el3_runtime/context_mgmt.h>
#include <lib/mmio.h>
#include <lib/xlat_tables/xlat_tables.h>
#include <plat/common/platform.h>

#include <imx8qm_pads.h>
#include <imx8_iomux.h>
#include <imx8_lpuart.h>
#include <plat_imx8.h>
#include <sci/sci.h>
#include <sec_rsrc.h>

static const unsigned long BL31_COHERENT_RAM_START	= BL_COHERENT_RAM_BASE;
static const unsigned long BL31_COHERENT_RAM_END	= BL_COHERENT_RAM_END;
static const unsigned long BL31_RO_START		= BL_CODE_BASE;
static const unsigned long BL31_RO_END			= BL_CODE_END;
static const unsigned long BL31_RW_END			= BL_END;

IMPORT_SYM(unsigned long, __RW_START__, BL31_RW_START);

static entry_point_info_t bl32_image_ep_info;
static entry_point_info_t bl33_image_ep_info;

#define UART_PAD_CTRL	(PADRING_IFMUX_EN_MASK | PADRING_GP_EN_MASK | \
			(SC_PAD_CONFIG_OUT_IN << PADRING_CONFIG_SHIFT) | \
			(SC_PAD_ISO_OFF << PADRING_LPCONFIG_SHIFT) | \
			(SC_PAD_28FDSOI_DSE_DV_LOW << PADRING_DSE_SHIFT) | \
			(SC_PAD_28FDSOI_PS_PD << PADRING_PULL_SHIFT))

#if defined(IMX_USE_UART0)
#define IMX_RES_UART			SC_R_UART_0
#define IMX_PAD_UART_RX			SC_P_UART0_RX
#define IMX_PAD_UART_TX			SC_P_UART0_TX
#define IMX_PAD_UART_RTS_B		SC_P_UART0_RTS_B
#define IMX_PAD_UART_CTS_B		SC_P_UART0_CTS_B
#elif defined(IMX_USE_UART1)
#define IMX_RES_UART			SC_R_UART_1
#define IMX_PAD_UART_RX			SC_P_UART1_RX
#define IMX_PAD_UART_TX			SC_P_UART1_TX
#define IMX_PAD_UART_RTS_B		SC_P_UART1_RTS_B
#define IMX_PAD_UART_CTS_B		SC_P_UART1_CTS_B
#else
#error "Provide proper UART number in IMX_DEBUG_UART"
#endif

const static int imx8qm_cci_map[] = {
	CLUSTER0_CCI_SLVAE_IFACE,
	CLUSTER1_CCI_SLVAE_IFACE
};

static const mmap_region_t imx_mmap[] = {
	MAP_REGION_FLAT(IMX_REG_BASE, IMX_REG_SIZE, MT_DEVICE | MT_RW),
	{0}
};

static uint32_t get_spsr_for_bl33_entry(void)
{
	unsigned long el_status;
	unsigned long mode;
	uint32_t spsr;

	/* figure out what mode we enter the non-secure world */
	el_status = read_id_aa64pfr0_el1() >> ID_AA64PFR0_EL2_SHIFT;
	el_status &= ID_AA64PFR0_ELX_MASK;

	mode = (el_status) ? MODE_EL2 : MODE_EL1;
	spsr = SPSR_64(mode, MODE_SP_ELX, DISABLE_ALL_EXCEPTIONS);

	return spsr;
}

#if DEBUG_CONSOLE_A53
static void lpuart32_serial_setbrg(unsigned int base, int baudrate)
{
	unsigned int sbr, osr, baud_diff, tmp_osr, tmp_sbr;
	unsigned int diff1, diff2, tmp, rate;

	if (baudrate == 0)
		panic();

	sc_pm_get_clock_rate(ipc_handle, IMX_RES_UART, 2, &rate);

	baud_diff = baudrate;
	osr = 0;
	sbr = 0;
	for (tmp_osr = 4; tmp_osr <= 32; tmp_osr++) {
		tmp_sbr = (rate / (baudrate * tmp_osr));
		if (tmp_sbr == 0)
			tmp_sbr = 1;

		/* calculate difference in actual baud w/ current values */
		diff1 = rate / (tmp_osr * tmp_sbr) - baudrate;
		diff2 = rate / (tmp_osr * (tmp_sbr + 1));

		/* select best values between sbr and sbr+1 */
		if (diff1 > (baudrate - diff2)) {
			diff1 = baudrate - diff2;
			tmp_sbr++;
		}

		if (diff1 <= baud_diff) {
			baud_diff = diff1;
			osr = tmp_osr;
			sbr = tmp_sbr;
		}
	}

	tmp = mmio_read_32(IMX_BOOT_UART_BASE + BAUD);

	if ((osr > 3) && (osr < 8))
		tmp |= LPUART_BAUD_BOTHEDGE_MASK;

	tmp &= ~LPUART_BAUD_OSR_MASK;
	tmp |= LPUART_BAUD_OSR(osr - 1);
	tmp &= ~LPUART_BAUD_SBR_MASK;
	tmp |= LPUART_BAUD_SBR(sbr);

	/* explicitly disable 10 bit mode & set 1 stop bit */
	tmp &= ~(LPUART_BAUD_M10_MASK | LPUART_BAUD_SBNS_MASK);

	mmio_write_32(IMX_BOOT_UART_BASE + BAUD, tmp);
}

static int lpuart32_serial_init(unsigned int base)
{
	unsigned int tmp;

	/* disable TX & RX before enabling clocks */
	tmp = mmio_read_32(IMX_BOOT_UART_BASE + CTRL);
	tmp &= ~(CTRL_TE | CTRL_RE);
	mmio_write_32(IMX_BOOT_UART_BASE + CTRL, tmp);

	mmio_write_32(IMX_BOOT_UART_BASE + MODIR, 0);
	mmio_write_32(IMX_BOOT_UART_BASE + FIFO, ~(FIFO_TXFE | FIFO_RXFE));

	mmio_write_32(IMX_BOOT_UART_BASE + MATCH, 0);

	/* provide data bits, parity, stop bit, etc */
	lpuart32_serial_setbrg(base, IMX_BOOT_UART_BAUDRATE);

	/* eight data bits no parity bit */
	tmp = mmio_read_32(IMX_BOOT_UART_BASE + CTRL);
	tmp &= ~(LPUART_CTRL_PE_MASK | LPUART_CTRL_PT_MASK | LPUART_CTRL_M_MASK);
	mmio_write_32(IMX_BOOT_UART_BASE + CTRL, tmp);

	mmio_write_32(IMX_BOOT_UART_BASE + CTRL, CTRL_RE | CTRL_TE);

	mmio_write_32(IMX_BOOT_UART_BASE + DATA, 0x55);
	mmio_write_32(IMX_BOOT_UART_BASE + DATA, 0x55);
	mmio_write_32(IMX_BOOT_UART_BASE + DATA, 0x0A);

	return 0;
}
#endif

void mx8_partition_resources(void)
{
	sc_rm_pt_t secure_part, os_part;
	sc_rm_mr_t mr, mr_record = 64;
	sc_faddr_t start, end;
	bool owned, owned2;
	sc_err_t err;
	int i;

	err = sc_rm_get_partition(ipc_handle, &secure_part);

	err = sc_rm_partition_alloc(ipc_handle, &os_part, false, false,
		false, false, false);

	err = sc_rm_set_parent(ipc_handle, os_part, secure_part);

	/* set secure resources to NOT-movable */
	for (i = 0; i < ARRAY_SIZE(secure_rsrcs); i++) {
		err = sc_rm_set_resource_movable(ipc_handle, secure_rsrcs[i],
			secure_rsrcs[i], false);
		if (err)
			ERROR("sc_rm_set_resource_movable: rsrc %u, ret %u\n",
				secure_rsrcs[i], err);
	}

	owned = sc_rm_is_resource_owned(ipc_handle, SC_R_M4_0_PID0);
	if (owned) {
		err = sc_rm_set_resource_movable(ipc_handle, SC_R_M4_0_PID0,
				SC_R_M4_0_PID0, false);
		if (err)
			ERROR("sc_rm_set_resource_movable: rsrc %u, ret %u\n",
				SC_R_M4_0_PID0, err);
	}

	owned2 = sc_rm_is_resource_owned(ipc_handle, SC_R_M4_1_PID0);
	if (owned2) {
		err = sc_rm_set_resource_movable(ipc_handle, SC_R_M4_1_PID0,
				SC_R_M4_1_PID0, false);
		if (err)
			ERROR("sc_rm_set_resource_movable: rsrc %u, ret %u\n",
				SC_R_M4_1_PID0, err);
	}
	/* move all movable resources and pins to non-secure partition */
	err = sc_rm_move_all(ipc_handle, secure_part, os_part, true, true);
	if (err)
		ERROR("sc_rm_move_all: %u\n", err);

	/* iterate through peripherals to give NS OS part access */
	for (i = 0; i < ARRAY_SIZE(ns_access_allowed); i++) {
		err = sc_rm_set_peripheral_permissions(ipc_handle, ns_access_allowed[i],
			os_part, SC_RM_PERM_FULL);
		if (err)
			ERROR("sc_rm_set_peripheral_permissions: rsrc %u, \
				ret %u\n", ns_access_allowed[i], err);
	}

	if (owned) {
		err = sc_rm_set_resource_movable(ipc_handle, SC_R_M4_0_PID0,
				SC_R_M4_0_PID0, true);
		if (err)
			ERROR("sc_rm_set_resource_movable: rsrc %u, ret %u\n",
				SC_R_M4_0_PID0, err);
		err = sc_rm_assign_resource(ipc_handle, os_part, SC_R_M4_0_PID0);
		if (err)
			ERROR("sc_rm_assign_resource: rsrc %u, ret %u\n",
				SC_R_M4_0_PID0, err);
	}
	if (owned2) {
		err = sc_rm_set_resource_movable(ipc_handle, SC_R_M4_1_PID0,
				SC_R_M4_1_PID0, true);
		if (err)
			ERROR("sc_rm_set_resource_movable: rsrc %u, ret %u\n",
				SC_R_M4_1_PID0, err);
		err = sc_rm_assign_resource(ipc_handle, os_part, SC_R_M4_1_PID0);
		if (err)
			ERROR("sc_rm_assign_resource: rsrc %u, ret %u\n",
				SC_R_M4_1_PID0, err);
	}

	/*
	 * sc_rm_set_peripheral_permissions
	 * sc_rm_set_memreg_permissions
	 * sc_rm_set_pin_movable
	 */

	for (mr = 0; mr < 64; mr++) {
		owned = sc_rm_is_memreg_owned(ipc_handle, mr);
		if (owned) {
			err = sc_rm_get_memreg_info(ipc_handle, mr, &start, &end);
			if (err)
				ERROR("Memreg get info failed, %u\n", mr);
			NOTICE("Memreg %u 0x%llx -- 0x%llx\n", mr, start, end);
			if (BL31_BASE >= start && (BL31_LIMIT - 1) <= end) {
				mr_record = mr; /* Record the mr for ATF running */
			} else {
				err = sc_rm_assign_memreg(ipc_handle, os_part, mr);
				if (err)
					ERROR("Memreg assign failed, 0x%llx -- 0x%llx, \
						err %d\n", start, end, err);
			}
		}
	}

	if (mr_record != 64) {
		err = sc_rm_get_memreg_info(ipc_handle, mr_record, &start, &end);
		if (err)
			ERROR("Memreg get info failed, %u\n", mr_record);
		if ((BL31_LIMIT - 1) < end) {
			err = sc_rm_memreg_alloc(ipc_handle, &mr, BL31_LIMIT, end);
			if (err)
				ERROR("sc_rm_memreg_alloc failed, 0x%llx -- 0x%llx\n",
					(sc_faddr_t)BL31_LIMIT, end);
			err = sc_rm_assign_memreg(ipc_handle, os_part, mr);
			if (err)
				ERROR("Memreg assign failed, 0x%llx -- 0x%llx\n",
					(sc_faddr_t)BL31_LIMIT, end);
		}

		if (start < (BL31_BASE - 1)) {
			err = sc_rm_memreg_alloc(ipc_handle, &mr, start, BL31_BASE - 1);
			if (err)
				ERROR("sc_rm_memreg_alloc failed, 0x%llx -- 0x%llx\n",
					start, (sc_faddr_t)BL31_BASE - 1);
			err = sc_rm_assign_memreg(ipc_handle, os_part, mr);
				if (err)
					ERROR("Memreg assign failed, 0x%llx -- 0x%llx\n",
						start, (sc_faddr_t)BL31_BASE - 1);
		}
	}

	if (err)
		NOTICE("Partitioning Failed\n");
	else
		NOTICE("Non-secure Partitioning Succeeded\n");

}

void bl31_early_platform_setup2(u_register_t arg0, u_register_t arg1,
				u_register_t arg2, u_register_t arg3)
{
#if DEBUG_CONSOLE
	static console_t console;
#endif
	if (sc_ipc_open(&ipc_handle, SC_IPC_BASE) != SC_ERR_NONE)
		panic();

#if DEBUG_CONSOLE_A53
	sc_pm_set_resource_power_mode(ipc_handle, IMX_RES_UART,
				      SC_PM_PW_MODE_ON);
	sc_pm_clock_rate_t rate = 80000000;
	sc_pm_set_clock_rate(ipc_handle, IMX_RES_UART, 2, &rate);
	sc_pm_clock_enable(ipc_handle, IMX_RES_UART, 2, true, false);

	/* configure UART pads */
	sc_pad_set(ipc_handle, IMX_PAD_UART_RX, UART_PAD_CTRL);
	sc_pad_set(ipc_handle, IMX_PAD_UART_TX, UART_PAD_CTRL);
	sc_pad_set(ipc_handle, IMX_PAD_UART_RTS_B, UART_PAD_CTRL);
	sc_pad_set(ipc_handle, IMX_PAD_UART_CTS_B, UART_PAD_CTRL);
	lpuart32_serial_init(IMX_BOOT_UART_BASE);
#endif

#if DEBUG_CONSOLE
	console_lpuart_register(IMX_BOOT_UART_BASE, IMX_BOOT_UART_CLK_IN_HZ,
		     IMX_CONSOLE_BAUDRATE, &console);
#endif

	/* turn on MU1 for non-secure OS/Hypervisor */
	sc_pm_set_resource_power_mode(ipc_handle, SC_R_MU_1A, SC_PM_PW_MODE_ON);
	/* Turn on GPT_0's power & clock for non-secure OS/Hypervisor */
	sc_pm_set_resource_power_mode(ipc_handle, SC_R_GPT_0, SC_PM_PW_MODE_ON);
	sc_pm_clock_enable(ipc_handle, SC_R_GPT_0, SC_PM_CLK_PER, true, 0);
	mmio_write_32(IMX_GPT_LPCG_BASE, mmio_read_32(IMX_GPT_LPCG_BASE) | (1 << 25));

	/*
	 * create new partition for non-secure OS/Hypervisor
	 * uses global structs defined in sec_rsrc.h
	 */
	mx8_partition_resources();

	bl33_image_ep_info.pc = PLAT_NS_IMAGE_OFFSET;
	bl33_image_ep_info.spsr = get_spsr_for_bl33_entry();
	SET_SECURITY_STATE(bl33_image_ep_info.h.attr, NON_SECURE);

	/* init the first cluster's cci slave interface */
	cci_init(PLAT_CCI_BASE, imx8qm_cci_map, PLATFORM_CLUSTER_COUNT);
	cci_enable_snoop_dvm_reqs(MPIDR_AFFLVL1_VAL(read_mpidr_el1()));
}

void bl31_plat_arch_setup(void)
{
	unsigned long ro_start = BL31_RO_START;
	unsigned long ro_size = BL31_RO_END - BL31_RO_START;
	unsigned long rw_start = BL31_RW_START;
	unsigned long rw_size = BL31_RW_END - BL31_RW_START;
#if USE_COHERENT_MEM
	unsigned long coh_start = BL31_COHERENT_RAM_START;
	unsigned long coh_size = BL31_COHERENT_RAM_END - BL31_COHERENT_RAM_START;
#endif

	mmap_add_region(ro_start, ro_start, ro_size,
		MT_RO | MT_MEMORY | MT_SECURE);
	mmap_add_region(rw_start, rw_start, rw_size,
		MT_RW | MT_MEMORY | MT_SECURE);
	mmap_add(imx_mmap);

#if USE_COHERENT_MEM
	mmap_add_region(coh_start, coh_start, coh_size,
			MT_DEVICE | MT_RW | MT_SECURE);
#endif

	/* setup xlat table */
	init_xlat_tables();
	/* enable the MMU */
	enable_mmu_el3(0);
}

void bl31_platform_setup(void)
{
	plat_gic_driver_init();
	plat_gic_init();
}

entry_point_info_t *bl31_plat_get_next_image_ep_info(unsigned int type)
{
	if (type == NON_SECURE)
		return &bl33_image_ep_info;
	if (type == SECURE)
		return &bl32_image_ep_info;

	return NULL;
}

unsigned int plat_get_syscnt_freq2(void)
{
	return COUNTER_FREQUENCY;
}

void bl31_plat_runtime_setup(void)
{
	return;
}
