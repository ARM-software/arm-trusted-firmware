/*
 * Copyright (c) 2016-2020, Broadcom
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch_helpers.h>
#include <common/bl_common.h>
#include <common/debug.h>
#include <drivers/arm/sp805.h>
#include <drivers/delay_timer.h>
#include <lib/mmio.h>

#include <chimp.h>
#include <chip_id.h>
#include <cmn_plat_util.h>
#include <dmu.h>
#include <emmc_api.h>
#include <fru.h>
#ifdef USE_GPIO
#include <drivers/gpio.h>
#include <iproc_gpio.h>
#endif
#include <platform_def.h>
#include <sotp.h>
#include <swreg.h>
#include <sr_utils.h>
#ifdef USE_DDR
#include <ddr_init.h>
#else
#include <ext_sram_init.h>
#endif
#if DRIVER_OCOTP_ENABLE
#include <ocotp.h>
#endif
#include "board_info.h"

#define WORD_SIZE              8
#define SWREG_AVS_OTP_OFFSET   (13 * WORD_SIZE) /* 13th row byte offset */
#define AON_GPIO_OTP_OFFSET    (28 * WORD_SIZE) /* 28th row byte offset */
#define BYTES_TO_READ          8

/* OTP voltage step definitions */
#define MVOLT_STEP_MAX         0x18  /* 1v */
#define MVOLT_PER_STEP         10    /* 0.01mv per step */
#define MVOLT_BASE             760   /* 0.76v */

#define STEP_TO_UVOLTS(step) \
	((MVOLT_BASE + (MVOLT_PER_STEP * (step))) * 1000)

#define GET_BITS(first, last, data) \
	((data >> first) & ((1 << (last - first + 1)) - 1))

/*
 * SW-REG OTP encoding:
 *
 * SWREG_bits[11:0]  = OTP 13th row 12 bits[55:44]
 * SWREG_bits[11:10] - Valid Bits (0x2 - valid, if not 0x2 - Invalid)
 * SWREG_bits[9:5]   - iHost03, iHost12
 * SWREG_bits[4:0]   - Core VDDC
 */
#define SWREG_OTP_BITS_START        12    /* 44th bit in MSB 32-bits */
#define SWREG_OTP_BITS_END          23    /* 55th bit in MSB 32-bits */
#define SWREG_VDDC_FIELD_START      0
#define SWREG_VDDC_FIELD_END        4
#define SWREG_IHOST_FIELD_START     5
#define SWREG_IHOST_FIELD_END       9
#define SWREG_VALID_BIT_START       10
#define SWREG_VALID_BIT_END         11
#define SWREG_VALID_BITS            0x2

/*
 * Row 13 bit 56 is programmed as '1' today. It is not being used, so plan
 * is to flip this bit to '0' for B1 rev. Hence SW can leverage this bit
 * to identify Bx chip to program different sw-regulators.
 */
#define SPARE_BIT             24

#define IS_SR_B0(data)        (((data) >> SPARE_BIT) & 0x1)

#if DRIVER_OCOTP_ENABLE
static struct otpc_map otp_stingray_map = {
	.otpc_row_size = 2,
	.data_r_offset = {0x10, 0x5c},
	.data_w_offset = {0x2c, 0x64},
	.word_size = 8,
	.stride = 8,
};
#endif

void plat_bcm_bl2_early_platform_setup(void)
{
	/* Select UART0 for AP via mux setting*/
	if (PLAT_BRCM_BOOT_UART_BASE == UART0_BASE_ADDR) {
		mmio_write_32(UART0_SIN_MODE_SEL_CONTROL, 1);
		mmio_write_32(UART0_SOUT_MODE_SEL_CONTROL, 1);
	}
}

#ifdef USE_NAND
static void brcm_stingray_nand_init(void)
{
	unsigned int val;
	unsigned int nand_idm_reset_control = 0x68e0a800;

	VERBOSE(" stingray nand init start.\n");

	/* Reset NAND */
	VERBOSE(" - reset nand\n");
	val = mmio_read_32((uintptr_t)(nand_idm_reset_control + 0x0));
	mmio_write_32((uintptr_t)(nand_idm_reset_control + 0x0), val | 0x1);
	udelay(500);
	val = mmio_read_32((uintptr_t)(nand_idm_reset_control + 0x0));
	mmio_write_32((uintptr_t)(nand_idm_reset_control + 0x0), val & ~0x1);
	udelay(500);

	VERBOSE(" stingray nand init done.\n");
}
#endif

