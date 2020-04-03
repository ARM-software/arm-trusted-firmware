/*
 * Copyright (c) 2016 - 2020, Broadcom
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <string.h>

#include <lib/mmio.h>

#include <platform_def.h>

#include "bcm_emmc.h"
#include "emmc_chal_types.h"
#include "emmc_chal_sd.h"
#include "emmc_pboot_hal_memory_drv.h"

extern void emmc_soft_reset(void);

#define SD_VDD_WINDOW_1_6_TO_1_7        0x00000010	// 1.6 V to 1.7 Volts
#define SD_VDD_WINDOW_1_7_TO_1_8        0x00000020	// 1.7 V to 1.8 Volts
#define SD_VDD_WINDOW_1_8_TO_1_9        0x00000040	// 1.8 V to 1.9 Volts
#define SD_VDD_WINDOW_1_9_TO_2_0        0x00000080	// 1.9 V to 2.0 Volts
#define SD_VDD_WINDOW_2_0_TO_2_1        0x00000100	// 2.0 V to 2.1 Volts
#define SD_VDD_WINDOW_2_1_TO_2_2        0x00000200	// 2.1 V to 2.2 Volts
#define SD_VDD_WINDOW_2_2_TO_2_3        0x00000400	// 2.2 V to 2.3 Volts
#define SD_VDD_WINDOW_2_3_TO_2_4        0x00000800	// 2.3 V to 2.4 Volts
#define SD_VDD_WINDOW_2_4_TO_2_5        0x00001000	// 2.4 V to 2.5 Volts
#define SD_VDD_WINDOW_2_5_TO_2_6        0x00002000	// 2.5 V to 2.6 Volts
#define SD_VDD_WINDOW_2_6_TO_2_7        0x00004000	// 2.6 V to 2.7 Volts
#define SD_VDD_WINDOW_2_7_TO_2_8        0x00008000	// 2.7 V to 2.8 Volts
#define SD_VDD_WINDOW_2_8_TO_2_9        0x00010000	// 2.8 V to 2.9 Volts
#define SD_VDD_WINDOW_2_9_TO_3_0        0x00020000	// 2.9 V to 3.0 Volts
#define SD_VDD_WINDOW_3_0_TO_3_1        0x00040000	// 3.0 V to 3.1 Volts
#define SD_VDD_WINDOW_3_1_TO_3_2        0x00080000	// 3.1 V to 3.2 Volts
#define SD_VDD_WINDOW_3_2_TO_3_3        0x00100000	// 3.2 V to 3.3 Volts
#define SD_VDD_WINDOW_3_3_TO_3_4        0x00200000	// 3.3 V to 3.4 Volts
#define SD_VDD_WINDOW_3_4_TO_3_5        0x00400000	// 3.4 V to 3.5 Volts
#define SD_VDD_WINDOW_3_5_TO_3_6        0x00800000	// 3.5 V to 3.6 Volts

#define SD_VDD_WINDOW_1_6_TO_2_6        (SD_VDD_WINDOW_1_6_TO_1_7 |	\
					 SD_VDD_WINDOW_1_7_TO_1_8 |	\
					 SD_VDD_WINDOW_1_8_TO_1_9 |	\
					 SD_VDD_WINDOW_1_9_TO_2_0 |	\
					 SD_VDD_WINDOW_2_0_TO_2_1 |	\
					 SD_VDD_WINDOW_2_1_TO_2_2 |	\
					 SD_VDD_WINDOW_2_2_TO_2_3 |	\
					 SD_VDD_WINDOW_2_3_TO_2_4 |	\
					 SD_VDD_WINDOW_2_4_TO_2_5 |	\
					 SD_VDD_WINDOW_2_5_TO_2_6)

#define SD_VDD_WINDOW_2_6_TO_3_2        (SD_VDD_WINDOW_2_6_TO_2_7 |	\
					 SD_VDD_WINDOW_2_7_TO_2_8 |	\
					 SD_VDD_WINDOW_2_8_TO_2_9 |	\
					 SD_VDD_WINDOW_2_9_TO_3_0 |	\
					 SD_VDD_WINDOW_3_0_TO_3_1 |	\
					 SD_VDD_WINDOW_3_1_TO_3_2)

#define SD_VDD_WINDOW_3_2_TO_3_6        (SD_VDD_WINDOW_3_2_TO_3_3 |	\
					 SD_VDD_WINDOW_3_3_TO_3_4 |	\
					 SD_VDD_WINDOW_3_4_TO_3_5 |	\
					 SD_VDD_WINDOW_3_5_TO_3_6)


static int32_t chal_sd_set_power(struct sd_dev *handle,
				 uint32_t voltage, uint32_t state);

static void chal_sd_set_dma_boundary(struct sd_dev *handle, uint32_t boundary);

static int32_t chal_sd_setup_handler(struct sd_dev *handle,
				     uint32_t sdBbase, uint32_t hostBase);

/*
 * Configure host controller pwr settings,
 * to match voltage requirements by SD Card
 */
