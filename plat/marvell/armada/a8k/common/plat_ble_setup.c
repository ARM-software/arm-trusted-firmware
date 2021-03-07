/*
 * Copyright (C) 2018 Marvell International Ltd.
 *
 * SPDX-License-Identifier:     BSD-3-Clause
 * https://spdx.org/licenses
 */

#include <common/debug.h>
#include <drivers/marvell/ap807_clocks_init.h>
#include <drivers/marvell/aro.h>
#include <drivers/marvell/ccu.h>
#include <drivers/marvell/io_win.h>
#include <drivers/marvell/mochi/ap_setup.h>
#include <drivers/marvell/mochi/cp110_setup.h>

#include <armada_common.h>
#include <efuse_def.h>
#include <mv_ddr_if.h>
#include <mvebu_def.h>
#include <plat_marvell.h>

/* Register for skip image use */
#define SCRATCH_PAD_REG2		0xF06F00A8
#define SCRATCH_PAD_SKIP_VAL		0x01
#define NUM_OF_GPIO_PER_REG 32

#define MMAP_SAVE_AND_CONFIG		0
#define MMAP_RESTORE_SAVED		1

/* SAR clock settings */
#define MVEBU_AP_SAR_REG_BASE(r)	(MVEBU_AP_GEN_MGMT_BASE + 0x200 +\
								((r) << 2))

#define SAR_CLOCK_FREQ_MODE_OFFSET	(0)
#define SAR_CLOCK_FREQ_MODE_MASK	(0x1f << SAR_CLOCK_FREQ_MODE_OFFSET)
#define SAR_PIDI_LOW_SPEED_OFFSET	(20)
#define SAR_PIDI_LOW_SPEED_MASK		(1 << SAR_PIDI_LOW_SPEED_OFFSET)
#define SAR_PIDI_LOW_SPEED_SHIFT	(15)
#define SAR_PIDI_LOW_SPEED_SET		(1 << SAR_PIDI_LOW_SPEED_SHIFT)

#define FREQ_MODE_AP_SAR_REG_NUM	(0)
#define SAR_CLOCK_FREQ_MODE(v)		(((v) & SAR_CLOCK_FREQ_MODE_MASK) >> \
					SAR_CLOCK_FREQ_MODE_OFFSET)

#define AVS_I2C_EEPROM_ADDR		0x57	/* EEPROM */
#define AVS_EN_CTRL_REG			(MVEBU_AP_GEN_MGMT_BASE + 0x130)
#define AVS_ENABLE_OFFSET		(0)
#define AVS_SOFT_RESET_OFFSET		(2)
#define AVS_TARGET_DELTA_OFFSET		(21)

#ifndef MVEBU_SOC_AP807
	/* AP806 SVC bits */
	#define AVS_LOW_VDD_LIMIT_OFFSET	(4)
	#define AVS_HIGH_VDD_LIMIT_OFFSET	(12)
	#define AVS_VDD_LOW_LIMIT_MASK	(0xFF << AVS_LOW_VDD_LIMIT_OFFSET)
	#define AVS_VDD_HIGH_LIMIT_MASK	(0xFF << AVS_HIGH_VDD_LIMIT_OFFSET)
#else
	/* AP807 SVC bits */
	#define AVS_LOW_VDD_LIMIT_OFFSET	(3)
	#define AVS_HIGH_VDD_LIMIT_OFFSET	(13)
	#define AVS_VDD_LOW_LIMIT_MASK	(0x3FF << AVS_LOW_VDD_LIMIT_OFFSET)
	#define AVS_VDD_HIGH_LIMIT_MASK	(0x3FF << AVS_HIGH_VDD_LIMIT_OFFSET)
#endif

/* VDD limit is 0.9V for A70x0 @ CPU frequency < 1600MHz */
#define AVS_A7K_LOW_CLK_VALUE		((0x80 << AVS_TARGET_DELTA_OFFSET) | \
					 (0x1A << AVS_HIGH_VDD_LIMIT_OFFSET) | \
					 (0x1A << AVS_LOW_VDD_LIMIT_OFFSET) | \
					 (0x1 << AVS_SOFT_RESET_OFFSET) | \
					 (0x1 << AVS_ENABLE_OFFSET))