#if defined(USE_PAXB) || defined(USE_PAXC) || defined(USE_SATA)
#define PCIE_RESCAL_CFG_0 0x40000130
#define PCIE_CFG_RESCAL_RSTB_R (1 << 16)
#define PCIE_CFG_RESCAL_PWRDNB_R (1 << 8)
#define PCIE_RESCAL_STATUS_0 0x4000014c
#define PCIE_STAT_PON_VALID_R (1 << 0)
#define PCIE_RESCAL_OUTPUT_STATUS 0x40000154
#define CDRU_PCIE_RESET_N_R (1 << CDRU_MISC_RESET_CONTROL__CDRU_PCIE_RESET_N_R)

#ifdef EMULATION_SETUP
static void brcm_stingray_pcie_reset(void)
{
}
#else
static void brcm_stingray_pcie_reset(void)
{
	unsigned int data;
	int try;

	if (bcm_chimp_is_nic_mode()) {
		INFO("NIC mode detected; PCIe reset/rescal not executed\n");
		return;
	}

	mmio_clrbits_32(CDRU_MISC_RESET_CONTROL, CDRU_PCIE_RESET_N_R);
	mmio_setbits_32(CDRU_MISC_RESET_CONTROL, CDRU_PCIE_RESET_N_R);
	/* Release reset */
	mmio_setbits_32(PCIE_RESCAL_CFG_0, PCIE_CFG_RESCAL_RSTB_R);
	mdelay(1);
	/* Power UP */
	mmio_setbits_32(PCIE_RESCAL_CFG_0,
			(PCIE_CFG_RESCAL_RSTB_R | PCIE_CFG_RESCAL_PWRDNB_R));

	try = 1000;
	do {
		udelay(1);
		data = mmio_read_32(PCIE_RESCAL_STATUS_0);
		try--;
	} while ((data & PCIE_STAT_PON_VALID_R) == 0x0 && (try > 0));

	if (try <= 0)
		ERROR("PCIE_RESCAL_STATUS_0: 0x%x\n", data);

	VERBOSE("PCIE_SATA_RESCAL_STATUS_0 0x%x.\n",
			mmio_read_32(PCIE_RESCAL_STATUS_0));
	VERBOSE("PCIE_SATA_RESCAL_OUTPUT_STATUS 0x%x.\n",
			mmio_read_32(PCIE_RESCAL_OUTPUT_STATUS));
	INFO("PCIE SATA Rescal Init done\n");
}
#endif /* EMULATION_SETUP */
#endif /* USE_PAXB || USE_PAXC || USE_SATA */

#ifdef USE_PAXC
void brcm_stingray_chimp_check_and_fastboot(void)
{
	int fastboot_init_result;

	if (bcm_chimp_is_nic_mode())
		/* Do not wait here */
		return;

#if WARMBOOT_DDR_S3_SUPPORT
	/*
	 * Currently DDR shmoo parameters and QSPI boot source are
	 * tied. DDR shmoo parameters are stored in QSPI, which is
	 * used for warmboot.
	 * Do not reset nitro for warmboot
	 */
	if (is_warmboot() && (boot_source_get() == BOOT_SOURCE_QSPI))
		return;
#endif /* WARMBOOT_DDR_S3_SUPPORT */

	/*
	 * Not in NIC mode,
	 * initiate fastboot (if enabled)
	 */
	if (FASTBOOT_TYPE == CHIMP_FASTBOOT_NITRO_RESET) {

		VERBOSE("Bring up Nitro/ChiMP\n");

		if (boot_source_get() == BOOT_SOURCE_QSPI)
			WARN("Nitro boots from QSPI when AP has booted from QSPI.\n");
		brcm_stingray_set_qspi_mux(0);
		VERBOSE("Nitro controls the QSPI\n");
	}

	fastboot_init_result = bcm_chimp_initiate_fastboot(FASTBOOT_TYPE);
	if (fastboot_init_result && boot_source_get() != BOOT_SOURCE_QSPI)
		ERROR("Nitro init error %d. Status: 0x%x; bpe_mod reg: 0x%x\n"
			"fastboot register: 0x%x; handshake register 0x%x\n",
			fastboot_init_result,
			bcm_chimp_read_ctrl(CHIMP_REG_CTRL_BPE_STAT_REG),
			bcm_chimp_read_ctrl(CHIMP_REG_CTRL_BPE_MODE_REG),
			bcm_chimp_read_ctrl(CHIMP_REG_CTRL_FSTBOOT_PTR_REG),
			bcm_chimp_read(CHIMP_REG_ECO_RESERVED));

	/*
	 * CRMU watchdog kicks is an example, which is L1 reset,
	 * does not clear Nitro scratch pad ram.
	 * For Nitro resets: Clear the Nitro health status memory.
	 */
	bcm_chimp_write((CHIMP_REG_CHIMP_SCPAD + CHIMP_HEALTH_STATUS_OFFSET),
			0);
}
#endif