static int32_t chal_sd_set_power(struct sd_dev *handle,
				 uint32_t voltage, uint32_t state)
{
	int32_t rc, rval = SD_FAIL;
	uint32_t time = 0;

	if (handle == NULL)
		return SD_INVALID_HANDLE;

	mmio_clrsetbits_32(handle->ctrl.sdRegBaseAddr +
			   SD4_EMMC_TOP_CTRL_OFFSET,
			   (SD4_EMMC_TOP_CTRL_SDVSELVDD1_MASK |
			    SD4_EMMC_TOP_CTRL_SDPWR_MASK),
			   (voltage << 9));

	/*
	 * Long delay is required here in emulation.  Without this, the initial
	 * commands sent to the eMMC card timeout.  We don't know if this
	 * delay is necessary with silicon, leaving in for safety.
	 * It is observed that 403ms on emulation system and as per the clock
	 * calculations it is expected to complete with in 1ms on chip
	 */
	do {
		rc =  mmio_read_32(handle->ctrl.sdRegBaseAddr +
				   SD4_EMMC_TOP_INTR_OFFSET);

		if ((rc & SD4_EMMC_TOP_INTR_CRDINS_MASK) ==
				SD4_EMMC_TOP_INTR_CRDINS_MASK)
			break;

		mdelay(1);
	} while (time++ < EMMC_CARD_DETECT_TIMEOUT_MS);

	if (time >= EMMC_CARD_DETECT_TIMEOUT_MS) {
		ERROR("EMMC: Card insert event detection timeout\n");
		return rval;
	}

	VERBOSE("EMMC: Card detection delay: %dms\n", time);

	if (state)
		mmio_setbits_32(handle->ctrl.sdRegBaseAddr + SD4_EMMC_TOP_CTRL_OFFSET,
				SD4_EMMC_TOP_CTRL_SDPWR_MASK);

	/* dummy write & ack to verify if the sdio is ready to send commads */
	mmio_write_32(handle->ctrl.sdRegBaseAddr + SD4_EMMC_TOP_ARG_OFFSET, 0);
	mmio_write_32(handle->ctrl.sdRegBaseAddr + SD4_EMMC_TOP_CMD_OFFSET, 0);

	/*
	 * 63ms observed on emulation system, As per clock calculations
	 * it will complete  < 1ms on chip.
	 */
	time = 0;
	do {
		rc = mmio_read_32(handle->ctrl.sdRegBaseAddr +
				  SD4_EMMC_TOP_INTR_OFFSET);

		if (rc & SD4_EMMC_TOP_INTR_ERRIRQ_MASK)
			break;

		if ((rc & SD4_EMMC_TOP_INTR_CMDDONE_MASK) ==
				SD4_EMMC_TOP_INTR_CMDDONE_MASK)
			break;

		mdelay(1);
	} while (time++ < EMMC_CMD_TIMEOUT_MS);

	if (time >= EMMC_CMD_TIMEOUT_MS) {
		WARN("%s %d Initial dummy command timeout is happened\n",
		      __func__, __LINE__);
		return rval;
	}

	VERBOSE("EMMC: Dummy Command delay: %dms\n", time);

	return SD_OK;
}

/*
 * Configure DMA Boundaries
 */
static void chal_sd_set_dma_boundary(struct sd_dev *handle, uint32_t boundary)
{
	if (handle == NULL)
		return;

	mmio_clrsetbits_32(handle->ctrl.sdRegBaseAddr +
			   SD4_EMMC_TOP_BLOCK_OFFSET,
			   SD4_EMMC_TOP_BLOCK_HSBS_MASK, boundary);
}

static int32_t chal_sd_setup_handler(struct sd_dev *handle, uint32_t sdBase,
				     uint32_t hostBase)
{
	if (handle == NULL)
		return SD_INVALID_HANDLE;

	handle->ctrl.sdRegBaseAddr = sdBase;
	handle->ctrl.hostRegBaseAddr = hostBase;
	handle->ctrl.present = 0;
	handle->ctrl.rca = 0;
	handle->ctrl.blkGapEnable = 0;
	handle->ctrl.cmdStatus = 0;

	return SD_OK;
}

/*
 * Initialize SD Host controller
 */
