/*
 * Copyright (c) 2015-2018, Renesas Electronics Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/debug.h>
#include <lib/mmio.h>
#include <lib/utils_def.h>

#include "cpg_registers.h"
#include "avs_driver.h"
#include "rcar_def.h"
#include "rcar_private.h"

#if (AVS_SETTING_ENABLE == 1)
#if PMIC_ROHM_BD9571
/* Read PMIC register for debug. 1:enable / 0:disable */
#define AVS_READ_PMIC_REG_ENABLE	0
/* The re-try number of times of the AVS setting. */
#define AVS_RETRY_NUM			(1U)
#endif /* PMIC_ROHM_BD9571 */

/* Base address of Adaptive Voltage Scaling module registers*/
#define	AVS_BASE			(0xE60A0000U)
/* Adaptive Dynamic Voltage ADJust Parameter2 registers */
#define	ADVADJP2			(AVS_BASE + 0x013CU)

/* Mask VOLCOND bit in ADVADJP2 registers */
#define	ADVADJP2_VOLCOND_MASK		(0x000001FFU)	/* VOLCOND[8:0] */

#if PMIC_ROHM_BD9571
/* I2C for DVFS bit in CPG registers for module standby and software reset*/
#define CPG_SYS_DVFS_BIT		(0x04000000U)
#endif /* PMIC_ROHM_BD9571 */
/* ADVFS Module bit in CPG registers for module standby and software reset*/
#define CPG_SYS_ADVFS_BIT		(0x02000000U)

#if PMIC_ROHM_BD9571
/* Base address of IICDVFS registers*/
#define	IIC_DVFS_BASE			(0xE60B0000U)
/* IIC bus data register */
#define	IIC_ICDR			(IIC_DVFS_BASE + 0x0000U)
/* IIC bus control register */
#define	IIC_ICCR			(IIC_DVFS_BASE + 0x0004U)
/* IIC bus status register */
#define	IIC_ICSR			(IIC_DVFS_BASE + 0x0008U)
/* IIC interrupt control register */
#define	IIC_ICIC			(IIC_DVFS_BASE + 0x000CU)
/* IIC clock control register low */
#define	IIC_ICCL			(IIC_DVFS_BASE + 0x0010U)
/* IIC clock control register high */
#define	IIC_ICCH			(IIC_DVFS_BASE + 0x0014U)

/* Bit in ICSR register */
#define ICSR_BUSY			(0x10U)
#define ICSR_AL				(0x08U)
#define ICSR_TACK			(0x04U)
#define ICSR_WAIT			(0x02U)
#define ICSR_DTE			(0x01U)

/* Bit in ICIC register */
#define ICIC_TACKE			(0x04U)
#define ICIC_WAITE			(0x02U)
#define ICIC_DTEE			(0x01U)

/* I2C bus interface enable */
#define ICCR_ENABLE			(0x80U)
/* Start condition */
#define ICCR_START			(0x94U)
/* Stop condition */
#define ICCR_STOP			(0x90U)
/* Restart condition with change to receive mode change */
#define ICCR_START_RECV			(0x81U)
/* Stop condition for receive mode */
#define ICCR_STOP_RECV			(0xC0U)

/* Low-level period of SCL */
#define	ICCL_FREQ_8p33M			(0x07U)	/* for CP Phy 8.3333MHz */
#define	ICCL_FREQ_10M			(0x09U)	/* for CP Phy 10MHz */
#define	ICCL_FREQ_12p5M			(0x0BU)	/* for CP Phy 12.5MHz */
#define	ICCL_FREQ_16p66M		(0x0EU)	/* for CP Phy 16.6666MHz */
/* High-level period of SCL */
#define	ICCH_FREQ_8p33M			(0x01U)	/* for CP Phy 8.3333MHz */
#define	ICCH_FREQ_10M			(0x02U)	/* for CP Phy 10MHz */
#define	ICCH_FREQ_12p5M			(0x03U)	/* for CP Phy 12.5MHz */
#define	ICCH_FREQ_16p66M		(0x05U)	/* for CP Phy 16.6666MHz */

