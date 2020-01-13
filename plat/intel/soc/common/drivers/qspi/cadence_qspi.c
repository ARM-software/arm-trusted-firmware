/*
 * Copyright (c) 2019, ARM Limited and Contributors. All rights reserved.
 * Copyright (c) 2019, Intel Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <common/debug.h>
#include <lib/mmio.h>
#include <string.h>
#include <drivers/delay_timer.h>
#include <drivers/console.h>

#include "cadence_qspi.h"

#define LESS(a, b)   (((a) < (b)) ? (a) : (b))
#define MORE(a, b)   (((a) > (b)) ? (a) : (b))


uint32_t qspi_device_size;
int cad_qspi_cs;

int cad_qspi_idle(void)
{
	return (mmio_read_32(CAD_QSPI_OFFSET + CAD_QSPI_CFG)
			& CAD_QSPI_CFG_IDLE) >> 31;
}

int cad_qspi_set_baudrate_div(uint32_t div)
{
	if (div > 0xf)
		return CAD_INVALID;

	mmio_clrsetbits_32(CAD_QSPI_OFFSET + CAD_QSPI_CFG,
			~CAD_QSPI_CFG_BAUDDIV_MSK,
			CAD_QSPI_CFG_BAUDDIV(div));

	return 0;
}

int cad_qspi_configure_dev_size(uint32_t addr_bytes,
		uint32_t bytes_per_dev, uint32_t bytes_per_block)
{

	mmio_write_32(CAD_QSPI_OFFSET + CAD_QSPI_DEVSZ,
			CAD_QSPI_DEVSZ_ADDR_BYTES(addr_bytes) |
			CAD_QSPI_DEVSZ_BYTES_PER_PAGE(bytes_per_dev) |
			CAD_QSPI_DEVSZ_BYTES_PER_BLOCK(bytes_per_block));
	return 0;
}

int cad_qspi_set_read_config(uint32_t opcode, uint32_t instr_type,
		uint32_t addr_type, uint32_t data_type,
		uint32_t mode_bit, uint32_t dummy_clk_cycle)
{
	mmio_write_32(CAD_QSPI_OFFSET + CAD_QSPI_DEVRD,
			CAD_QSPI_DEV_OPCODE(opcode) |
			CAD_QSPI_DEV_INST_TYPE(instr_type) |
			CAD_QSPI_DEV_ADDR_TYPE(addr_type) |
			CAD_QSPI_DEV_DATA_TYPE(data_type) |
			CAD_QSPI_DEV_MODE_BIT(mode_bit) |
			CAD_QSPI_DEV_DUMMY_CLK_CYCLE(dummy_clk_cycle));

	return 0;
}

int cad_qspi_set_write_config(uint32_t opcode, uint32_t addr_type,
		uint32_t data_type, uint32_t dummy_clk_cycle)
{
	mmio_write_32(CAD_QSPI_OFFSET + CAD_QSPI_DEVWR,
			CAD_QSPI_DEV_OPCODE(opcode) |
			CAD_QSPI_DEV_ADDR_TYPE(addr_type) |
			CAD_QSPI_DEV_DATA_TYPE(data_type) |
			CAD_QSPI_DEV_DUMMY_CLK_CYCLE(dummy_clk_cycle));

	return 0;
}

int cad_qspi_timing_config(uint32_t clkphase, uint32_t clkpol, uint32_t csda,
		uint32_t csdads, uint32_t cseot, uint32_t cssot,
		uint32_t rddatacap)
{
	uint32_t cfg = mmio_read_32(CAD_QSPI_OFFSET + CAD_QSPI_CFG);

	cfg &= CAD_QSPI_CFG_SELCLKPHASE_CLR_MSK &
		CAD_QSPI_CFG_SELCLKPOL_CLR_MSK;
	cfg |= CAD_QSPI_SELCLKPHASE(clkphase) | CAD_QSPI_SELCLKPOL(clkpol);

	mmio_write_32(CAD_QSPI_OFFSET + CAD_QSPI_CFG, cfg);

	mmio_write_32(CAD_QSPI_OFFSET + CAD_QSPI_DELAY,
		CAD_QSPI_DELAY_CSSOT(cssot) | CAD_QSPI_DELAY_CSEOT(cseot) |
		CAD_QSPI_DELAY_CSDADS(csdads) | CAD_QSPI_DELAY_CSDA(csda));

	return 0;
}

int cad_qspi_stig_cmd_helper(int cs, uint32_t cmd)
{
	uint32_t count = 0;

	/* chip select */
	mmio_write_32(CAD_QSPI_OFFSET + CAD_QSPI_CFG,
			(mmio_read_32(CAD_QSPI_OFFSET + CAD_QSPI_CFG)
			 & CAD_QSPI_CFG_CS_MSK) | CAD_QSPI_CFG_CS(cs));

	mmio_write_32(CAD_QSPI_OFFSET + CAD_QSPI_FLASHCMD, cmd);
	mmio_write_32(CAD_QSPI_OFFSET + CAD_QSPI_FLASHCMD,
			cmd | CAD_QSPI_FLASHCMD_EXECUTE);

	do {
		uint32_t reg = mmio_read_32(CAD_QSPI_OFFSET +
					CAD_QSPI_FLASHCMD);
		if (!(reg & CAD_QSPI_FLASHCMD_EXECUTE_STAT))
			break;
		count++;
	} while (count < CAD_QSPI_COMMAND_TIMEOUT);

	if (count >= CAD_QSPI_COMMAND_TIMEOUT) {
		ERROR("Error sending QSPI command %x, timed out\n",
				cmd);
		return CAD_QSPI_ERROR;
	}

	return 0;
}

