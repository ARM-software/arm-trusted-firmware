/*
 * Copyright (c) 2026, STMicroelectronics - All Rights Reserved
 *
 * SPDX-License-Identifier: GPL-2.0+ OR BSD-3-Clause
 */

#include <inttypes.h>

#include <libfdt.h>

#include <common/debug.h>
#include <common/fdt_wrappers.h>
#include <drivers/clk.h>
#include <drivers/delay_timer.h>
#if STM32MP_HYPERFLASH
#include <drivers/hyperflash.h>
#else
#include <drivers/spi_mem.h>
#endif
#include <drivers/st/stm32_gpio.h>
#include <drivers/st/stm32_ospi.h>
#include <drivers/st/stm32mp_reset.h>
#include <lib/mmio.h>
#include <lib/utils.h>
#include <lib/utils_def.h>

#include <platform_def.h>

/* Timeout for device interface reset */
#define _TIMEOUT_US_1_MS	1000U

/* OCTOSPI registers */
#define _OSPI_CR		0x00U
#define _OSPI_DCR1		0x08U
#define _OSPI_DCR2		0x0CU
#define _OSPI_SR		0x20U
#define _OSPI_FCR		0x24U
#define _OSPI_DLR		0x40U
#define _OSPI_AR		0x48U
#define _OSPI_DR		0x50U
#define _OSPI_CCR		0x100U
#define _OSPI_TCR		0x108U
#define _OSPI_IR		0x110U
#define _OSPI_ABR		0x120U
#define _OSPI_HLCR		0x200U

/* OCTOSPI control register */
#define _OSPI_CR_EN		BIT_32(0)
#define _OSPI_CR_ABORT		BIT_32(1)
#define _OSPI_CR_CSSEL		BIT_32(24)
#define _OSPI_CR_FMODE		GENMASK_32(29, 28)
#define _OSPI_CR_FMODE_SHIFT	28U
#define _OSPI_CR_FMODE_INDW	0U
#define _OSPI_CR_FMODE_INDR	1U
#define _OSPI_CR_FMODE_MM	3U

/* OCTOSPI device configuration register 1 */
#define _OSPI_DCR1_CKMODE	BIT_32(0)
#define _OSPI_DCR1_DLYBYP	BIT_32(3)
#define _OSPI_DCR1_CSHT		GENMASK_32(13, 8)
#define _OSPI_DCR1_CSHT_SHIFT	8U
#define _OSPI_DCR1_DEVSIZE	GENMASK_32(20, 16)
#define _OSPI_DCR1_MTYP		GENMASK_32(26, 24)
#define _OSPI_DCR1_MTYP_SHIFT	24U
#define _OSPI_DCR1_MTYP_MX_MODE	1U
#define _OSPI_DCR1_MTYP_HB_MM	4U

/* OCTOSPI device configuration register 2 */
#define _OSPI_DCR2_PRESCALER	GENMASK_32(7, 0)

/* OCTOSPI status register */
#define _OSPI_SR_TEF		BIT_32(0)
#define _OSPI_SR_TCF		BIT_32(1)
#define _OSPI_SR_FTF		BIT_32(2)
#define _OSPI_SR_SMF		BIT_32(3)
#define _OSPI_SR_BUSY		BIT_32(5)

/* OCTOSPI flag clear register */
#define _OSPI_FCR_CTEF		BIT_32(0)
#define _OSPI_FCR_CTCF		BIT_32(1)
#define _OSPI_FCR_CSMF		BIT_32(3)

/* OCTOSPI communication configuration register */
#define _OSPI_CCR_IMODE		GENMASK_32(2, 0)
#define _OSPI_CCR_IDTR		BIT_32(3)
#define _OSPI_CCR_ISIZE		GENMASK_32(5, 4)
#define _OSPI_CCR_ISIZE_SHIFT	4U
#define _OSPI_CCR_ADMODE	GENMASK_32(10, 8)
#define _OSPI_CCR_ADMODE_SHIFT	8U
#define _OSPI_CCR_ADMODE_8LINES 4U
#define _OSPI_CCR_ADDTR		BIT_32(11)
#define _OSPI_CCR_ADSIZE	GENMASK_32(13, 12)
#define _OSPI_CCR_ADSIZE_SHIFT	12U
#define _OSPI_CCR_ADSIZE_32BITS 3U
#define _OSPI_CCR_DMODE		GENMASK_32(26, 24)
#define _OSPI_CCR_DMODE_SHIFT	24U
#define _OSPI_CCR_DMODE_8LINES	4U
#define _OSPI_CCR_DDTR		BIT_32(27)
#define _OSPI_CCR_DQSE		BIT_32(29)

/* OCTOSPI timing configuration register */
#define _OSPI_TCR_DCYC		GENMASK_32(4, 0)
#define _OSPI_TCR_DHQC		BIT_32(28)
#define _OSPI_TCR_SSHIFT	BIT_32(30)

/* OCTOSPI hyperbus latency configuration register */
#define _OSPI_HLCR_WZL		BIT_32(1)
#define _OSPI_HLCR_TACC		GENMASK_32(15, 8)
#define _OSPI_HLCR_TACC_SHIFT	8U

#define _OSPI_MAX_CHIP		2U
#define _OSPI_MAX_RESET		2U

#define _OSPI_FIFO_TIMEOUT_US	30U
#define _OSPI_CMD_TIMEOUT_US	1000U
#define _OSPI_BUSY_TIMEOUT_US	100U
#define _OSPI_ABT_TIMEOUT_US	100U

#define _OMM_MAX_OSPI		2U
#define _OSPI_MAX_CS		2U

#define _DT_IOM_COMPAT		"st,stm32mp25-omm"
#define _DT_OSPI_COMPAT		"st,stm32mp25-ospi"

#define _FREQ_100MHZ		100000000U
#define _DLYB_FREQ_50MHZ	50000000U
#define _OSPI_NSEC_PER_SEC	1000000000U

#define _OP_READ_ID		0x9FU
#define _MAX_ID_LEN		8U

#define _MACRONIX_ID		0xC2U

