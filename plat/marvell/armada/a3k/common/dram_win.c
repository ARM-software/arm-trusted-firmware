/*
 * Copyright (C) 2018-2021 Marvell International Ltd.
 *
 * SPDX-License-Identifier:	BSD-3-Clause
 * https://spdx.org/licenses
 */

#include <string.h>

#include <lib/mmio.h>

#include <dram_win.h>
#include <marvell_plat_priv.h>
#include <mvebu.h>
#include <plat_marvell.h>

/* Armada 3700 has 5 configurable windows */
#define MV_CPU_WIN_NUM		5

#define CPU_WIN_DISABLED	0
#define CPU_WIN_ENABLED		1

/*
 * There are 2 different cpu decode window configuration cases:
 * - DRAM size is not over 2GB;
 * - DRAM size is 4GB.
 */
enum cpu_win_config_num {
	CPU_WIN_CONFIG_DRAM_NOT_OVER_2GB = 0,
	CPU_WIN_CONFIG_DRAM_4GB,
	CPU_WIN_CONFIG_MAX
};

enum cpu_win_target {
	CPU_WIN_TARGET_DRAM = 0,
	CPU_WIN_TARGET_INTERNAL_REG,
	CPU_WIN_TARGET_PCIE,
	CPU_WIN_TARGET_PCIE_OVER_MCI,
	CPU_WIN_TARGET_BOOT_ROM,
	CPU_WIN_TARGET_MCI_EXTERNAL,
	CPU_WIN_TARGET_RWTM_RAM = 7,
	CPU_WIN_TARGET_CCI400_REG
};

struct cpu_win_configuration {
	uint32_t		enabled;
	enum cpu_win_target	target;
	uint64_t		base_addr;
	uint64_t		size;
	uint64_t		remap_addr;
};

struct cpu_win_configuration mv_cpu_wins[CPU_WIN_CONFIG_MAX][MV_CPU_WIN_NUM] = {
	/*
	 * When total dram size is not over 2GB:
	 * DDR window 0 is configured in tim header, its size may be not 512MB,
	 * but the actual dram size, no need to configure it again;
	 * other cpu windows are kept as default.
	 */
	{
		/* enabled
		 *	target
		 *		base
		 *			size
		 *				remap
		 */
		{CPU_WIN_ENABLED,
			CPU_WIN_TARGET_DRAM,
				0x0,
					0x08000000,
						0x0},
		{CPU_WIN_ENABLED,
			CPU_WIN_TARGET_MCI_EXTERNAL,
				0xe0000000,
					0x08000000,
						0xe0000000},
		{CPU_WIN_ENABLED,
			CPU_WIN_TARGET_PCIE,
				0xe8000000,
					0x08000000,
						0xe8000000},
		{CPU_WIN_ENABLED,
			CPU_WIN_TARGET_RWTM_RAM,
				0xf0000000,
					0x00020000,
						0x1fff0000},
		{CPU_WIN_ENABLED,
			CPU_WIN_TARGET_PCIE_OVER_MCI,
				0x80000000,
					0x10000000,
						0x80000000},
	},

	/*
	 * If total DRAM size is more than 2GB, now there is only one case:
	 * 4GB of DRAM; to better utilize address space (for maximization of
	 * DRAM usage), we will use the configuration of CPU windows below:
	 *  - Internal Regs and Boot ROM windows are kept as default;
	 *  - CCI-400 is moved from its default address to another address
	 *    (this is actually done even if DRAM size is not more than 2 GB,
	 *     because the firmware is compiled with that address as a
	 *     constant);
	 *  - PCIe window is moved to another address;
	 *  - Use 4 CPU decode windows for DRAM, which cover 3.75GB DRAM;
	 *    DDR window 0 is configured in tim header with 2G B size, no need
	 *    to configure it again here;
	 *
	 *	0xFFFFFFFF ---> +-----------------------+
	 *			|	 Boot ROM	| 64 KB
	 *	0xFFF00000 ---> +-----------------------+
	 *			:			:
	 *	0xFE010000 ---> +-----------------------+
	 *			|	 CCI Regs	| 64 KB
	 *	0xFE000000 ---> +-----------------------+
	 *			:			:
	 *	0xFA000000 ---> +-----------------------+
	 *			|	 PCIE		| 128 MB
	 *	0xF2000000 ---> +-----------------------+
	 *			|	 DDR window 3	| 512 MB
	 *	0xD2000000 ---> +-----------------------+
	 *			|	 Internal Regs	| 32 MB
	 *	0xD0000000 ---> |-----------------------|
	 *			|	 DDR window 2	| 256 MB
	 *	0xC0000000 ---> |-----------------------|
	 *			|			|
	 *			|	 DDR window 1	| 1 GB
	 *			|			|
	 *	0x80000000 ---> |-----------------------|
	 *			|			|
	 *			|			|
	 *			|	 DDR window 0	| 2 GB
	 *			|			|
	 *			|			|
	 *	0x00000000 ---> +-----------------------+
	 */
	{
		/* win_id
		 *	target
		 *		base
		 *			size
		 *				remap
		 */
		{CPU_WIN_ENABLED,
			CPU_WIN_TARGET_DRAM,
				0x0,
					0x80000000,
						0x0},
		{CPU_WIN_ENABLED,
			CPU_WIN_TARGET_DRAM,
				0x80000000,
					0x40000000,
						0x80000000},
		{CPU_WIN_ENABLED,
			CPU_WIN_TARGET_DRAM,
				0xc0000000,
					0x10000000,
						0xc0000000},
		{CPU_WIN_ENABLED,
			CPU_WIN_TARGET_DRAM,
				0xd2000000,
					0x20000000,
						0xd2000000},
		{CPU_WIN_ENABLED,
			CPU_WIN_TARGET_PCIE,
				0xf2000000,
					0x08000000,
						0xf2000000},
	},
};

