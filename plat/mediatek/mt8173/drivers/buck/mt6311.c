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
#include <delay_timer.h>
#include <mmio.h>
#include <mt8173_def.h>

#define I2C1_BASE		(I2C_BASE + 0x1000)
#define I2C1_DATA_PORT		(I2C1_BASE + 0x00)
#define I2C1_SLAVE_ADDR		(I2C1_BASE + 0x04)
#define I2C1_INTR_MASK		(I2C1_BASE + 0x08)
#define I2C1_INTR_STAT		(I2C1_BASE + 0x0C)
#define I2C1_CONTROL		(I2C1_BASE + 0x10)
#define I2C1_TRANSFER_LEN	(I2C1_BASE + 0x14)
#define I2C1_TIMING		(I2C1_BASE + 0x20)
#define I2C1_START		(I2C1_BASE + 0x24)
#define I2C_MASTER_WRITE	(0U)
#define I2C_MASK_TRANSAC_COMP	(1U)
#define I2C_START		(1U)
#define I2C_ALLOW_ALL_INTR	(0xFF)

#define PERI_GLOBALCON_PDN0_SET	(0x10003008)
#define PERI_GLOBALCON_PDN0_CLR	(0x10003010)
#define I2C1_PDN		(1U << 24)

void mt6311_control(unsigned int enable)
{
	unsigned int val;
	const unsigned int slave_addr = 0x6b;
	const unsigned int data_port = 0x8a;

	/* i2c1 clock on */
	mmio_write_32(PERI_GLOBALCON_PDN0_CLR, I2C1_PDN);
	mmio_write_32(I2C1_TRANSFER_LEN, 2);
	mmio_write_32(I2C1_TIMING, 1);
	mmio_write_32(I2C1_SLAVE_ADDR, (slave_addr << 1) | I2C_MASTER_WRITE);
	mmio_write_32(I2C1_INTR_MASK, I2C_ALLOW_ALL_INTR);
	mmio_write_32(I2C1_DATA_PORT, data_port);
	mmio_write_32(I2C1_DATA_PORT, enable);
	mmio_write_32(I2C1_START, I2C_START);

	while (!(val = mmio_read_32(I2C1_INTR_STAT) & I2C_MASK_TRANSAC_COMP))
		;
	mmio_write_32(I2C1_INTR_STAT, val);

	/* i2c1 clock off */
	mmio_setbits_32(PERI_GLOBALCON_PDN0_SET, I2C1_PDN);

	/* Add 1ms delay for powering on mt6311 */
	if (enable)
		mdelay(1);
}