void set_ihost_vddc_swreg(uint32_t ihost_uvolts, uint32_t vddc_uvolts)
{
	NOTICE("ihost_uvolts: %duv, vddc_uvolts: %duv\n",
	       ihost_uvolts, vddc_uvolts);

	set_swreg(VDDC_CORE, vddc_uvolts);
	set_swreg(IHOST03, ihost_uvolts);
	set_swreg(IHOST12, ihost_uvolts);
}

/*
 * Reads SWREG AVS OTP bits (13th row) with ECC enabled and get voltage
 * defined in OTP if valid OTP is found
 */
void read_avs_otp_bits(uint32_t *ihost_uvolts, uint32_t *vddc_uvolts)
{
	uint32_t offset = SWREG_AVS_OTP_OFFSET;
	uint32_t ihost_step, vddc_step;
	uint32_t avs_bits;
	uint32_t buf[2];

	if (bcm_otpc_read(offset, &buf[0], BYTES_TO_READ, 1) == -1)
		return;

	VERBOSE("AVS OTP %d ROW: 0x%x.0x%x\n",
		offset/WORD_SIZE, buf[1], buf[0]);

	/* get voltage readings from AVS OTP bits */
	avs_bits = GET_BITS(SWREG_OTP_BITS_START,
			    SWREG_OTP_BITS_END,
			    buf[1]);

	/* check for valid otp bits */
	if (GET_BITS(SWREG_VALID_BIT_START, SWREG_VALID_BIT_END, avs_bits) !=
	    SWREG_VALID_BITS) {
		WARN("Invalid AVS OTP bits at %d row\n", offset/WORD_SIZE);
		return;
	}

	/* get ihost and vddc step value */
	vddc_step = GET_BITS(SWREG_VDDC_FIELD_START,
			     SWREG_VDDC_FIELD_END,
			     avs_bits);

	ihost_step = GET_BITS(SWREG_IHOST_FIELD_START,
			      SWREG_IHOST_FIELD_END,
			      avs_bits);

	if ((ihost_step > MVOLT_STEP_MAX) || (vddc_step > MVOLT_STEP_MAX)) {
		WARN("OTP entry invalid\n");
		return;
	}

	/* get voltage in micro-volts */
	*ihost_uvolts = STEP_TO_UVOLTS(ihost_step);
	*vddc_uvolts = STEP_TO_UVOLTS(vddc_step);
}

/*
 * This api reads otp bits and program internal swreg's - ihos12, ihost03,
 * vddc_core and ddr_core based on different chip. External swreg's
 * programming will be done from crmu.
 *
 * For A2 chip:
 *   Read OTP row 20, bit 50. This bit will be set for A2 chip. Once A2 chip is
 *   found, read AVS OTP row 13, 12bits[55:44], if valid otp bits are found
 *   then set ihost and vddc according to avs otp bits else set them to 0.94v
 *   and 0.91v respectively. Also update the firmware after setting voltage.
 *
 * For B0 chip:
 *   Read OTP row 13, bit 56. This bit will be set for B0 chip. Once B0 chip is
 *   found then set ihost and vddc to 0.95v and ddr_core to 1v. No AVS OTP bits
 *   are used get ihost/vddc voltages.
 *
 * For B1 chip:
 *   Read AVS OTP row 13, 12bits[55:44], if valid otp bits are found then set
 *   ihost and vddc according to avs otp bits else set them to 0.94v and 0.91v
 *   respectively.
 */
