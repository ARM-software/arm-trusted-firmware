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

#include <arch.h>
#include <assert.h>
#include <cci.h>
#include <debug.h>
#include <mmio.h>
#include <stdint.h>

static uintptr_t g_cci_base;
static unsigned int g_max_master_id;
static const int *g_cci_slave_if_map;

#if DEBUG
static int validate_cci_map(const int *map)
{
	unsigned int valid_cci_map = 0;
	int slave_if_id;
	int i;

	/* Validate the map */
	for (i = 0; i <= g_max_master_id; i++) {
		slave_if_id = map[i];

		if (slave_if_id < 0)
			continue;

		if (slave_if_id >= CCI_SLAVE_INTERFACE_COUNT) {
			tf_printf("Slave interface ID is invalid\n");
			return 0;
		}

		if (valid_cci_map & (1 << slave_if_id)) {
			tf_printf("Multiple masters are assigned same"
						" slave interface ID\n");
			return 0;
		}
		valid_cci_map |= 1 << slave_if_id;
	}

	if (!valid_cci_map) {
		tf_printf("No master is assigned a valid slave interface\n");
		return 0;
	}

	return 1;
}
#endif /* DEBUG */

void cci_init(uintptr_t cci_base,
		const int *map,
		unsigned int num_cci_masters)
{
	assert(map);
	assert(cci_base);

	g_cci_base = cci_base;

	/*
	 * Master Id's are assigned from zero, So in an array of size n
	 * the max master id is (n - 1).
	 */
	g_max_master_id = num_cci_masters - 1;

	assert(validate_cci_map(map));
	g_cci_slave_if_map = map;
}

void cci_enable_snoop_dvm_reqs(unsigned int master_id)
{
	int slave_if_id;

	assert(g_cci_base);
	assert(master_id <= g_max_master_id);

	slave_if_id = g_cci_slave_if_map[master_id];
	assert((slave_if_id < CCI_SLAVE_INTERFACE_COUNT) && (slave_if_id >= 0));

	/*
	 * Enable Snoops and DVM messages, no need for Read/Modify/Write as
	 * rest of bits are write ignore
	 */
	mmio_write_32(g_cci_base +
		      SLAVE_IFACE_OFFSET(slave_if_id) +
		      SNOOP_CTRL_REG, DVM_EN_BIT | SNOOP_EN_BIT);

	/* Wait for the dust to settle down */
	while (mmio_read_32(g_cci_base + STATUS_REG) & CHANGE_PENDING_BIT)
		;
}

void cci_disable_snoop_dvm_reqs(unsigned int master_id)
{
	int slave_if_id;

	assert(g_cci_base);
	assert(master_id <= g_max_master_id);

	slave_if_id = g_cci_slave_if_map[master_id];
	assert((slave_if_id < CCI_SLAVE_INTERFACE_COUNT) && (slave_if_id >= 0));

	/*
	 * Disable Snoops and DVM messages, no need for Read/Modify/Write as
	 * rest of bits are write ignore.
	 */
	mmio_write_32(g_cci_base +
		      SLAVE_IFACE_OFFSET(slave_if_id) +
		      SNOOP_CTRL_REG, ~(DVM_EN_BIT | SNOOP_EN_BIT));

	/* Wait for the dust to settle down */
	while (mmio_read_32(g_cci_base + STATUS_REG) & CHANGE_PENDING_BIT)
		;
}

