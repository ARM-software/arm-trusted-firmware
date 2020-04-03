/*
 * Copyright (c) 2016 - 2020, Broadcom
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <string.h>

#include <drivers/delay_timer.h>

#include <chimp.h>
#include <chimp_nv_defs.h>

#define CHIMP_DEFAULT_STARTUP_ADDR 0xb4300000

/* ChiMP's view of APE scratchpad memory for fastboot */
#define CHIMP_FASTBOOT_ADDR 0x61000000

#define CHIMP_PREPARE_ACCESS_WINDOW(addr) \
	(\
		mmio_write_32(\
			NIC400_NITRO_CHIMP_S_IDM_IO_CONTROL_DIRECT, \
			addr & 0xffc00000)\
	)
#define CHIMP_INDIRECT_TGT_ADDR(addr) \
	(CHIMP_INDIRECT_BASE + (addr & CHIMP_INDIRECT_ADDR_MASK))

#define CHIMP_CTRL_ADDR(x) (CHIMP_REG_CTRL_BASE + x)

/* For non-PAXC builds */
#ifndef CHIMP_FB1_ENTRY
#define CHIMP_FB1_ENTRY 0
#endif

#define CHIMP_DBG	VERBOSE

void bcm_chimp_write(uintptr_t addr, uint32_t value)
{
	CHIMP_PREPARE_ACCESS_WINDOW(addr);
	mmio_write_32(CHIMP_INDIRECT_TGT_ADDR(addr), value);
}

uint32_t bcm_chimp_read(uintptr_t addr)
{
	CHIMP_PREPARE_ACCESS_WINDOW(addr);
	return mmio_read_32(CHIMP_INDIRECT_TGT_ADDR(addr));
}

void bcm_chimp_clrbits(uintptr_t addr, uint32_t bits)
{
	CHIMP_PREPARE_ACCESS_WINDOW(addr);
	mmio_clrbits_32(CHIMP_INDIRECT_TGT_ADDR(addr), bits);
}

void bcm_chimp_setbits(uintptr_t addr, uint32_t bits)
{
	CHIMP_PREPARE_ACCESS_WINDOW(addr);
	mmio_setbits_32(CHIMP_INDIRECT_TGT_ADDR(addr), bits);
}

int bcm_chimp_is_nic_mode(void)
{
	uint32_t val;

	/* Check if ChiMP straps are set */
	val = mmio_read_32(CDRU_CHIP_STRAP_DATA_LSW);
	val &= CDRU_CHIP_STRAP_DATA_LSW__NIC_MODE_MASK;

	return val == CDRU_CHIP_STRAP_DATA_LSW__NIC_MODE_MASK;
}

void bcm_chimp_fru_prog_done(bool is_done)
{
	uint32_t val;

	val = is_done ? (1 << CHIMP_FRU_PROG_DONE_BIT) : 0;
	bcm_chimp_setbits(CHIMP_REG_ECO_RESERVED, val);
}

int bcm_chimp_handshake_done(void)
{
	uint32_t value;

	value = bcm_chimp_read(CHIMP_REG_ECO_RESERVED);
	value &= (1 << CHIMP_FLASH_ACCESS_DONE_BIT);

	return value != 0;
}

int bcm_chimp_wait_handshake(void)
{
	uint32_t timeout = CHIMP_HANDSHAKE_TIMEOUT_MS;
	uint32_t status;

	INFO("Waiting for ChiMP handshake...\n");
	do {
		if (bcm_chimp_handshake_done())
			break;
		/* No need to wait if ChiMP reported an error */
		status = bcm_chimp_read_ctrl(CHIMP_REG_CTRL_BPE_STAT_REG);
		if (status & CHIMP_ERROR_MASK) {
			ERROR("ChiMP error 0x%x. Wait aborted\n", status);
			break;
		}
		mdelay(1);
	} while (--timeout);

	if (!bcm_chimp_handshake_done()) {
		if (timeout == 0) {
			WARN("Timeout waiting for ChiMP handshake\n");
		}
	} else {
		INFO("Got handshake from ChiMP!\n");
	}

	return bcm_chimp_handshake_done();
}

uint32_t bcm_chimp_read_ctrl(uint32_t offset)
{
	return bcm_chimp_read(CHIMP_CTRL_ADDR(offset));
}

static int bcm_chimp_nitro_reset(void)
{
	uint32_t timeout;

	/* Perform tasks done by M0 in NIC mode */
	CHIMP_DBG("Taking Nitro out of reset\n");
	mmio_setbits_32(CDRU_MISC_RESET_CONTROL,
		/* MHB_RESET_N */
		(1 << CDRU_MISC_RESET_CONTROL__CDRU_MHB_RESET_N_R)  |
		/* PCI_RESET_N */
		(1 << CDRU_MISC_RESET_CONTROL__CDRU_PCIE_RESET_N_R) |
		/* PM_RESET_N */
		(1 << CDRU_MISC_RESET_CONTROL__CDRU_PM_RESET_N_R)   |
		/* NIC_RESET_N */
		(1 << CDRU_MISC_RESET_CONTROL__CDRU_NITRO_RESET_N_R)
	);

	/* Wait until Nitro is out of reset */
	timeout = NIC_RESET_RELEASE_TIMEOUT_US;
	do {
		uint32_t value;

		value = bcm_chimp_read_ctrl(CHIMP_REG_CTRL_BPE_MODE_REG);
		if ((value & CHIMP_BPE_MODE_ID_MASK) ==
				CHIMP_BPE_MODE_ID_PATTERN)
			break;
		udelay(1);
	} while (--timeout);

	if (timeout == 0) {
		ERROR("NIC reset release timed out\n");
		return -1;
	}

	return 0;
}