/* PMIC */
#define	PMIC_W_SLAVE_ADDRESS		(0x60U)	/* ROHM BD9571 slave address + (W) */
#define	PMIC_R_SLAVE_ADDRESS		(0x61U)	/* ROHM BD9571 slave address + (R) */
#define	PMIC_DVFS_SETVID		(0x54U)	/* ROHM BD9571 DVFS SetVID register */
#endif /* PMIC_ROHM_BD9571  */

/* Individual information */
#define EFUSE_AVS0			(0U)
#define EFUSE_AVS_NUM			ARRAY_SIZE(init_vol_tbl)

typedef struct {
	uint32_t avs;		/* AVS code */
	uint8_t vol;		/* Voltage */
} initial_voltage_t;

static const initial_voltage_t init_vol_tbl[] = {
	/*      AVS code,       RHOM BD9571 DVFS SetVID register */
	{0x00U, 0x53U},		/* AVS0, 0.83V */
	{0x01U, 0x52U},		/* AVS1, 0.82V */
	{0x02U, 0x51U},		/* AVS2, 0.81V */
	{0x04U, 0x50U},		/* AVS3, 0.80V */
	{0x08U, 0x4FU},		/* AVS4, 0.79V */
	{0x10U, 0x4EU},		/* AVS5, 0.78V */
	{0x20U, 0x4DU},		/* AVS6, 0.77V */
	{0x40U, 0x4CU}		/* AVS7, 0.76V */
};

#if PMIC_ROHM_BD9571
/* Kind of AVS settings status */
typedef enum {
	avs_status_none = 0,
	avs_status_init,
	avs_status_start_condition,
	avs_status_set_slave_addr,
	avs_status_write_reg_addr,
	avs_status_write_reg_data,
	avs_status_stop_condition,
	avs_status_end,
	avs_status_complete,
	avs_status_al_start,
	avs_status_al_transfer,
	avs_status_nack,
	avs_status_error_stop,
	ave_status_error_end
} avs_status_t;

/* Kind of AVS error */
typedef enum {
	avs_error_none = 0,
	avs_error_al,
	avs_error_nack
} avs_error_t;

static avs_status_t avs_status;
static uint32_t avs_retry;
#endif /* PMIC_ROHM_BD9571  */
static uint32_t efuse_avs = EFUSE_AVS0;

#if PMIC_ROHM_BD9571
/* prototype */
static avs_error_t avs_check_error(void);
static void avs_set_iic_clock(void);
#if AVS_READ_PMIC_REG_ENABLE == 1
static uint8_t avs_read_pmic_reg(uint8_t addr);
static void avs_poll(uint8_t bit_pos, uint8_t val);
#endif
#endif /* PMIC_ROHM_BD9571 */
#endif /* (AVS_SETTING_ENABLE==1) */

/*
 * Initialize to enable the AVS setting.
 */