void set_swreg_based_on_otp(void)
{
	/* default voltage if no valid OTP */
	uint32_t vddc_uvolts = VDDC_CORE_DEF_VOLT;
	uint32_t ihost_uvolts = IHOST_DEF_VOLT;
	uint32_t ddrc_uvolts;
	uint32_t offset;
	uint32_t buf[2];

	offset = SWREG_AVS_OTP_OFFSET;
	if (bcm_otpc_read(offset, &buf[0], BYTES_TO_READ, 1) == -1)
		return;

	VERBOSE("OTP %d ROW: 0x%x.0x%x\n",
		offset/WORD_SIZE, buf[1], buf[0]);

	if (IS_SR_B0(buf[1])) {
		/* don't read AVS OTP for B0 */
		ihost_uvolts = B0_IHOST_DEF_VOLT;
		vddc_uvolts = B0_VDDC_CORE_DEF_VOLT;
		ddrc_uvolts = B0_DDR_VDDC_DEF_VOLT;
	} else {
		read_avs_otp_bits(&ihost_uvolts, &vddc_uvolts);
	}

#if (IHOST_REG_TYPE == IHOST_REG_INTEGRATED) && \
	(VDDC_REG_TYPE == VDDC_REG_INTEGRATED)
	/* enable IHOST12 cluster before changing voltage */
	NOTICE("Switching on the Regulator idx: %u\n",
	       SWREG_IHOST1_DIS);
	mmio_clrsetbits_32(CRMU_SWREG_CTRL_ADDR,
			   BIT(SWREG_IHOST1_DIS),
			   BIT(SWREG_IHOST1_REG_RESETB));

	/* wait for regulator supply gets stable */
	while (!(mmio_read_32(CRMU_SWREG_STATUS_ADDR) &
	       (1 << SWREG_IHOST1_PMU_STABLE)))
		;

	INFO("Regulator supply got stable\n");

#ifndef DEFAULT_SWREG_CONFIG
	swreg_firmware_update();
#endif

	set_ihost_vddc_swreg(ihost_uvolts, vddc_uvolts);
#endif
	if (IS_SR_B0(buf[1])) {
		NOTICE("ddrc_uvolts: %duv\n", ddrc_uvolts);
		set_swreg(DDR_VDDC, ddrc_uvolts);
	}
}

#ifdef USE_DDR
static struct ddr_info ddr_info;
#endif
#ifdef USE_FRU
static struct fru_area_info fru_area[FRU_MAX_NR_AREAS];
static struct fru_board_info board_info;
static struct fru_time fru_tm;
static uint8_t fru_tbl[BCM_MAX_FRU_LEN];