/* VDD limit is 1.0V for all A80x0 devices */
#define AVS_A8K_CLK_VALUE		((0x80 << AVS_TARGET_DELTA_OFFSET) | \
					 (0x24 << AVS_HIGH_VDD_LIMIT_OFFSET) | \
					 (0x24 << AVS_LOW_VDD_LIMIT_OFFSET) | \
					 (0x1 << AVS_SOFT_RESET_OFFSET) | \
					 (0x1 << AVS_ENABLE_OFFSET))

/* VDD is 0.88V for 2GHz clock on CN913x devices */
#define AVS_AP807_CLK_VALUE		((0x80UL << 24) | \
					 (0x2dc << 13) | \
					 (0x2dc << 3) | \
					 (0x1 << AVS_SOFT_RESET_OFFSET) | \
					 (0x1 << AVS_ENABLE_OFFSET))

/*
 * - Identification information in the LD-0 eFuse:
 *	DRO:           LD0[74:65] - Not used by the SW
 *	Revision:      LD0[78:75] - Not used by the SW
 *	Bin:           LD0[80:79] - Not used by the SW
 *	SW Revision:   LD0[115:113]
 *	Cluster 1 PWR: LD0[193] - if set to 1, power down CPU Cluster-1
 *				  resulting in 2 CPUs active only (7020)
 */
/* Offsets for 2 efuse fields combined into single 64-bit value [125:63] */
#define EFUSE_AP_LD0_DRO_OFFS		2		/* LD0[74:65] */
#define EFUSE_AP_LD0_DRO_MASK		0x3FF
#define EFUSE_AP_LD0_REVID_OFFS		12		/* LD0[78:75] */
#define EFUSE_AP_LD0_REVID_MASK		0xF
#define EFUSE_AP_LD0_BIN_OFFS		16		/* LD0[80:79] */
#define EFUSE_AP_LD0_BIN_MASK		0x3
#define EFUSE_AP_LD0_SWREV_MASK		0x7

#ifndef MVEBU_SOC_AP807
	/* AP806 AVS work points in the LD0 eFuse
	 * SVC1 work point:     LD0[88:81]
	 * SVC2 work point:     LD0[96:89]
	 * SVC3 work point:     LD0[104:97]
	 * SVC4 work point:     LD0[112:105]
	 */
	#define EFUSE_AP_LD0_SVC1_OFFS		18	/* LD0[88:81] */
	#define EFUSE_AP_LD0_SVC2_OFFS		26	/* LD0[96:89] */
	#define EFUSE_AP_LD0_SVC3_OFFS		34	/* LD0[104:97] */
	#define EFUSE_AP_LD0_WP_MASK		0xFF
	#define EFUSE_AP_LD0_SWREV_OFFS		50	/* LD0[115:113] */
#else
	/* AP807 AVS work points in the LD0 eFuse
	 * SVC1 work point:     LD0[91:81]
	 * SVC2 work point:     LD0[102:92]
	 * SVC3 work point:     LD0[113:103]
	 */
	#define EFUSE_AP_LD0_SVC1_OFFS		18	/* LD0[91:81] */
	#define EFUSE_AP_LD0_SVC2_OFFS		29	/* LD0[102:92] */
	#define EFUSE_AP_LD0_SVC3_OFFS		40	/* LD0[113:103] */
	#define EFUSE_AP_LD0_WP_MASK		0x7FF	/* 10 data,1 parity */
	#define EFUSE_AP_LD0_SWREV_OFFS		51	/* LD0[116:114] */
#endif

#define EFUSE_AP_LD0_SVC4_OFFS			42	/* LD0[112:105] */

#define EFUSE_AP_LD0_CLUSTER_DOWN_OFFS		4

#if MARVELL_SVC_TEST
#define MVEBU_CP_MPP_CTRL37_OFFS	20
#define MVEBU_CP_MPP_CTRL38_OFFS	24
#define MVEBU_CP_MPP_I2C_FUNC		2
#define MVEBU_MPP_CTRL_MASK		0xf
#endif