#if !STM32MP_HYPERFLASH
struct stm32_ospi_flash {
	uint64_t str_idcode;
	uint64_t dtr_idcode;
	bool is_spi_nor;
	bool str_calibration_done_once;
	bool dtr_calibration_done_once;
	bool octal_dtr;
};
#endif /* STM32MP_HYPERFLASH */

struct stm32_ospi_ctrl {
	uintptr_t reg_base;
	uintptr_t mm_base;
	size_t mm_size;
	unsigned long clock_id;
#if !STM32MP_HYPERFLASH
	struct stm32_ospi_flash flash;
#endif /* STM32MP_HYPERFLASH */
	uint8_t bank;
	bool is_calibrating;
};

static struct stm32_ospi_ctrl stm32_ospi;

static uintptr_t ospi_base(void)
{
	return stm32_ospi.reg_base;
}

static int stm32_ospi_wait_for_not_busy(void)
{
	uint64_t timeout = timeout_init_us(_OSPI_BUSY_TIMEOUT_US);

	while ((mmio_read_32(ospi_base() + _OSPI_SR) & _OSPI_SR_BUSY) != 0U) {
		if (timeout_elapsed(timeout)) {
			ERROR("%s: busy timeout\n", __func__);
			return -ETIMEDOUT;
		}
	}

	return 0;
}

static int stm32_ospi_wait_cmd(void)
{
	int ret = 0;
	uint64_t timeout;

	timeout = timeout_init_us(_OSPI_CMD_TIMEOUT_US);
	while ((mmio_read_32(ospi_base() + _OSPI_SR) & _OSPI_SR_TCF) == 0U) {
		if (timeout_elapsed(timeout)) {
			ret = -ETIMEDOUT;
			ERROR("%s: cmd timeout\n", __func__);
			break;
		}
	}

	if ((ret == 0) && ((mmio_read_32(ospi_base() + _OSPI_SR) &
			   _OSPI_SR_TEF) != 0U)) {
		ERROR("%s: transfer error\n", __func__);
		ret = -EIO;
	}

	/* Clear flags */
	mmio_write_32(ospi_base() + _OSPI_FCR, _OSPI_FCR_CTCF | _OSPI_FCR_CTEF);

	if (ret == 0) {
		ret = stm32_ospi_wait_for_not_busy();
	}

	return ret;
}

static void stm32_ospi_read_fifo(uint8_t *val, uintptr_t addr)
{
	*val = mmio_read_8(addr);
}

static void stm32_ospi_write_fifo(uint8_t *val, uintptr_t addr)
{
	mmio_write_8(addr, *val);
}

static int stm32_ospi_poll(uint8_t *buf, uint32_t nbytes, bool read)
{
	void (*fifo)(uint8_t *val, uintptr_t addr);
	uint32_t len;

	if (read) {
		fifo = stm32_ospi_read_fifo;
	} else {
		fifo = stm32_ospi_write_fifo;
	}

	for (len = nbytes; len != 0U; len--) {
		uint64_t timeout = timeout_init_us(_OSPI_FIFO_TIMEOUT_US);

		while ((mmio_read_32(ospi_base() + _OSPI_SR) &
			_OSPI_SR_FTF) == 0U) {
			if (timeout_elapsed(timeout)) {
				if (!stm32_ospi.is_calibrating) {
					ERROR("%s: fifo timeout\n", __func__);
				}

				return -ETIMEDOUT;
			}
		}

		fifo(buf++, ospi_base() + _OSPI_DR);
	}

	return 0;
}

static int stm32_ospi_mm(uint8_t *buf, uint32_t nbytes, size_t addr)
{
	uintptr_t from = stm32_ospi.mm_base + addr;
	uint8_t *buff = buf;
	uint64_t length = nbytes;
	uint64_t tmp;
	uint64_t i = 0U;

	while ((length != 0U) && ((from & (sizeof(uint64_t) - UL(1))) != 0U)) {
		buff[i] = mmio_read_8(from);
		i++;
		from++;
		length--;
	}

	while (length >= sizeof(uint64_t)) {
		tmp = mmio_read_64(from);
		(void)memcpy((void *)&buff[i], (const void *)&tmp, sizeof(uint64_t));
		i += sizeof(uint64_t);
		from += sizeof(uint64_t);
		length -= sizeof(uint64_t);
	}

	while (length != 0U) {
		buff[i] = mmio_read_8(from);
		i++;
		from++;
		length--;
	}

	return 0;
}

static int stm32_ospi_dtr_calibrate(uint32_t prescaler, unsigned int bus_freq,
				    int (*check_transfer)(void))
{
	uint32_t period_ps = 0U;
	uint8_t window_len = 0U;
	int ret;
	bool bypass_mode = false;

	mmio_clrbits_32(ospi_base() + _OSPI_DCR1, _OSPI_DCR1_DLYBYP);

	if (prescaler != 0U) {
		mmio_setbits_32(ospi_base() + _OSPI_TCR, _OSPI_TCR_DHQC);
	}

	if (bus_freq <= _DLYB_FREQ_50MHZ) {
		bypass_mode = true;
		period_ps = _OSPI_NSEC_PER_SEC / (bus_freq / 1000U);
	}

	ret = stm32mp_syscfg_dlyb_init(stm32_ospi.bank, bypass_mode,
				       period_ps);
	if (ret != 0) {
		return ret;
	}

	if (bypass_mode || (prescaler != 0U)) {
		/* perform only RX TAP selection */
		ret = stm32mp_syscfg_dlyb_find_tap(stm32_ospi.bank,
						   check_transfer,
						   true, &window_len);
	} else {
		/* perform RX/TX TAP selection */
		ret = stm32mp_syscfg_dlyb_find_tap(stm32_ospi.bank,
						   check_transfer,
						   false, &window_len);
	}

	if (ret != 0) {
		ERROR("Calibration failed\n");

		if (!bypass_mode) {
			stm32mp_syscfg_dlyb_stop(stm32_ospi.bank);
		}
	}

	return ret;
}

#if STM32MP_HYPERFLASH
static int stm32_ospi_hb_read(unsigned int offset, uint8_t *buffer,
			      size_t length, size_t *length_read)
{
	*length_read = length;

	return stm32_ospi_mm(buffer, length, offset);
}

