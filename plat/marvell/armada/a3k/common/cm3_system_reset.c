/*
 * Copyright (C) 2020 Marek Behun, CZ.NIC
 *
 * SPDX-License-Identifier:     BSD-3-Clause
 * https://spdx.org/licenses
 */

#include <stdbool.h>

#include <common/debug.h>
#include <drivers/arm/gic_common.h>
#include <drivers/arm/gicv3.h>
#include <drivers/delay_timer.h>
#include <lib/mmio.h>
#include <lib/utils_def.h>

#include <a3700_pm.h>
#include <platform_def.h>
#include <mvebu_def.h>

/* IO Decoder Error Interrupt Status Registers */
#define MVEBU_DEC_WIN_REGS_BASE(p)		(MVEBU_REGS_BASE + 0xC000 + \
						 (p) * 0x100)
#define MVEBU_DEC_WIN_ERR_INT_STS_REG(p)	(MVEBU_DEC_WIN_REGS_BASE(p) + \
						 0xF8)

/* Cortex-M3 Secure Processor Mailbox Registers */
#define MVEBU_RWTM_PARAM0_REG			(MVEBU_RWTM_REG_BASE)
#define MVEBU_RWTM_CMD_REG			(MVEBU_RWTM_REG_BASE + 0x40)
#define MVEBU_RWTM_HOST_INT_RESET_REG		(MVEBU_RWTM_REG_BASE + 0xC8)
#define MVEBU_RWTM_HOST_INT_MASK_REG		(MVEBU_RWTM_REG_BASE + 0xCC)
#define MVEBU_RWTM_HOST_INT_SP_COMPLETE		BIT(0)

#define MVEBU_RWTM_REBOOT_CMD		0x0009
#define MVEBU_RWTM_REBOOT_MAGIC		0xDEADBEEF

static inline uint32_t a3700_gicd_read(uint32_t reg)
{
	return mmio_read_32(PLAT_MARVELL_GICD_BASE + reg);
}

static inline void a3700_gicd_write(uint32_t reg, uint32_t value)
{
	mmio_write_32(PLAT_MARVELL_GICD_BASE + reg, value);
}

static void a3700_gicd_ctlr_clear_bits(uint32_t bits)
{
	uint32_t val;

	val = a3700_gicd_read(GICD_CTLR);
	if ((val & bits) != 0U) {
		a3700_gicd_write(GICD_CTLR, val & ~bits);
		mdelay(1);

		if ((a3700_gicd_read(GICD_CTLR) & GICD_CTLR_RWP_BIT) != 0U) {
			ERROR("could not clear bits 0x%x in GIC distributor control\n",
			      bits);
		}
	}
}

static void a3700_gic_dist_disable_irqs(void)
{
	int i;

	for (i = 32; i < 224; i += 32) {
		a3700_gicd_write(GICD_ICENABLER + (i >> 3), GENMASK_32(31, 0));
	}
}

static inline uintptr_t a3700_rdist_base(unsigned int proc)
{
	return PLAT_MARVELL_GICR_BASE + (proc << GICR_V3_PCPUBASE_SHIFT);
}

static inline uint32_t a3700_gicr_read(unsigned int proc, uint32_t reg)
{
	return mmio_read_32(a3700_rdist_base(proc) + reg);
}

static inline void a3700_gicr_write(unsigned int proc, uint32_t reg,
				    uint32_t value)
{
	mmio_write_32(a3700_rdist_base(proc) + reg, value);
}

static void a3700_gic_redist_disable_irqs(unsigned int proc)
{
	a3700_gicr_write(proc, GICR_ICENABLER0, GENMASK_32(31, 0));
	mdelay(1);

	if ((a3700_gicr_read(proc, GICR_CTLR) & GICR_CTLR_RWP_BIT) != 0U) {
		ERROR("could not disable core %u PPIs & SGIs\n", proc);
	}
}

static void a3700_gic_redist_mark_asleep(unsigned int proc)
{
	a3700_gicr_write(proc, GICR_WAKER,
			 a3700_gicr_read(proc, GICR_WAKER) | WAKER_PS_BIT);
	mdelay(1);

	if ((a3700_gicr_read(proc, GICR_WAKER) & WAKER_CA_BIT) == 0U) {
		ERROR("could not mark core %u redistributor asleep\n", proc);
	}
}

static void a3700_io_addr_dec_ack_err_irq(void)
{
	unsigned int periph;

	for (periph = 0; periph < 16; ++periph) {
		/* periph 6 does not exist */
		if (periph == 6)
			continue;

		mmio_write_32(MVEBU_DEC_WIN_ERR_INT_STS_REG(periph),
			      GENMASK_32(1, 0));
	}
}

static void a3700_gic_reset(void)
{
	a3700_gic_redist_disable_irqs(0);
	a3700_gic_redist_disable_irqs(1);

	a3700_gic_redist_mark_asleep(0);
	a3700_gic_redist_mark_asleep(1);

	a3700_io_addr_dec_ack_err_irq();

	a3700_pm_ack_irq();

	a3700_gic_dist_disable_irqs();

	a3700_gicd_ctlr_clear_bits(CTLR_ENABLE_G0_BIT | CTLR_ENABLE_G1NS_BIT |
				   CTLR_ENABLE_G1S_BIT);

	/* Clearing ARE_S and ARE_NS bits is undefined in the specification, but
	 * works if the previous operations are successful. We need to do it in
	 * order to put GIC into the same state it was in just after reset. If
	 * this is successful, the rWTM firmware in the secure coprocessor will
	 * reset all other peripherals one by one, load new firmware and boot
	 * it, all without triggering the true warm reset via the WARM_RESET
	 * register (which may hang the board).
	 */

	a3700_gicd_ctlr_clear_bits(CTLR_ARE_S_BIT);
	a3700_gicd_ctlr_clear_bits(CTLR_ARE_NS_BIT);
}

static inline bool rwtm_completed(void)
{
	return (mmio_read_32(MVEBU_RWTM_HOST_INT_RESET_REG) &
		MVEBU_RWTM_HOST_INT_SP_COMPLETE) != 0;
}

static bool rwtm_wait(int ms)
{
	while (ms && !rwtm_completed()) {
		mdelay(1);
		--ms;
	}

	return rwtm_completed();
}

void cm3_system_reset(void)
{
	int tries = 5;

	/* Put GIC into the same state it was just after reset. This is needed
	 * for the reset issue workaround to work.
	 */
	a3700_gic_reset();

	for (; tries > 0; --tries) {
		mmio_clrbits_32(MVEBU_RWTM_HOST_INT_RESET_REG,
				MVEBU_RWTM_HOST_INT_SP_COMPLETE);

		mmio_write_32(MVEBU_RWTM_PARAM0_REG, MVEBU_RWTM_REBOOT_MAGIC);
		mmio_write_32(MVEBU_RWTM_CMD_REG, MVEBU_RWTM_REBOOT_CMD);

		if (rwtm_wait(10)) {
			break;
		}

		mdelay(100);
	}

	/* If we reach here, the command is not implemented. */
	WARN("System reset command not implemented in WTMI firmware!\n");
}