void rcar_avs_init(void)
{
#if (AVS_SETTING_ENABLE == 1)
	uint32_t val;

#if PMIC_ROHM_BD9571
	/* Initialize AVS status */
	avs_status = avs_status_init;
#endif /* PMIC_ROHM_BD9571 */

	/* Enable clock supply to ADVFS. */
	mstpcr_write(CPG_SMSTPCR9, CPG_MSTPSR9, CPG_SYS_ADVFS_BIT);

	/* Read AVS code (Initial values are derived from eFuse) */
	val = mmio_read_32(ADVADJP2) & ADVADJP2_VOLCOND_MASK;

	for (efuse_avs = 0U; efuse_avs < EFUSE_AVS_NUM; efuse_avs++) {
		if (val == init_vol_tbl[efuse_avs].avs)
			break;
	}

	if (efuse_avs >= EFUSE_AVS_NUM)
		efuse_avs = EFUSE_AVS0;	/* Not applicable */
#if PMIC_ROHM_BD9571
	/* Enable clock supply to DVFS. */
	mstpcr_write(CPG_SMSTPCR9, CPG_MSTPSR9, CPG_SYS_DVFS_BIT);

	/* Disable I2C module and All internal registers initialized. */
	mmio_write_8(IIC_ICCR, 0x00U);
	while ((mmio_read_8(IIC_ICCR) & ICCR_ENABLE) != 0U) {
		/* Disable I2C module and All internal registers initialized. */
		mmio_write_8(IIC_ICCR, 0x00U);
	}

	/* Set next status */
	avs_status = avs_status_start_condition;

#endif /* PMIC_ROHM_BD9571 */
#endif /* (AVS_SETTING_ENABLE==1) */
}

/*
 * Set the value of register corresponding to the voltage
 * by transfer of I2C to PIMC.
 */
