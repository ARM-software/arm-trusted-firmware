/*
 * Copyright (c) 2015-2018, Renesas Electronics Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/debug.h>
#include <lib/mmio.h>

#include "dram_sub_func.h"

#define PRR				(0xFFF00044U)
#define PRR_PRODUCT_MASK		(0x00007F00U)
#define PRR_CUT_MASK			(0x000000FFU)
#define PRR_PRODUCT_H3			(0x00004F00U)	/* R-Car H3  */
#define PRR_PRODUCT_M3			(0x00005200U)	/* R-Car M3  */
#define PRR_PRODUCT_M3N			(0x00005500U)	/* R-Car M3N */
#define PRR_PRODUCT_E3			(0x00005700U)	/* R-Car E3  */
#define PRR_PRODUCT_V3H			(0x00005600U)	/* R-Car V3H */

#if RCAR_SYSTEM_SUSPEND
#include "iic_dvfs.h"

#define DRAM_BACKUP_GPIO_USE		(0)
#if PMIC_ROHM_BD9571
#define	PMIC_BKUP_MODE_CNT		(0x20U)
#define	PMIC_QLLM_CNT			(0x27U)
#define	BIT_BKUP_CTRL_OUT		((uint8_t)(1U << 4U))
#define	BIT_QLLM_DDR0_EN		((uint8_t)(1U << 0U))
#define	BIT_QLLM_DDR1_EN		((uint8_t)(1U << 1U))
#endif

#define	GPIO_OUTDT1			(0xE6051008U)
#define GPIO_INDT1			(0xE605100CU)
#define GPIO_OUTDT3			(0xE6053008U)
#define GPIO_INDT3			(0xE605300CU)
#define GPIO_OUTDT6			(0xE6055408U)
#define GPIO_INDT6			(0xE605540CU)

#if DRAM_BACKUP_GPIO_USE == 1
#define GPIO_BKUP_REQB_SHIFT_SALVATOR	(9U)	/* GP1_9 (BKUP_REQB) */
#define GPIO_BKUP_REQB_SHIFT_EBISU	(14U)	/* GP6_14(BKUP_REQB) */
#define GPIO_BKUP_REQB_SHIFT_CONDOR	(1U)	/* GP3_1 (BKUP_REQB) */
#endif
#define GPIO_BKUP_TRG_SHIFT_SALVATOR	(8U)	/* GP1_8 (BKUP_TRG) */
#define GPIO_BKUP_TRG_SHIFT_EBISU	(13U)	/* GP6_13(BKUP_TRG) */
#define GPIO_BKUP_TRG_SHIFT_CONDOR	(0U)	/* GP3_0 (BKUP_TRG) */

#define DRAM_BKUP_TRG_LOOP_CNT	(1000U)
#endif

void rcar_dram_get_boot_status(uint32_t * status)
{
#if RCAR_SYSTEM_SUSPEND
	uint32_t shift = GPIO_BKUP_TRG_SHIFT_SALVATOR;
	uint32_t gpio = GPIO_INDT1;
	uint32_t reg, product;

	product = mmio_read_32(PRR) & PRR_PRODUCT_MASK;

	if (product == PRR_PRODUCT_V3H) {
		shift = GPIO_BKUP_TRG_SHIFT_CONDOR;
		gpio = GPIO_INDT3;
	} else if (product == PRR_PRODUCT_E3) {
		shift = GPIO_BKUP_TRG_SHIFT_EBISU;
		gpio = GPIO_INDT6;
	}

	reg = mmio_read_32(gpio) & (1U << shift);
	*status = reg ? DRAM_BOOT_STATUS_WARM : DRAM_BOOT_STATUS_COLD;
#else
	*status = DRAM_BOOT_STATUS_COLD;
#endif
}

int32_t rcar_dram_update_boot_status(uint32_t status)
{
	int32_t ret = 0;
#if RCAR_SYSTEM_SUSPEND
#if PMIC_ROHM_BD9571
#if DRAM_BACKUP_GPIO_USE == 0
	uint8_t mode = 0U;
#else
	uint32_t reqb, outd;
#endif
	uint8_t qllm = 0;
#endif
	uint32_t i, product, trg, gpio;

	product = mmio_read_32(PRR) & PRR_PRODUCT_MASK;
	if (product == PRR_PRODUCT_V3H) {
#if DRAM_BACKUP_GPIO_USE == 1
		reqb = GPIO_BKUP_REQB_SHIFT_CONDOR;
		outd = GPIO_OUTDT3;
#endif
		trg = GPIO_BKUP_TRG_SHIFT_CONDOR;
		gpio = GPIO_INDT3;
	} else if (product == PRR_PRODUCT_E3) {
#if DRAM_BACKUP_GPIO_USE == 1
		reqb = GPIO_BKUP_REQB_SHIFT_EBISU;
		outd = GPIO_OUTDT6;
#endif
		trg = GPIO_BKUP_TRG_SHIFT_EBISU;
		gpio = GPIO_INDT6;
	} else {
#if DRAM_BACKUP_GPIO_USE == 1
		reqb = GPIO_BKUP_REQB_SHIFT_SALVATOR;
		outd = GPIO_OUTDT1;
#endif
		trg = GPIO_BKUP_TRG_SHIFT_SALVATOR;
		gpio = GPIO_INDT1;
	}

	if (status != DRAM_BOOT_STATUS_WARM)
		goto cold;

#if DRAM_BACKUP_GPIO_USE==1
	mmio_setbits_32(outd, 1U << reqb);
#else

#if PMIC_ROHM_BD9571
	if (rcar_iic_dvfs_receive(PMIC, PMIC_BKUP_MODE_CNT, &mode)) {
		ERROR("BKUP mode cnt READ ERROR.\n");
		return DRAM_UPDATE_STATUS_ERR;
	}

	mode &= ~BIT_BKUP_CTRL_OUT;
	if (rcar_iic_dvfs_send(PMIC, PMIC_BKUP_MODE_CNT, mode)) {
		ERROR("BKUP mode cnt WRITE ERROR. value = %d\n", mode);
		return DRAM_UPDATE_STATUS_ERR;
	}
#endif
#endif
	for (i = 0; i < DRAM_BKUP_TRG_LOOP_CNT; i++) {
		if (mmio_read_32(gpio) & (1U << trg))
			continue;

		goto cold;
	}

	ERROR("\nWarm booting Error...\n"
	      " The potential of BKUP_TRG did not switch "
	      "to Low.\n If you expect the operation of "
	      "cold boot,\n check the board configuration"
	      " (ex, Dip-SW) and/or the H/W failure.\n");

	return DRAM_UPDATE_STATUS_ERR;

cold:
#if PMIC_ROHM_BD9571
	if (ret)
		return ret;

	qllm = (BIT_QLLM_DDR0_EN | BIT_QLLM_DDR1_EN);
	if (rcar_iic_dvfs_send(PMIC, PMIC_QLLM_CNT, qllm)) {
		ERROR("QLLM cnt WRITE ERROR. value = %d\n", qllm);
		ret = DRAM_UPDATE_STATUS_ERR;
	}
#endif
#endif
	return ret;
}