static int stm32_ospi_hb_xfer(unsigned int offset, uint16_t wdata,
			      uint16_t *rdata)
{
	int ret;

	mmio_clrsetbits_32(ospi_base() + _OSPI_CR, _OSPI_CR_FMODE,
			   rdata != NULL ?
			   _OSPI_CR_FMODE_INDR << _OSPI_CR_FMODE_SHIFT :
			   _OSPI_CR_FMODE_INDW << _OSPI_CR_FMODE_SHIFT);
	mmio_write_32(ospi_base() + _OSPI_AR, offset);

	if (rdata != NULL) {
		ret = stm32_ospi_poll((uint8_t *)rdata, 2U, true);
	} else {
		ret = stm32_ospi_poll((uint8_t *)&wdata, 2U, false);
	}
	if (ret != 0) {
		return ret;
	}

	/* Wait end of tx in indirect mode */
	ret = stm32_ospi_wait_cmd();
	if (ret != 0) {
		return ret;
	}

	VERBOSE("%s: %s 0x%x @ 0x%x\n", __func__,
		rdata != NULL ? "read" : "write",
		rdata != NULL ? (unsigned int)*rdata : (unsigned int)wdata,
		offset);

	return ret;
}

static uint16_t stm32_ospi_hb_read16(uint32_t offset)
{
	int ret;
	uint16_t rdata = 0U;

	ret = stm32_ospi_hb_xfer(offset, 0U, &rdata);
	if (ret != 0) {
		ERROR("%s failed, ret=%i\n", __func__, ret);
		panic();
	}

	return rdata;
}

static void stm32_ospi_hb_write16(uint16_t value, uint32_t offset)
{
	int ret;

	ret = stm32_ospi_hb_xfer(offset, value, NULL);
	if (ret != 0) {
		ERROR("%s failed, ret=%i\n", __func__, ret);
		panic();
	}
};

static int stm32_ospi_hb_test_cfi(void)
{
	int ret = -EIO;
	uint16_t qry[3];

	/* Reset/Exit from CFI */
	stm32_ospi_hb_write16(0xF0U, 0U);
	/* Enter in CFI */
	stm32_ospi_hb_write16(0x98U, 0xAAU);

	qry[0] = stm32_ospi_hb_read16(0x20U);
	qry[1] = stm32_ospi_hb_read16(0x22U);
	qry[2] = stm32_ospi_hb_read16(0x24U);
	if ((qry[0] == 'Q') && (qry[1] == 'R') && (qry[2] == 'Y')) {
		ret = 0;
	}

	/* Reset/Exit from CFI */
	stm32_ospi_hb_write16(0xF0U, 0U);
	stm32_ospi_hb_write16(0xFFU, 0U);

	return ret;
}

static int stm32_ospi_hb_test_jedec(void)
{
	int ret = -EIO;
	uint16_t sfdp[2];

	stm32_ospi_hb_write16(0xF0U, 0U);
	stm32_ospi_hb_write16(0xAAU, 0xAAAU);
	stm32_ospi_hb_write16(0x55U, 0x554U);
	stm32_ospi_hb_write16(0x90U, 0xAAAU);

	sfdp[0] = stm32_ospi_hb_read16(0U);
	sfdp[1] = stm32_ospi_hb_read16(0x2U);

	/* compare with "SF" & "DP" */
	if ((sfdp[0] == 0x4653) && (sfdp[1] == 0x5044)) {
		ret = 0;
	}

	stm32_ospi_hb_write16(0xF0U, 0U);

	return ret;
}

static int stm32_ospi_hb_init(void *fdt, int bus_node)
{
	int ret;
	int nchips = 0;
	int bus_subnode = 0;
	const fdt32_t *cuint = NULL;
	bool wzl = false;
	bool jedec_flash = false;
	unsigned int tacc = 0U;
	unsigned int max_freq = 0U;
	unsigned long ospi_clk = clk_get_rate(stm32_ospi.clock_id);
	uint32_t prescaler = UINT8_MAX;
	unsigned int bus_freq;
	unsigned int period;
	uint32_t ccr;

	if (ospi_clk == 0U) {
		return -EINVAL;
	}

	fdt_for_each_subnode(bus_subnode, fdt, bus_node) {
		nchips++;
	}

	if (nchips != 1) {
		ERROR("Only one hyperflash device is currently supported\n");
		return -EINVAL;
	}

	fdt_for_each_subnode(bus_subnode, fdt, bus_node) {
		cuint = fdt_getprop(fdt, bus_subnode, "st,max-frequency", NULL);
		if (cuint == NULL) {
			ERROR("Max frequency not set\n");
			return -EINVAL;
		}
		max_freq = fdt32_to_cpu(*cuint);

		cuint = fdt_getprop(fdt, bus_subnode, "st,tacc-ns", NULL);
		if (cuint != NULL) {
			tacc = fdt32_to_cpu(*cuint);
		}

		if ((fdt_getprop(fdt, bus_subnode, "st,wzl", NULL)) != NULL) {
			wzl = true;
		}

		if (fdt_node_check_compatible(fdt, bus_subnode,
					      "jedec-flash") == 0) {
			jedec_flash = true;
		}
	}

	/* Enable the controller */
	mmio_setbits_32(ospi_base() + _OSPI_CR, _OSPI_CR_EN);

	/* Set MTYP to HyperBus memory-map mode */
	mmio_clrsetbits_32(ospi_base() + _OSPI_DCR1, _OSPI_DCR1_MTYP,
			   _OSPI_DCR1_MTYP_HB_MM << _OSPI_DCR1_MTYP_SHIFT);

	/* Set the prescaler */
	if (max_freq > 0U) {
		prescaler = div_round_up(ospi_clk, max_freq) - 1U;
		if (prescaler > UINT8_MAX) {
			prescaler = UINT8_MAX;
		}
	}

	mmio_clrsetbits_32(ospi_base() + _OSPI_DCR2, _OSPI_DCR2_PRESCALER,
			   prescaler);

	/* Configure data length for 2 bytes read/write */
	mmio_write_32(ospi_base() + _OSPI_DLR, 1U);

	/*
	 * Set access time latency
	 * Set write zero latency
	 */
	bus_freq = div_round_up(ospi_clk, prescaler + 1U);
	period = _OSPI_NSEC_PER_SEC / bus_freq;
	mmio_clrsetbits_32(ospi_base() + _OSPI_HLCR,
			   _OSPI_HLCR_TACC | _OSPI_HLCR_WZL,
			   div_round_up(tacc, period) << _OSPI_HLCR_TACC_SHIFT);
	if (wzl) {
		mmio_setbits_32(ospi_base() + _OSPI_HLCR, _OSPI_HLCR_WZL);
	}

	/* Configure read access */
	ccr = _OSPI_CCR_DQSE | _OSPI_CCR_DDTR | _OSPI_CCR_ADDTR;
	ccr |= _OSPI_CCR_DMODE_8LINES << _OSPI_CCR_DMODE_SHIFT;
	ccr |= _OSPI_CCR_ADSIZE_32BITS << _OSPI_CCR_ADSIZE_SHIFT;
	ccr |= _OSPI_CCR_ADMODE_8LINES << _OSPI_CCR_ADMODE_SHIFT;
	mmio_write_32(ospi_base() + _OSPI_CCR, ccr);

	/* Calibrate the DLL */
	if (jedec_flash) {
		ret = stm32_ospi_dtr_calibrate(prescaler, bus_freq,
					       stm32_ospi_hb_test_jedec);
	} else {
		ret = stm32_ospi_dtr_calibrate(prescaler, bus_freq,
					       stm32_ospi_hb_test_cfi);
	}
	if (ret != 0) {
		return ret;
	}

	/* Set the controller in memory map mode */
	mmio_clrsetbits_32(ospi_base() + _OSPI_CR, _OSPI_CR_FMODE,
			   _OSPI_CR_FMODE_MM << _OSPI_CR_FMODE_SHIFT);

	return 0;
}

