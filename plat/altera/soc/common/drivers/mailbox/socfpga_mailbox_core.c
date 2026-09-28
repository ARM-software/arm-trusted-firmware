/*
 * Copyright (c) 2024-2026, Intel Corporation. All rights reserved.
 * Copyright (c) 2024-2026, Altera Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Common mailbox core dispatcher.
 *
 * This file is backend-agnostic: it holds the active backend pointer and the
 * mailbox_core_init() selector.  The actual MCSR / SCSR implementations live
 * in the mcsr/ and scsr/ subfolders.  The backend built for the current
 * platform provides mailbox_backend_get(), so this dispatcher never has to
 * name a backend that is not compiled in.
 */

#include <assert.h>

#include <common/debug.h>

#include "mailbox/socfpga_mailbox_core.h"
#include <platform_def.h>

struct mailbox_backend *mailbox = MAILBOX_BACKEND_NONE;

/*
 * Public core API
 */
int mailbox_core_init(enum mailbox_backend_type type)
{
	mailbox = mailbox_backend_get(type);

	assert(mailbox != MAILBOX_BACKEND_NONE);
	return mailbox->init();
}
