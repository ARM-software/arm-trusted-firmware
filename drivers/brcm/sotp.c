/*
 * Copyright (c) 2016-2020, Broadcom
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <string.h>

#include <common/debug.h>
#include <lib/mmio.h>
#include <sotp.h>

#include <platform_def.h>
#include <platform_sotp.h>

#ifdef USE_SOFT_SOTP
extern uint64_t soft_sotp[];
#endif

#define SOTP_PROG_CONTROL (SOTP_REGS_OTP_BASE + 0x0000)
#define SOTP_PROG_CONTROL__OTP_CPU_MODE_EN 15
#define SOTP_PROG_CONTROL__OTP_DISABLE_ECC 9
#define SOTP_PROG_CONTROL__OTP_ECC_WREN 8

#define SOTP_WRDATA_0 (SOTP_REGS_OTP_BASE + 0x0004)
#define SOTP_WRDATA_1 (SOTP_REGS_OTP_BASE + 0x0008)

#define SOTP_ADDR (SOTP_REGS_OTP_BASE + 0x000c)
#define SOTP_ADDR__OTP_ROW_ADDR_R 6
#define SOTP_ADDR_MASK 0x3FF

#define SOTP_CTRL_0 (SOTP_REGS_OTP_BASE + 0x0010)
#define SOTP_CTRL_0__START 0
#define SOTP_CTRL_0__OTP_CMD 1

#define SOTP_STATUS_0 (SOTP_REGS_OTP_BASE + 0x0018)
#define SOTP_STATUS__FDONE 3

#define SOTP_STATUS_1 (SOTP_REGS_OTP_BASE + 0x001c)
#define SOTP_STATUS_1__CMD_DONE 1
#define SOTP_STATUS_1__ECC_DET 17

#define SOTP_RDDATA_0 (SOTP_REGS_OTP_BASE + 0x0020)
#define SOTP_RDDATA_1 (SOTP_REGS_OTP_BASE + 0x0024)

#define SOTP_READ 0

#define SOTP_PROG_WORD 10
#define SOTP_STATUS__PROGOK 2
#define SOTP_PROG_ENABLE 2

#define SOTP_ROW_DATA_MASK 0xffffffff
#define SOTP_ECC_ERR_BITS_MASK 0x1ff00000000

#define SOTP_CHIP_CTRL_SW_OVERRIDE_CHIP_STATES 4
#define SOTP_CHIP_CTRL_SW_MANU_PROG 5
#define SOTP_CHIP_CTRL_SW_CID_PROG 6
#define SOTP_CHIP_CTRL_SW_AB_DEVICE 8
#define SOTP_CHIP_CTRL_SW_AB_DEV_MODE 9
#define CHIP_STATE_UNPROGRAMMED 0x1
#define CHIP_STATE_UNASSIGNED 0x2

uint64_t sotp_mem_read(uint32_t offset, uint32_t sotp_add_ecc)
{
#ifdef USE_SOFT_SOTP
	(void)sotp_add_ecc;

	return soft_sotp[offset];
#else
	uint64_t read_data = 0;
	uint64_t read_data1 = 0;
	uint64_t read_data2 = 0;

	/* Check for FDONE status */
	while ((mmio_read_32(SOTP_STATUS_0) & BIT(SOTP_STATUS__FDONE)) !=
	       BIT(SOTP_STATUS__FDONE))
		;

	/* Enable OTP access by CPU */
	mmio_setbits_32(SOTP_PROG_CONTROL,
			BIT(SOTP_PROG_CONTROL__OTP_CPU_MODE_EN));

	if (sotp_add_ecc == 1) {
		mmio_clrbits_32(SOTP_PROG_CONTROL,
				BIT(SOTP_PROG_CONTROL__OTP_DISABLE_ECC));
	}

	if (sotp_add_ecc == 0) {
		mmio_setbits_32(SOTP_PROG_CONTROL,
				BIT(SOTP_PROG_CONTROL__OTP_DISABLE_ECC));
	}

	mmio_write_32(SOTP_ADDR,
		      ((offset & SOTP_ADDR_MASK) << SOTP_ADDR__OTP_ROW_ADDR_R));
	mmio_write_32(SOTP_CTRL_0, (SOTP_READ << SOTP_CTRL_0__OTP_CMD));

	/* Start bit to tell SOTP to send command to the OTP controller */
	mmio_setbits_32(SOTP_CTRL_0, BIT(SOTP_CTRL_0__START));

	/* Wait for SOTP command done to be set */
	while ((mmio_read_32(SOTP_STATUS_1) & BIT(SOTP_STATUS_1__CMD_DONE)) !=
	      BIT(SOTP_STATUS_1__CMD_DONE))
		;

	/* Clr Start bit after command done */
	mmio_clrbits_32(SOTP_CTRL_0, BIT(SOTP_CTRL_0__START));

	if ((offset > SOTP_DEVICE_SECURE_CFG3_ROW) &&
	    (mmio_read_32(SOTP_STATUS_1) & BIT(SOTP_STATUS_1__ECC_DET))) {
		ERROR("SOTP ECC ERROR Detected row offset %d\n", offset);
		read_data = SOTP_ECC_ERR_DETECT;
	} else {
		read_data1 = (uint64_t)mmio_read_32(SOTP_RDDATA_0);
		read_data1 = read_data1 & 0xFFFFFFFF;
		read_data2 = (uint64_t)mmio_read_32(SOTP_RDDATA_1);
		read_data2 = (read_data2 & 0x1ff) << 32;
		read_data = read_data1 | read_data2;
	}

	/* Command done is cleared */
	mmio_setbits_32(SOTP_STATUS_1, BIT(SOTP_STATUS_1__CMD_DONE));

	/* disable OTP access by CPU */
	mmio_clrbits_32(SOTP_PROG_CONTROL,
			BIT(SOTP_PROG_CONTROL__OTP_CPU_MODE_EN));

	return read_data;