void rcar_avs_setting(void)
{
#if (AVS_SETTING_ENABLE == 1)
#if PMIC_ROHM_BD9571
	avs_error_t err;

	switch (avs_status) {
	case avs_status_start_condition:
		/* Set ICCR.ICE=1 to activate the I2C module. */
		mmio_write_8(IIC_ICCR, mmio_read_8(IIC_ICCR) | ICCR_ENABLE);
		/* Set frequency of 400kHz */
		avs_set_iic_clock();
		/* Set ICIC.TACKE=1, ICIC.WAITE=1, ICIC.DTEE=1 to */
		/* enable interrupt control.                      */
		mmio_write_8(IIC_ICIC, mmio_read_8(IIC_ICIC)
			     | ICIC_TACKE | ICIC_WAITE | ICIC_DTEE);
		/* Write H'94 in ICCR to issue start condition */
		mmio_write_8(IIC_ICCR, ICCR_START);
		/* Set next status */
		avs_status = avs_status_set_slave_addr;
		break;
	case avs_status_set_slave_addr:
		/* Check error. */
		err = avs_check_error();
		if (err == avs_error_al) {
			/* Recovery sequence of just after start. */
			avs_status = avs_status_al_start;
		} else if (err == avs_error_nack) {
			/* Recovery sequence of detected NACK */
			avs_status = avs_status_nack;
		} else {
			/* Was data transmission enabled ? */
			if ((mmio_read_8(IIC_ICSR) & ICSR_DTE) == ICSR_DTE) {
				/* Clear ICIC.DTEE to disable a DTE interrupt */
				mmio_write_8(IIC_ICIC, mmio_read_8(IIC_ICIC)
					     & (uint8_t) (~ICIC_DTEE));
				/* Send PMIC slave address + (W) */
				mmio_write_8(IIC_ICDR, PMIC_W_SLAVE_ADDRESS);
				/* Set next status */
				avs_status = avs_status_write_reg_addr;
			}
		}
		break;
	case avs_status_write_reg_addr:
		/* Check error. */
		err = avs_check_error();
		if (err == avs_error_al) {
			/* Recovery sequence of during data transfer. */
			avs_status = avs_status_al_transfer;
		} else if (err == avs_error_nack) {
			/* Recovery sequence of detected NACK */
			avs_status = avs_status_nack;
		} else {
			/* If wait state after data transmission. */
			if ((mmio_read_8(IIC_ICSR) & ICSR_WAIT) == ICSR_WAIT) {
				/* Write PMIC DVFS_SetVID address */
				mmio_write_8(IIC_ICDR, PMIC_DVFS_SETVID);
				/* Clear ICSR.WAIT to exit from wait state. */
				mmio_write_8(IIC_ICSR, mmio_read_8(IIC_ICSR)
					     & (uint8_t) (~ICSR_WAIT));
				/* Set next status */
				avs_status = avs_status_write_reg_data;
			}
		}
		break;
	case avs_status_write_reg_data:
		/* Check error. */
		err = avs_check_error();
		if (err == avs_error_al) {
			/* Recovery sequence of during data transfer. */
			avs_status = avs_status_al_transfer;
		} else if (err == avs_error_nack) {
			/* Recovery sequence of detected NACK */
			avs_status = avs_status_nack;
		} else {
			/* If wait state after data transmission. */
			if ((mmio_read_8(IIC_ICSR) & ICSR_WAIT) == ICSR_WAIT) {
				/* Dose efuse_avs exceed the number of */
				/* the tables? */
				if (efuse_avs >= EFUSE_AVS_NUM) {
					ERROR("AVS number of eFuse is out "
					      "of a range. number=%u\n",
					      efuse_avs);
					/* Infinite loop */
					panic();
				}
				/* Write PMIC DVFS_SetVID value */
				mmio_write_8(IIC_ICDR,
					     init_vol_tbl[efuse_avs].vol);
				/* Clear ICSR.WAIT to exit from wait state. */
				mmio_write_8(IIC_ICSR, mmio_read_8(IIC_ICSR)
					     & (uint8_t) (~ICSR_WAIT));
				/* Set next status */
				avs_status = avs_status_stop_condition;
			}
		}
		break;
	case avs_status_stop_condition:
		err = avs_check_error();
		if (err == avs_error_al) {
			/* Recovery sequence of during data transfer. */
			avs_status = avs_status_al_transfer;
		} else if (err == avs_error_nack) {
			/* Recovery sequence of detected NACK */
			avs_status = avs_status_nack;
		} else {
			/* If wait state after data transmission. */
			if ((mmio_read_8(IIC_ICSR) & ICSR_WAIT) == ICSR_WAIT) {
				/* Write H'90 in ICCR to issue stop condition */
				mmio_write_8(IIC_ICCR, ICCR_STOP);
				/* Clear ICSR.WAIT to exit from wait state. */
				mmio_write_8(IIC_ICSR, mmio_read_8(IIC_ICSR)
					     & (uint8_t) (~ICSR_WAIT));
				/* Set next status */
				avs_status = avs_status_end;
			}
		}
		break;
	case avs_status_end:
		/* Is this module not busy?. */
		if ((mmio_read_8(IIC_ICSR) & ICSR_BUSY) == 0U) {
			/* Set ICCR=H'00 to disable the I2C module. */
			mmio_write_8(IIC_ICCR, 0x00U);
			/* Set next status */
			avs_status = avs_status_complete;
		}
		break;
	case avs_status_al_start:
		/* Clear ICSR.AL bit */
		mmio_write_8(IIC_ICSR, (mmio_read_8(IIC_ICSR)
					& (uint8_t) (~ICSR_AL)));
		/* Transmit a clock pulse */
		mmio_write_8(IIC_ICDR, init_vol_tbl[EFUSE_AVS0].vol);
		/* Set next status */
		avs_status = avs_status_error_stop;
		break;
	case avs_status_al_transfer:
		/* Clear ICSR.AL bit */
		mmio_write_8(IIC_ICSR, (mmio_read_8(IIC_ICSR)
					& (uint8_t) (~ICSR_AL)));
		/* Set next status */
		avs_status = avs_status_error_stop;
		break;
	case avs_status_nack:
		/* Write H'90 in ICCR to issue stop condition */
		mmio_write_8(IIC_ICCR, ICCR_STOP);
		/* Disable a WAIT and DTEE interrupt. */
		mmio_write_8(IIC_ICIC, mmio_read_8(IIC_ICIC)
			     & (uint8_t) (~(ICIC_WAITE | ICIC_DTEE)));
		/* Clear ICSR.TACK bit */
		mmio_write_8(IIC_ICSR, mmio_read_8(IIC_ICSR)
			     & (uint8_t) (~ICSR_TACK));
		/* Set next status */
		avs_status = ave_status_error_end;
		break;
	case avs_status_error_stop:
		/* If wait state after data transmission. */
		if ((mmio_read_8(IIC_ICSR) & ICSR_WAIT) == ICSR_WAIT) {
			/* Write H'90 in ICCR to issue stop condition */
			mmio_write_8(IIC_ICCR, ICCR_STOP);
			/* Clear ICSR.WAIT to exit from wait state. */
			mmio_write_8(IIC_ICSR, mmio_read_8(IIC_ICSR)
				     & (uint8_t) (~ICSR_WAIT));
			/* Set next status */
			avs_status = ave_status_error_end;
		}
		break;
	case ave_status_error_end:
		/* Is this module not busy?. */
		if ((mmio_read_8(IIC_ICSR) & ICSR_BUSY) == 0U) {
			/* Set ICCR=H'00 to disable the I2C module. */
			mmio_write_8(IIC_ICCR, 0x00U);
			/* Increment the re-try number of times. */
			avs_retry++;
			/* Set start a re-try to status. */
			avs_status = avs_status_start_condition;
		}
		break;
	case avs_status_complete:
		/* After "avs_status" became the "avs_status_complete", */
		/* "avs_setting()" function may be called. */
		break;
	default:
		/* This case is not possible. */
		ERROR("AVS setting is in invalid status. status=%u\n",
		      avs_status);
		/* Infinite loop */
		panic();
		break;
	}
#endif /* PMIC_ROHM_BD9571 */
#endif /* (AVS_SETTING_ENABLE==1) */
}

