/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright 2026 Free Mobile - Vincent Jardin
 *
 * nbxv3 runtime flash bootstrap entry points.
 */

#ifndef NBXV3_FLASH_BOOTSTRAP_H
#define NBXV3_FLASH_BOOTSTRAP_H

#include <cdefs.h>

/*
 * FLASH_BOOTSTRAP path: open flash_manifest.json over ARM semihosting,
 * erase NOR, write each manifest region, signal SYS_EXIT to the host.
 * Does not return.
 */
void __dead2 nbxv3_flash_bootstrap(void);

/*
 * FLASH_NO_HOST path: bootrom RCW is in effect but no OpenOCD scratch
 * magic was seen. Wait up to 5 s for the host to attach and write the
 * magic. If it does, dispatch into nbxv3_flash_bootstrap(). If the
 * timeout fires, issue RSTCNTL.SW_RST_REQ_INIT (which on this board
 * becomes a true POR via the RESET_REQ_B/NPN/PORESET_B path) so the
 * SP retries the boot loop.
 */
void __dead2 nbxv3_wait_or_por_retry(void);

#endif /* NBXV3_FLASH_BOOTSTRAP_H */