static void board_detect_fru(void)
{
	uint32_t i, result;
	int ret = -1;

	result = bcm_emmc_init(false);
	if (!result) {
		ERROR("eMMC init failed\n");
		return;
	}

	/* go through eMMC boot partitions looking for FRU table */
	for (i = EMMC_BOOT_PARTITION1; i <= EMMC_BOOT_PARTITION2; i++) {
		result = emmc_partition_select(i);
		if (!result) {
			ERROR("Switching to eMMC part %u failed\n", i);
			return;
		}

		result = emmc_read(BCM_FRU_TBL_OFFSET, (uintptr_t)fru_tbl,
				   BCM_MAX_FRU_LEN, BCM_MAX_FRU_LEN);
		if (!result) {
			ERROR("Failed to read from eMMC part %u\n", i);
			return;
		}

		/*
		 * Run sanity check and checksum to make sure valid FRU table
		 * is detected
		 */
		ret = fru_validate(fru_tbl, fru_area);
		if (ret < 0) {
			WARN("FRU table not found in eMMC part %u\n", i);
			continue;
		}

		/* parse DDR information from FRU table */
		ret = fru_parse_ddr(fru_tbl, &fru_area[FRU_AREA_INTERNAL],
				    &ddr_info);
		if (ret < 0) {
			WARN("No FRU DDR info found in eMMC part %u\n", i);
			continue;
		}

		/* parse board information from FRU table */
		ret = fru_parse_board(fru_tbl, &fru_area[FRU_AREA_BOARD_INFO],
				      &board_info);
		if (ret < 0) {
			WARN("No FRU board info found in eMMC part %u\n", i);
			continue;
		}

		/* if we reach here, valid FRU table is parsed */
		break;
	}

	if (ret < 0) {
		WARN("FRU table missing for this board\n");
		return;
	}

	for (i = 0; i < BCM_MAX_NR_DDR; i++) {
		INFO("DDR channel index: %d\n", ddr_info.mcb[i].idx);
		INFO("DDR size %u GB\n", ddr_info.mcb[i].size_mb / 1024);
		INFO("DDR ref ID by SW (Not MCB Ref ID) 0x%x\n",
		     ddr_info.mcb[i].ref_id);
	}

	fru_format_time(board_info.mfg_date, &fru_tm);

	INFO("**** FRU board information ****\n");
	INFO("Language 0x%x\n", board_info.lang);
	INFO("Manufacturing Date %u.%02u.%02u, %02u:%02u\n",
	     fru_tm.year, fru_tm.month, fru_tm.day,
	     fru_tm.hour, fru_tm.min);
	INFO("Manufacturing Date(Raw) 0x%x\n", board_info.mfg_date);
	INFO("Manufacturer %s\n", board_info.manufacturer);
	INFO("Product Name %s\n", board_info.product_name);
	INFO("Serial number %s\n", board_info.serial_number);
	INFO("Part number %s\n", board_info.part_number);
	INFO("File ID %s\n", board_info.file_id);
}
#endif /* USE_FRU */

#ifdef USE_GPIO

#define INVALID_GPIO    0xffff

static const int gpio_cfg_bitmap[MAX_NR_GPIOS] = {
#ifdef BRD_DETECT_GPIO_BIT0
	BRD_DETECT_GPIO_BIT0,
#else
	INVALID_GPIO,
#endif
#ifdef BRD_DETECT_GPIO_BIT1
	BRD_DETECT_GPIO_BIT1,
#else
	INVALID_GPIO,
#endif
#ifdef BRD_DETECT_GPIO_BIT2
	BRD_DETECT_GPIO_BIT2,
#else
	INVALID_GPIO,
#endif
#ifdef BRD_DETECT_GPIO_BIT3
	BRD_DETECT_GPIO_BIT3,
#else
	INVALID_GPIO,
#endif
};

static uint8_t gpio_bitmap;

/*
 * Use an odd number to avoid potential conflict with public GPIO level
 * defines
 */
#define GPIO_STATE_FLOAT         15

/*
 * If GPIO_SUPPORT_FLOAT_DETECTION is disabled, simply return GPIO level
 *
 * If GPIO_SUPPORT_FLOAT_DETECTION is enabled, add additional test for possible
 * pin floating (unconnected) scenario. This support is assuming externally
 * applied pull up / pull down will have a stronger pull than the internal pull
 * up / pull down.
 */
static uint8_t gpio_get_state(int gpio)
{
	uint8_t val;

	/* set direction to GPIO input */
	gpio_set_direction(gpio, GPIO_DIR_IN);

#ifndef GPIO_SUPPORT_FLOAT_DETECTION
	if (gpio_get_value(gpio) == GPIO_LEVEL_HIGH)
		val = GPIO_LEVEL_HIGH;
	else
		val = GPIO_LEVEL_LOW;

	return val;
#else
	/*
	 * Enable internal pull down. If GPIO level is still high, there must
	 * be an external pull up
	 */
	gpio_set_pull(gpio, GPIO_PULL_DOWN);
	if (gpio_get_value(gpio) == GPIO_LEVEL_HIGH) {
		val = GPIO_LEVEL_HIGH;
		goto exit;
	}

	/*
	 * Enable internal pull up. If GPIO level is still low, there must
	 * be an external pull down
	 */
	gpio_set_pull(gpio, GPIO_PULL_UP);
	if (gpio_get_value(gpio) == GPIO_LEVEL_LOW) {
		val = GPIO_LEVEL_LOW;
		goto exit;
	}

	/* if reached here, the pin must be not connected */
	val = GPIO_STATE_FLOAT;

exit:
	/* make sure internall pull is disabled */
	if (gpio_get_pull(gpio) != GPIO_PULL_NONE)
		gpio_set_pull(gpio, GPIO_PULL_NONE);

	return val;
#endif
}