static const struct hyperflash_ctrl_ops stm32_ospi_ctrl_ops = {
	.read = stm32_ospi_hb_read,
};
#else /* STM32MP_HYPERFLASH */
static int stm32_ospi_tx(const struct spi_mem_op *op, uint8_t fmode)
{
	struct stm32_ospi_flash *flash = &stm32_ospi.flash;
	uint8_t dummy = 0xFFU;
	int ret;

	if (op->data.nbytes == 0U) {
		return 0;
	}

	if (fmode == _OSPI_CR_FMODE_MM) {
		return stm32_ospi_mm((uint8_t *)op->data.buf, op->data.nbytes,
				     (size_t)op->addr.val);
	}

	if (flash->octal_dtr && ((op->addr.val % 2U) != 0U)) {
		/* Read/write dummy byte */
		ret = stm32_ospi_poll(&dummy, 1U,
				      op->data.dir == SPI_MEM_DATA_IN);
		if (ret != 0) {
			return ret;
		}
	}

	ret = stm32_ospi_poll((uint8_t *)op->data.buf, op->data.nbytes,
			      op->data.dir == SPI_MEM_DATA_IN);
	if (ret != 0) {
		return ret;
	}

	if (flash->octal_dtr &&
	    (((op->addr.val + op->data.nbytes) % 2U) != 0U)) {
		/* Read/write dummy byte */
		ret = stm32_ospi_poll(&dummy, 1U,
				      op->data.dir == SPI_MEM_DATA_IN);
		if (ret != 0) {
			return ret;
		}
	}

	return ret;
}

static unsigned int stm32_ospi_get_mode(uint8_t buswidth)
{
	switch (buswidth) {
	case SPI_MEM_BUSWIDTH_8_LINE:
		return 4U;
	case SPI_MEM_BUSWIDTH_4_LINE:
		return 3U;
	default:
		return buswidth;
	}
}