int32_t chal_sd_init(CHAL_HANDLE *sd_handle)
{
	uint32_t cap_val_l = 0;
	uint32_t ctl_val, voltage;
	uint32_t timeout_val;
	struct sd_dev *handle;
	uint32_t reg_val;
	int32_t rval = SD_FAIL;

	if (sd_handle == NULL)
		return SD_INVALID_HANDLE;

	handle = (struct sd_dev *)sd_handle;

	/*
	 * Set SDIO Host Controller capabilities register
	 */
	EMMC_TRACE("Set Host Controller Capabilities register\n");

	reg_val = 0;
	reg_val |= (1 << ICFG_SDIO0_CAP0__SLOT_TYPE_R);
	reg_val |= (0 << ICFG_SDIO0_CAP0__INT_MODE_R);
	reg_val |= (0 << ICFG_SDIO0_CAP0__SYS_BUS_64BIT_R);
	reg_val |= (1 << ICFG_SDIO0_CAP0__VOLTAGE_1P8V_R);
	reg_val |= (1 << ICFG_SDIO0_CAP0__VOLTAGE_3P0V_R);
	reg_val |= (1 << ICFG_SDIO0_CAP0__VOLTAGE_3P3V_R);
	reg_val |= (1 << ICFG_SDIO0_CAP0__SUSPEND_RESUME_R);
	reg_val |= (1 << ICFG_SDIO0_CAP0__SDMA_R);
	reg_val |= (1 << ICFG_SDIO0_CAP0__HIGH_SPEED_R);
	reg_val |= (1 << ICFG_SDIO0_CAP0__ADMA2_R);
	reg_val |= (1 << ICFG_SDIO0_CAP0__EXTENDED_MEDIA_R);
	reg_val |= (2 << ICFG_SDIO0_CAP0__MAX_BLOCK_LEN_R);
	reg_val |= (0xd0 << ICFG_SDIO0_CAP0__BASE_CLK_FREQ_R);
	reg_val |= (1 << ICFG_SDIO0_CAP0__TIMEOUT_UNIT_R);
	reg_val |= (0x30 << ICFG_SDIO0_CAP0__TIMEOUT_CLK_FREQ_R);

	mmio_write_32(ICFG_SDIO0_CAP0, reg_val);

	reg_val = 0;
	reg_val |= (1 << ICFG_SDIO0_CAP1__SPI_BLOCK_MODE_R);
	reg_val |= (1 << ICFG_SDIO0_CAP1__SPI_MODE_R);
	reg_val |= (0 << ICFG_SDIO0_CAP1__CLK_MULT_R);
	reg_val |= (0 << ICFG_SDIO0_CAP1__RETUNING_MODE_R);
	reg_val |= (1 << ICFG_SDIO0_CAP1__TUNE_SDR50_R);
	reg_val |= (1 << ICFG_SDIO0_CAP1__TIME_RETUNE_R);
	reg_val |= (1 << ICFG_SDIO0_CAP1__DRIVER_D_R);
	reg_val |= (1 << ICFG_SDIO0_CAP1__DRIVER_C_R);
	reg_val |= (1 << ICFG_SDIO0_CAP1__DRIVER_A_R);
	reg_val |= (1 << ICFG_SDIO0_CAP1__DDR50_R);
	reg_val |= (1 << ICFG_SDIO0_CAP1__SDR104_R);
	reg_val |= (1 << ICFG_SDIO0_CAP1__SDR50_R);

	mmio_write_32(ICFG_SDIO0_CAP1, reg_val);

	/* Reset the SDIO controller */
	chal_sd_stop();

	/* Turn on SD clock */
	chal_sd_set_clock(sd_handle,
			  chal_sd_freq_2_div_ctrl_setting(INIT_CLK_FREQ), 1);

	/* program data time out value to the max */
	timeout_val = SD_HOST_CORE_TIMEOUT;

	ctl_val = mmio_read_32(handle->ctrl.sdRegBaseAddr +
			       SD4_EMMC_TOP_CTRL1_OFFSET);
	ctl_val |= ((timeout_val & 0xf) << SD4_EMMC_TOP_CTRL1_DTCNT_SHIFT);

	mmio_write_32(handle->ctrl.sdRegBaseAddr + SD4_EMMC_TOP_CTRL1_OFFSET,
		      ctl_val);

	/* enable all interrupt status */
	mmio_write_32(handle->ctrl.sdRegBaseAddr + SD4_EMMC_TOP_INTREN1_OFFSET,
		      0);
	mmio_write_32(handle->ctrl.sdRegBaseAddr + SD4_EMMC_TOP_INTREN2_OFFSET,
		      0);

	SD_US_DELAY(100);

	mmio_write_32(handle->ctrl.sdRegBaseAddr + SD4_EMMC_TOP_INTREN1_OFFSET,
		      SD_NOR_INTERRUPTS | SD_ERR_INTERRUPTS);
	mmio_write_32(handle->ctrl.sdRegBaseAddr + SD4_EMMC_TOP_INTREN2_OFFSET,
		      SD_NOR_INTERRUPTS | SD_ERR_INTERRUPTS);

	/* Select SD bus voltage */
	cap_val_l = mmio_read_32(handle->ctrl.sdRegBaseAddr +
				 SD4_EMMC_TOP_CAPABILITIES1_OFFSET);
	handle->cfg.voltage = 0;
	voltage = 0x7;

	if (cap_val_l & SD4_EMMC_TOP_CAPABILITIES1_V33_MASK) {
		handle->cfg.voltage |= SD_VDD_WINDOW_3_3_TO_3_4;
		voltage = 0x7;
	} else if (cap_val_l & SD4_EMMC_TOP_CAPABILITIES1_V3_MASK) {
		handle->cfg.voltage |= SD_VDD_WINDOW_3_0_TO_3_1;
		voltage = 0x6;
	} else if (cap_val_l & SD4_EMMC_TOP_CAPABILITIES1_V18_MASK) {
		handle->cfg.voltage |= SD_VDD_WINDOW_1_8_TO_1_9;
		voltage = 0x5;
	}

	rval = chal_sd_set_power(handle, voltage, SD4_EMMC_TOP_CTRL_SDPWR_MASK);

	ctl_val = mmio_read_32(handle->ctrl.sdRegBaseAddr +
			       SD4_EMMC_TOP_HCVERSIRQ_OFFSET);
	handle->ctrl.version = ((ctl_val >> 16) & 0xFF);

	return rval;
}