/*
 * dram_win_map_build
 *
 * This function builds cpu dram windows mapping
 * which includes base address and window size by
 * reading cpu dram decode windows registers.
 *
 * @input: N/A
 *
 * @output:
 *     - win_map: cpu dram windows mapping
 *
 * @return:  N/A
 */
void dram_win_map_build(struct dram_win_map *win_map)
{
	int32_t win_id;
	struct dram_win *win;
	uint32_t base_reg, ctrl_reg, size_reg, enabled, target;

	memset(win_map, 0, sizeof(struct dram_win_map));
	for (win_id = 0; win_id < DRAM_WIN_MAP_NUM_MAX; win_id++) {
		ctrl_reg = mmio_read_32(CPU_DEC_WIN_CTRL_REG(win_id));
		target = (ctrl_reg & CPU_DEC_CR_WIN_TARGET_MASK) >>
			  CPU_DEC_CR_WIN_TARGET_OFFS;
		enabled = ctrl_reg & CPU_DEC_CR_WIN_ENABLE;
		/* Ignore invalid and non-dram windows*/
		if ((enabled == 0) || (target != DRAM_CPU_DEC_TARGET_NUM))
			continue;

		win = win_map->dram_windows + win_map->dram_win_num;
		base_reg = mmio_read_32(CPU_DEC_WIN_BASE_REG(win_id));
		size_reg = mmio_read_32(CPU_DEC_WIN_SIZE_REG(win_id));
		/* Base reg [15:0] corresponds to transaction address [39:16] */
		win->base_addr = (base_reg & CPU_DEC_BR_BASE_MASK) >>
				  CPU_DEC_BR_BASE_OFFS;
		win->base_addr *= CPU_DEC_CR_WIN_SIZE_ALIGNMENT;
		/*
		 * Size reg [15:0] is programmed from LSB to MSB as a sequence
		 * of 1s followed by a sequence of 0s and the number of 1s
		 * specifies the size of the window in 64 KB granularity,
		 * for example, a value of 00FFh specifies 256 x 64 KB = 16 MB
		 */
		win->win_size = (size_reg & CPU_DEC_CR_WIN_SIZE_MASK) >>
				 CPU_DEC_CR_WIN_SIZE_OFFS;
		win->win_size = (win->win_size + 1) *
				 CPU_DEC_CR_WIN_SIZE_ALIGNMENT;

		win_map->dram_win_num++;
	}
}

static void cpu_win_set(uint32_t win_id, struct cpu_win_configuration *win_cfg)
{
	uint32_t base_reg, ctrl_reg, size_reg, remap_reg;

	/* Disable window */
	ctrl_reg = mmio_read_32(CPU_DEC_WIN_CTRL_REG(win_id));
	ctrl_reg &= ~CPU_DEC_CR_WIN_ENABLE;
	mmio_write_32(CPU_DEC_WIN_CTRL_REG(win_id), ctrl_reg);

	/* For an disabled window, only disable it. */
	if (!win_cfg->enabled)
		return;

	/* Set Base Register */
	base_reg = (uint32_t)(win_cfg->base_addr /
		   CPU_DEC_CR_WIN_SIZE_ALIGNMENT);
	base_reg <<= CPU_DEC_BR_BASE_OFFS;
	base_reg &= CPU_DEC_BR_BASE_MASK;
	mmio_write_32(CPU_DEC_WIN_BASE_REG(win_id), base_reg);

	/* Set Remap Register with the same value
	 * as the <Base> field in Base Register
	 */
	remap_reg = (uint32_t)(win_cfg->remap_addr /
		    CPU_DEC_CR_WIN_SIZE_ALIGNMENT);
	remap_reg <<= CPU_DEC_RLR_REMAP_LOW_OFFS;
	remap_reg &= CPU_DEC_RLR_REMAP_LOW_MASK;
	mmio_write_32(CPU_DEC_REMAP_LOW_REG(win_id), remap_reg);

	/* Set Size Register */
	size_reg = (win_cfg->size / CPU_DEC_CR_WIN_SIZE_ALIGNMENT) - 1;
	size_reg <<= CPU_DEC_CR_WIN_SIZE_OFFS;
	size_reg &= CPU_DEC_CR_WIN_SIZE_MASK;
	mmio_write_32(CPU_DEC_WIN_SIZE_REG(win_id), size_reg);

	/* Set Control Register - set target id and enable window */
	ctrl_reg &= ~CPU_DEC_CR_WIN_TARGET_MASK;
	ctrl_reg |= (win_cfg->target << CPU_DEC_CR_WIN_TARGET_OFFS);
	ctrl_reg |= CPU_DEC_CR_WIN_ENABLE;
	mmio_write_32(CPU_DEC_WIN_CTRL_REG(win_id), ctrl_reg);
}

void cpu_wins_init(void)
{
	uint32_t cfg_idx, win_id;

	if (mvebu_get_dram_size(MVEBU_REGS_BASE) <= _2GB_)
		cfg_idx = CPU_WIN_CONFIG_DRAM_NOT_OVER_2GB;
	else
		cfg_idx = CPU_WIN_CONFIG_DRAM_4GB;

	/* Window 0 is configured always for DRAM in tim header
	 * already, no need to configure it again here
	 */
	for (win_id = 1; win_id < MV_CPU_WIN_NUM; win_id++)
		cpu_win_set(win_id, &mv_cpu_wins[cfg_idx][win_id]);
}