#endif
}

void sotp_mem_write(uint32_t addr, uint32_t sotp_add_ecc, uint64_t wdata)
{
#ifdef USE_SOFT_SOTP
	(void)sotp_add_ecc;

	soft_sotp[addr] = wdata;
#else
	uint32_t loop;
	uint8_t prog_array[4] = { 0x0F, 0x04, 0x08, 0x0D };

	uint32_t chip_state_default =
		(CHIP_STATE_UNASSIGNED|CHIP_STATE_UNPROGRAMMED);
	uint32_t chip_state = mmio_read_32(SOTP_REGS_SOTP_CHIP_STATES);
	uint32_t chip_ctrl_default = 0;

	/*
	 * The override settings is required to allow the customer to program
	 * the application specific keys into SOTP, before the conversion to
	 * one of the AB modes.
	 * At the end of write operation, the chip ctrl settings will restored
	 * to the state prior to write call
	 */
	if (chip_state & chip_state_default) {
		uint32_t chip_ctrl;

		chip_ctrl_default = mmio_read_32(SOTP_CHIP_CTRL);
		INFO("SOTP: enable special prog mode\n");

		chip_ctrl = BIT(SOTP_CHIP_CTRL_SW_OVERRIDE_CHIP_STATES) |
			    BIT(SOTP_CHIP_CTRL_SW_MANU_PROG) |
			    BIT(SOTP_CHIP_CTRL_SW_CID_PROG) |
			    BIT(SOTP_CHIP_CTRL_SW_AB_DEVICE);
		mmio_write_32(SOTP_CHIP_CTRL, chip_ctrl);
	}

	/*  Check for FDONE status */
	while ((mmio_read_32(SOTP_STATUS_0) & BIT(SOTP_STATUS__FDONE)) !=
	       BIT(SOTP_STATUS__FDONE))
		;

	/*  Enable OTP acces by CPU */
	mmio_setbits_32(SOTP_PROG_CONTROL,
			BIT(SOTP_PROG_CONTROL__OTP_CPU_MODE_EN));

	if (addr > SOTP_DEVICE_SECURE_CFG3_ROW) {
		if (sotp_add_ecc == 0) {
			mmio_clrbits_32(SOTP_PROG_CONTROL,
					BIT(SOTP_PROG_CONTROL__OTP_ECC_WREN));
		}
		if (sotp_add_ecc == 1) {
			mmio_setbits_32(SOTP_PROG_CONTROL,
					BIT(SOTP_PROG_CONTROL__OTP_ECC_WREN));
		}
	} else {
		mmio_clrbits_32(SOTP_PROG_CONTROL,
				BIT(SOTP_PROG_CONTROL__OTP_ECC_WREN));
	}

	mmio_write_32(SOTP_CTRL_0, (SOTP_PROG_ENABLE << 1));

	/*
	 * In order to avoid unintentional writes / programming of the OTP
	 * array, the OTP Controller must be put into programming mode before
	 * it will accept program commands. This is done by writing 0xF, 0x4,
	 * 0x8, 0xD with program commands prior to starting the actual
	 * programming sequence
	 */
	for (loop = 0; loop < 4; loop++) {
		mmio_write_32(SOTP_WRDATA_0, prog_array[loop]);

		/*
		 * Start bit to tell SOTP to send command to the OTP controller
		 */
		mmio_setbits_32(SOTP_CTRL_0, BIT(SOTP_CTRL_0__START));

		/*  Wait for SOTP command done to <-- be set */
		while ((mmio_read_32(SOTP_STATUS_1) &
			BIT(SOTP_STATUS_1__CMD_DONE)) !=
		       BIT(SOTP_STATUS_1__CMD_DONE))
			;

		/* Command done is cleared w1c */
		mmio_setbits_32(SOTP_STATUS_1, BIT(SOTP_STATUS_1__CMD_DONE));

		/* Clr Start bit after command done */
		mmio_clrbits_32(SOTP_CTRL_0, BIT(SOTP_CTRL_0__START));
	}

	/*  Check for PROGOK */
	while ((mmio_read_32(SOTP_STATUS_0) & 0x4) != BIT(SOTP_STATUS__PROGOK))
		;

	/* Set  10 bit row address */
	mmio_write_32(SOTP_ADDR,
		      ((addr & SOTP_ADDR_MASK) << SOTP_ADDR__OTP_ROW_ADDR_R));

	/* Set SOTP Row data */
	mmio_write_32(SOTP_WRDATA_0, (wdata & SOTP_ROW_DATA_MASK));

	/* Set SOTP ECC and error bits */
	mmio_write_32(SOTP_WRDATA_1, ((wdata & SOTP_ECC_ERR_BITS_MASK) >> 32));

	/* Set prog_word command */
	mmio_write_32(SOTP_CTRL_0, (SOTP_PROG_WORD << 1));

	/*  Start bit to tell SOTP to send command to the OTP controller */
	mmio_setbits_32(SOTP_CTRL_0, BIT(SOTP_CTRL_0__START));

	/*  Wait for SOTP command done to be set */
	while ((mmio_read_32(SOTP_STATUS_1) & BIT(SOTP_STATUS_1__CMD_DONE)) !=
	       BIT(SOTP_STATUS_1__CMD_DONE))
		;

	/* Command done is cleared w1c */
	mmio_setbits_32(SOTP_STATUS_1, BIT(SOTP_STATUS_1__CMD_DONE));

	/* disable OTP acces by CPU */
	mmio_clrbits_32(SOTP_PROG_CONTROL,
			BIT(SOTP_PROG_CONTROL__OTP_CPU_MODE_EN));

	/* Clr Start bit after command done */
	mmio_clrbits_32(SOTP_CTRL_0, BIT(SOTP_CTRL_0__START));

	if (chip_state & chip_state_default)
		mmio_write_32(SOTP_CHIP_CTRL, chip_ctrl_default);

#endif
}

