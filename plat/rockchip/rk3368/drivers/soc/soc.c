/*
 * Copyright (C) 2016, Fuzhou Rockchip Electronics Co., Ltd.
 * All rights reserved.
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
#include <debug.h>
#include <mmio.h>
#include <platform_def.h>
#include <rk3368_def.h>
#include <soc.h>

/* The RockChip power domain tree descriptor */
const unsigned char rockchip_power_domain_tree_desc[] = {
	/* No of root nodes */
	PLATFORM_SYSTEM_COUNT,
	/* No of children for the root node */
	PLATFORM_CLUSTER_COUNT,
	/* No of children for the first cluster node */
	PLATFORM_CLUSTER0_CORE_COUNT,
	/* No of children for the second cluster node */
	PLATFORM_CLUSTER1_CORE_COUNT
};

void secure_timer_init(void)
{
	mmio_write_32(STIMER1_BASE + TIMER_LOADE_COUNT0, 0xffffffff);
	mmio_write_32(STIMER1_BASE + TIMER_LOADE_COUNT1, 0xffffffff);

	/* auto reload & enable the timer */
	mmio_write_32(STIMER1_BASE + TIMER_CONTROL_REG, TIMER_EN);
}

void sgrf_init(void)
{
	/* setting all configurable ip into no-secure */
	mmio_write_32(SGRF_BASE + SGRF_SOC_CON(5), SGRF_SOC_CON_NS);
	mmio_write_32(SGRF_BASE + SGRF_SOC_CON(6), SGRF_SOC_CON7_BITS);
	mmio_write_32(SGRF_BASE + SGRF_SOC_CON(7), SGRF_SOC_CON_NS);

	/* secure dma to no sesure */
	mmio_write_32(SGRF_BASE + SGRF_BUSDMAC_CON(0), SGRF_BUSDMAC_CON0_NS);
	mmio_write_32(SGRF_BASE + SGRF_BUSDMAC_CON(1), SGRF_BUSDMAC_CON1_NS);
	dsb();

	/* rst dma1 */
	mmio_write_32(CRU_BASE + CRU_SOFTRSTS_CON(1),
		      RST_DMA1_MSK | (RST_DMA1_MSK << 16));
	/* rst dma2 */
	mmio_write_32(CRU_BASE + CRU_SOFTRSTS_CON(4),
		      RST_DMA2_MSK | (RST_DMA2_MSK << 16));

	dsb();

	/* release dma1 rst*/
	mmio_write_32(CRU_BASE + CRU_SOFTRSTS_CON(1), (RST_DMA1_MSK << 16));
	/* release dma2 rst*/
	mmio_write_32(CRU_BASE + CRU_SOFTRSTS_CON(4), (RST_DMA2_MSK << 16));
}

void platform_soc_init(void)
{
	secure_timer_init();
	sgrf_init();
}