int cad_qspi_stig_cmd(uint32_t opcode, uint32_t dummy)
{
	if (dummy > ((1 << CAD_QSPI_FLASHCMD_NUM_DUMMYBYTES_MAX) - 1)) {
		ERROR("Faulty dummy bytes\n");
		return -1;
	}

	return cad_qspi_stig_cmd_helper(cad_qspi_cs,
			CAD_QSPI_FLASHCMD_OPCODE(opcode) |
			CAD_QSPI_FLASHCMD_NUM_DUMMYBYTES(dummy));
}

int cad_qspi_stig_read_cmd(uint32_t opcode, uint32_t dummy, uint32_t num_bytes,
		uint32_t *output)
{
	if (dummy > ((1 << CAD_QSPI_FLASHCMD_NUM_DUMMYBYTES_MAX) - 1)) {
		ERROR("Faulty dummy byes\n");
		return -1;
	}

	if ((num_bytes > 8) || (num_bytes == 0))
		return -1;

	uint32_t cmd =
		CAD_QSPI_FLASHCMD_OPCODE(opcode) |
		CAD_QSPI_FLASHCMD_ENRDDATA(1) |
		CAD_QSPI_FLASHCMD_NUMRDDATABYTES(num_bytes - 1) |
		CAD_QSPI_FLASHCMD_ENCMDADDR(0) |
		CAD_QSPI_FLASHCMD_ENMODEBIT(0) |
		CAD_QSPI_FLASHCMD_NUMADDRBYTES(0) |
		CAD_QSPI_FLASHCMD_ENWRDATA(0) |
		CAD_QSPI_FLASHCMD_NUMWRDATABYTES(0) |
		CAD_QSPI_FLASHCMD_NUMDUMMYBYTES(dummy);

	if (cad_qspi_stig_cmd_helper(cad_qspi_cs, cmd)) {
		ERROR("failed to send stig cmd\n");
		return -1;
	}

	output[0] = mmio_read_32(CAD_QSPI_OFFSET + CAD_QSPI_FLASHCMD_RDDATA0);

	if (num_bytes > 4) {
		output[1] = mmio_read_32(CAD_QSPI_OFFSET +
				CAD_QSPI_FLASHCMD_RDDATA1);
	}

	return 0;
}