int sotp_read_key(uint8_t *key, size_t keysize, int start_row, int end_row)
{
	int row;
	uint32_t status = 0;
	uint32_t status2 = 0xFFFFFFFF;
	uint64_t row_data;
	uint32_t data;
	uint32_t *temp_key = (uint32_t *)key;

	row = start_row;
	while ((keysize > 0) && (row <= end_row)) {
		row_data = sotp_mem_read(row, SOTP_ROW_ECC);
		if (!(row_data & (SOTP_ECC_ERR_DETECT | SOTP_FAIL_BITS))) {
			memcpy(temp_key++, &row_data, sizeof(uint32_t));
			keysize -= sizeof(uint32_t);
			data = (uint32_t)(row_data & SOTP_ROW_DATA_MASK);
			status |= data;
			status2 &= data;
		}
		row++;
	}

	if ((status2 == 0xFFFFFFFF) || (status == 0) || (row > end_row))
		return -1;

	return 0;
}

int sotp_key_erased(void)
{
	uint64_t row_data;
	int status = 0;

	row_data = sotp_mem_read(SOTP_DEVICE_SECURE_CFG0_ROW, 0);
	if (row_data & SOTP_DEVICE_SECURE_CFG0_OTP_ERASED_MASK)
		status = 1;

	else if (mmio_read_32(SOTP_REGS_SOTP_CHIP_STATES) &
			SOTP_REGS_SOTP_CHIP_STATES_OTP_ERASED_MASK)
		status = 1;

	return status;
}

/*
 * This function optimise the SOTP redundancy
 * by considering the 00- zero and 01,10,11 - one
 */
uint32_t sotp_redundancy_reduction(uint32_t sotp_row_data)
{
	uint32_t opt_data;
	uint32_t opt_loop;
	uint32_t temp_data;

	opt_data = 0;

	for (opt_loop = 0; opt_loop < 16; opt_loop = opt_loop + 1) {
		temp_data = ((sotp_row_data >> (opt_loop * 2)) & 0x3);

		if (temp_data != 0x0)
			opt_data = (opt_data | (1 << opt_loop));
	}
	return opt_data;
}