static void board_detect_gpio(void)
{
	unsigned int i, val;
	int gpio;

	iproc_gpio_init(IPROC_GPIO_S_BASE, IPROC_GPIO_NR,
			IPROC_IOPAD_MODE_BASE, HSLS_IOPAD_BASE);

	gpio_bitmap = 0;
	for (i = 0; i < MAX_NR_GPIOS; i++) {
		if (gpio_cfg_bitmap[i] == INVALID_GPIO)
			continue;

		/*
		 * Construct the bitmap based on GPIO value. Floating pin
		 * detection is a special case. As soon as a floating pin is
		 * detected, a special value of MAX_GPIO_BITMAP_VAL is
		 * assigned and we break out of the loop immediately
		 */
		gpio = gpio_cfg_bitmap[i];
		val = gpio_get_state(gpio);
		if (val == GPIO_STATE_FLOAT) {
			gpio_bitmap = MAX_GPIO_BITMAP_VAL;
			break;
		}

		if (val == GPIO_LEVEL_HIGH)
			gpio_bitmap |= BIT(i);
	}

	memcpy(&ddr_info, &gpio_ddr_info[gpio_bitmap], sizeof(ddr_info));
	INFO("Board detection GPIO bitmap = 0x%x\n", gpio_bitmap);
}
#endif /* USE_GPIO */

static void bcm_board_detect(void)
{
#ifdef DDR_LEGACY_MCB_SUPPORTED
	/* Loading default DDR info */
	memcpy(&ddr_info, &default_ddr_info, sizeof(ddr_info));
#endif
#ifdef USE_FRU
	board_detect_fru();
#endif
#ifdef USE_GPIO
	board_detect_gpio();
#endif
}

static void dump_persistent_regs(void)
{
	NOTICE("pr0: %x\n", mmio_read_32(CRMU_IHOST_SW_PERSISTENT_REG0));
	NOTICE("pr1: %x\n", mmio_read_32(CRMU_IHOST_SW_PERSISTENT_REG1));
	NOTICE("pr2: %x\n", mmio_read_32(CRMU_IHOST_SW_PERSISTENT_REG2));
	NOTICE("pr3: %x\n", mmio_read_32(CRMU_IHOST_SW_PERSISTENT_REG3));
	NOTICE("pr4: %x\n", mmio_read_32(CRMU_IHOST_SW_PERSISTENT_REG4));
	NOTICE("pr5: %x\n", mmio_read_32(CRMU_IHOST_SW_PERSISTENT_REG5));
	NOTICE("pr6: %x\n", mmio_read_32(CRMU_IHOST_SW_PERSISTENT_REG6));
	NOTICE("pr7: %x\n", mmio_read_32(CRMU_IHOST_SW_PERSISTENT_REG7));
	NOTICE("pr8: %x\n", mmio_read_32(CRMU_IHOST_SW_PERSISTENT_REG8));
	NOTICE("pr9: %x\n", mmio_read_32(CRMU_IHOST_SW_PERSISTENT_REG9));
	NOTICE("pr10: %x\n", mmio_read_32(CRMU_IHOST_SW_PERSISTENT_REG10));
	NOTICE("pr11: %x\n", mmio_read_32(CRMU_IHOST_SW_PERSISTENT_REG11));
}

