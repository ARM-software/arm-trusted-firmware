/*
 * Copyright (C) 2018 Marvell International Ltd.
 *
 * SPDX-License-Identifier:     BSD-3-Clause
 * https://spdx.org/licenses
 */

#ifndef MARVELL_PM_H
#define MARVELL_PM_H

#define MVEBU_MAILBOX_MAGIC_NUM		PLAT_MARVELL_MAILBOX_MAGIC_NUM
#define MVEBU_MAILBOX_SUSPEND_STATE	0xb007de7c

/* Mailbox entry indexes */
/* Magic number for validity check */
#define	MBOX_IDX_MAGIC			0
/* Recovery from suspend entry point */
#define	MBOX_IDX_SEC_ADDR		1
/* Suspend state magic number */
#define	MBOX_IDX_SUSPEND_MAGIC		2
/* Recovery jump address for ROM bypass */
#define	MBOX_IDX_ROM_EXIT_ADDR		3
/* BLE execution start counter value */
#define	MBOX_IDX_START_CNT		4

#endif /* MARVELL_PM_H */