static void bcm_nitro_secure_mode_enable(void)
{
	mmio_setbits_32(CDRU_NITRO_CONTROL,
		(1 << CDRU_NITRO_CONTROL__CDRU_NITRO_SEC_MODE_R) |
		(1 << CDRU_NITRO_CONTROL__CDRU_NITRO_SEC_OVERRIDE_R));
	mmio_write_32(NITRO_TZPC_TZPCDECPROT0clr,
		/* NITRO_TZPC */
		1 << NITRO_TZPC_TZPCDECPROT0clr__DECPROT0_chimp_m_clr_R);
}

static int bcm_chimp_reset_and_initial_setup(void)
{

	int err;
	uint32_t handshake_reg;

	err = bcm_chimp_nitro_reset();
	if (err)
		return err;

	/* Enable Nitro secure mode */
	bcm_nitro_secure_mode_enable();

	/* Force ChiMP back into reset */
	bcm_chimp_setbits(CHIMP_CTRL_ADDR(CHIMP_REG_CTRL_BPE_MODE_REG),
		1 << CHIMP_REG_CHIMP_REG_CTRL_BPE_MODE_REG__cm3_rst_R);

	handshake_reg = (1 << SR_IN_SMARTNIC_MODE_BIT);

	/* Get OTP secure Chimp boot status */
	if (mmio_read_32(CRMU_OTP_STATUS) & (1 << CRMU_OTP_STATUS_BIT))
		handshake_reg |= (1 << SR_CHIMP_SECURE_BOOT_BIT);

	bcm_chimp_write(CHIMP_REG_ECO_RESERVED, handshake_reg);

	CHIMP_DBG("ChiMP reset and initial handshake parameters set\n");

	return 0;
}

static void bcm_nitro_chimp_release_reset(void)
{
	bcm_chimp_clrbits(CHIMP_CTRL_ADDR(CHIMP_REG_CTRL_BPE_MODE_REG),
		1 << CHIMP_REG_CHIMP_REG_CTRL_BPE_MODE_REG__cm3_rst_R);

	CHIMP_DBG("Nitro Reset Released\n");
}

static void bcm_chimp_set_fastboot(int mode)
{
	uint32_t fb_entry;

	/* 1. Enable fastboot */
	bcm_chimp_setbits(CHIMP_CTRL_ADDR(CHIMP_REG_CTRL_BPE_MODE_REG),
			(1 << CHIMP_FAST_BOOT_MODE_BIT));
	fb_entry = CHIMP_FASTBOOT_ADDR | mode;
	if (mode == CHIMP_FASTBOOT_JUMP_IN_PLACE)
		fb_entry = CHIMP_FB1_ENTRY;
	/* 2. Write startup address and mode */
	INFO("Setting fastboot type %d entry to 0x%x\n", mode, fb_entry);
	bcm_chimp_write(
			CHIMP_CTRL_ADDR(CHIMP_REG_CTRL_FSTBOOT_PTR_REG),
			fb_entry);
}

#ifndef CHIMPFW_USE_SIDELOAD
static void bcm_chimp_load_fw_from_spi(uintptr_t spi_addr, size_t size)
{
	uintptr_t ape_scpad;
	uintptr_t dest;
	size_t bytes_left;

	ape_scpad = CHIMP_REG_CHIMP_APE_SCPAD;
	dest = CHIMP_INDIRECT_TGT_ADDR(CHIMP_REG_CHIMP_APE_SCPAD);
	bytes_left = size;

	while (bytes_left) {
		uint32_t delta;

		delta = bytes_left > CHIMP_WINDOW_SIZE ?
			bytes_left - CHIMP_WINDOW_SIZE : bytes_left;
		CHIMP_PREPARE_ACCESS_WINDOW(ape_scpad);
		INFO("Transferring %d byte(s) from 0x%lx to 0x%lx\n",
			delta, spi_addr, dest);
	/*
	 * This single memcpy call takes significant amount of time
	 * on Palladium. Be patient
	 */
		memcpy((void *)dest, (void *)spi_addr, delta);
		bytes_left -= delta;
		INFO("Transferred %d byte(s) from 0x%lx to 0x%lx (%lu%%)\n",
			delta, spi_addr, dest,
			((size - bytes_left) * 100)/size);
		spi_addr += delta;
		dest += delta;
		ape_scpad += delta;
	}
}