static int stm32_ospi_send(const struct spi_mem_op *op, uint8_t fmode)
{
	struct stm32_ospi_flash *flash = &stm32_ospi.flash;
	uint64_t timeout;
	uint64_t addr = op->addr.val;
	uint32_t ccr;
	uint32_t dcyc = 0U;
	unsigned int nbytes = op->data.nbytes;
	int ret;

	VERBOSE("%s: cmd:%x dtr:%d mode:%d.%d.%d.%d addr:%" PRIx64 " len:%x\n",
		__func__, op->cmd.opcode,  op->cmd.dtr, op->cmd.buswidth,
		op->addr.buswidth, op->dummy.buswidth, op->data.buswidth,
		op->addr.val, op->data.nbytes);

	ret = stm32_ospi_wait_for_not_busy();
	if (ret != 0) {
		return ret;
	}

	if (flash->octal_dtr && (fmode != _OSPI_CR_FMODE_MM) &&
	    (op->data.nbytes != 0U)) {
		if ((op->addr.val % 2U) != 0U) {
			addr--;
			nbytes++;
		}

		if (((op->addr.val + op->data.nbytes) % 2U) != 0U) {
			nbytes++;
		}
	}

	mmio_clrbits_32(ospi_base() + _OSPI_DCR1, _OSPI_DCR1_MTYP);

	if ((op->data.nbytes != 0U) && (fmode != _OSPI_CR_FMODE_MM)) {
		mmio_write_32(ospi_base() + _OSPI_DLR, nbytes - 1U);
	}

	if ((op->dummy.buswidth != 0U) && (op->dummy.nbytes != 0U)) {
		dcyc = op->dummy.nbytes * 8U / op->dummy.buswidth;

		if (op->dummy.dtr) {
			dcyc /= 2U;
		}
	}

	mmio_clrsetbits_32(ospi_base() + _OSPI_TCR, _OSPI_TCR_DCYC, dcyc);

	mmio_clrsetbits_32(ospi_base() + _OSPI_CR, _OSPI_CR_FMODE,
			   fmode << _OSPI_CR_FMODE_SHIFT);

	if (op->data.dtr_swab16) {
		mmio_clrsetbits_32(ospi_base() + _OSPI_DCR1, _OSPI_DCR1_MTYP,
				   _OSPI_DCR1_MTYP_MX_MODE <<
				   _OSPI_DCR1_MTYP_SHIFT);
	}

	ccr = stm32_ospi_get_mode(op->cmd.buswidth);
	ccr |= (op->cmd.nbytes - 1U) << _OSPI_CCR_ISIZE_SHIFT;

	if (op->cmd.dtr) {
		ccr |= _OSPI_CCR_IDTR;
		ccr |= _OSPI_CCR_DQSE;
	}

	if (op->addr.dtr) {
		ccr |= _OSPI_CCR_ADDTR;
	}

	if (op->data.dtr) {
		ccr |= _OSPI_CCR_DDTR;
	}

	if (op->addr.nbytes != 0U) {
		ccr |= (op->addr.nbytes - 1U) << _OSPI_CCR_ADSIZE_SHIFT;
		ccr |= stm32_ospi_get_mode(op->addr.buswidth) <<
		       _OSPI_CCR_ADMODE_SHIFT;
	}

	if (op->data.nbytes != 0U) {
		ccr |= stm32_ospi_get_mode(op->data.buswidth) <<
		       _OSPI_CCR_DMODE_SHIFT;
	}

	mmio_write_32(ospi_base() + _OSPI_CCR, ccr);

	mmio_write_32(ospi_base() + _OSPI_IR, op->cmd.opcode);

	if ((op->addr.nbytes != 0U) && (fmode != _OSPI_CR_FMODE_MM)) {
		mmio_write_32(ospi_base() + _OSPI_AR, addr);
	}

	ret = stm32_ospi_tx(op, fmode);

	/*
	 * Abort in:
	 * - Error case.
	 * - Memory mapped read: prefetching must be stopped if we read the last
	 *   byte of device (device size - fifo size). If device size is not
	 *   known then prefetching is always stopped.
	 */
	if ((ret != 0) || (fmode == _OSPI_CR_FMODE_MM)) {
		goto abort;
	}

	/* Wait end of TX in indirect mode */
	ret = stm32_ospi_wait_cmd();
	if (ret != 0) {
		goto abort;
	}

	return 0;

abort:
	mmio_setbits_32(ospi_base() + _OSPI_CR, _OSPI_CR_ABORT);

	/* Wait clear of abort bit by hardware */
	timeout = timeout_init_us(_OSPI_ABT_TIMEOUT_US);
	while ((mmio_read_32(ospi_base() + _OSPI_CR) & _OSPI_CR_ABORT) != 0U) {
		if (timeout_elapsed(timeout)) {
			ret = -ETIMEDOUT;
			break;
		}
	}

	mmio_write_32(ospi_base() + _OSPI_FCR, _OSPI_FCR_CTCF);

	if ((ret != 0) && !stm32_ospi.is_calibrating) {
		ERROR("%s: exec op error\n", __func__);
	}

	return ret;
}

static int stm32_ospi_set_speed(unsigned int hz)
{
	unsigned long ospi_clk = clk_get_rate(stm32_ospi.clock_id);
	unsigned int bus_freq;
	uint32_t prescaler = UINT8_MAX;
	uint32_t csht;
	int ret;

	if (ospi_clk == 0U) {
		return -EINVAL;
	}

	if (hz > 0U) {
		prescaler = div_round_up(ospi_clk, hz) - 1U;
		if (prescaler > UINT8_MAX) {
			prescaler = UINT8_MAX;
		}
	}

	csht = div_round_up((5U * ospi_clk) / (prescaler + 1U), _FREQ_100MHZ);
	csht = ((csht - 1U) << _OSPI_DCR1_CSHT_SHIFT) & _OSPI_DCR1_CSHT;

	ret = stm32_ospi_wait_for_not_busy();
	if (ret != 0) {
		return ret;
	}

	mmio_clrsetbits_32(ospi_base() + _OSPI_DCR2, _OSPI_DCR2_PRESCALER,
			   prescaler);

	mmio_clrsetbits_32(ospi_base() + _OSPI_DCR1, _OSPI_DCR1_CSHT, csht);

	bus_freq = div_round_up(ospi_clk, prescaler + 1U);
	if (bus_freq <= _DLYB_FREQ_50MHZ) {
		mmio_setbits_32(ospi_base() + _OSPI_DCR1, _OSPI_DCR1_DLYBYP);
	} else {
		mmio_clrbits_32(ospi_base() + _OSPI_DCR1, _OSPI_DCR1_DLYBYP);
	}

	VERBOSE("%s: speed=%lu\n", __func__, ospi_clk / (prescaler + 1U));

	return 0;
}