/* Return the AP revision of the chip */
static unsigned int ble_get_ap_type(void)
{
	unsigned int chip_rev_id;

	chip_rev_id = mmio_read_32(MVEBU_CSS_GWD_CTRL_IIDR2_REG);
	chip_rev_id = ((chip_rev_id & GWD_IIDR2_CHIP_ID_MASK) >>
			GWD_IIDR2_CHIP_ID_OFFSET);

	return chip_rev_id;
}

/******************************************************************************
 * The routine allows to save the CCU and IO windows configuration during DRAM
 * setup and restore them afterwards before exiting the BLE stage.
 * Such window configuration is required since not all default settings coming
 * from the HW and the BootROM allow access to peripherals connected to
 * all available CPn components.
 * For instance, when the boot device is located on CP0, the IO window to CP1
 * is not opened automatically by the HW and if the DRAM SPD is located on CP1
 * i2c channel, it cannot be read at BLE stage.
 * Therefore the DRAM init procedure have to provide access to all available
 * CPn peripherals during the BLE stage by setting the CCU IO window to all
 * CPnph addresses and by enabling the IO windows accordingly.
 * Additionally this function configures the CCU GCR to DRAM, which allows
 * usage or more than 4GB DRAM as it configured by the default CCU DRAM window.
 *
 * IN:
 *	MMAP_SAVE_AND_CONFIG	- save the existing configuration and update it
 *	MMAP_RESTORE_SAVED	- restore saved configuration
 * OUT:
 *	NONE
 ****************************************************************************
 */
static void ble_plat_mmap_config(int restore)
{
	if (restore == MMAP_RESTORE_SAVED) {
		/* Restore all orig. settings that were modified by BLE stage */
		ccu_restore_win_all(MVEBU_AP0);
		/* Restore CCU */
		iow_restore_win_all(MVEBU_AP0);
		return;
	}

	/* Store original values */
	ccu_save_win_all(MVEBU_AP0);
	/* Save CCU */
	iow_save_win_all(MVEBU_AP0);

	init_ccu(MVEBU_AP0);
	/* The configuration saved, now all the changes can be done */
	init_io_win(MVEBU_AP0);
}

/****************************************************************************
 * Setup Adaptive Voltage Switching - this is required for some platforms
 ****************************************************************************
 */
#if !MARVELL_SVC_TEST
static void ble_plat_avs_config(void)
{
	uint32_t freq_mode, device_id;
	uint32_t avs_val = 0;

	freq_mode =
		SAR_CLOCK_FREQ_MODE(mmio_read_32(MVEBU_AP_SAR_REG_BASE(
						 FREQ_MODE_AP_SAR_REG_NUM)));
	/* Check which SoC is running and act accordingly */
	if (ble_get_ap_type() == CHIP_ID_AP807) {

		avs_val = AVS_AP807_CLK_VALUE;

	} else {
		/* Check which SoC is running and act accordingly */
		device_id = cp110_device_id_get(MVEBU_CP_REGS_BASE(0));
		switch (device_id) {
		case MVEBU_80X0_DEV_ID:
		case MVEBU_80X0_CP115_DEV_ID:
			/* Always fix the default AVS value on A80x0 */
			avs_val = AVS_A8K_CLK_VALUE;
			break;
		case MVEBU_70X0_DEV_ID:
		case MVEBU_70X0_CP115_DEV_ID:
			/* Fix AVS for CPU clocks lower than 1600MHz on A70x0 */
			if ((freq_mode > CPU_1600_DDR_900_RCLK_900_2) &&
			    (freq_mode < CPU_DDR_RCLK_INVALID))
				avs_val = AVS_A7K_LOW_CLK_VALUE;
			break;
		default:
			ERROR("Unsupported Device ID 0x%x\n", device_id);
			return;
		}
	}

	if (avs_val) {
		VERBOSE("AVS: Setting AVS CTRL to 0x%x\n", avs_val);
		mmio_write_32(AVS_EN_CTRL_REG, avs_val);
	}
}
#endif
/******************************************************************************
 * Update or override current AVS work point value using data stored in EEPROM
 * This is only required by QA/validation flows and activated by
 * MARVELL_SVC_TEST flag.
 *
 * The function is expected to be called twice.
 *
 * First time with AVS value of 0 for testing if the EEPROM requests completely
 * override the AVS value and bypass the eFuse test
 *
 * Second time - with non-zero AVS value obtained from eFuses as an input.
 * In this case the EEPROM may contain AVS correction value (either positive
 * or negative) that is added to the input AVS value and returned back for
 * further processing.
 ******************************************************************************
 */