void chal_sd_set_speed(CHAL_HANDLE *sd_handle, uint32_t speed)
{
	struct sd_dev *handle;

	if (sd_handle == NULL)
		return;

	handle = (struct sd_dev *) sd_handle;

	if (speed) {
		EMMC_TRACE("enable HighSpeed\n");
		mmio_setbits_32(handle->ctrl.sdRegBaseAddr +
				SD4_EMMC_TOP_CTRL_OFFSET,
				SD4_EMMC_TOP_CTRL_HSEN_MASK);
	} else {
		EMMC_TRACE("disable HighSpeed\n");
		mmio_clrbits_32(handle->ctrl.sdRegBaseAddr +
				SD4_EMMC_TOP_CTRL_OFFSET,
				SD4_EMMC_TOP_CTRL_HSEN_MASK);
	}
}

int32_t chal_sd_stop(void)
{
	uintptr_t idm_rst_ctrl_addr = EMMC_IDM_RESET_CTRL_ADDR;

	/* Configure IO pins */
	emmc_soft_reset();

	/* Reset the SDIO controller */
	mmio_write_32(idm_rst_ctrl_addr, 1);
	SD_US_DELAY(100);
	mmio_write_32(idm_rst_ctrl_addr, 0);
	SD_US_DELAY(100);

	return SD_OK;
}

/*
 * Check if host supports specified capability
 * returns -ve val on error, 0 if capability not supported else 1.
 */
int32_t chal_sd_check_cap(CHAL_HANDLE *sd_handle, uint32_t caps)
{
	struct sd_dev *handle;

	if (sd_handle == NULL)
		return SD_INVALID_HANDLE;

	handle = (struct sd_dev *) sd_handle;

	if (caps & mmio_read_32(handle->ctrl.sdRegBaseAddr +
				SD4_EMMC_TOP_CAPABILITIES1_OFFSET))
		return 1;
	else
		return 0;
}

int32_t chal_sd_start(CHAL_HANDLE *sd_handle,
		      uint32_t mode, uint32_t sd_base, uint32_t host_base)
{

	struct sd_dev *handle;
	int32_t rval = SD_FAIL;

	if (sd_handle == NULL)
		return SD_INVALID_HANDLE;

	handle = (struct sd_dev *) sd_handle;

	handle->cfg.mode = SD_PIO_MODE;	/* set to PIO mode first for init */
	handle->cfg.dma = SD_DMA_OFF;

	chal_sd_setup_handler(handle, sd_base, host_base);

	/* init and start hw */
	rval = chal_sd_init(sd_handle);
	if (rval != SD_OK)
		return rval;

	chal_sd_clear_pending_irq(sd_handle);

	handle->ctrl.eventList = 0;
	handle->cfg.mode = mode;

	return SD_OK;
}

/*
 * Function to check 8bits of err generated from auto CMD12
 */
int32_t chal_sd_get_atuo12_error(CHAL_HANDLE *sd_handle)
{
	struct sd_dev *handle;

	if (sd_handle == NULL)
		return SD_INVALID_HANDLE;

	handle = (struct sd_dev *) sd_handle;

	return (mmio_read_32(handle->ctrl.sdRegBaseAddr +
			     SD4_EMMC_TOP_ERRSTAT_OFFSET) & 0xFF);
}

/*
 * Read present state register
 */
uint32_t chal_sd_get_present_status(CHAL_HANDLE *sd_handle)
{
	struct sd_dev *handle;

	if (sd_handle == NULL)
		return SD_INVALID_HANDLE;

	handle = (struct sd_dev *) sd_handle;

	return mmio_read_32(handle->ctrl.sdRegBaseAddr +
			    SD4_EMMC_TOP_PSTATE_OFFSET);
}

/*
 * Set SD bus width
 */
int32_t chal_sd_config_bus_width(CHAL_HANDLE *sd_handle, int32_t width)
{
	uint32_t ctl_val;
	struct sd_dev *handle;

	if (sd_handle == NULL)
		return SD_INVALID_HANDLE;

	handle = (struct sd_dev *)sd_handle;

	ctl_val = mmio_read_32(handle->ctrl.sdRegBaseAddr +
			       SD4_EMMC_TOP_CTRL_OFFSET);

	switch (width) {
#ifdef DRIVER_EMMC_ENABLE_DATA_WIDTH_8BIT
	case SD_BUS_DATA_WIDTH_8BIT:
		ctl_val &= ~SD_BUS_DATA_WIDTH_4BIT;
		ctl_val |= SD_BUS_DATA_WIDTH_8BIT;
		break;
#endif
	case SD_BUS_DATA_WIDTH_4BIT:
		ctl_val &= ~SD_BUS_DATA_WIDTH_8BIT;
		ctl_val |= SD_BUS_DATA_WIDTH_4BIT;
		break;
	case SD_BUS_DATA_WIDTH_1BIT:
		ctl_val &= ~(SD_BUS_DATA_WIDTH_4BIT | SD_BUS_DATA_WIDTH_8BIT);
		break;
	default:
		return SD_INV_DATA_WIDTH;
	};

	mmio_write_32(handle->ctrl.sdRegBaseAddr + SD4_EMMC_TOP_CTRL_OFFSET,
		      ctl_val);

	return SD_OK;
}