static int stm32_ospi_readid(void)
{
	struct stm32_ospi_flash *flash = &stm32_ospi.flash;
	uint64_t id;
	struct spi_mem_op readid_op;
	int ret;

	if (!flash->str_calibration_done_once) {
		uint8_t nb_dummy_bytes = flash->is_spi_nor ? 0U : 1U;

		zeromem(&readid_op, sizeof(struct spi_mem_op));
		readid_op.cmd.nbytes = 1U;
		readid_op.cmd.opcode = _OP_READ_ID;
		readid_op.cmd.buswidth = SPI_MEM_BUSWIDTH_1_LINE;
		readid_op.dummy.nbytes = nb_dummy_bytes;
		readid_op.dummy.buswidth = SPI_MEM_BUSWIDTH_1_LINE;
		readid_op.data.nbytes = _MAX_ID_LEN;
		readid_op.data.buf = &id;
		readid_op.data.buswidth = SPI_MEM_BUSWIDTH_1_LINE;
	} else {
		if (flash->octal_dtr && flash->is_spi_nor) {
			uint16_t opcode;
			uint8_t nb_addr_bytes;
			uint8_t nb_dummy_bytes;

			if ((flash->dtr_idcode & 0xffU) == _MACRONIX_ID) {
				opcode = 0x9f60U;
				nb_addr_bytes = 4U;
				nb_dummy_bytes = 8U;
			} else {
				/*
				 * All memory providers are not currently
				 * supported, feel free to add them
				 */
				return -EOPNOTSUPP;
			}

			zeromem(&readid_op, sizeof(struct spi_mem_op));
			readid_op.cmd.nbytes = 2U;
			readid_op.cmd.opcode = opcode;
			readid_op.cmd.buswidth = SPI_MEM_BUSWIDTH_8_LINE;
			readid_op.cmd.dtr = true;
			readid_op.addr.nbytes = nb_addr_bytes;
			readid_op.addr.buswidth = SPI_MEM_BUSWIDTH_8_LINE;
			readid_op.addr.dtr = true;
			readid_op.dummy.nbytes = nb_dummy_bytes;
			readid_op.dummy.buswidth = SPI_MEM_BUSWIDTH_8_LINE;
			readid_op.dummy.dtr = true;
			readid_op.data.nbytes = _MAX_ID_LEN;
			readid_op.data.buf = &id;
			readid_op.data.buswidth = SPI_MEM_BUSWIDTH_8_LINE;
			readid_op.data.dtr = true;
		} else {
			/*
			 * Only OCTAL DTR calibration on SPI NOR devices
			 * is currently supported
			 */
			return -EOPNOTSUPP;
		}
	}

	stm32_ospi.is_calibrating = true;
	ret = stm32_ospi_send(&readid_op, _OSPI_CR_FMODE_INDR);
	stm32_ospi.is_calibrating = false;
	if (ret != 0) {
		return ret;
	}

	VERBOSE("Flash ID 0x%08lx\n", id);

	if (!flash->str_calibration_done_once) {
		/* On stm32_ospi_readid() first execution, save the golden read id */
		if (flash->str_idcode == 0U) {
			flash->str_idcode = id;

			if (flash->is_spi_nor) {
				/* Build DTR id code */
				if ((id & 0xFFU) == _MACRONIX_ID) {
					/*
					 * Retrieve odd array and re-sort id
					 * because of read id format will be
					 * A-A-B-B-C-C after enter into octal
					 * dtr mode for Macronix flashes.
					 */
					flash->dtr_idcode = id & 0xFFU;
					flash->dtr_idcode |= (id & 0xFFU) << 8U;
					flash->dtr_idcode |= (id & 0xFF00U) << 8U;
					flash->dtr_idcode |= (id & 0xFF00U) << 16U;
					flash->dtr_idcode |= (id & 0xFF0000U) << 16U;
					flash->dtr_idcode |= (id & 0xFF0000U) << 24U;
					flash->dtr_idcode |= (id & 0xFF000000U) << 24U;
					flash->dtr_idcode |= (id & 0xFF000000U) << 32U;
				} else {
					flash->dtr_idcode = id;
				}
			}
		}

		if (id == flash->str_idcode) {
			return 0;
		}
	} else if (id == flash->dtr_idcode) {
		return 0;
	}

	return -EIO;
}

static int stm32_ospi_str_calibration(void)
{
	uint32_t dlyb_cr = 0U;
	uint8_t window_len_tcr0 = 0U;
	uint8_t window_len_tcr1 = 0U;
	int ret;
	int ret_tcr0;
	int ret_tcr1;
	uint32_t prescaler = mmio_read_32(ospi_base() + _OSPI_DCR2) &
					  _OSPI_DCR2_PRESCALER;
	unsigned int bus_freq = div_round_up(clk_get_rate(stm32_ospi.clock_id),
					     prescaler + 1U);

	/*
	 * Set memory device at low frequency (50 MHz) and sent
	 * READID (0x9F) command, save the answer as golden answer
	 */
	ret = stm32_ospi_set_speed(_DLYB_FREQ_50MHZ);
	if (ret != 0) {
		return ret;
	}

	ret = stm32_ospi_readid();
	if (ret != 0) {
		return ret;
	}

	/* Set frequency at requested value */
	ret = stm32_ospi_set_speed(bus_freq);
	if (ret != 0) {
		return ret;
	}

	/* Calibration needed above 50MHz */
	if (bus_freq <= _DLYB_FREQ_50MHZ) {
		return 0;
	}

	/* Perform calibration */
	ret = stm32mp_syscfg_dlyb_init(stm32_ospi.bank, false, 0U);
	if (ret != 0) {
		return ret;
	}

	/* Perform only RX TAP selection */
	ret_tcr0 = stm32mp_syscfg_dlyb_find_tap(stm32_ospi.bank,
						stm32_ospi_readid,
						true, &window_len_tcr0);
	if (ret_tcr0 == 0) {
		stm32mp_syscfg_dlyb_get_cr(stm32_ospi.bank, &dlyb_cr);
	}

	stm32mp_syscfg_dlyb_stop(stm32_ospi.bank);

	ret = stm32mp_syscfg_dlyb_init(stm32_ospi.bank, false, 0U);
	if (ret != 0) {
		return ret;
	}

	mmio_setbits_32(ospi_base() + _OSPI_TCR, _OSPI_TCR_SSHIFT);

	ret_tcr1 = stm32mp_syscfg_dlyb_find_tap(stm32_ospi.bank,
						stm32_ospi_readid,
						true, &window_len_tcr1);
	if ((ret_tcr0 != 0) && (ret_tcr1 != 0)) {
		WARN("Calibration phase failed\n");

		return ret_tcr0;
	}

	if (window_len_tcr0 >= window_len_tcr1) {
		mmio_clrbits_32(ospi_base() + _OSPI_TCR, _OSPI_TCR_SSHIFT);
		stm32mp_syscfg_dlyb_stop(stm32_ospi.bank);

		ret = stm32mp_syscfg_dlyb_set_cr(stm32_ospi.bank, dlyb_cr);
		if (ret != 0) {
			return ret;
		}
	}

	return 0;
}

static int stm32_ospi_dtr_calibration(bool octal_dtr)
{
	struct stm32_ospi_flash *flash = &stm32_ospi.flash;
	uint32_t prescaler;
	unsigned int bus_freq;
	int ret;

	if (flash->dtr_calibration_done_once) {
		return 0;
	}

	prescaler = mmio_read_32(ospi_base() + _OSPI_DCR2) &
				 _OSPI_DCR2_PRESCALER;
	bus_freq = div_round_up(clk_get_rate(stm32_ospi.clock_id),
				prescaler + 1U);

	stm32mp_syscfg_dlyb_stop(stm32_ospi.bank);
	mmio_clrbits_32(ospi_base() + _OSPI_TCR, _OSPI_TCR_SSHIFT);
	flash->octal_dtr = octal_dtr;

	ret = stm32_ospi_dtr_calibrate(prescaler, bus_freq, stm32_ospi_readid);
	if (ret != 0) {
		return ret;
	}

	flash->dtr_calibration_done_once = true;

	return 0;
}

