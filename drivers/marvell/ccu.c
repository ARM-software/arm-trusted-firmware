/*
 * Copyright (C) 2018 Marvell International Ltd.
 *
 * SPDX-License-Identifier:     BSD-3-Clause
 * https://spdx.org/licenses
 */

/* CCU unit device driver for Marvell AP807, AP807 and AP810 SoCs */

#include <common/debug.h>
#include <drivers/marvell/ccu.h>
#include <lib/mmio.h>

#include <armada_common.h>
#include <mvebu.h>
#include <mvebu_def.h>

#if LOG_LEVEL >= LOG_LEVEL_INFO
#define DEBUG_ADDR_MAP
#endif

/* common defines */
#define WIN_ENABLE_BIT			(0x1)
/* Physical address of the base of the window = {AddrLow[19:0],20'h0} */
#define ADDRESS_SHIFT			(20 - 4)
#define ADDRESS_MASK			(0xFFFFFFF0)
#define CCU_WIN_ALIGNMENT		(0x100000)

#define IS_DRAM_TARGET(tgt)		((((tgt) == DRAM_0_TID) || \
					((tgt) == DRAM_1_TID) || \
					((tgt) == RAR_TID)) ? 1 : 0)

/* For storage of CR, SCR, ALR, AHR abd GCR */
static uint32_t ccu_regs_save[MVEBU_CCU_MAX_WINS * 4 + 1];

#ifdef DEBUG_ADDR_MAP
static void dump_ccu(int ap_index)
{
	uint32_t win_id, win_cr, alr, ahr;
	uint8_t target_id;
	uint64_t start, end;

	/* Dump all AP windows */
	printf("\tbank  target     start              end\n");
	printf("\t----------------------------------------------------\n");
	for (win_id = 0; win_id < MVEBU_CCU_MAX_WINS; win_id++) {
		win_cr = mmio_read_32(CCU_WIN_CR_OFFSET(ap_index, win_id));
		if (win_cr & WIN_ENABLE_BIT) {
			target_id = (win_cr >> CCU_TARGET_ID_OFFSET) &
				     CCU_TARGET_ID_MASK;
			alr = mmio_read_32(CCU_WIN_ALR_OFFSET(ap_index,
							      win_id));
			ahr = mmio_read_32(CCU_WIN_AHR_OFFSET(ap_index,
							      win_id));
			start = ((uint64_t)alr << ADDRESS_SHIFT);
			end = (((uint64_t)ahr + 0x10) << ADDRESS_SHIFT);
			printf("\tccu    %02x     0x%016llx 0x%016llx\n",
			       target_id, start, end);
		}
	}
	win_cr = mmio_read_32(CCU_WIN_GCR_OFFSET(ap_index));
	target_id = (win_cr >> CCU_GCR_TARGET_OFFSET) & CCU_GCR_TARGET_MASK;
	printf("\tccu   GCR %d - all other transactions\n", target_id);
}
#endif

void ccu_win_check(struct addr_map_win *win)
{
	/* check if address is aligned to 1M */
	if (IS_NOT_ALIGN(win->base_addr, CCU_WIN_ALIGNMENT)) {
		win->base_addr = ALIGN_UP(win->base_addr, CCU_WIN_ALIGNMENT);
		NOTICE("%s: Align up the base address to 0x%llx\n",
		       __func__, win->base_addr);
	}

	/* size parameter validity check */
	if (IS_NOT_ALIGN(win->win_size, CCU_WIN_ALIGNMENT)) {
		win->win_size = ALIGN_UP(win->win_size, CCU_WIN_ALIGNMENT);
		NOTICE("%s: Aligning size to 0x%llx\n",
		       __func__, win->win_size);
	}
}

void ccu_enable_win(int ap_index, struct addr_map_win *win, uint32_t win_id)
{
	uint32_t ccu_win_reg;
	uint32_t alr, ahr;
	uint64_t end_addr;

	if ((win_id == 0) || (win_id > MVEBU_CCU_MAX_WINS)) {
		ERROR("Enabling wrong CCU window %d!\n", win_id);
		return;
	}

	end_addr = (win->base_addr + win->win_size - 1);
	alr = (uint32_t)((win->base_addr >> ADDRESS_SHIFT) & ADDRESS_MASK);
	ahr = (uint32_t)((end_addr >> ADDRESS_SHIFT) & ADDRESS_MASK);

	mmio_write_32(CCU_WIN_ALR_OFFSET(ap_index, win_id), alr);
	mmio_write_32(CCU_WIN_AHR_OFFSET(ap_index, win_id), ahr);

	ccu_win_reg = WIN_ENABLE_BIT;
	ccu_win_reg |= (win->target_id & CCU_TARGET_ID_MASK)
			<< CCU_TARGET_ID_OFFSET;
	mmio_write_32(CCU_WIN_CR_OFFSET(ap_index, win_id), ccu_win_reg);
}

