/*
 * Copyright (c) 2017 - 2020, Broadcom
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <errno.h>
#include <stdint.h>

#include <common/debug.h>
#include <drivers/delay_timer.h>
#include <lib/mmio.h>

#include <sr_utils.h>
#include <swreg.h>

#define MIN_VOLT                760000
#define MAX_VOLT                1060000

#define BSTI_WRITE              0x1
#define BSTI_READ               0x2
#define BSTI_COMMAND_TA         0x2
#define BSTI_COMMAND_DATA       0xFF
#define BSTI_CONTROL_VAL        0x81
#define BSTI_CONTROL_BUSY       0x100
#define BSTI_TOGGLE_BIT         0x2
#define BSTI_CONFI_DONE_MASK    0xFFFFFFFD
#define BSTI_REG_DATA_MASK      0xFFFF
#define BSTI_CMD(sb, op, pa, ra, ta, data) \
	((((sb) & 0x3) << 30) | (((op) & 0x3) << 28) | \
	(((pa) & 0x1F) << 23) | (((ra) & 0x1F) << 18) | \
	(((ta) & 0x3) << 16) | (data))

#define PHY_REG0        0x0
#define PHY_REG1        0x1
#define PHY_REG4        0x4
#define PHY_REG5        0x5
#define PHY_REG6        0x6
#define PHY_REG7        0x7
#define PHY_REGC        0xc

#define IHOST_VDDC_DATA 0x560
#define DDR_CORE_DATA   0x2560
#define UPDATE_POS_EDGE(data, set)    ((data) | ((set) << 1))

/*
 * Formula for SR A2 reworked board:
 * step = ((vol/(1.4117 * 0.98)) - 500000)/3125
 * where,
 *      vol    - input voltage
 *      500000 - Reference voltage
 *      3125   - one step value
 */
#define A2_VOL_REF         500000
#define ONE_STEP_VALUE  3125
#define VOL_DIV(vol)    (((vol*10000ull)/(14117*98ull)) * 100ull)
#define STEP_VALUE(vol) \
	((((((VOL_DIV(vol)) - A2_VOL_REF) / ONE_STEP_VALUE) & 0xFF) << 8) | 4)

#define B0_VOL_REF         ((500000/100)*98)
#define B0_ONE_STEP_VALUE  3125
/*
 * Formula for SR B0 chip for IHOST12/03 and VDDC_CORE
 * step = ((vol/1.56) - (500000 * 0.98))/3125
 * where,
 *      vol    - input voltage
 *      500000 - Reference voltage
 *      3125   - one step value
 */
#define B0_VOL_DIV(vol)    (((vol)*100ull)/156)
#define B0_STEP_VALUE(vol) \
	((((((B0_VOL_DIV(vol)) - B0_VOL_REF) / B0_ONE_STEP_VALUE) \
		& 0xFF) << 8) | 4)

/*
 * Formula for SR B0 chip for DDR-CORE
 * step = ((vol/1) - (500000 * 0.98))/3125
 * where,
 *      vol    - input voltage
 *      500000 - Reference voltage
 *      3125   - one step value
 */
#define B0_DDR_VDDC_VOL_DIV(vol)    ((vol)/1)
#define B0_DDR_VDDC_STEP_VALUE(vol) \
	((((((B0_DDR_VDDC_VOL_DIV(vol)) - B0_VOL_REF) / B0_ONE_STEP_VALUE) \
		& 0xFF) << 8) | 4)

#define MAX_SWREG_CNT       8
#define MAX_ADDR_PER_SWREG  16
#define MAX_REG_ADDR        0xF
#define MIN_REG_ADDR        0x0

static const char *sw_reg_name[MAX_SWREG_CNT] = {
	"DDR_VDDC",
	"IHOST03",
	"IHOST12",
	"IHOST_ARRAY",
	"DDRIO_SLAVE",
	"VDDC_CORE",
	"VDDC1",
	"DDRIO_MASTER"
};