/*
 * Finish the AVS setting.
 */
void rcar_avs_end(void)
{
#if (AVS_SETTING_ENABLE == 1)
	uint32_t mstp;

#if PMIC_ROHM_BD9571
	/* While status is not completion, be repeated. */
	while (avs_status != avs_status_complete)
		rcar_avs_setting();

	NOTICE("AVS setting succeeded. DVFS_SetVID=0x%x\n",
	       init_vol_tbl[efuse_avs].vol);

#if AVS_READ_PMIC_REG_ENABLE == 1
	{
		uint8_t addr = PMIC_DVFS_SETVID;
		uint8_t value = avs_read_pmic_reg(addr);
		NOTICE("Read PMIC register. address=0x%x value=0x%x \n",
		       addr, value);
	}
#endif

	/* Bit of the module which wants to disable clock supply. */
	mstp = CPG_SYS_DVFS_BIT;
	/* Disables the supply of clock signal to a module. */
	cpg_write(CPG_SMSTPCR9, mmio_read_32(CPG_SMSTPCR9) | mstp);
#endif /* PMIC_ROHM_BD9571 */

	/* Bit of the module which wants to disable clock supply. */
	mstp = CPG_SYS_ADVFS_BIT;
	/* Disables the supply of clock signal to a module. */
	cpg_write(CPG_SMSTPCR9, mmio_read_32(CPG_SMSTPCR9) | mstp);

#endif /* (AVS_SETTING_ENABLE==1) */
}

#if (AVS_SETTING_ENABLE == 1)
#if PMIC_ROHM_BD9571
/*
 * Check error and judge re-try.
 */
static avs_error_t avs_check_error(void)
{
	avs_error_t ret;

	if ((mmio_read_8(IIC_ICSR) & ICSR_AL) == ICSR_AL) {
		NOTICE("Loss of arbitration is detected. "
		       "AVS status=%d Retry=%u\n", avs_status, avs_retry);
		/* Check of retry number of times */
		if (avs_retry >= AVS_RETRY_NUM) {
			ERROR("AVS setting failed in retry. max=%u\n",
			      AVS_RETRY_NUM);
			/* Infinite loop */
			panic();
		}
		/* Set the error detected to error status. */
		ret = avs_error_al;
	} else if ((mmio_read_8(IIC_ICSR) & ICSR_TACK) == ICSR_TACK) {
		NOTICE("Non-acknowledge is detected. "
		       "AVS status=%d Retry=%u\n", avs_status, avs_retry);
		/* Check of retry number of times */
		if (avs_retry >= AVS_RETRY_NUM) {
			ERROR("AVS setting failed in retry. max=%u\n",
			      AVS_RETRY_NUM);
			/* Infinite loop */
			panic();
		}
		/* Set the error detected to error status. */
		ret = avs_error_nack;
	} else {
		/* Not error. */
		ret = avs_error_none;
	}
	return ret;
}