/*
 * Function to enable or disable DMA control.
 */
int32_t chal_sd_set_dma(CHAL_HANDLE *sd_handle, uint32_t mode)
{
	uint32_t val;
	struct sd_dev *handle;
	int32_t rc;

	if (sd_handle == NULL)
		return SD_INVALID_HANDLE;

	handle = (struct sd_dev *)sd_handle;

	if (mode) {
		rc = chal_sd_check_cap(sd_handle,
				       SD4_EMMC_TOP_CAPABILITIES1_SDMA_MASK |
				       SD4_EMMC_TOP_CAPABILITIES1_ADMA2_MASK);
		if (rc < 0)
			return rc;

		if (rc) {

			handle->cfg.dma = mode;
			val = mmio_read_32(handle->ctrl.sdRegBaseAddr +
					   SD4_EMMC_TOP_CTRL_OFFSET);
			val &= ~(SD4_EMMC_TOP_CTRL_DMASEL_MASK);
			val |= handle->cfg.dma - 1;
			mmio_write_32(handle->ctrl.sdRegBaseAddr +
				      SD4_EMMC_TOP_CTRL_OFFSET, val);
			return SD_OK;
		}
	}
	handle->cfg.dma = 0;

	return SD_FAIL;
}

/*
 * Get current DMA address.
 * Called only when there is no data transaction activity.
 */
uintptr_t chal_sd_get_dma_addr(CHAL_HANDLE *sd_handle)
{
	struct sd_dev *handle;

	if (sd_handle == NULL)
		return SD_INVALID_HANDLE;

	handle = (struct sd_dev *) sd_handle;

	if (handle->cfg.dma == SD_DMA_OFF)
		return 0;

	return (uintptr_t)mmio_read_32(handle->ctrl.sdRegBaseAddr +
				       SD4_EMMC_TOP_SYSADDR_OFFSET);
}

int32_t chal_sd_send_cmd(CHAL_HANDLE *sd_handle, uint32_t cmd_idx,
			 uint32_t argument, uint32_t options)
{
	uint32_t cmd_mode_reg = 0;
	struct sd_dev *handle;

	if (sd_handle == NULL)
		return SD_INVALID_HANDLE;

	handle = (struct sd_dev *) sd_handle;

	EMMC_TRACE("%s %d cmd:%d argReg:%x options:%x\n",
		   __func__, __LINE__, cmd_idx, argument, options);

	/* Configure the value for command and mode registers */
	cmd_mode_reg = (cmd_idx << 24) | options;

	/*
	 * 1. Write block size reg & block count reg,
	 * this is done in the tx or rx setup
	 */
	mmio_write_32(handle->ctrl.sdRegBaseAddr + SD4_EMMC_TOP_BLOCK_OFFSET,
		      handle->ctrl.blkReg);

	/* 2. Write argument reg */
	mmio_write_32(handle->ctrl.sdRegBaseAddr + SD4_EMMC_TOP_ARG_OFFSET,
		      argument);
	handle->ctrl.argReg = argument;

	/*
	 * 3. Write transfer mode reg & command reg, check the DMA bit which is
	 *    set before this function call if it is selected.
	 */
	if (cmd_idx == 24 || cmd_idx == 25 || cmd_idx == 18 || cmd_idx == 17 ||
	    cmd_idx == 42 || cmd_idx == 51 || cmd_idx == 53)
		cmd_mode_reg |= ((handle->cfg.dma) ? 1 : 0);

	mmio_write_32(handle->ctrl.sdRegBaseAddr + SD4_EMMC_TOP_CMD_OFFSET,
		      cmd_mode_reg);

	handle->ctrl.cmdIndex = cmd_idx;

	return SD_OK;
}

int32_t chal_sd_set_dma_addr(CHAL_HANDLE *sd_handle, uintptr_t address)
{
	struct sd_dev *handle;

	if (sd_handle == NULL)
		return SD_INVALID_HANDLE;

	handle = (struct sd_dev *) sd_handle;

	if (handle->cfg.dma == SD_DMA_OFF)
		return SD_FAIL;

	mmio_write_32(handle->ctrl.sdRegBaseAddr + SD4_EMMC_TOP_SYSADDR_OFFSET,
		      address);
	return SD_OK;
}