static int stm32_ospi_exec_op(const struct spi_mem_op *op)
{
	uint8_t fmode = _OSPI_CR_FMODE_INDW;

	if (op->cmd.dtr) {
		int ret = stm32_ospi_dtr_calibration(op->cmd.nbytes == 2U);

		if (ret != 0) {
			return ret;
		}
	}

	if ((op->data.dir == SPI_MEM_DATA_IN) && (op->data.nbytes != 0U)) {
		fmode = _OSPI_CR_FMODE_INDR;
	}

	return stm32_ospi_send(op, fmode);
}

static int stm32_ospi_dirmap_read(const struct spi_mem_op *op)
{
	size_t addr_max;
	uint8_t fmode = _OSPI_CR_FMODE_INDR;

	if (op->cmd.dtr) {
		int ret = stm32_ospi_dtr_calibration(op->cmd.nbytes == 2U);

		if (ret != 0) {
			return ret;
		}
	}

	addr_max = op->addr.val + op->data.nbytes + 1U;
	if ((addr_max < stm32_ospi.mm_size) && (op->addr.buswidth != 0U)) {
		fmode = _OSPI_CR_FMODE_MM;
	}

	return stm32_ospi_send(op, fmode);
}

static int stm32_ospi_claim_bus(unsigned int cs)
{
	struct stm32_ospi_flash *flash = &stm32_ospi.flash;
	uint32_t cr;
	int ret = 0;

	if (cs >= _OSPI_MAX_CHIP) {
		return -ENODEV;
	}

	/* Set chip select and enable the controller */
	cr = _OSPI_CR_EN;
	if (cs == 1U) {
		cr |= _OSPI_CR_CSSEL;
	}

	mmio_clrsetbits_32(ospi_base() + _OSPI_CR, _OSPI_CR_CSSEL, cr);

	/* Calibration is done once */
	if (!flash->str_calibration_done_once) {
		stm32mp_syscfg_dlyb_stop(stm32_ospi.bank);
		mmio_clrbits_32(ospi_base() + _OSPI_TCR, _OSPI_TCR_SSHIFT);

		if (stm32_ospi_str_calibration() != 0) {
			WARN("Set flash frequency to a safe value (%u Hz)\n",
			     _DLYB_FREQ_50MHZ);

			stm32mp_syscfg_dlyb_stop(stm32_ospi.bank);
			mmio_clrbits_32(ospi_base() + _OSPI_TCR,
					_OSPI_TCR_SSHIFT);

			ret = stm32_ospi_set_speed(_DLYB_FREQ_50MHZ);
		}

		flash->str_calibration_done_once = true;
	}

	return ret;
}

static void stm32_ospi_release_bus(void)
{
	mmio_clrbits_32(ospi_base() + _OSPI_CR, _OSPI_CR_EN);
}

static int stm32_ospi_set_mode(unsigned int mode)
{
	int ret;

	if ((mode & SPI_CS_HIGH) != 0U) {
		return -ENODEV;
	}

	ret = stm32_ospi_wait_for_not_busy();
	if (ret != 0) {
		return ret;
	}

	if (((mode & SPI_CPHA) != 0U) && ((mode & SPI_CPOL) != 0U)) {
		mmio_setbits_32(ospi_base() + _OSPI_DCR1, _OSPI_DCR1_CKMODE);
	} else if (((mode & SPI_CPHA) == 0U) && ((mode & SPI_CPOL) == 0U)) {
		mmio_clrbits_32(ospi_base() + _OSPI_DCR1, _OSPI_DCR1_CKMODE);
	} else {
		return -ENODEV;
	}

#if DEBUG
	VERBOSE("%s: mode=0x%x\n", __func__, mode);

	if ((mode & SPI_RX_OCTAL) != 0U) {
		VERBOSE("rx: octal\n");
	} else if ((mode & SPI_RX_QUAD) != 0U) {
		VERBOSE("rx: quad\n");
	} else if ((mode & SPI_RX_DUAL) != 0U) {
		VERBOSE("rx: dual\n");
	} else {
		VERBOSE("rx: single\n");
	}

	if ((mode & SPI_TX_OCTAL) != 0U) {
		VERBOSE("tx: octal\n");
	} else if ((mode & SPI_TX_QUAD) != 0U) {
		VERBOSE("tx: quad\n");
	} else if ((mode & SPI_TX_DUAL) != 0U) {
		VERBOSE("tx: dual\n");
	} else {
		VERBOSE("tx: single\n");
	}
#endif

	return 0;
}

static bool stm32_ospi_mem_supports_op(const struct spi_mem_op *op)
{
	if ((op->data.buswidth > 8U) || (op->addr.buswidth > 8U) ||
	    (op->dummy.buswidth > 8U) || (op->cmd.buswidth > 8U)) {
		return false;
	}

	if ((op->cmd.nbytes > 4U) || (op->addr.nbytes > 4U)) {
		return false;
	}

	if ((!op->dummy.dtr && (op->dummy.nbytes > 32U)) ||
	    (op->dummy.dtr && (op->dummy.nbytes > 64U))) {
		return false;
	}

	if (!op->cmd.dtr && !op->addr.dtr && !op->dummy.dtr && !op->data.dtr) {
		return spi_mem_default_supports_op(op);
	}

	return spi_mem_dtr_supports_op(op);
}

static const struct spi_bus_ops stm32_ospi_bus_ops = {
	.claim_bus = stm32_ospi_claim_bus,
	.release_bus = stm32_ospi_release_bus,
	.set_speed = stm32_ospi_set_speed,
	.set_mode = stm32_ospi_set_mode,
	.exec_op = stm32_ospi_exec_op,
	.dirmap_read = stm32_ospi_dirmap_read,
	.supports_op = stm32_ospi_mem_supports_op,
};
#endif /* STM32MP_HYPERFLASH */

