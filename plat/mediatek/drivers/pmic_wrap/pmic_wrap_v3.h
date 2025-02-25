/*
 * Copyright (c) 2025, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PMIC_WRAP_V3_H
#define PMIC_WRAP_V3_H

#include <stdint.h>

#include <common/debug.h>
#include <drivers/console.h>

#define PMIF_REG_RANGE			(0x774)
#define PMICSPI_MST_REG_RANGE		(0x80)
#define DEFAULT_CMD			0
#define DEFAULT_SLVID			0
#define DEFAULT_BYTECNT			0
#define PMIF_SPI_PMIFID			0
/* #define PWRAP_DEBUG 1 */

#if PWRAP_DEBUG
#define PWRAP_LOG(fmts, args...) do {\
		console_init(gteearg.atf_log_port, UART_CLOCK, UART_BAUDRATE);\
		INFO("[%s:%d] -" fmts, __func__, __LINE__, ##args);\
		console_uninit();\
	} while (0)
#endif

/**********************************************************/
#define DEFAULT_VALUE_READ_TEST		(0x5aa5)
#define PWRAP_WRITE_TEST_VALUE		(0xa55a)
#define PWRAP_POLL_STEP_US		(10)

/* timeout setting */
enum {
	TIMEOUT_RESET		= 50,	/* us */
	TIMEOUT_READ		= 50,	/* us */
	TIMEOUT_WAIT_IDLE	= 50	/* us */
};

#endif /* PMIC_WRAP_V3_H */
