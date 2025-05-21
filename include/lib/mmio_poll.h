/*
 * Copyright 2025 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MMIO_POLL_H
#define MMIO_POLL_H

#include <errno.h>
#include <drivers/delay_timer.h>
#include <lib/mmio.h>

/**
 * mmio_read_poll_timeout - Continuously check an address until a specific
 * condition is satisfied or a timeout is reached.
 * @op: The mmio_read_* operator to read the register
 * @val: The variable where the read value is stored.
 * @cond: The condition used to stop polling, which can be a macro using @val.
 * @timeout_us: Timeout in microseconds.
 * @args: Arguments to be passed to @op.
 *
 * Return: 0 if the condition @cond is evaluated to true within @timeout_us
 *         microseconds, or -ETIMEOUT in case of a timeout. In either case,
 *         the last read value will be stored in @val.
 */
#define mmio_read_poll_timeout(op, val, cond, timeout_us, args...)\
({\
	int _rv = -ETIMEDOUT; \
	uint32_t _tout_us = (timeout_us); \
	uint64_t _tout = timeout_init_us(_tout_us);\
	do {\
		(val) = (op)(args);\
		if (cond) { \
			_rv = 0;\
			break;\
		} \
	} while (!timeout_elapsed(_tout));\
	_rv;\
})

#define mmio_read_32_poll_timeout(addr, val, cond, timeout_us) \
	mmio_read_poll_timeout(&mmio_read_32, val, cond, timeout_us, addr)

#endif /* MMIO_POLL_H */