int cad_qspi_stig_wr_cmd(uint32_t opcode, uint32_t dummy, uint32_t num_bytes,
		uint32_t *input)
{
	if (dummy > ((1 << CAD_QSPI_FLASHCMD_NUM_DUMMYBYTES_MAX) - 1)) {
		ERROR("Faulty dummy byes\n");
		return -1;
	}

	if ((num_bytes > 8) || (num_bytes == 0))
		return -1;

	uint32_t cmd = CAD_QSPI_FLASHCMD_OPCODE(opcode) |
		CAD_QSPI_FLASHCMD_ENRDDATA(0) |
		CAD_QSPI_FLASHCMD_NUMRDDATABYTES(0) |
		CAD_QSPI_FLASHCMD_ENCMDADDR(0) |
		CAD_QSPI_FLASHCMD_ENMODEBIT(0) |
		CAD_QSPI_FLASHCMD_NUMADDRBYTES(0) |
		CAD_QSPI_FLASHCMD_ENWRDATA(1) |
		CAD_QSPI_FLASHCMD_NUMWRDATABYTES(num_bytes - 1) |
		CAD_QSPI_FLASHCMD_NUMDUMMYBYTES(dummy);

	mmio_write_32(CAD_QSPI_OFFSET + CAD_QSPI_FLASHCMD_WRDATA0, input[0]);

	if (num_bytes > 4)
		mmio_write_32(CAD_QSPI_OFFSET + CAD_QSPI_FLASHCMD_WRDATA1,
				input[1]);

	return cad_qspi_stig_cmd_helper(cad_qspi_cs, cmd);
}

int cad_qspi_stig_addr_cmd(uint32_t opcode, uint32_t dummy, uint32_t addr)
{
	uint32_t cmd;

	if (dummy > ((1 << CAD_QSPI_FLASHCMD_NUM_DUMMYBYTES_MAX) - 1))
		return -1;

	cmd = CAD_QSPI_FLASHCMD_OPCODE(opcode) |
		CAD_QSPI_FLASHCMD_NUMDUMMYBYTES(dummy) |
		CAD_QSPI_FLASHCMD_ENCMDADDR(1) |
		CAD_QSPI_FLASHCMD_NUMADDRBYTES(2);

	mmio_write_32(CAD_QSPI_OFFSET + CAD_QSPI_FLASHCMD_ADDR, addr);

	return cad_qspi_stig_cmd_helper(cad_qspi_cs, cmd);
}

int cad_qspi_device_bank_select(uint32_t bank)
{
	int status = 0;

	status = cad_qspi_stig_cmd(CAD_QSPI_STIG_OPCODE_WREN, 0);
	if (status != 0)
		return status;

	status = cad_qspi_stig_wr_cmd(CAD_QSPI_STIG_OPCODE_WREN_EXT_REG,
			0, 1, &bank);
	if (status != 0)
		return status;

	return cad_qspi_stig_cmd(CAD_QSPI_STIG_OPCODE_WRDIS, 0);
}

int cad_qspi_device_status(uint32_t *status)
{
	return cad_qspi_stig_read_cmd(CAD_QSPI_STIG_OPCODE_RDSR, 0, 1, status);
}

#if CAD_QSPI_MICRON_N25Q_SUPPORT
int cad_qspi_n25q_enable(void)
{
	cad_qspi_set_read_config(QSPI_FAST_READ, CAD_QSPI_INST_SINGLE,
			CAD_QSPI_ADDR_FASTREAD, CAT_QSPI_ADDR_SINGLE_IO, 1,
			0);
	cad_qspi_set_write_config(QSPI_WRITE, 0, 0, 0);

	return 0;
}