/*
 * Set I2C for DVFS clock.
 */
static void avs_set_iic_clock(void)
{
	uint32_t md_pin;

	/* Read Mode pin register. */
	md_pin = mmio_read_32(RCAR_MODEMR) & CHECK_MD13_MD14;
	/* Set the module clock (CP phy) for the IIC-DVFS. */
	/* CP phy is EXTAL / 2.                            */
	switch (md_pin) {
	case MD14_MD13_TYPE_0:	/* EXTAL = 16.6666MHz */
		mmio_write_8(IIC_ICCL, ICCL_FREQ_8p33M);
		mmio_write_8(IIC_ICCH, ICCH_FREQ_8p33M);
		break;
	case MD14_MD13_TYPE_1:	/* EXTAL = 20MHz */
		mmio_write_8(IIC_ICCL, ICCL_FREQ_10M);
		mmio_write_8(IIC_ICCH, ICCH_FREQ_10M);
		break;
	case MD14_MD13_TYPE_2:	/* EXTAL = 25MHz (H3/M3) */
		mmio_write_8(IIC_ICCL, ICCL_FREQ_12p5M);
		mmio_write_8(IIC_ICCH, ICCH_FREQ_12p5M);
		break;
	case MD14_MD13_TYPE_3:	/* EXTAL = 33.3333MHz */
		mmio_write_8(IIC_ICCL, ICCL_FREQ_16p66M);
		mmio_write_8(IIC_ICCH, ICCH_FREQ_16p66M);
		break;
	default:		/* This case is not possible. */
		/* CP Phy frequency is to be set for the 16.66MHz */
		mmio_write_8(IIC_ICCL, ICCL_FREQ_16p66M);
		mmio_write_8(IIC_ICCH, ICCH_FREQ_16p66M);
		break;
	}
}

#if AVS_READ_PMIC_REG_ENABLE == 1
/*
 * Read the value of the register of PMIC.
 */