static uint32_t avs_update_from_eeprom(uint32_t avs_workpoint)
{
	uint32_t new_wp = avs_workpoint;
#if MARVELL_SVC_TEST
	/* ---------------------------------------------------------------------
	 * EEPROM  |  Data description (avs_step)
	 * address |
	 * ---------------------------------------------------------------------
	 * 0x120   | AVS workpoint correction value
	 *         | if not 0 and not 0xff, correct the AVS taken from eFuse
	 *         | by the number of steps indicated by bit[6:0]
	 *         | bit[7] defines correction direction.
	 *         | If bit[7]=1, add the value from bit[6:0] to AVS workpoint,
	 *         | othervise substruct this value from AVS workpoint.
	 * ---------------------------------------------------------------------
	 * 0x121   | AVS workpoint override value
	 *         | Override the AVS workpoint with the value stored in this
	 *         | byte. When running on AP806, the AVS workpoint is 7 bits
	 *         | wide and override value is valid when bit[6:0] holds
	 *         | value greater than zero and smaller than 0x33.
	 *         | When running on AP807, the AVS workpoint is 10 bits wide.
	 *         | Additional 2 MSB bits are supplied by EEPROM byte 0x122.
	 *         | AVS override value is valid when byte @ 0x121 and bit[1:0]
	 *         | of byte @ 0x122 combined have non-zero value.
	 * ---------------------------------------------------------------------
	 * 0x122   | Extended AVS workpoint override value
	 *         | Valid only for AP807 platforms and must be less than 0x4
	 * ---------------------------------------------------------------------
	 */
	static uint8_t  avs_step[3] = {0};
	uintptr_t reg;
	uint32_t val;
	unsigned int ap_type = ble_get_ap_type();

	/* Always happens on second call to this function */
	if (avs_workpoint != 0) {
		/* Get correction steps from the EEPROM */
		if ((avs_step[0] != 0) && (avs_step[0] != 0xff)) {
			NOTICE("AVS request to step %s by 0x%x from old 0x%x\n",
				avs_step[0] & 0x80 ? "DOWN" : "UP",
				avs_step[0] & 0x7f, new_wp);
			if (avs_step[0] & 0x80)
				new_wp -= avs_step[0] & 0x7f;
			else
				new_wp += avs_step[0] & 0x7f;
		}

		return new_wp;
	}

	/* AVS values are located in EEPROM
	 * at CP0 i2c bus #0, device 0x57 offset 0x120
	 * The SDA and SCK pins of CP0 i2c-0: MPP[38:37], i2c function 0x2.
	 */
	reg = MVEBU_CP_MPP_REGS(0, 4);
	val = mmio_read_32(reg);
	val &= ~((MVEBU_MPP_CTRL_MASK << MVEBU_CP_MPP_CTRL37_OFFS) |
		 (MVEBU_MPP_CTRL_MASK << MVEBU_CP_MPP_CTRL38_OFFS));
	val |= (MVEBU_CP_MPP_I2C_FUNC << MVEBU_CP_MPP_CTRL37_OFFS) |
		(MVEBU_CP_MPP_I2C_FUNC << MVEBU_CP_MPP_CTRL38_OFFS);
	mmio_write_32(reg, val);

	/* Init CP0 i2c-0 */
	i2c_init((void *)(MVEBU_CP0_I2C_BASE));

	/* Read EEPROM only once at the fist call! */
	i2c_read(AVS_I2C_EEPROM_ADDR, 0x120, 2, avs_step, 3);
	NOTICE("== SVC test build ==\n");
	NOTICE("EEPROM holds values 0x%x, 0x%x and 0x%x\n",
		avs_step[0], avs_step[1], avs_step[2]);

	/* Override the AVS value? */
	if ((ap_type != CHIP_ID_AP807) && (avs_step[1] < 0x33)) {
		/* AP806 - AVS is 7 bits */
		new_wp = avs_step[1];

	} else if (ap_type == CHIP_ID_AP807 && (avs_step[2] < 0x4)) {
		/* AP807 - AVS is 10 bits */
		new_wp = avs_step[2];
		new_wp <<= 8;
		new_wp |= avs_step[1];
	}

	if (new_wp == 0)
		NOTICE("Ignore BAD AVS Override value in EEPROM!\n");
	else
		NOTICE("Override AVS by EEPROM value 0x%x\n", new_wp);
#endif /* MARVELL_SVC_TEST */
	return new_wp;
}