int cad_qspi_n25q_wait_for_program_and_erase(int program_only)
{
	uint32_t status, flag_sr;
	int count = 0;

	while (count < CAD_QSPI_COMMAND_TIMEOUT) {
		status = cad_qspi_device_status(&status);
		if (status != 0) {
			ERROR("Error getting device status\n");
			return -1;
		}
		if (!CAD_QSPI_STIG_SR_BUSY(status))
			break;
		count++;
	}

	if (count >= CAD_QSPI_COMMAND_TIMEOUT) {
		ERROR("Timed out waiting for idle\n");
		return -1;
	}

	count = 0;

	while (count < CAD_QSPI_COMMAND_TIMEOUT) {
		status = cad_qspi_stig_read_cmd(CAD_QSPI_STIG_OPCODE_RDFLGSR,
				0, 1, &flag_sr);
		if (status != 0) {
			ERROR("Error waiting program and erase.\n");
			return status;
		}

		if ((program_only &&
			CAD_QSPI_STIG_FLAGSR_PROGRAMREADY(flag_sr)) ||
			(!program_only &&
			CAD_QSPI_STIG_FLAGSR_ERASEREADY(flag_sr)))
			break;
	}

	if (count >= CAD_QSPI_COMMAND_TIMEOUT)
		ERROR("Timed out waiting for program and erase\n");

	if ((program_only && CAD_QSPI_STIG_FLAGSR_PROGRAMERROR(flag_sr)) ||
			(!program_only &&
			CAD_QSPI_STIG_FLAGSR_ERASEERROR(flag_sr))) {
		ERROR("Error programming/erasing flash\n");
		cad_qspi_stig_cmd(CAD_QSPI_STIG_OPCODE_CLFSR, 0);
		return -1;
	}

	return 0;
}
#endif

int cad_qspi_indirect_read_start_bank(uint32_t flash_addr, uint32_t num_bytes)
{
	mmio_write_32(CAD_QSPI_OFFSET + CAD_QSPI_INDRDSTADDR, flash_addr);
	mmio_write_32(CAD_QSPI_OFFSET + CAD_QSPI_INDRDCNT, num_bytes);
	mmio_write_32(CAD_QSPI_OFFSET + CAD_QSPI_INDRD,
			CAD_QSPI_INDRD_START |
			CAD_QSPI_INDRD_IND_OPS_DONE);

	return 0;
}


int cad_qspi_indirect_write_start_bank(uint32_t flash_addr,
					uint32_t num_bytes)
{
	mmio_write_32(CAD_QSPI_OFFSET + CAD_QSPI_INDWRSTADDR, flash_addr);
	mmio_write_32(CAD_QSPI_OFFSET + CAD_QSPI_INDWRCNT, num_bytes);
	mmio_write_32(CAD_QSPI_OFFSET + CAD_QSPI_INDWR,
			CAD_QSPI_INDWR_START |
			CAD_QSPI_INDWR_INDDONE);

	return 0;
}

int cad_qspi_indirect_write_finish(void)
{
#if CAD_QSPI_MICRON_N25Q_SUPPORT
	return cad_qspi_n25q_wait_for_program_and_erase(1);
#else
	return 0;
#endif

}

int cad_qspi_enable(void)
{
	int status;

	mmio_setbits_32(CAD_QSPI_OFFSET + CAD_QSPI_CFG, CAD_QSPI_CFG_ENABLE);

#if CAD_QSPI_MICRON_N25Q_SUPPORT
	status = cad_qspi_n25q_enable();
	if (status != 0)
		return status;
#endif
	return 0;
}

int cad_qspi_enable_subsector_bank(uint32_t addr)
{
	int status = 0;

	status = cad_qspi_stig_cmd(CAD_QSPI_STIG_OPCODE_WREN, 0);
	if (status != 0)
		return status;

	status = cad_qspi_stig_addr_cmd(CAD_QSPI_STIG_OPCODE_SUBSEC_ERASE, 0,
					addr);
	if (status != 0)
		return status;

#if CAD_QSPI_MICRON_N25Q_SUPPORT
	status = cad_qspi_n25q_wait_for_program_and_erase(0);
#endif
	return status;
}