/* firmware values for all SWREG for 3.3V input operation */
static const uint16_t swreg_fm_data_bx[MAX_SWREG_CNT][MAX_ADDR_PER_SWREG] = {
	/* DDR logic: Power Domains independent of 12v or 3p3v */
	{0x25E0, 0x2D54, 0x0EC6, 0x01EC, 0x28BB, 0x1144, 0x0200, 0x69C0,
	 0x0010, 0x0EDF, 0x90D7, 0x8000, 0x820C, 0x0003, 0x0001, 0x0000},

	/* ihost03, 3p3V */
	{0x05E0, 0x39E5, 0x03C1, 0x007C, 0x8BA9, 0x4444, 0x3300, 0x6B80,
	 0x003F, 0x0FFF, 0x90D7, 0x8000, 0x240C, 0x0003, 0x0001, 0x0000},

	/* ihost12 3p3v */
	{0x05E0, 0x39E5, 0x03C1, 0x007C, 0x8BA9, 0x4444, 0x3300, 0x6B80,
	 0x003F, 0x0FFF, 0x90D7, 0x8000, 0x240C, 0x0003, 0x0001, 0x0000},

	/* ihost array */
	{0x25E0, 0x2D94, 0x0EC6, 0x01EC, 0x2ABB, 0x1144, 0x0340, 0x69C0,
	 0x0010, 0x0EDF, 0x90D7, 0x8000, 0x860C, 0x0003, 0x0001, 0x0000},

	/* ddr io slave : 3p3v */
	{0x0560, 0x4438, 0x0000, 0x001F, 0x8028, 0x4444, 0x0300, 0x4380,
	 0x003F, 0x0FFF, 0x10D7, 0x8000, 0xA70C, 0x0003, 0x0001, 0x0000},

	/* core master 3p3v */
	{0x05E0, 0x39E5, 0x03C1, 0x007C, 0x8BA9, 0x4444, 0x3300, 0x6B80,
	 0x003F, 0x0FFF, 0x90D7, 0x8000, 0x240C, 0x0003, 0x0001, 0x0000},

	/* core slave 3p3v */
	{0x0560, 0x4438, 0x0000, 0x001F, 0x8028, 0x4444, 0x0300, 0x4380,
	 0x003F, 0x0FFF, 0x10D7, 0x8000, 0x240C, 0x0003, 0x0001, 0x0000},

	/* ddr io master : 3p3v */
	{0x05E0, 0x39E5, 0x03C1, 0x007C, 0x8BA9, 0x4444, 0x3300, 0x6B80,
	 0x003F, 0x0FFF, 0x90D7, 0x8000, 0xA70C, 0x0003, 0x0001, 0x0000},
};

#define FM_DATA swreg_fm_data_bx

static int swreg_poll(void)
{
	uint32_t data;
	int retry = 100;

	do {
		data = mmio_read_32(BSTI_CONTROL_OFFSET);
		if ((data & BSTI_CONTROL_BUSY) != BSTI_CONTROL_BUSY)
			return 0;
		retry--;
		udelay(1);
	} while (retry > 0);

	return -ETIMEDOUT;
}

static int write_swreg_config(enum sw_reg reg_id, uint32_t addr, uint32_t data)
{
	uint32_t cmd;
	int ret;

	cmd = BSTI_CMD(0x1, BSTI_WRITE, reg_id, addr, BSTI_COMMAND_TA, data);
	mmio_write_32(BSTI_CONTROL_OFFSET, BSTI_CONTROL_VAL);
	mmio_write_32(BSTI_COMMAND_OFFSET, cmd);
	ret = swreg_poll();
	if (ret) {
		ERROR("Failed to write swreg %s addr 0x%x\n",
			sw_reg_name[reg_id-1], addr);
		return ret;
	}
	return ret;
}

static int read_swreg_config(enum sw_reg reg_id, uint32_t addr, uint32_t *data)
{
	uint32_t cmd;
	int ret;

	cmd = BSTI_CMD(0x1, BSTI_READ, reg_id, addr, BSTI_COMMAND_TA, PHY_REG0);
	mmio_write_32(BSTI_CONTROL_OFFSET, BSTI_CONTROL_VAL);
	mmio_write_32(BSTI_COMMAND_OFFSET, cmd);
	ret = swreg_poll();
	if (ret) {
		ERROR("Failed to read swreg %s addr 0x%x\n",
			sw_reg_name[reg_id-1], addr);
		return ret;
	}

	*data = mmio_read_32(BSTI_COMMAND_OFFSET);
	*data &= BSTI_REG_DATA_MASK;
	return ret;
}

static int swreg_config_done(enum sw_reg reg_id)
{
	uint32_t read_data;
	int ret;

	ret = read_swreg_config(reg_id, PHY_REG0, &read_data);
	if (ret)
		return ret;

	read_data &= BSTI_CONFI_DONE_MASK;
	read_data |= BSTI_TOGGLE_BIT;
	ret = write_swreg_config(reg_id, PHY_REG0, read_data);
	if (ret)
		return ret;

	ret = read_swreg_config(reg_id, PHY_REG0, &read_data);
	if (ret)
		return ret;

	read_data &= BSTI_CONFI_DONE_MASK;
	ret = write_swreg_config(reg_id, PHY_REG0, read_data);
	if (ret)
		return ret;

	return ret;
}

