/*
 * Copyright (c) 2015, ARM Limited and Contributors. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * Neither the name of ARM nor the names of its contributors may be used
 * to endorse or promote products derived from this software without specific
 * prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <mmio.h>
#include <sp805.h>
#include <stdint.h>

/* Inline register access functions */

static inline void sp805_write_wdog_load(uintptr_t base, unsigned long value)
{
	mmio_write_32(base + SP805_WDOG_LOAD_OFF, value);
}

static inline void sp805_write_wdog_ctrl(uintptr_t base, unsigned long value)
{
	mmio_write_32(base + SP805_WDOG_CTR_OFF, value);
}

static inline void sp805_write_wdog_lock(uintptr_t base, unsigned long value)
{
	mmio_write_32(base + SP805_WDOG_LOCK_OFF, value);
}


/* Public API implementation */

void sp805_start(uintptr_t base, unsigned long ticks)
{
	sp805_write_wdog_load(base, ticks);
	sp805_write_wdog_ctrl(base, SP805_CTR_RESEN | SP805_CTR_INTEN);
	/* Lock registers access */
	sp805_write_wdog_lock(base, 0);
}

void sp805_stop(uintptr_t base)
{
	sp805_write_wdog_lock(base, WDOG_UNLOCK_KEY);
	sp805_write_wdog_ctrl(base, 0);
}

void sp805_refresh(uintptr_t base, unsigned long ticks)
{
	sp805_write_wdog_lock(base, WDOG_UNLOCK_KEY);
	sp805_write_wdog_load(base, ticks);
	sp805_write_wdog_lock(base, 0);
}