/****************************************************************************
 * SVC flow - v0.10
 * The feature is intended to configure AVS value according to eFuse values
 * that are burned individually for each SoC during the test process.
 * Primary AVS value is stored in HD efuse and processed on power on
 * by the HW engine
 * Secondary AVS value is located in LD efuse and contains 4 work points for
 * various CPU frequencies.
 * The Secondary AVS value is only taken into account if the SW Revision stored
 * in the efuse is greater than 0 and the CPU is running in a certain speed.
 ****************************************************************************
 */
static void ble_plat_svc_config(void)
{
	uint32_t reg_val, avs_workpoint, freq_pidi_mode;
	uint64_t efuse;
	uint32_t device_id, single_cluster;
	uint16_t  svc[4], perr[4], i, sw_ver;
	uint8_t	 avs_data_bits, min_sw_ver, svc_fields;
	unsigned int ap_type;

	/* Get test EERPOM data */
	avs_workpoint = avs_update_from_eeprom(0);
	if (avs_workpoint)
		goto set_aws_wp;

	/* Set access to LD0 */
	reg_val = mmio_read_32(MVEBU_AP_EFUSE_SRV_CTRL_REG);
	reg_val &= ~EFUSE_SRV_CTRL_LD_SELECT_MASK;
	mmio_write_32(MVEBU_AP_EFUSE_SRV_CTRL_REG, reg_val);

	/* Obtain the value of LD0[125:63] */
	efuse = mmio_read_32(MVEBU_AP_LDX_125_95_EFUSE_OFFS);
	efuse <<= 32;
	efuse |= mmio_read_32(MVEBU_AP_LDX_94_63_EFUSE_OFFS);

	/* SW Revision:
	 * Starting from SW revision 1 the SVC flow is supported.
	 * SW version 0 (efuse not programmed) should follow the
	 * regular AVS update flow.
	 */
	sw_ver = (efuse >> EFUSE_AP_LD0_SWREV_OFFS) & EFUSE_AP_LD0_SWREV_MASK;
	if (sw_ver < 1) {
		NOTICE("SVC: SW Revision 0x%x. SVC is not supported\n", sw_ver);
#if MARVELL_SVC_TEST
		NOTICE("SVC_TEST: AVS bypassed\n");

#else
		ble_plat_avs_config();
#endif
		return;
	}

	/* Frequency mode from SAR */
	freq_pidi_mode = SAR_CLOCK_FREQ_MODE(
				mmio_read_32(
					MVEBU_AP_SAR_REG_BASE(
						FREQ_MODE_AP_SAR_REG_NUM)));

	/* Decode all SVC work points */
	svc[0] = (efuse >> EFUSE_AP_LD0_SVC1_OFFS) & EFUSE_AP_LD0_WP_MASK;
	svc[1] = (efuse >> EFUSE_AP_LD0_SVC2_OFFS) & EFUSE_AP_LD0_WP_MASK;
	svc[2] = (efuse >> EFUSE_AP_LD0_SVC3_OFFS) & EFUSE_AP_LD0_WP_MASK;

	/* Fetch AP type to distinguish between AP806 and AP807 */
	ap_type = ble_get_ap_type();

	if (ap_type != CHIP_ID_AP807) {
		svc[3] = (efuse >> EFUSE_AP_LD0_SVC4_OFFS)
			 & EFUSE_AP_LD0_WP_MASK;
		INFO("SVC: Efuse WP: [0]=0x%x, [1]=0x%x, [2]=0x%x, [3]=0x%x\n",
		     svc[0], svc[1], svc[2], svc[3]);
		avs_data_bits = 7;
		min_sw_ver = 2; /* parity check from sw revision 2 */
		svc_fields = 4;
	} else {
		INFO("SVC: Efuse WP: [0]=0x%x, [1]=0x%x, [2]=0x%x\n",
		     svc[0], svc[1], svc[2]);
		avs_data_bits = 10;
		min_sw_ver = 1; /* parity check required from sw revision 1 */
		svc_fields = 3;
	}

	/* Validate parity of SVC workpoint values */
	for (i = 0; i < svc_fields; i++) {
		uint8_t parity, bit;
		perr[i] = 0;

		for (bit = 1, parity = (svc[i] & 1); bit < avs_data_bits; bit++)
			parity ^= (svc[i] >> bit) & 1;

		/* From SW version 1 or 2 (AP806/AP807), check parity */
		if ((sw_ver >= min_sw_ver) &&
		    (parity != ((svc[i] >> avs_data_bits) & 1)))
			perr[i] = 1; /* register the error */
	}

	single_cluster = mmio_read_32(MVEBU_AP_LDX_220_189_EFUSE_OFFS);
	single_cluster = (single_cluster >> EFUSE_AP_LD0_CLUSTER_DOWN_OFFS) & 1;

	device_id = cp110_device_id_get(MVEBU_CP_REGS_BASE(0));
	if (device_id == MVEBU_80X0_DEV_ID ||
	    device_id == MVEBU_80X0_CP115_DEV_ID) {
		/* A8040/A8020 */
		NOTICE("SVC: DEV ID: %s, FREQ Mode: 0x%x\n",
			single_cluster == 0 ? "8040" : "8020", freq_pidi_mode);
		switch (freq_pidi_mode) {
		case CPU_1800_DDR_1050_RCLK_1050:
			if (perr[1])
				goto perror;
			avs_workpoint = svc[1];
			break;
		case CPU_1600_DDR_1050_RCLK_1050:
		case CPU_1600_DDR_900_RCLK_900_2:
			if (perr[2])
				goto perror;
			avs_workpoint = svc[2];
			break;
		case CPU_1300_DDR_800_RCLK_800:
		case CPU_1300_DDR_650_RCLK_650:
			if (perr[3])
				goto perror;
			avs_workpoint = svc[3];
			break;
		case CPU_2000_DDR_1200_RCLK_1200:
		case CPU_2000_DDR_1050_RCLK_1050:
		default:
			if (perr[0])
				goto perror;
			avs_workpoint = svc[0];
			break;
		}
	} else if (device_id == MVEBU_70X0_DEV_ID ||
		   device_id == MVEBU_70X0_CP115_DEV_ID) {
		/* A7040/A7020/A6040 */
		NOTICE("SVC: DEV ID: %s, FREQ Mode: 0x%x\n",
			single_cluster == 0 ? "7040" : "7020", freq_pidi_mode);
		switch (freq_pidi_mode) {
		case CPU_1400_DDR_800_RCLK_800:
			if (single_cluster) {/* 7020 */
				if (perr[1])
					goto perror;
				avs_workpoint = svc[1];
			} else {
				if (perr[0])
					goto perror;
				avs_workpoint = svc[0];
			}
			break;
		case CPU_1200_DDR_800_RCLK_800:
			if (single_cluster) {/* 7020 */
				if (perr[2])
					goto perror;
				avs_workpoint = svc[2];
			} else {
				if (perr[1])
					goto perror;
				avs_workpoint = svc[1];
			}
			break;
		case CPU_800_DDR_800_RCLK_800:
		case CPU_1000_DDR_800_RCLK_800:
			if (single_cluster) {/* 7020 */
				if (perr[3])
					goto perror;
				avs_workpoint = svc[3];
			} else {
				if (perr[2])
					goto perror;
				avs_workpoint = svc[2];
			}
			break;
		case CPU_600_DDR_800_RCLK_800:
			if (perr[3])
				goto perror;
			avs_workpoint = svc[3]; /* Same for 6040 and 7020 */
			break;
		case CPU_1600_DDR_800_RCLK_800: /* 7020 only */
		default:
			if (single_cluster) {/* 7020 */
				if (perr[0])
					goto perror;
				avs_workpoint = svc[0];
			} else {
#if MARVELL_SVC_TEST
				reg_val = mmio_read_32(AVS_EN_CTRL_REG);
				avs_workpoint = (reg_val &
					AVS_VDD_LOW_LIMIT_MASK) >>
					AVS_LOW_VDD_LIMIT_OFFSET;
				NOTICE("7040 1600Mhz, avs = 0x%x\n",
					avs_workpoint);
#else
				NOTICE("SVC: AVS work point not changed\n");
				return;
#endif
			}
			break;
		}
	} else if (device_id == MVEBU_3900_DEV_ID) {
		NOTICE("SVC: DEV ID: %s, FREQ Mode: 0x%x\n",
		       "3900", freq_pidi_mode);
		switch (freq_pidi_mode) {
		case CPU_1600_DDR_1200_RCLK_1200:
			if (perr[0])
				goto perror;
			avs_workpoint = svc[0];
			break;
		case CPU_1300_DDR_800_RCLK_800:
			if (perr[1])
				goto perror;
			avs_workpoint = svc[1];
			break;
		default:
			if (perr[0])
				goto perror;
			avs_workpoint = svc[0];
			break;
		}
	} else if (device_id == MVEBU_CN9130_DEV_ID) {
		NOTICE("SVC: DEV ID: %s, FREQ Mode: 0x%x\n",
		       "CN913x", freq_pidi_mode);
		switch (freq_pidi_mode) {
		case CPU_2200_DDR_1200_RCLK_1200:
			if (perr[0])
				goto perror;
			avs_workpoint = svc[0];
			break;
		case CPU_2000_DDR_1200_RCLK_1200:
			if (perr[1])
				goto perror;
			avs_workpoint = svc[1];
			break;
		case CPU_1600_DDR_1200_RCLK_1200:
			if (perr[2])
				goto perror;
			avs_workpoint = svc[2];
			break;
		default:
			ERROR("SVC: Unsupported Frequency 0x%x\n",
				freq_pidi_mode);
			return;

		}
	} else {
		ERROR("SVC: Unsupported Device ID 0x%x\n", device_id);
		return;
	}

	/* Set AVS control if needed */
	if (avs_workpoint == 0) {
		ERROR("SVC: You are using a frequency setup which is\n");
		ERROR("Not supported by this device\n");
		ERROR("This may result in malfunction of the device\n");
		return;
	}

	/* Remove parity bit */
	if (ap_type != CHIP_ID_AP807)
		avs_workpoint &= 0x7F;
	else
		avs_workpoint &= 0x3FF;

	/* Update WP from EEPROM if needed */
	avs_workpoint = avs_update_from_eeprom(avs_workpoint);

set_aws_wp:
	reg_val  = mmio_read_32(AVS_EN_CTRL_REG);
	NOTICE("SVC: AVS work point changed from 0x%x to 0x%x\n",
		(reg_val & AVS_VDD_LOW_LIMIT_MASK) >> AVS_LOW_VDD_LIMIT_OFFSET,
		avs_workpoint);
	reg_val &= ~(AVS_VDD_LOW_LIMIT_MASK | AVS_VDD_HIGH_LIMIT_MASK);
	reg_val |= 0x1 << AVS_ENABLE_OFFSET;
	reg_val |= avs_workpoint << AVS_HIGH_VDD_LIMIT_OFFSET;
	reg_val |= avs_workpoint << AVS_LOW_VDD_LIMIT_OFFSET;
	mmio_write_32(AVS_EN_CTRL_REG, reg_val);
	return;

perror:
	ERROR("Failed SVC WP[%d] parity check!\n", i);
	ERROR("Ignoring the WP values\n");
}