#ifdef DUMP_SWREG
static void dump_swreg_firmware(void)
{
	enum sw_reg reg_id;
	uint32_t data;
	int addr;
	int ret;

	for (reg_id = DDR_VDDC; reg_id <= DDRIO_MASTER; reg_id++) {
		INFO("SWREG: %s\n", sw_reg_name[reg_id - 1]);
		for (addr = MIN_REG_ADDR; addr <= MAX_REG_ADDR; addr++) {
			ret = read_swreg_config(reg_id, addr, &data);
			if (ret)
				ERROR("Failed to read offset %d\n", addr);
			INFO("\t0x%x: 0x%04x\n", addr, data);
		}
	}
}
#endif

int set_swreg(enum sw_reg reg_id, uint32_t micro_volts)
{
	uint32_t step, programmed_step;
	uint32_t data = IHOST_VDDC_DATA;
	int ret;

	if ((micro_volts > MAX_VOLT) || (micro_volts < MIN_VOLT)) {
		ERROR("input voltage out-of-range\n");
		ret = -EINVAL;
		goto failed;
	}

	ret = read_swreg_config(reg_id, PHY_REGC, &programmed_step);
	if (ret)
		goto failed;

	if (reg_id == DDR_VDDC)
		step = B0_DDR_VDDC_STEP_VALUE(micro_volts);
	else
		step = B0_STEP_VALUE(micro_volts);

	if ((step >> 8) != (programmed_step >> 8)) {
		ret = write_swreg_config(reg_id, PHY_REGC, step);
		if (ret)
			goto failed;

		if (reg_id == DDR_VDDC)
			data = DDR_CORE_DATA;

		ret = write_swreg_config(reg_id, PHY_REG0,
					UPDATE_POS_EDGE(data, 1));
		if (ret)
			goto failed;

		ret = write_swreg_config(reg_id, PHY_REG0,
					UPDATE_POS_EDGE(data, 0));
		if (ret)
			goto failed;
	}

	INFO("%s voltage updated to %duV\n", sw_reg_name[reg_id-1],
		micro_volts);
	return ret;

failed:
	/*
	 * Stop booting if voltages are not set
	 * correctly. Booting will fail at random point
	 * if we continue with wrong voltage settings.
	 */
	ERROR("Failed to set %s voltage to %duV\n", sw_reg_name[reg_id-1],
		micro_volts);
	assert(0);

	return ret;
}

/* Update SWREG firmware for all power doman for A2 chip */
int swreg_firmware_update(void)
{
	enum sw_reg reg_id;
	uint32_t data;
	int addr;
	int ret;

	/* write firmware values */
	for (reg_id = DDR_VDDC; reg_id <= DDRIO_MASTER; reg_id++) {
		/* write higher location first */
		for (addr = MAX_REG_ADDR; addr >= MIN_REG_ADDR; addr--) {
			ret = write_swreg_config(reg_id, addr,
						 FM_DATA[reg_id - 1][addr]);
			if (ret)
				goto exit;
		}
	}

	/* trigger SWREG firmware update */
	for (reg_id = DDR_VDDC; reg_id <= DDRIO_MASTER; reg_id++) {
		/*
		 * Slave regulator doesn't have to be updated,
		 * Updating Master is enough
		 */
		if ((reg_id == DDRIO_SLAVE) || (reg_id == VDDC1))
			continue;

		ret = swreg_config_done(reg_id);
		if (ret) {
			ERROR("Failed to trigger SWREG firmware update for %s\n"
				, sw_reg_name[reg_id-1]);
			return ret;
		}
	}

	for (reg_id = DDR_VDDC; reg_id <= DDRIO_MASTER; reg_id++) {
		/*
		 * IHOST_ARRAY will be used on some boards like STRATUS and
		 * there will not be any issue even if it is updated on other
		 * boards where it is not used.
		 */
		if (reg_id == IHOST_ARRAY)
			continue;

		for (addr = MIN_REG_ADDR; addr <= MAX_REG_ADDR; addr++) {
			ret = read_swreg_config(reg_id, addr, &data);
			if (ret || (!ret &&
				(data != FM_DATA[reg_id - 1][addr]))) {
				ERROR("swreg fm update failed: %s at off %d\n",
					sw_reg_name[reg_id - 1], addr);
				ERROR("Read val: 0x%x, expected val: 0x%x\n",
					data, FM_DATA[reg_id - 1][addr]);
				return -1;
			}
		}
	}

	INFO("Updated SWREG firmware\n");

#ifdef DUMP_SWREG
	dump_swreg_firmware();
#endif
	return ret;

exit:
	/*
	 * Stop booting if swreg firmware update fails.
	 * Booting will fail at random point if we
	 * continue with wrong voltage settings.
	 */
	ERROR("Failed to update firmware for %s SWREG\n",
		sw_reg_name[reg_id-1]);
	assert(0);

	return ret;
}
