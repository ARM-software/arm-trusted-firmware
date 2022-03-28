/*
 * Copyright 2022 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include <mmio.h>
#include <soc_default_base_addr.h>

void erratum_a009660(void)
{
	mmio_write_32(NXP_SCFG_ADDR + 0x20c, 0x63b20042);
}
