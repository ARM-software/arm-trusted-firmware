/*
 * Copyright (c) 2014, ARM Limited and Contributors. All rights reserved.
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

#include <arch_helpers.h>
#include <bakery_lock.h>
#include <mmio.h>
#include "juno_def.h"
#include "mhu.h"

/* SCP MHU secure channel registers */
#define SCP_INTR_S_STAT		0x200
#define SCP_INTR_S_SET		0x208
#define SCP_INTR_S_CLEAR	0x210

/* CPU MHU secure channel registers */
#define CPU_INTR_S_STAT		0x300
#define CPU_INTR_S_SET		0x308
#define CPU_INTR_S_CLEAR	0x310


static bakery_lock_t mhu_secure_lock __attribute__ ((section("tzfw_coherent_mem")));


void mhu_secure_message_start(void)
{
	bakery_lock_get(&mhu_secure_lock);

	/* Make sure any previous command has finished */
	while (mmio_read_32(MHU_BASE + CPU_INTR_S_STAT) != 0)
		;
}

void mhu_secure_message_send(uint32_t command)
{
	/* Send command to SCP and wait for it to pick it up */
	mmio_write_32(MHU_BASE + CPU_INTR_S_SET, command);
	while (mmio_read_32(MHU_BASE + CPU_INTR_S_STAT) != 0)
		;
}

uint32_t mhu_secure_message_wait(void)
{
	/* Wait for response from SCP */
	uint32_t response;
	while (!(response = mmio_read_32(MHU_BASE + SCP_INTR_S_STAT)))
		;

	return response;
}

void mhu_secure_message_end(void)
{
	/* Clear any response we got by writing all ones to the CLEAR register */
	mmio_write_32(MHU_BASE + SCP_INTR_S_CLEAR, 0xffffffffu);

	bakery_lock_release(&mhu_secure_lock);
}

void mhu_secure_init(void)
{
	bakery_lock_init(&mhu_secure_lock);

	/*
	 * Clear the CPU's INTR register to make sure we don't see a stale
	 * or garbage value and think it's a message we've already sent.
	 */
	mmio_write_32(MHU_BASE + CPU_INTR_S_CLEAR, 0xffffffffu);
}
