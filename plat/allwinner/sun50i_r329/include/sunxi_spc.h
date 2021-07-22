/*
 * Copyright (c) 2021 Sipeed
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SUNXI_SPC_H
#define SUNXI_SPC_H

/* Get by REing stock ATF and checking initialization loop boundary */
#define SUNXI_SPC_NUM_PORTS		11

#define SUNXI_SPC_DECPORT_STA_REG(p)	(SUNXI_SPC_BASE + 0x0000 + 0x10 * (p))
#define SUNXI_SPC_DECPORT_SET_REG(p)	(SUNXI_SPC_BASE + 0x0004 + 0x10 * (p))
#define SUNXI_SPC_DECPORT_CLR_REG(p)	(SUNXI_SPC_BASE + 0x0008 + 0x10 * (p))

#endif /* SUNXI_SPC_H */