int cad_qspi_erase_subsector(uint32_t addr)
{
	int status = 0;

	status = cad_qspi_device_bank_select(addr >> 24);
	if (status != 0)
		return status;

	return cad_qspi_enable_subsector_bank(addr);
}

int cad_qspi_erase_sector(uint32_t addr)
{
	int status = 0;

	status = cad_qspi_device_bank_select(addr >> 24);
	if (status != 0)
		return status;

	status = cad_qspi_stig_cmd(CAD_QSPI_STIG_OPCODE_WREN, 0);
	if (status != 0)
		return status;

	status = cad_qspi_stig_addr_cmd(CAD_QSPI_STIG_OPCODE_SEC_ERASE, 0,
					addr);
	if (status != 0)
		return status;

#if CAD_QSPI_MICRON_N25Q_SUPPORT
	status = cad_qspi_n25q_wait_for_program_and_erase(0);
#endif
	return status;
}

void cad_qspi_calibration(uint32_t dev_clk, uint32_t qspi_clk_mhz)
{
	int status;
	uint32_t dev_sclk_mhz = 27; /*min value to get biggest 0xF div factor*/
	uint32_t data_cap_delay;
	uint32_t sample_rdid;
	uint32_t rdid;
	uint32_t div_actual;
	uint32_t div_bits;
	int first_pass, last_pass;

	/*1.  Set divider to bigger value (slowest SCLK)
	 *2.  RDID and save the value
	 */
	div_actual = (qspi_clk_mhz + (dev_sclk_mhz - 1)) / dev_sclk_mhz;
	div_bits = (((div_actual + 1) / 2) - 1);
	status = cad_qspi_set_baudrate_div(0xf);

	status = cad_qspi_stig_read_cmd(CAD_QSPI_STIG_OPCODE_RDID,
					0, 3, &sample_rdid);
	if (status != 0)
		return;

	/*3. Set divider to the intended frequency
	 *4.  Set the read delay = 0
	 *5.  RDID and check whether the value is same as item 2
	 *6.  Increase read delay and compared the value against item 2
	 *7.  Find the range of read delay that have same as
	 *    item 2 and divide it to 2
	 */
	div_actual = (qspi_clk_mhz + (dev_clk - 1)) / dev_clk;
	div_bits = (((div_actual + 1) / 2) - 1);
	status = cad_qspi_set_baudrate_div(div_bits);
	if (status != 0)
		return;

	data_cap_delay = 0;
	first_pass = -1;
	last_pass = -1;

	do {
		if (status != 0)
			break;
		status = cad_qspi_stig_read_cmd(CAD_QSPI_STIG_OPCODE_RDID, 0,
						3, &rdid);
		if (status != 0)
			break;
		if (rdid == sample_rdid) {
			if (first_pass == -1)
				first_pass = data_cap_delay;
			else
				last_pass = data_cap_delay;
		}

		data_cap_delay++;

		mmio_write_32(CAD_QSPI_OFFSET + CAD_QSPI_RDDATACAP,
				CAD_QSPI_RDDATACAP_BYP(1) |
				CAD_QSPI_RDDATACAP_DELAY(data_cap_delay));

	} while (data_cap_delay < 0x10);

	if (first_pass > 0) {
		int diff = first_pass - last_pass;

		data_cap_delay = first_pass + diff / 2;
	}

	mmio_write_32(CAD_QSPI_OFFSET + CAD_QSPI_RDDATACAP,
			CAD_QSPI_RDDATACAP_BYP(1) |
			CAD_QSPI_RDDATACAP_DELAY(data_cap_delay));
	status = cad_qspi_stig_read_cmd(CAD_QSPI_STIG_OPCODE_RDID, 0, 3, &rdid);

	if (status != 0)
		return;
}