#if PLAT_RECOVERY_IMAGE_ENABLE
static int ble_skip_image_i2c(struct skip_image *skip_im)
{
	ERROR("skipping image using i2c is not supported\n");
	/* not supported */
	return 0;
}

static int ble_skip_image_other(struct skip_image *skip_im)
{
	ERROR("implementation missing for skip image request\n");
	/* not supported, make your own implementation */
	return 0;
}

static int ble_skip_image_gpio(struct skip_image *skip_im)
{
	unsigned int val;
	unsigned int mpp_address = 0;
	unsigned int offset = 0;

	switch (skip_im->info.test.cp_ap) {
	case(CP):
		mpp_address = MVEBU_CP_GPIO_DATA_IN(skip_im->info.test.cp_index,
						    skip_im->info.gpio.num);
		if (skip_im->info.gpio.num > NUM_OF_GPIO_PER_REG)
			offset = skip_im->info.gpio.num - NUM_OF_GPIO_PER_REG;
		else
			offset = skip_im->info.gpio.num;
		break;
	case(AP):
		mpp_address = MVEBU_AP_GPIO_DATA_IN;
		offset = skip_im->info.gpio.num;
		break;
	}

	val = mmio_read_32(mpp_address);
	val &= (1 << offset);
	if ((!val && skip_im->info.gpio.button_state == HIGH) ||
	    (val && skip_im->info.gpio.button_state == LOW)) {
		mmio_write_32(SCRATCH_PAD_REG2, SCRATCH_PAD_SKIP_VAL);
		return 1;
	}

	return 0;
}