uint32_t chal_sd_freq_2_div_ctrl_setting(uint32_t desired_freq)
{
	/*
	 * Divider control setting represents 1/2 of the actual divider value.
	 *
	 * DesiredFreq = BaseClockFreq / (2 * div_ctrl_setting)
	 *
	 * ==> div_ctrl_setting = BaseClockFreq / (2 * DesiredFreq)
	 */
	uint32_t div_ctrl_setting;
	uint32_t actual_freq;

	assert(desired_freq != 0);

	/* Special case, 0 = divider of 1. */
	if (desired_freq >= BASE_CLK_FREQ)
		return 0;

	/* Normal case, desired_freq < BASE_CLK_FREQ */
	div_ctrl_setting = BASE_CLK_FREQ / (2 * desired_freq);

	actual_freq = BASE_CLK_FREQ / (2 * div_ctrl_setting);

	if (actual_freq > desired_freq) {
		/*
		 * Division does not result in exact freqency match.
		 * Make sure resulting frequency does not exceed requested freq.
		 */
		div_ctrl_setting++;
	}

	return div_ctrl_setting;
}

int32_t chal_sd_set_clock(CHAL_HANDLE *sd_handle, uint32_t div_ctrl_setting,
			  uint32_t on)
{
	uint32_t value;
	struct sd_dev *handle;
	uint32_t time;
	uint32_t clk_sel_high_byte = 0xFF & (div_ctrl_setting >> 8);
	uint32_t clk_sel_low_byte = 0xFF & div_ctrl_setting;

	if (sd_handle == NULL)
		return SD_INVALID_HANDLE;

	EMMC_TRACE("set_clock(div_ctrl_setting=%d,on=%d)\n",
		   div_ctrl_setting, on);

	handle = (struct sd_dev *) sd_handle;

	/* Read control register content. */
	value = mmio_read_32(handle->ctrl.sdRegBaseAddr +
			     SD4_EMMC_TOP_CTRL1_OFFSET);

	/* Disable Clock */
	value &= ~(SD4_EMMC_TOP_CTRL1_SDCLKEN_MASK);

	mmio_write_32(handle->ctrl.sdRegBaseAddr + SD4_EMMC_TOP_CTRL1_OFFSET,
		      value);

	/* Clear bits of interest. */
	value &= ~(SD4_EMMC_TOP_CTRL1_SDCLKSEL_MASK |
		   SD4_EMMC_TOP_CTRL1_SDCLKSEL_UP_MASK);

	/* Set bits of interest to new value. */
	value |= (SD4_EMMC_TOP_CTRL1_SDCLKSEL_MASK &
		  (clk_sel_low_byte << SD4_EMMC_TOP_CTRL1_SDCLKSEL_SHIFT));
	value |= (SD4_EMMC_TOP_CTRL1_SDCLKSEL_UP_MASK &
		  (clk_sel_high_byte << SD4_EMMC_TOP_CTRL1_SDCLKSEL_UP_SHIFT));
	value |= SD4_EMMC_TOP_CTRL1_ICLKEN_MASK;

	/* Write updated value back to control register. */
	mmio_write_32(handle->ctrl.sdRegBaseAddr + SD4_EMMC_TOP_CTRL1_OFFSET,
		      value);

	time = 0;
	do {
		value = mmio_read_32(handle->ctrl.sdRegBaseAddr +
				     SD4_EMMC_TOP_CTRL1_OFFSET);

		if ((value & SD4_EMMC_TOP_CTRL1_ICLKSTB_MASK) ==
				SD4_EMMC_TOP_CTRL1_ICLKSTB_MASK)
			break;

		mdelay(1);
	} while (time++ < EMMC_CLOCK_SETTING_TIMEOUT_MS);

	if (time >= EMMC_CLOCK_SETTING_TIMEOUT_MS)
		WARN("%s %d clock settings timeout happenedi (%dms)\n",
			 __func__, __LINE__, time);

	VERBOSE("EMMC: clock settings delay: %dms\n", time);

	value = mmio_read_32(handle->ctrl.sdRegBaseAddr +
			     SD4_EMMC_TOP_CTRL1_OFFSET);

	if (on)
		value |= SD4_EMMC_TOP_CTRL1_SDCLKEN_MASK;

	mmio_write_32(handle->ctrl.sdRegBaseAddr + SD4_EMMC_TOP_CTRL1_OFFSET,
		      value);

	return SD_OK;
}

/*
 * function to setup DMA buffer and data length, calculates block
 * size and the number of blocks to be transferred and return
 * the DMA buffer address.
 */
int32_t chal_sd_setup_xfer(CHAL_HANDLE *sd_handle,
			   uint8_t *data, uint32_t length, int32_t dir)
{
	uint32_t blocks = 0;
	struct sd_dev *handle;

	if (sd_handle == NULL)
		return SD_INVALID_HANDLE;

	handle = (struct sd_dev *) sd_handle;

	if (length <= handle->cfg.blockSize) {
		handle->ctrl.blkReg = length | handle->cfg.dmaBoundary;
	} else {
		blocks = length / handle->cfg.blockSize;
		handle->ctrl.blkReg = (blocks << 16) | handle->cfg.blockSize |
					handle->cfg.dmaBoundary;
	}

	if (handle->cfg.dma != SD_DMA_OFF) {
		/* For DMA target address setting, physical address should be used */
		mmio_write_32(handle->ctrl.sdRegBaseAddr + SD4_EMMC_TOP_SYSADDR_OFFSET,
				(uintptr_t)data);
	}

	return SD_OK;
}