int cad_qspi_int_disable(uint32_t mask)
{
	if (cad_qspi_idle() == 0)
		return -1;

	if ((CAD_QSPI_INT_STATUS_ALL & mask) == 0)
		return -1;

	mmio_write_32(CAD_QSPI_OFFSET + CAD_QSPI_IRQMSK, mask);
	return 0;
}

void cad_qspi_set_chip_select(int cs)
{
	cad_qspi_cs = cs;
}

int cad_qspi_init(uint32_t desired_clk_freq, uint32_t clk_phase,
			uint32_t clk_pol, uint32_t csda, uint32_t csdads,
			uint32_t cseot, uint32_t cssot, uint32_t rddatacap)
{
	int status = 0;
	uint32_t qspi_desired_clk_freq;
	uint32_t rdid = 0;
	uint32_t cap_code;

	INFO("Initializing Qspi\n");

	if (cad_qspi_idle() == 0) {
		ERROR("device not idle\n");
		return -1;
	}


	status = cad_qspi_timing_config(clk_phase, clk_pol, csda, csdads,
					cseot, cssot, rddatacap);

	if (status != 0) {
		ERROR("config set timing failure\n");
		return status;
	}

	mmio_write_32(CAD_QSPI_OFFSET + CAD_QSPI_REMAPADDR,
			CAD_QSPI_REMAPADDR_VALUE_SET(0));

	status = cad_qspi_int_disable(CAD_QSPI_INT_STATUS_ALL);
	if (status != 0) {
		ERROR("failed disable\n");
		return status;
	}

	cad_qspi_set_baudrate_div(0xf);
	status = cad_qspi_enable();
	if (status != 0) {
		ERROR("failed enable\n");
		return status;
	}

	qspi_desired_clk_freq = 100;
	cad_qspi_calibration(qspi_desired_clk_freq, 50000000);

	status = cad_qspi_stig_read_cmd(CAD_QSPI_STIG_OPCODE_RDID, 0, 3,
					&rdid);

	if (status != 0) {
		ERROR("Error reading RDID\n");
		return status;
	}

	/*
	 * NOTE: The Size code seems to be a form of BCD (binary coded decimal).
	 * The first nibble is the 10's digit and the second nibble is the 1's
	 * digit in the number of bytes.
	 *
	 * Capacity ID samples:
	 * 0x15 :   16 Mb =>   2 MiB => 1 << 21 ; BCD=15
	 * 0x16 :   32 Mb =>   4 MiB => 1 << 22 ; BCD=16
	 * 0x17 :   64 Mb =>   8 MiB => 1 << 23 ; BCD=17
	 * 0x18 :  128 Mb =>  16 MiB => 1 << 24 ; BCD=18
	 * 0x19 :  256 Mb =>  32 MiB => 1 << 25 ; BCD=19
	 * 0x1a
	 * 0x1b
	 * 0x1c
	 * 0x1d
	 * 0x1e
	 * 0x1f
	 * 0x20 :  512 Mb =>  64 MiB => 1 << 26 ; BCD=20
	 * 0x21 : 1024 Mb => 128 MiB => 1 << 27 ; BCD=21
	 */

	cap_code = CAD_QSPI_STIG_RDID_CAPACITYID(rdid);

	if (!(((cap_code >> 4) > 0x9) || ((cap_code & 0xf) > 0x9))) {
		uint32_t decoded_cap = ((cap_code >> 4) * 10) +
					(cap_code & 0xf);
		qspi_device_size = 1 << (decoded_cap + 6);
		INFO("QSPI Capacity: %x\n\n", qspi_device_size);

	} else {
		ERROR("Invalid CapacityID encountered: 0x%02x\n",
				cap_code);
		return -1;
	}

	cad_qspi_configure_dev_size(INTEL_QSPI_ADDR_BYTES,
				INTEL_QSPI_BYTES_PER_DEV,
				INTEL_BYTES_PER_BLOCK);

	INFO("Flash size: %d Bytes\n", qspi_device_size);

	return status;
}