static int bcm_chimp_find_fw_in_spi(uintptr_t *addr, size_t *size)
{
	int i;
	bnxnvm_master_block_header_t *master_block_hdr;
	bnxnvm_directory_block_header_t *dir_block_hdr;
	bnxnvm_directory_entry_t *dir_entry;
	int found;

	found = 0;

	/* Read the master block */
	master_block_hdr =
		(bnxnvm_master_block_header_t *)(uintptr_t)QSPI_BASE_ADDR;
	if (master_block_hdr->sig != BNXNVM_MASTER_BLOCK_SIG) {
		WARN("Invalid masterblock 0x%x (expected 0x%x)\n",
			master_block_hdr->sig,
			BNXNVM_MASTER_BLOCK_SIG);
		return -NV_NOT_NVRAM;
	}
	if ((master_block_hdr->block_size > NV_MAX_BLOCK_SIZE) ||
		(master_block_hdr->directory_offset >=
			master_block_hdr->nvram_size)) {
		WARN("Invalid masterblock block size 0x%x or directory offset 0x%x\n",
			master_block_hdr->block_size,
			master_block_hdr->directory_offset);
		return -NV_BAD_MB;
	}

	/* Skip to the Directory block start */
	dir_block_hdr =
		(bnxnvm_directory_block_header_t *)
			((uintptr_t)QSPI_BASE_ADDR +
				master_block_hdr->directory_offset);
	if (dir_block_hdr->sig != BNXNVM_DIRECTORY_BLOCK_SIG) {
		WARN("Invalid directory header 0x%x (expected 0x%x)\n",
			dir_block_hdr->sig,
			BNXNVM_DIRECTORY_BLOCK_SIG);
		return -NV_BAD_DIR_HEADER;
	}

	/* Locate the firmware */
	for (i = 0; i < dir_block_hdr->entries; i++) {
		*addr = ((uintptr_t)dir_block_hdr + dir_block_hdr->length +
			i * dir_block_hdr->entry_length);
		dir_entry = (bnxnvm_directory_entry_t *)(*addr);
		if ((dir_entry->type == BNX_DIR_TYPE_BOOTCODE) ||
				(dir_entry->type == BNX_DIR_TYPE_BOOTCODE_2)) {
			found = 1;
			break;
		}
	}

	if (!found)
		return -NV_FW_NOT_FOUND;

	*addr = QSPI_BASE_ADDR + dir_entry->item_location;
	*size = dir_entry->data_length;

	INFO("Found chimp firmware at 0x%lx, size %lu byte(s)\n",
			*addr, *size);

	return NV_OK;
}
#endif

int bcm_chimp_initiate_fastboot(int fastboot_type)
{
	int err;

	if ((fastboot_type != CHIMP_FASTBOOT_NITRO_RESET) &&
			(fastboot_type <= CHIMP_FASTBOOT_JUMP_DECOMPRESS)) {
		CHIMP_DBG("Initiating ChiMP fastboot type %d\n", fastboot_type);
	}

	/*
	 * If we are here, M0 did not setup Nitro because NIC mode
	 * strap was not present
	 */
	err = bcm_chimp_reset_and_initial_setup();
	if (err)
		return err;

	if (fastboot_type > CHIMP_FASTBOOT_JUMP_DECOMPRESS) {
		WARN("ChiMP setup deferred\n");
		return -1;
	}

	if (fastboot_type != CHIMP_FASTBOOT_NITRO_RESET) {

		if ((fastboot_type == CHIMP_FASTBOOT_JUMP_IN_PLACE) &&
			(CHIMP_FB1_ENTRY == 0)) {
			ERROR("Missing ESAL entry point for fastboot type 1.\n"
			"Fastboot failed\n");
			return -1;
		}

		/*
		 * TODO: We need to think of the way to load the ChiMP fw.
		 * This could be SPI, NAND, etc.
		 * For now we temporarily stick to the SPI load unless
		 * CHIMPFW_USE_SIDELOAD is defined. Note that for the SPI NVRAM
		 * image we need to parse directory and get the image.
		 * When we load image from other media there is no need to
		 * parse because fw image can be directly placed into the APE's
		 * scratchpad.
		 * For sideload method we simply reset the ChiMP, set bpe_reg
		 * to do fastboot with the type we define, and release from
		 * reset so that ROM loader would initiate fastboot immediately
		 */
#ifndef CHIMPFW_USE_SIDELOAD
		{
			uintptr_t spi_addr;
			size_t size;

			err = bcm_chimp_find_fw_in_spi(&spi_addr, &size);
			if (!err) {
				INFO("Loading ChiMP firmware, addr 0x%lx, size %lu byte(s)\n",
					spi_addr, size);
				bcm_chimp_load_fw_from_spi(spi_addr, size);
			} else {
				ERROR("Error %d ChiMP firmware not in NVRAM directory!\n",
					err);
			}
		}
#else
		INFO("Skip ChiMP QSPI fastboot type %d due to sideload requested\n",
		     fastboot_type);
#endif
		if (!err) {
			INFO("Instruct ChiMP to fastboot\n");
			bcm_chimp_set_fastboot(fastboot_type);
			INFO("Fastboot mode set\n");
		}
	}

	bcm_nitro_chimp_release_reset();

	return err;
}