static void ccu_disable_win(int ap_index, uint32_t win_id)
{
	uint32_t win_reg;

	if ((win_id == 0) || (win_id > MVEBU_CCU_MAX_WINS)) {
		ERROR("Disabling wrong CCU window %d!\n", win_id);
		return;
	}

	win_reg = mmio_read_32(CCU_WIN_CR_OFFSET(ap_index, win_id));
	win_reg &= ~WIN_ENABLE_BIT;
	mmio_write_32(CCU_WIN_CR_OFFSET(ap_index, win_id), win_reg);
}

/* Insert/Remove temporary window for using the out-of reset default
 * CPx base address to access the CP configuration space prior to
 * the further base address update in accordance with address mapping
 * design.
 *
 * NOTE: Use the same window array for insertion and removal of
 *       temporary windows.
 */
void ccu_temp_win_insert(int ap_index, struct addr_map_win *win, int size)
{
	uint32_t win_id;

	for (int i = 0; i < size; i++) {
		win_id = MVEBU_CCU_MAX_WINS - 1 - i;
		ccu_win_check(win);
		ccu_enable_win(ap_index, win, win_id);
		win++;
	}
}

/*
 * NOTE: Use the same window array for insertion and removal of
 *       temporary windows.
 */
void ccu_temp_win_remove(int ap_index, struct addr_map_win *win, int size)
{
	uint32_t win_id;

	for (int i = 0; i < size; i++) {
		uint64_t base;
		uint32_t target;

		win_id = MVEBU_CCU_MAX_WINS - 1 - i;

		target = mmio_read_32(CCU_WIN_CR_OFFSET(ap_index, win_id));
		target >>= CCU_TARGET_ID_OFFSET;
		target &= CCU_TARGET_ID_MASK;

		base = mmio_read_32(CCU_WIN_ALR_OFFSET(ap_index, win_id));
		base <<= ADDRESS_SHIFT;

		if ((win->target_id != target) || (win->base_addr != base)) {
			ERROR("%s: Trying to remove bad window-%d!\n",
			      __func__, win_id);
			continue;
		}
		ccu_disable_win(ap_index, win_id);
		win++;
	}
}

/* Returns current DRAM window target (DRAM_0_TID, DRAM_1_TID, RAR_TID)
 * NOTE: Call only once for each AP.
 * The AP0 DRAM window is located at index 2 only at the BL31 execution start.
 * Then it relocated to index 1 for matching the rest of APs DRAM settings.
 * Calling this function after relocation will produce wrong results on AP0
 */
static uint32_t ccu_dram_target_get(int ap_index)
{
	/* On BLE stage the AP0 DRAM window is opened by the BootROM at index 2.
	 * All the rest of detected APs will use window at index 1.
	 * The AP0 DRAM window is moved from index 2 to 1 during
	 * init_ccu() execution.
	 */
	const uint32_t win_id = (ap_index == 0) ? 2 : 1;
	uint32_t target;

	target = mmio_read_32(CCU_WIN_CR_OFFSET(ap_index, win_id));
	target >>= CCU_TARGET_ID_OFFSET;
	target &= CCU_TARGET_ID_MASK;

	return target;
}

void ccu_dram_target_set(int ap_index, uint32_t target)
{
	/* On BLE stage the AP0 DRAM window is opened by the BootROM at index 2.
	 * All the rest of detected APs will use window at index 1.
	 * The AP0 DRAM window is moved from index 2 to 1
	 * during init_ccu() execution.
	 */
	const uint32_t win_id = (ap_index == 0) ? 2 : 1;
	uint32_t dram_cr;

	dram_cr = mmio_read_32(CCU_WIN_CR_OFFSET(ap_index, win_id));
	dram_cr &= ~(CCU_TARGET_ID_MASK << CCU_TARGET_ID_OFFSET);
	dram_cr |= (target & CCU_TARGET_ID_MASK) << CCU_TARGET_ID_OFFSET;
	mmio_write_32(CCU_WIN_CR_OFFSET(ap_index, win_id), dram_cr);
}

/* Setup CCU DRAM window and enable it */
void ccu_dram_win_config(int ap_index, struct addr_map_win *win)
{
#if IMAGE_BLE /* BLE */
	/* On BLE stage the AP0 DRAM window is opened by the BootROM at index 2.
	 * Since the BootROM is not accessing DRAM at BLE stage,
	 * the DRAM window can be temporarely disabled.
	 */
	const uint32_t win_id = (ap_index == 0) ? 2 : 1;
#else /* end of BLE */
	/* At the ccu_init() execution stage, DRAM windows of all APs
	 * are arranged at index 1.
	 * The AP0 still has the old window BootROM DRAM at index 2, so
	 * the window-1 can be safely disabled without breaking the DRAM access.
	 */
	const uint32_t win_id = 1;
#endif

	ccu_disable_win(ap_index, win_id);
	/* enable write secure (and clear read secure) */
	mmio_write_32(CCU_WIN_SCR_OFFSET(ap_index, win_id),
		      CCU_WIN_ENA_WRITE_SECURE);
	ccu_win_check(win);
	ccu_enable_win(ap_index, win, win_id);
}