int cad_qspi_indirect_page_bound_write(uint32_t offset,
		uint8_t *buffer, uint32_t len)
{
	int status = 0, i;
	uint32_t write_count, write_capacity, *write_data, space,
		write_fill_level, sram_partition;

	status = cad_qspi_indirect_write_start_bank(offset, len);
	if (status != 0)
		return status;

	write_count = 0;
	sram_partition = CAD_QSPI_SRAMPART_ADDR(mmio_read_32(CAD_QSPI_OFFSET +
			 CAD_QSPI_SRAMPART));
	write_capacity = (uint32_t) CAD_QSPI_SRAM_FIFO_ENTRY_COUNT -
			sram_partition;

	while (write_count < len) {
		write_fill_level = CAD_QSPI_SRAMFILL_INDWRPART(
					mmio_read_32(CAD_QSPI_OFFSET +
							CAD_QSPI_SRAMFILL));
		space = LESS(write_capacity - write_fill_level,
				(len - write_count) / sizeof(uint32_t));
		write_data = (uint32_t *)(buffer + write_count);
		for (i = 0; i < space; ++i)
			mmio_write_32(CAD_QSPIDATA_OFST, *write_data++);

		write_count += space * sizeof(uint32_t);
	}
	return cad_qspi_indirect_write_finish();
}

int cad_qspi_read_bank(uint8_t *buffer, uint32_t offset, uint32_t size)
{
	int status;
	uint32_t read_count = 0, *read_data;
	int level = 1, count = 0, i;

	status = cad_qspi_indirect_read_start_bank(offset, size);

	if (status != 0)
		return status;

	while (read_count < size) {
		do {
			level = CAD_QSPI_SRAMFILL_INDRDPART(
				mmio_read_32(CAD_QSPI_OFFSET +
					CAD_QSPI_SRAMFILL));
			read_data = (uint32_t *)(buffer + read_count);
			for (i = 0; i < level; ++i)
				*read_data++ = mmio_read_32(CAD_QSPIDATA_OFST);

			read_count += level * sizeof(uint32_t);
			count++;
		} while (level > 0);
	}

	return 0;
}

int cad_qspi_write_bank(uint32_t offset, uint8_t *buffer, uint32_t size)
{
	int status = 0;
	uint32_t page_offset  = offset & (CAD_QSPI_PAGE_SIZE - 1);
	uint32_t write_size = LESS(size, CAD_QSPI_PAGE_SIZE - page_offset);

	while (size) {
		status = cad_qspi_indirect_page_bound_write(offset, buffer,
							write_size);
		if (status != 0)
			break;

		offset  += write_size;
		buffer  += write_size;
		size -= write_size;
		write_size = LESS(size, CAD_QSPI_PAGE_SIZE);
	}
	return status;
}