static uint8_t avs_read_pmic_reg(uint8_t addr)
{
	uint8_t reg;

	/* Set ICCR.ICE=1 to activate the I2C module. */
	mmio_write_8(IIC_ICCR, mmio_read_8(IIC_ICCR) | ICCR_ENABLE);

	/* Set frequency of 400kHz */
	avs_set_iic_clock();

	/* Set ICIC.WAITE=1, ICIC.DTEE=1 to enable data transmission    */
	/* interrupt and wait interrupt.                                */
	mmio_write_8(IIC_ICIC, mmio_read_8(IIC_ICIC) | ICIC_WAITE | ICIC_DTEE);

	/* Write H'94 in ICCR to issue start condition */
	mmio_write_8(IIC_ICCR, ICCR_START);

	/* Wait for a until ICSR.DTE becomes 1. */
	avs_poll(ICSR_DTE, 1U);

	/* Clear ICIC.DTEE to disable a DTE interrupt. */
	mmio_write_8(IIC_ICIC, mmio_read_8(IIC_ICIC) & (uint8_t) (~ICIC_DTEE));
	/* Send slave address of PMIC */
	mmio_write_8(IIC_ICDR, PMIC_W_SLAVE_ADDRESS);

	/* Wait for a until ICSR.WAIT becomes 1. */
	avs_poll(ICSR_WAIT, 1U);

	/* write PMIC address */
	mmio_write_8(IIC_ICDR, addr);
	/* Clear ICSR.WAIT to exit from WAIT status. */
	mmio_write_8(IIC_ICSR, mmio_read_8(IIC_ICSR) & (uint8_t) (~ICSR_WAIT));

	/* Wait for a until ICSR.WAIT becomes 1. */
	avs_poll(ICSR_WAIT, 1U);

	/* Write H'94 in ICCR to issue restart condition */
	mmio_write_8(IIC_ICCR, ICCR_START);
	/* Clear ICSR.WAIT to exit from WAIT status. */
	mmio_write_8(IIC_ICSR, mmio_read_8(IIC_ICSR) & (uint8_t) (~ICSR_WAIT));
	/* Set ICIC.DTEE=1 to enable data transmission interrupt. */
	mmio_write_8(IIC_ICIC, mmio_read_8(IIC_ICIC) | ICIC_DTEE);

	/* Wait for a until ICSR.DTE becomes 1. */
	avs_poll(ICSR_DTE, 1U);

	/* Clear ICIC.DTEE to disable a DTE interrupt. */
	mmio_write_8(IIC_ICIC, mmio_read_8(IIC_ICIC) & (uint8_t) (~ICIC_DTEE));
	/* Send slave address of PMIC */
	mmio_write_8(IIC_ICDR, PMIC_R_SLAVE_ADDRESS);

	/* Wait for a until ICSR.WAIT becomes 1. */
	avs_poll(ICSR_WAIT, 1U);

	/* Write H'81 to ICCR to issue the repeated START condition     */
	/* for changing the transmission mode to the receive mode.      */
	mmio_write_8(IIC_ICCR, ICCR_START_RECV);
	/* Clear ICSR.WAIT to exit from WAIT status. */
	mmio_write_8(IIC_ICSR, mmio_read_8(IIC_ICSR) & (uint8_t) (~ICSR_WAIT));

	/* Wait for a until ICSR.WAIT becomes 1. */
	avs_poll(ICSR_WAIT, 1U);

	/* Set ICCR to H'C0 for the STOP condition */
	mmio_write_8(IIC_ICCR, ICCR_STOP_RECV);
	/* Clear ICSR.WAIT to exit from WAIT status. */
	mmio_write_8(IIC_ICSR, mmio_read_8(IIC_ICSR) & (uint8_t) (~ICSR_WAIT));
	/* Set ICIC.DTEE=1 to enable data transmission interrupt. */
	mmio_write_8(IIC_ICIC, mmio_read_8(IIC_ICIC) | ICIC_DTEE);

	/* Wait for a until ICSR.DTE becomes 1. */
	avs_poll(ICSR_DTE, 1U);

	/* Receive DVFS SetVID register */
	/* Clear ICIC.DTEE to disable a DTE interrupt. */
	mmio_write_8(IIC_ICIC, mmio_read_8(IIC_ICIC) & (uint8_t) (~ICIC_DTEE));
	/* Receive DVFS SetVID register */
	reg = mmio_read_8(IIC_ICDR);

	/* Wait until ICSR.BUSY is cleared. */
	avs_poll(ICSR_BUSY, 0U);

	/* Set ICCR=H'00 to disable the I2C module. */
	mmio_write_8(IIC_ICCR, 0x00U);

	return reg;
}

/*
 * Wait processing by the polling.
 */
static void avs_poll(uint8_t bit_pos, uint8_t val)
{
	uint8_t bit_val = 0U;

	if (val != 0U)
		bit_val = bit_pos;

	while (1) {
		if ((mmio_read_8(IIC_ICSR) & bit_pos) == bit_val)
			break;
	}
}
#endif /* AVS_READ_PMIC_REG_ENABLE */
#endif /* PMIC_ROHM_BD9571 */
#endif /* (AVS_SETTING_ENABLE==1) */
