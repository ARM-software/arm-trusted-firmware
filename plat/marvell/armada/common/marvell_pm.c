/*
 * Copyright (C) 2018 Marvell International Ltd.
 *
 * SPDX-License-Identifier:     BSD-3-Clause
 * https://spdx.org/licenses
 */

#include <assert.h>

#include <arch_helpers.h>
#include <lib/psci/psci.h>

#include <marvell_pm.h>

/* Standard ARM platforms are expected to export plat_arm_psci_pm_ops */
extern const plat_psci_ops_t plat_arm_psci_pm_ops;

/*****************************************************************************
 * Private function to program the mailbox for a cpu before it is released
 * from reset. This function assumes that the mail box base is within
 * the MARVELL_SHARED_RAM region
 *****************************************************************************
 */
void marvell_program_mailbox(uintptr_t address)
{
	uintptr_t *mailbox = (void *)PLAT_MARVELL_MAILBOX_BASE;

	/*
	 * Ensure that the PLAT_MARVELL_MAILBOX_BASE is within
	 * MARVELL_SHARED_RAM region.
	 */
	assert((PLAT_MARVELL_MAILBOX_BASE >= MARVELL_SHARED_RAM_BASE) &&
	       ((PLAT_MARVELL_MAILBOX_BASE + sizeof(*mailbox)) <=
	       (MARVELL_SHARED_RAM_BASE + MARVELL_SHARED_RAM_SIZE)));

	mailbox[MBOX_IDX_MAGIC] = MVEBU_MAILBOX_MAGIC_NUM;
	mailbox[MBOX_IDX_SEC_ADDR] = address;

	/* Flush data cache if the mail box shared RAM is cached */
#if PLAT_MARVELL_SHARED_RAM_CACHED
	flush_dcache_range((uintptr_t)PLAT_MARVELL_MAILBOX_BASE +
			   8 * MBOX_IDX_MAGIC,
			   2 * sizeof(uint64_t));
#endif
}

/*****************************************************************************
 * The ARM Standard platform definition of platform porting API
 * `plat_setup_psci_ops`.
 *****************************************************************************
 */
int plat_setup_psci_ops(uintptr_t sec_entrypoint,
			const plat_psci_ops_t **psci_ops)
{
	*psci_ops = &plat_arm_psci_pm_ops;

	/* Setup mailbox with entry point. */
	marvell_program_mailbox(sec_entrypoint);
	return 0;
}