int cad_qspi_read(void *buffer, uint32_t  offset, uint32_t  size)
{
	uint32_t bank_count, bank_addr, bank_offset, copy_len;
	uint8_t *read_data;
	int i, status;

	status = 0;

	if ((offset >= qspi_device_size) ||
			(offset + size - 1 >= qspi_device_size) ||
			(size == 0)) {
		ERROR("Invalid read parameter\n");
		return -1;
	}

	if (CAD_QSPI_INDRD_RD_STAT(mmio_read_32(CAD_QSPI_OFFSET +
						CAD_QSPI_INDRD))) {
		ERROR("Read in progress\n");
		return -1;
	}

	/*
	 * bank_count : Number of bank(s) affected, including partial banks.
	 * bank_addr  : Aligned address of the first bank,
	 *		including partial bank.
	 * bank_ofst  : The offset of the bank to read.
	 *		Only used when reading the first bank.
	 */
	bank_count = CAD_QSPI_BANK_ADDR(offset + size - 1) -
			CAD_QSPI_BANK_ADDR(offset) + 1;
	bank_addr  = offset & CAD_QSPI_BANK_ADDR_MSK;
	bank_offset  = offset & (CAD_QSPI_BANK_SIZE - 1);

	read_data = (uint8_t *)buffer;

	copy_len = LESS(size, CAD_QSPI_BANK_SIZE - bank_offset);

	for (i = 0; i < bank_count; ++i) {
		status = cad_qspi_device_bank_select(CAD_QSPI_BANK_ADDR(
								bank_addr));
		if (status != 0)
			break;
		status = cad_qspi_read_bank(read_data, bank_offset, copy_len);
		if (status != 0)
			break;

		bank_addr += CAD_QSPI_BANK_SIZE;
		read_data += copy_len;
		size -= copy_len;
		bank_offset = 0;
		copy_len = LESS(size, CAD_QSPI_BANK_SIZE);
	}

	return status;
}

int cad_qspi_erase(uint32_t offset, uint32_t size)
{
	int status = 0;
	uint32_t subsector_offset  = offset & (CAD_QSPI_SUBSECTOR_SIZE - 1);
	uint32_t erase_size = LESS(size,
				CAD_QSPI_SUBSECTOR_SIZE - subsector_offset);

	while (size) {
		status = cad_qspi_erase_subsector(offset);
		if (status != 0)
			break;

		offset  += erase_size;
		size -= erase_size;
		erase_size = LESS(size, CAD_QSPI_SUBSECTOR_SIZE);
	}
	return status;
}

int cad_qspi_write(void *buffer, uint32_t offset, uint32_t size)
{
	int status, i;
	uint32_t bank_count, bank_addr, bank_offset, copy_len;
	uint8_t *write_data;

	status = 0;

	if ((offset >= qspi_device_size) ||
			(offset + size - 1 >= qspi_device_size) ||
			(size == 0)) {
		return -2;
	}

	if (CAD_QSPI_INDWR_RDSTAT(mmio_read_32(CAD_QSPI_OFFSET +
						CAD_QSPI_INDWR))) {
		ERROR("QSPI Error: Write in progress\n");
		return -1;
	}

	bank_count = CAD_QSPI_BANK_ADDR(offset + size - 1) -
			CAD_QSPI_BANK_ADDR(offset) + 1;
	bank_addr = offset & CAD_QSPI_BANK_ADDR_MSK;
	bank_offset = offset & (CAD_QSPI_BANK_SIZE - 1);

	write_data = buffer;

	copy_len = LESS(size, CAD_QSPI_BANK_SIZE - bank_offset);

	for (i = 0; i < bank_count; ++i) {
		status = cad_qspi_device_bank_select(
				CAD_QSPI_BANK_ADDR(bank_addr));
		if (status != 0)
			break;

		status = cad_qspi_write_bank(bank_offset, write_data,
						copy_len);
		if (status != 0)
			break;

		bank_addr += CAD_QSPI_BANK_SIZE;
		write_data += copy_len;
		size -= copy_len;
		bank_offset = 0;

		copy_len = LESS(size, CAD_QSPI_BANK_SIZE);
	}
	return status;
}

int cad_qspi_update(void *Buffer, uint32_t offset, uint32_t size)
{
	int status = 0;

	status = cad_qspi_erase(offset, size);
	if (status != 0)
		return status;

	return cad_qspi_write(Buffer, offset, size);
}

void cad_qspi_reset(void)
{
	cad_qspi_stig_cmd(CAD_QSPI_STIG_OPCODE_RESET_EN, 0);
	cad_qspi_stig_cmd(CAD_QSPI_STIG_OPCODE_RESET_MEM, 0);
}