#ifdef INCLUDE_EMMC_DRIVER_WRITE_CODE
/*
 * function to write one block data directly to the
 * host controller's FIFO which is 1K uint8_t or
 * 2K uint8_t in size.
 * It is used in Non-DMA mode for data transmission.
 */
int32_t chal_sd_write_buffer(CHAL_HANDLE *sd_handle, uint32_t length,
			     uint8_t *data)
{
	uint32_t i, leftOver = 0, blockSize, size, value = 0;
	struct sd_dev *handle;

	if (sd_handle == NULL)
		return SD_INVALID_HANDLE;

	handle = (struct sd_dev *) sd_handle;

	blockSize = handle->cfg.blockSize;

	if (length == 0)
		return SD_OK;

	/* PIO mode, push into fifo word by word */
	if (length >= blockSize) {
		size = blockSize;
	} else {
		size = ((length >> 2) << 2);
		leftOver = length % 4;
	}

	for (i = 0; i < size; i += 4) {
		value = *(uint32_t *)(data + i);
		mmio_write_32(handle->ctrl.sdRegBaseAddr +
			      SD4_EMMC_TOP_BUFDAT_OFFSET, value);
	}
/*
 * BUG ALERT:
 *    This implementation has TWO issues that must be addressed before you
 *    can safely INCLUDE_EMMC_DRIVER_WRITE_CODE.
 *
 *    (1) For the last leftOver bytes, driver writes full word, which means
 *        some of the eMMC content (i.e. "4 - leftOver" will be erroneously
 *        overwritten).
 *    (2) eMMC is a block device. What happens when less than a full block of
 *        data is submitted???
 */
	if (leftOver > 0) {
		value = ((*(uint32_t *)(data + i)) << (4 - leftOver));
		mmio_write_32(handle->ctrl.sdRegBaseAddr +
			      SD4_EMMC_TOP_BUFDAT_OFFSET, value);
	}

	return SD_OK;
}
#endif /* INCLUDE_EMMC_DRIVER_WRITE_CODE */

/*
 * Function to read maximal one block data directly
 * from the data port of the host controller (FIFO). It is used
 * in Non-DMA mode for data transmission.
 */
int32_t chal_sd_read_buffer(CHAL_HANDLE *sd_handle, uint32_t length,
			    uint8_t *data)
{
	uint32_t i, size, leftOver, blockSize, value;
	struct sd_dev *handle;

	if (sd_handle == NULL)
		return SD_INVALID_HANDLE;

	handle = (struct sd_dev *)sd_handle;

	value = 0;

	blockSize = handle->cfg.blockSize;

	/* PIO mode, extract fifo word by word */
	if (length >= blockSize) {
		size = blockSize;
		leftOver = 0;
	} else {
		leftOver = length % 4;
		size = ((length >> 2) << 2);
	}

	for (i = 0; i < size; i += 4) {
		value =
		    mmio_read_32(handle->ctrl.sdRegBaseAddr +
				    SD4_EMMC_TOP_BUFDAT_OFFSET);
		memcpy((void *)(data + i), &value, sizeof(uint32_t));
	}

	if (leftOver > 0) {
		value = mmio_read_32(handle->ctrl.sdRegBaseAddr +
				     SD4_EMMC_TOP_BUFDAT_OFFSET);

		/*
		 * Copy remaining non-full word bytes.
		 * (We run ARM as Little Endian)
		 */
		uint8_t j = 0;

		for (j = 0; j < leftOver; j++) {
			data[i + j] = (value >> (j * 8)) & 0xFF;
		}
	}

	return SD_OK;
}

/*
 * Resets both DAT or CMD line.
 */
int32_t chal_sd_reset_line(CHAL_HANDLE *sd_handle, uint32_t line)
{
	uint32_t control, flag;
	struct sd_dev *handle;

	if (sd_handle == NULL)
		return SD_INVALID_HANDLE;

	handle = (struct sd_dev *) sd_handle;

	flag = SD4_EMMC_TOP_CTRL1_CMDRST_MASK | SD4_EMMC_TOP_CTRL1_DATRST_MASK;

	if (flag != (line | flag))
		return SD_FAIL;

	control = mmio_read_32(handle->ctrl.sdRegBaseAddr +
			       SD4_EMMC_TOP_CTRL1_OFFSET);
	control |= line;
	mmio_write_32(handle->ctrl.sdRegBaseAddr + SD4_EMMC_TOP_CTRL1_OFFSET,
		      control);

	/* reset CMD and DATA line should always work, no need to timed out */
	do {
		control = mmio_read_32(handle->ctrl.sdRegBaseAddr +
				       SD4_EMMC_TOP_CTRL1_OFFSET);
	} while (control & line);

	return SD_OK;
}

/*
 * Function to be called once a SD command is done to read
 * back it's response data.
 */