/*
 * This function checks if there's a skip image request:
 * return values:
 * 1: (true) images request been made.
 * 0: (false) no image request been made.
 */
static int  ble_skip_current_image(void)
{
	struct skip_image *skip_im;

	/*fetching skip image info*/
	skip_im = (struct skip_image *)plat_marvell_get_skip_image_data();

	if (skip_im == NULL)
		return 0;

	/* check if skipping image request has already been made */
	if (mmio_read_32(SCRATCH_PAD_REG2) == SCRATCH_PAD_SKIP_VAL)
		return 0;

	switch (skip_im->detection_method) {
	case GPIO:
		return ble_skip_image_gpio(skip_im);
	case I2C:
		return ble_skip_image_i2c(skip_im);
	case USER_DEFINED:
		return ble_skip_image_other(skip_im);
	}

	return 0;
}
#endif


int ble_plat_setup(int *skip)
{
	int ret, cp;
	unsigned int freq_mode;

	/* Power down unused CPUs */
	plat_marvell_early_cpu_powerdown();

	/*
	 * Save the current CCU configuration and make required changes:
	 * - Allow access to DRAM larger than 4GB
	 * - Open memory access to all CPn peripherals
	 */
	ble_plat_mmap_config(MMAP_SAVE_AND_CONFIG);

#if PLAT_RECOVERY_IMAGE_ENABLE
	/* Check if there's a skip request to bootRom recovery Image */
	if (ble_skip_current_image()) {
		/* close memory access to all CPn peripherals. */
		ble_plat_mmap_config(MMAP_RESTORE_SAVED);
		*skip = 1;
		return 0;
	}
#endif
	/* Do required CP-110 setups for BLE stage */
	cp110_ble_init(MVEBU_CP_REGS_BASE(0));

	/* Config address for each cp other than cp0 */
	for (cp = 1; cp < CP_COUNT; cp++)
		update_cp110_default_win(cp);

	/* Setup AVS */
	ble_plat_svc_config();

	/* read clk option from sampled-at-reset register */
	freq_mode =
		SAR_CLOCK_FREQ_MODE(mmio_read_32(MVEBU_AP_SAR_REG_BASE(
						 FREQ_MODE_AP_SAR_REG_NUM)));

	/* work with PLL clock driver in AP807 */
	if (ble_get_ap_type() == CHIP_ID_AP807)
		ap807_clocks_init(freq_mode);

	/* Do required AP setups for BLE stage */
	ap_ble_init();

	/* Update DRAM topology (scan DIMM SPDs) */
	plat_marvell_dram_update_topology();

	/* Kick it in */
	ret = dram_init();

	/* Restore the original CCU configuration before exit from BLE */
	ble_plat_mmap_config(MMAP_RESTORE_SAVED);

	return ret;
}