int stm32_ospi_init(void)
{
	int iom_node;
	int ospi_node;
#if !STM32MP_HYPERFLASH
	int flash_node;
	int nflash = 0;
#endif /* STM32MP_HYPERFLASH */
	int ret;
	int len;
	const fdt32_t *cuint;
	void *fdt = NULL;
	unsigned int i;
	unsigned int nb_ospi_nodes = 0U;
	unsigned int reset_id;
	uintptr_t bank_address[_OMM_MAX_OSPI] = { 0U, 0U };
	uintptr_t mm_base;
	uint8_t bank_assigned = 0U;
	uint8_t bank;
	size_t mm_size;

	if (fdt_get_address(&fdt) == 0) {
		return -FDT_ERR_NOTFOUND;
	}

	iom_node = fdt_node_offset_by_compatible(fdt, -1, _DT_IOM_COMPAT);
	if (iom_node < 0) {
		struct dt_node_info info;

		ospi_node = dt_get_node(&info, -1, _DT_OSPI_COMPAT);
		if (ospi_node < 0) {
			return ospi_node;
		}

		if (info.status == DT_DISABLED) {
			return -FDT_ERR_NOTFOUND;
		}

		stm32_ospi.reg_base = info.base;
		stm32_ospi.mm_size = SZ_256M;
		stm32_ospi.mm_base = STM32MP_OSPI_MM_BASE;
		stm32_ospi.bank = 0U;

		if (info.clock < 0) {
			return -FDT_ERR_BADVALUE;
		}

		stm32_ospi.clock_id = (unsigned long)info.clock;

		if (dt_set_pinctrl_config(ospi_node) != 0) {
			return -FDT_ERR_BADVALUE;
		}
	} else {
		if (fdt_get_status(iom_node) == DT_DISABLED) {
			return -FDT_ERR_NOTFOUND;
		}

		ret = fdt_get_reg_props_by_name(fdt, iom_node, "memory_map",
						&mm_base, &mm_size);
		if (ret != 0) {
			return ret;
		}

		cuint = fdt_getprop(fdt, iom_node, "ranges", NULL);
		if (cuint == NULL) {
			return -FDT_ERR_BADVALUE;
		}

		for (i = 0U; i < _OMM_MAX_OSPI; i++) {
			bank = (uint8_t)fdt32_to_cpu(*cuint);
			if ((bank >= _OMM_MAX_OSPI) ||
			    ((bank_assigned & BIT(bank)) != 0U)) {
				return -FDT_ERR_BADVALUE;
			}

			bank_assigned |= BIT(bank);
			bank_address[bank] = fdt32_to_cpu(*(cuint + 2U));
			cuint += 4U;
		}

		if (dt_set_pinctrl_config(iom_node) != 0) {
			return -FDT_ERR_BADVALUE;
		}

		fdt_for_each_subnode(ospi_node, fdt, iom_node) {
			nb_ospi_nodes++;
		}

		if (nb_ospi_nodes != 1U) {
			WARN("Only one OSPI node supported\n");
			return -FDT_ERR_BADVALUE;
		}

		/* Parse OSPI controller node */
		ospi_node = fdt_node_offset_by_compatible(fdt, iom_node,
							  _DT_OSPI_COMPAT);
		if (ospi_node < 0) {
			return ospi_node;
		}

		if (fdt_get_status(ospi_node) == DT_DISABLED) {
			return -FDT_ERR_NOTFOUND;
		}

		cuint = fdt_getprop(fdt, ospi_node, "reg", NULL);
		if (cuint == NULL) {
			return -FDT_ERR_BADVALUE;
		}

		bank = (uint8_t)fdt32_to_cpu(*cuint);
		if (bank >= _OMM_MAX_OSPI) {
			return -FDT_ERR_BADVALUE;
		}

		stm32_ospi.reg_base = fdt32_to_cpu(*(cuint + 1U)) + bank_address[bank];
		stm32_ospi.mm_size = stm32mp_syscfg_get_mm_size(bank);
		stm32_ospi.mm_base = bank == 0U ?
				     mm_base : mm_base + mm_size - stm32_ospi.mm_size;
		stm32_ospi.bank = bank;

		cuint = fdt_getprop(fdt, ospi_node, "clocks", NULL);
		if (cuint == NULL) {
			return -FDT_ERR_BADVALUE;
		}

		cuint++;
		stm32_ospi.clock_id = (unsigned long)fdt32_to_cpu(*cuint);
	}

	clk_enable(stm32_ospi.clock_id);

	cuint = fdt_getprop(fdt, ospi_node, "resets", &len);
	if (cuint != NULL) {
		cuint++;

		/* Reset: array of <phandle, reset_id> */
		for (i = 0U;
		     i < ((uint32_t)len / (sizeof(uint32_t) * _OSPI_MAX_RESET));
		     i++, cuint += _OSPI_MAX_RESET) {
			reset_id = (unsigned int)fdt32_to_cpu(*cuint);

			ret = stm32mp_reset_assert(reset_id, _TIMEOUT_US_1_MS);
			if (ret != 0) {
				panic();
			}

			ret = stm32mp_reset_deassert(reset_id,
						     _TIMEOUT_US_1_MS);
			if (ret != 0) {
				panic();
			}
		}
	}

	mmio_write_32(ospi_base() + _OSPI_DCR1, _OSPI_DCR1_DEVSIZE);

#if STM32MP_HYPERFLASH
	/* Init Hyperflash framework */
	hyperflash_ctrl_init(&stm32_ospi_ctrl_ops);

	return stm32_ospi_hb_init(fdt, ospi_node);
#else /* STM32MP_HYPERFLASH */
	/* Find memory model on each child node (SPI NOR or SPI NAND) */
	fdt_for_each_subnode(flash_node, fdt, ospi_node) {
		nflash++;
	}

	if (nflash != 1) {
		ERROR("Only one SPI device is currently supported\n");
		return -EINVAL;
	}

	if (fdt_node_offset_by_compatible(fdt, ospi_node,
					  "jedec,spi-nor") >= 0) {
		struct stm32_ospi_flash *flash = &stm32_ospi.flash;

		flash->is_spi_nor = true;
	}

	return spi_mem_init_slave(fdt, ospi_node, &stm32_ospi_bus_ops);
#endif /* STM32MP_HYPERFLASH */
};