void plat_bcm_bl2_plat_arch_setup(void)
{
	if (chip_get_rev_id_major() == CHIP_REV_MAJOR_AX) {
		if (!(sotp_mem_read(SOTP_ATF_CFG_ROW_ID, SOTP_ROW_NO_ECC) &
		      SOTP_ATF_WATCHDOG_ENABLE_MASK)) {
			/*
			 * Stop sp805 watchdog timer immediately.
			 * It might has been set up by MCU patch earlier for
			 * eMMC workaround.
			 *
			 * Note the watchdog timer started in CRMU has a very
			 * short timeout and needs to be stopped immediately.
			 * Down below we restart it with a much longer timeout
			 * for BL2 and BL31
			 */
			sp805_stop(ARM_SP805_TWDG_BASE);
		}
	}

#if !BRCM_DISABLE_TRUSTED_WDOG
	/*
	 * start secure watchdog for BL2 and BL31.
	 * Note that UART download can take a longer time,
	 * so do not allow watchdog for UART download,
	 * as this boot source is not a standard modus operandi.
	 */
	if (boot_source_get() != BOOT_SOURCE_UART)
		sp805_start(ARM_SP805_TWDG_BASE, ARM_TWDG_LOAD_VAL);
#endif

#ifdef BCM_ELOG
	/* Ensure logging is started out fresh in BL2. */
	mmio_write_32(BCM_ELOG_BL2_BASE, 0);
#endif
	/*
	 * In BL2, since we have very limited space to store logs, we only
	 * save logs that are >= the WARNING level.
	 */
	bcm_elog_init((void *)BCM_ELOG_BL2_BASE, BCM_ELOG_BL2_SIZE,
		      LOG_LEVEL_WARNING);

	dump_persistent_regs();

	/* Read CRMU mailbox 0 */
	NOTICE("RESET (reported by CRMU): 0x%x\n",
	       mmio_read_32(CRMU_READ_MAIL_BOX0));

	/*
	 * All non-boot-source PADs are in forced input-mode at
	 * reset so clear the force on non-boot-source PADs using
	 * CDRU register.
	 */
	mmio_clrbits_32((uintptr_t)CDRU_CHIP_IO_PAD_CONTROL,
		(1 << CDRU_CHIP_IO_PAD_CONTROL__CDRU_IOMUX_FORCE_PAD_IN_R));

#if DRIVER_OCOTP_ENABLE
	bcm_otpc_init(&otp_stingray_map);
#endif

	set_swreg_based_on_otp();

#if IHOST_PLL_FREQ != 0
	bcm_set_ihost_pll_freq(0x0, IHOST_PLL_FREQ);
#endif

#ifdef INCLUDE_EMMC_DRIVER_ERASE_CODE
	/* The erasable unit of the eMMC is the "Erase Group";
	 * Erase group is measured in write blocks which are the
	 * basic writable units of the Device.
	 * The size of the Erase Group is a Device specific parameter
	 */
	emmc_erase(EMMC_ERASE_START_BLOCK, EMMC_ERASE_BLOCK_COUNT,
		   EMMC_ERASE_PARTITION);
#endif

	bcm_board_detect();
#ifdef DRIVER_EMMC_ENABLE
	/* Initialize the card, if it is not */
	if (bcm_emmc_init(true) == 0)
		WARN("eMMC Card Initialization Failed!!!\n");
#endif

#if BL2_TEST_I2C
	i2c_test();
#endif

#ifdef USE_DDR
	ddr_initialize(&ddr_info);

	ddr_secure_region_config(SECURE_DDR_BASE_ADDRESS,
				 SECURE_DDR_END_ADDRESS);
#ifdef NITRO_SECURE_ACCESS
	ddr_secure_region_config(DDR_NITRO_SECURE_REGION_START,
				 DDR_NITRO_SECURE_REGION_END);
#endif
#else
	ext_sram_init();
#endif

#if BL2_TEST_MEM
	ddr_test();
#endif

#ifdef USE_NAND
	brcm_stingray_nand_init();
#endif

#if defined(USE_PAXB) || defined(USE_PAXC) || defined(USE_SATA)
	brcm_stingray_pcie_reset();
#endif

#ifdef USE_PAXC
	if (boot_source_get() != BOOT_SOURCE_QSPI)
		brcm_stingray_chimp_check_and_fastboot();
#endif

#if ((!CLEAN_DDR || MMU_DISABLED))
	/*
	 * Now DDR has been initialized. We want to copy all the logs in SRAM
	 * into DDR so we will have much more space to store the logs in the
	 * next boot stage
	 */
	bcm_elog_copy_log((void *)BCM_ELOG_BL31_BASE,
			   MIN(BCM_ELOG_BL2_SIZE, BCM_ELOG_BL31_SIZE)
			 );

	/*
	 * We are not yet at the end of BL2, but we can stop log here so we do
	 * not need to add 'bcm_elog_exit' to the standard BL2 code. The
	 * benefit of capturing BL2 logs after this is very minimal in a
	 * production system
	 * NOTE: BL2 logging must be exited before going forward to setup
	 * page tables
	 */
	bcm_elog_exit();
#endif
}