/* Save content of CCU window + GCR */
static void ccu_save_win_range(int ap_id, int win_first,
			       int win_last, uint32_t *buffer)
{
	int win_id, idx;
	/* Save CCU */
	for (idx = 0, win_id = win_first; win_id <= win_last; win_id++) {
		buffer[idx++] = mmio_read_32(CCU_WIN_CR_OFFSET(ap_id, win_id));
		buffer[idx++] = mmio_read_32(CCU_WIN_SCR_OFFSET(ap_id, win_id));
		buffer[idx++] = mmio_read_32(CCU_WIN_ALR_OFFSET(ap_id, win_id));
		buffer[idx++] = mmio_read_32(CCU_WIN_AHR_OFFSET(ap_id, win_id));
	}
	buffer[idx] = mmio_read_32(CCU_WIN_GCR_OFFSET(ap_id));
}

/* Restore content of CCU window + GCR */
static void ccu_restore_win_range(int ap_id, int win_first,
				  int win_last, uint32_t *buffer)
{
	int win_id, idx;
	/* Restore CCU */
	for (idx = 0, win_id = win_first; win_id <= win_last; win_id++) {
		mmio_write_32(CCU_WIN_CR_OFFSET(ap_id, win_id),  buffer[idx++]);
		mmio_write_32(CCU_WIN_SCR_OFFSET(ap_id, win_id), buffer[idx++]);
		mmio_write_32(CCU_WIN_ALR_OFFSET(ap_id, win_id), buffer[idx++]);
		mmio_write_32(CCU_WIN_AHR_OFFSET(ap_id, win_id), buffer[idx++]);
	}
	mmio_write_32(CCU_WIN_GCR_OFFSET(ap_id), buffer[idx]);
}

void ccu_save_win_all(int ap_id)
{
	ccu_save_win_range(ap_id, 0, MVEBU_CCU_MAX_WINS - 1, ccu_regs_save);
}

void ccu_restore_win_all(int ap_id)
{
	ccu_restore_win_range(ap_id, 0, MVEBU_CCU_MAX_WINS - 1, ccu_regs_save);
}

int init_ccu(int ap_index)
{
	struct addr_map_win *win, *dram_win;
	uint32_t win_id, win_reg;
	uint32_t win_count, array_id;
	uint32_t dram_target;
#if IMAGE_BLE
	/* In BootROM context CCU Window-1
	 * has SRAM_TID target and should not be disabled
	 */
	const uint32_t win_start = 2;
#else
	const uint32_t win_start = 1;
#endif

	INFO("Initializing CCU Address decoding\n");

	/* Get the array of the windows and fill the map data */
	marvell_get_ccu_memory_map(ap_index, &win, &win_count);
	if (win_count <= 0) {
		INFO("No windows configurations found\n");
	} else if (win_count > (MVEBU_CCU_MAX_WINS - 1)) {
		ERROR("CCU mem map array > than max available windows (%d)\n",
		      MVEBU_CCU_MAX_WINS);
		win_count = MVEBU_CCU_MAX_WINS;
	}

	/* Need to set GCR to DRAM before all CCU windows are disabled for
	 * securing the normal access to DRAM location, which the ATF is running
	 * from. Once all CCU windows are set, which have to include the
	 * dedicated DRAM window as well, the GCR can be switched to the target
	 * defined by the platform configuration.
	 */
	dram_target = ccu_dram_target_get(ap_index);
	win_reg = (dram_target & CCU_GCR_TARGET_MASK) << CCU_GCR_TARGET_OFFSET;
	mmio_write_32(CCU_WIN_GCR_OFFSET(ap_index), win_reg);

	/* If the DRAM window was already configured at the BLE stage,
	 * only the window target considered valid, the address range should be
	 * updated according to the platform configuration.
	 */
	for (dram_win = win, array_id = 0; array_id < win_count;
	     array_id++, dram_win++) {
		if (IS_DRAM_TARGET(dram_win->target_id)) {
			dram_win->target_id = dram_target;
			break;
		}
	}

	/* Disable all AP CCU windows
	 * Window-0 is always bypassed since it already contains
	 * data allowing the internal configuration space access
	 */
	for (win_id = win_start; win_id < MVEBU_CCU_MAX_WINS; win_id++) {
		ccu_disable_win(ap_index, win_id);
		/* enable write secure (and clear read secure) */
		mmio_write_32(CCU_WIN_SCR_OFFSET(ap_index, win_id),
			      CCU_WIN_ENA_WRITE_SECURE);
	}

	/* win_id is the index of the current ccu window
	 * array_id is the index of the current memory map window entry
	 */
	for (win_id = win_start, array_id = 0;
	    ((win_id < MVEBU_CCU_MAX_WINS) && (array_id < win_count));
	    win_id++) {
		ccu_win_check(win);
		ccu_enable_win(ap_index, win, win_id);
		win++;
		array_id++;
	}

	/* Get & set the default target according to board topology */
	win_reg = (marvell_get_ccu_gcr_target(ap_index) & CCU_GCR_TARGET_MASK)
		   << CCU_GCR_TARGET_OFFSET;
	mmio_write_32(CCU_WIN_GCR_OFFSET(ap_index), win_reg);

#ifdef DEBUG_ADDR_MAP
	dump_ccu(ap_index);
#endif

	INFO("Done CCU Address decoding Initializing\n");

	return 0;
}