int32_t chal_sd_get_response(CHAL_HANDLE *sd_handle, uint32_t *resp)
{
	struct sd_dev *handle;

	if (sd_handle == NULL)
		return SD_INVALID_HANDLE;

	handle = (struct sd_dev *) sd_handle;
	resp[0] = mmio_read_32(handle->ctrl.sdRegBaseAddr +
			       SD4_EMMC_TOP_RESP0_OFFSET);
	resp[1] = mmio_read_32(handle->ctrl.sdRegBaseAddr +
			       SD4_EMMC_TOP_RESP2_OFFSET);
	resp[2] = mmio_read_32(handle->ctrl.sdRegBaseAddr +
			       SD4_EMMC_TOP_RESP4_OFFSET);
	resp[3] = mmio_read_32(handle->ctrl.sdRegBaseAddr +
			       SD4_EMMC_TOP_RESP6_OFFSET);

	return SD_OK;
}

/*
 * The function is called to clean all the pending interrupts.
 */
int32_t chal_sd_clear_pending_irq(CHAL_HANDLE *sd_handle)
{
	uint32_t status = SD_OK;
	struct sd_dev *handle;

	if (sd_handle == NULL)
		return SD_INVALID_HANDLE;

	handle = (struct sd_dev *)sd_handle;

	/* Make sure clean all interrupts */
	do {
		mmio_write_32(handle->ctrl.sdRegBaseAddr +
			      SD4_EMMC_TOP_INTR_OFFSET, 0xFFFFFFFF);
		SD_US_DELAY(10);
	} while (mmio_read_32(handle->ctrl.sdRegBaseAddr +
			      SD4_EMMC_TOP_INTR_OFFSET));

	return status;
}

/*
 * The function returns interrupt status register value.
 */
int32_t chal_sd_get_irq_status(CHAL_HANDLE *sd_handle)
{
	struct sd_dev *handle;

	if (sd_handle == NULL)
		return SD_INVALID_HANDLE;

	handle = (struct sd_dev *) sd_handle;

	return (mmio_read_32(handle->ctrl.sdRegBaseAddr +
			     SD4_EMMC_TOP_INTR_OFFSET));
}

/*
 * The function clears interrupt(s) specified in the mask.
 */
int32_t chal_sd_clear_irq(CHAL_HANDLE *sd_handle, uint32_t mask)
{
	struct sd_dev *handle;

	if (sd_handle == NULL)
		return SD_INVALID_HANDLE;

	handle = (struct sd_dev *) sd_handle;

	/* Make sure clean masked interrupts */
	do {
		mmio_write_32(handle->ctrl.sdRegBaseAddr +
			      SD4_EMMC_TOP_INTR_OFFSET, mask);
		SD_US_DELAY(10);
	} while (mask &
		 mmio_read_32(handle->ctrl.sdRegBaseAddr +
			      SD4_EMMC_TOP_INTR_OFFSET));

	return SD_OK;
}

/*
 * Description: The function configures the SD host controller.
 */
int32_t chal_sd_config(CHAL_HANDLE *sd_handle, uint32_t speed, uint32_t retry,
		       uint32_t boundary, uint32_t blkSize, uint32_t dma)
{
	struct sd_dev *handle;

	if (sd_handle == NULL)
		return SD_INVALID_HANDLE;

	handle = (struct sd_dev *) sd_handle;

	handle->cfg.speedMode = speed;
	handle->cfg.retryLimit = retry;
	handle->cfg.dmaBoundary = boundary;
	handle->cfg.blockSize = blkSize;

	chal_sd_set_dma(sd_handle, dma);
	SD_US_DELAY(100);
	chal_sd_set_dma_boundary(handle, boundary);
	SD_US_DELAY(100);

	chal_sd_set_speed(sd_handle, speed);

	SD_US_DELAY(100);
	return SD_OK;
}

/*
 * Cleans up HC FIFO.
 */
void chal_sd_dump_fifo(CHAL_HANDLE *sd_handle)
{
	struct sd_dev *handle;

	if (sd_handle == NULL)
		return;

	handle = (struct sd_dev *)sd_handle;

	/* in case there still data in the host buffer */
	while (mmio_read_32(handle->ctrl.sdRegBaseAddr +
			    SD4_EMMC_TOP_PSTATE_OFFSET) & 0x800) {
		mmio_read_32(handle->ctrl.sdRegBaseAddr +
			     SD4_EMMC_TOP_BUFDAT_OFFSET);
	};
}

/*
 * Enable or disable a SD interrupt signal.
 */
void chal_sd_set_irq_signal(CHAL_HANDLE *sd_handle, uint32_t mask,
			    uint32_t state)
{
	struct sd_dev *handle;

	if (sd_handle == NULL)
		return;

	handle = (struct sd_dev *)sd_handle;

	if (state)
		mmio_setbits_32(handle->ctrl.sdRegBaseAddr +
				SD4_EMMC_TOP_INTREN2_OFFSET, mask);
	else
		mmio_clrbits_32(handle->ctrl.sdRegBaseAddr +
				SD4_EMMC_TOP_INTREN2_OFFSET, mask);
}
