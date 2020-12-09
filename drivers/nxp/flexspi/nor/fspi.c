// SPDX-License-Identifier: BSD-3-Clause
/*
 * NXP FlexSpi Controller Driver.
 * Copyright 2021 NXP
 *
 */
#include <endian.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <common/debug.h>
#include <flash_info.h>
#include "fspi.h"
#include <fspi_api.h>
#include <xspi_error_codes.h>

#ifdef DEBUG_FLEXSPI
#define PR printf("In [%s][%d]\n", __func__, __LINE__)
#define PRA(a, b) printf("In [%s][%d] %s="a"\n", __func__, __LINE__, #b, b)
#else
#define PR
#define PRA(a, b)
#endif

/*
 * This errata is valid for all NXP SoC.
 */
#define ERRATA_FLASH_A050272 1

static uintptr_t fspi_base_reg_addr;
static uintptr_t fspi_flash_base_addr;

static void fspi_RDSR(uint32_t *, const void *, uint32_t);

static void fspi_writel(uint32_t x_addr, uint32_t x_val)
{
	fspi_out32((uint32_t *)(fspi_base_reg_addr + x_addr),
		 (uint32_t) x_val);
}

static uint32_t fspi_readl(uint32_t x_addr)
{
	return fspi_in32((uint32_t *)(fspi_base_reg_addr + x_addr));
}

static void fspi_MDIS(uint8_t x_disable)
{
	uint32_t ui_reg;

	ui_reg = fspi_readl(FSPI_MCR0);
	if (x_disable != 0U) {
		ui_reg |= FSPI_MCR0_MDIS;
	} else {
		ui_reg &= (uint32_t) (~FSPI_MCR0_MDIS);
	}

	fspi_writel(FSPI_MCR0, ui_reg);
}

static void fspi_lock_LUT(void)
{
	fspi_writel(FSPI_LUTKEY, FSPI_LUTKEY_VALUE);
	VERBOSE("%s 0x%x\n", __func__, fspi_readl(FSPI_LCKCR));
	fspi_writel(FSPI_LCKCR, FSPI_LCKER_LOCK);
	VERBOSE("%s 0x%x\n", __func__, fspi_readl(FSPI_LCKCR));
}

static void fspi_unlock_LUT(void)
{
	fspi_writel(FSPI_LUTKEY,  FSPI_LUTKEY_VALUE);
	VERBOSE("%s 0x%x\n", __func__, fspi_readl(FSPI_LCKCR));
	fspi_writel(FSPI_LCKCR, FSPI_LCKER_UNLOCK);
	VERBOSE("%s 0x%x\n", __func__, fspi_readl(FSPI_LCKCR));
}

static void fspi_op_setup(uint32_t fspi_op_seq_id, bool ignore_flash_sz)
{
	uint32_t x_addr, x_instr0 = 0, x_instr1 = 0, x_instr2 = 0;
	uint32_t cmd_id1, cmd_id2;

	VERBOSE("In func %s\n", __func__);

	switch (fspi_op_seq_id) {
	case FSPI_READ_SEQ_ID:
		cmd_id1 = FSPI_NOR_CMD_READ;
		cmd_id2 = FSPI_NOR_CMD_READ_4B;
		x_instr2 = FSPI_INSTR_OPRND0(0) | FSPI_INSTR_PAD0(FSPI_LUT_PAD1)
				| FSPI_INSTR_OPCODE0(FSPI_LUT_READ);
		break;
	case FSPI_FASTREAD_SEQ_ID:
		cmd_id1 = FSPI_NOR_CMD_FASTREAD;
		cmd_id2 = FSPI_NOR_CMD_FASTREAD_4B;
		x_instr2 = FSPI_INSTR_OPRND0(8) | FSPI_INSTR_PAD0(FSPI_LUT_PAD1)
				| FSPI_INSTR_OPCODE0(FSPI_DUMMY_SDR)
				| FSPI_INSTR_OPRND1(0)
				| FSPI_INSTR_PAD1(FSPI_LUT_PAD1)
				| FSPI_INSTR_OPCODE1(FSPI_LUT_READ);
		break;
	case FSPI_WRITE_SEQ_ID:
		cmd_id1 = FSPI_NOR_CMD_PP;
		cmd_id2 = FSPI_NOR_CMD_PP_4B;
		x_instr2 = FSPI_INSTR_OPRND0(0) | FSPI_INSTR_PAD0(FSPI_LUT_PAD1)
				| FSPI_INSTR_OPCODE0(FSPI_LUT_WRITE);
		break;
	case FSPI_WREN_SEQ_ID:
		cmd_id1 = FSPI_NOR_CMD_WREN;
		cmd_id2 = FSPI_NOR_CMD_WREN;
		break;
	case FSPI_SE_SEQ_ID:
		cmd_id1 = FSPI_NOR_CMD_SE_64K;
		cmd_id2 = FSPI_NOR_CMD_SE_64K_4B;
		break;
	case FSPI_4K_SEQ_ID:
		cmd_id1 = FSPI_NOR_CMD_SE_4K;
		cmd_id2 = FSPI_NOR_CMD_SE_4K_4B;
		break;
	case FSPI_BE_SEQ_ID:
		cmd_id1 = FSPI_NOR_CMD_BE;
		cmd_id2 = FSPI_NOR_CMD_BE;
		break;
	case FSPI_RDSR_SEQ_ID:
		cmd_id1 = FSPI_NOR_CMD_RDSR;
		cmd_id2 = FSPI_NOR_CMD_RDSR;
		break;
	}

	x_addr = FSPI_LUTREG_OFFSET + (uint32_t)(0x10 * fspi_op_seq_id);
	if ((F_FLASH_SIZE_BYTES <= SZ_16M_BYTES) || (ignore_flash_sz)) {
		x_instr0 = FSPI_INSTR_OPRND0(cmd_id1);
		x_instr1 = FSPI_INSTR_OPRND1(FSPI_LUT_ADDR24BIT);
		VERBOSE("CMD_ID = %x offset = 0x%x\n", cmd_id1, x_addr);
	} else {
		x_instr0 = FSPI_INSTR_OPRND0(cmd_id2);
		x_instr1 = FSPI_INSTR_OPRND1(FSPI_LUT_ADDR32BIT);
		VERBOSE("CMD_ID = %x offset = 0x%x\n", cmd_id2, x_addr);
	}
	x_instr0 |= FSPI_INSTR_PAD0(FSPI_LUT_PAD1)
		| FSPI_INSTR_OPCODE0(FSPI_LUT_CMD);

	x_instr1 |= FSPI_INSTR_PAD1(FSPI_LUT_PAD1)
		| FSPI_INSTR_OPCODE1(FSPI_LUT_ADDR);

	if (fspi_op_seq_id == FSPI_RDSR_SEQ_ID) {
		x_instr0 |= FSPI_INSTR_OPRND1(1) | FSPI_INSTR_PAD1(FSPI_LUT_PAD1)
					| FSPI_INSTR_OPCODE1(FSPI_LUT_READ);
	} else if ((fspi_op_seq_id != FSPI_BE_SEQ_ID)
			&& (fspi_op_seq_id != FSPI_WREN_SEQ_ID)) {
		x_instr0 |= x_instr1;
	}

	fspi_writel((x_addr), x_instr0);
	fspi_writel((x_addr + U(0x4)), x_instr2);
	fspi_writel((x_addr + U(0x8)), (uint32_t) 0x0);	/* STOP command */
	fspi_writel((x_addr + U(0xc)), (uint32_t) 0x0);	/* STOP command */
}

static void fspi_setup_LUT(void)
{
	VERBOSE("In func %s\n", __func__);
	fspi_unlock_LUT();

	/* LUT Setup for READ Command 3-Byte low Frequency */
	fspi_op_setup(FSPI_READ_SEQ_ID, false);

	/* LUT Setup for FAST READ Command 3-Byte/4-Byte high Frequency */
	fspi_op_setup(FSPI_FASTREAD_SEQ_ID, false);

	/* LUT Setup for Page Program */
	fspi_op_setup(FSPI_WRITE_SEQ_ID, false);

	/* LUT Setup for WREN */
	fspi_op_setup(FSPI_WREN_SEQ_ID, true);

	/* LUT Setup for Sector_Erase */
	fspi_op_setup(FSPI_SE_SEQ_ID, false);

	/* LUT Setup for Sub Sector 4K Erase */
	fspi_op_setup(FSPI_4K_SEQ_ID, false);

	/* LUT Setup for Bulk_Erase */
	fspi_op_setup(FSPI_BE_SEQ_ID, true);

	/* Read Status */
	fspi_op_setup(FSPI_RDSR_SEQ_ID, true);

	fspi_lock_LUT();
}

static inline void fspi_ahb_invalidate(void)
{
	uint32_t reg;

	VERBOSE("In func %s %d\n", __func__, __LINE__);
	reg = fspi_readl(FSPI_MCR0);
	reg |= FSPI_MCR0_SWRST;
	fspi_writel(FSPI_MCR0, reg);
	while ((fspi_readl(FSPI_MCR0) & FSPI_MCR0_SWRST) != 0)
		;  /* FSPI_MCR0_SWRESET_MASK */
	VERBOSE("In func %s %d\n", __func__, __LINE__);
}

#if defined(CONFIG_FSPI_AHB)
static void fspi_init_ahb(void)
{
	uint32_t i, x_flash_cr2, seq_id;

	x_flash_cr2 = 0;
	/* Reset AHB RX buffer CR configuration */
	for (i = 0; i < 8; i++) {
		fspi_writel((FSPI_AHBRX_BUF0CR0 + 4 * i), 0U);
	}

	/* Set ADATSZ with the maximum AHB buffer size */
	fspi_writel(FSPI_AHBRX_BUF7CR0,
			((uint32_t) ((FSPI_RX_MAX_AHBBUF_SIZE / 8U) |
				    FSPI_AHBRXBUF0CR7_PREF)));

	/* Known limitation handling: prefetch and
	 * no start address alignment.*/
	fspi_writel(FSPI_AHBCR, FSPI_AHBCR_PREF_EN);
	INFO("xAhbcr=0x%x\n", fspi_readl(FSPI_AHBCR));

	// Setup AHB READ sequenceID for all flashes.
	x_flash_cr2 = fspi_readl(FSPI_FLSHA1CR2);
	INFO("x_flash_cr2=0x%x\n", x_flash_cr2);

	seq_id = CONFIG_FSPI_FASTREAD ?
			FSPI_FASTREAD_SEQ_ID : FSPI_READ_SEQ_ID;
	x_flash_cr2 |= ((seq_id << FSPI_FLSHXCR2_ARDSEQI_SHIFT) & 0x1f);

	INFO("x_flash_cr2=0x%x\n", x_flash_cr2);

	fspi_writel(FSPI_FLSHA1CR2,  x_flash_cr2);
	x_flash_cr2 = fspi_readl(FSPI_FLSHA1CR2);
	INFO("x_flash_cr2=0x%x\n", x_flash_cr2);
}
#endif

int xspi_read(uint32_t pc_rx_addr, uint32_t *pc_rx_buf, uint32_t x_size_bytes)
{
	if (x_size_bytes == 0) {
		ERROR("Zero length reads are not allowed\n");
		return XSPI_READ_FAIL;
	}

#if defined(CONFIG_FSPI_AHB)
	return xspi_ahb_read(pc_rx_addr, pc_rx_buf, x_size_bytes);
#else
	return xspi_ip_read(pc_rx_addr, pc_rx_buf, x_size_bytes);
#endif
}
#if defined(CONFIG_FSPI_AHB)
int xspi_ahb_read(uint32_t pc_rx_addr, uint32_t *pc_rx_buf, uint32_t x_size_bytes)
{
	VERBOSE("In func %s 0x%x\n", __func__, (pc_rx_addr));

	if (F_FLASH_SIZE_BYTES <= SZ_16M_BYTES) {
		pc_rx_addr = ((uint32_t)(pcRxAddr & MASK_24BIT_ADDRESS));
	} else {
		pc_rx_addr = ((uint32_t)(pcRxAddr & MASK_32BIT_ADDRESS));
	}

	pc_rx_addr = ((uint32_t)(pcRxAddr + fspi_flash_base_addr));

	if (((pc_rx_addr % 4) != 0) || (((uintptr_t)pc_rx_buf % 4) != 0)) {
		WARN("%s: unaligned Start Address src=%ld dst=0x%p\n",
		     __func__, (pc_rx_addr - fspi_flash_base_addr), pc_rx_buf);
	}

	/* Directly copy from AHB Buffer */
	memcpy(pc_rx_buf, (void *)(uintptr_t)pc_rx_addr, x_size_bytes);

	fspi_ahb_invalidate();
	return XSPI_SUCCESS;
}
#endif

int xspi_ip_read(uint32_t pc_rx_addr, uint32_t *pv_rx_buf, uint32_t ui_len)
{

	uint32_t i = 0U, j = 0U, x_rem = 0U;
	uint32_t x_iteration = 0U, x_size_rx = 0U, x_size_wm, temp_size;
	uint32_t data = 0U;
	uint32_t x_len_bytes;
	uint32_t x_addr, sts0, intr, seq_id;

	x_addr = (uint32_t) pc_rx_addr;
	x_len_bytes = ui_len;

	/* Watermark level : 8 bytes. (BY DEFAULT) */
	x_size_wm = 8U;

	/* Clear  RX Watermark interrupt in INT register, if any existing.  */
	fspi_writel(FSPI_INTR, FSPI_INTR_IPRXWA);
	PRA("0x%x", fspi_readl(FSPI_INTR));
	/* Invalid the RXFIFO, to run next IP Command */
	/* Clears data entries in IP Rx FIFOs, Also reset R/W pointers */
	fspi_writel(FSPI_IPRXFCR, FSPI_IPRXFCR_CLR);
	fspi_writel(FSPI_INTR, FSPI_INTEN_IPCMDDONE);

	while (x_len_bytes) {

		/* FlexSPI can store no more than  FSPI_RX_IPBUF_SIZE */
		x_size_rx = (x_len_bytes >  FSPI_RX_IPBUF_SIZE) ?
			   FSPI_RX_IPBUF_SIZE : x_len_bytes;

		/* IP Control Register0 - SF Address to be read */
		fspi_writel(FSPI_IPCR0, x_addr);
		PRA("0x%x", fspi_readl(FSPI_IPCR0));
		/* IP Control Register1 - SEQID_READ operation, Size */

		seq_id = CONFIG_FSPI_FASTREAD ?
				FSPI_FASTREAD_SEQ_ID : FSPI_READ_SEQ_ID;

		fspi_writel(FSPI_IPCR1,
			    (uint32_t)(seq_id << FSPI_IPCR1_ISEQID_SHIFT) |
			    (uint16_t) x_size_rx);

		PRA("0x%x", fspi_readl(FSPI_IPCR1));

		do {
			sts0 = fspi_readl(FSPI_STS0);
		} while (((sts0 & FSPI_STS0_ARB_IDLE) == 0) &&
			 ((sts0 & FSPI_STS0_SEQ_IDLE) == 0));

		/* Trigger IP Read Command */
		fspi_writel(FSPI_IPCMD, FSPI_IPCMD_TRG_MASK);
		PRA("0x%x", fspi_readl(FSPI_IPCMD));

		intr = fspi_readl(FSPI_INTR);
		if (((intr & FSPI_INTR_IPCMDGE) != 0) ||
		    ((intr & FSPI_INTR_IPCMDERR) != 0)) {
			ERROR("Error in IP READ INTR=0x%x\n", intr);
			return -XSPI_IP_READ_FAIL;
		}
		/* Will read in n iterations of each 8 FIFO's(WM level) */
		x_iteration = x_size_rx / x_size_wm;
		for (i = 0U; i < x_iteration; i++) {
			if ((fspi_readl(FSPI_INTR) & FSPI_INTR_IPRXWA_MASK) == 0) {
				PRA("0x%x", fspi_readl(FSPI_INTR));
			}
			/* Wait for IP Rx Watermark Fill event */
			while (!(fspi_readl(FSPI_INTR) & FSPI_INTR_IPRXWA_MASK)) {
				PRA("0x%x", fspi_readl(FSPI_INTR));
			}

			/* Read RX FIFO's(upto WM level) & copy to rxbuffer */
			for (j = 0U; j < x_size_wm; j += 4U) {
				/* Read FIFO Data Register */
				data = fspi_readl(FSPI_RFDR + j);
#if FSPI_IPDATA_SWAP /* Just In case you want swap */
				data = bswap32(data);
#endif
				memcpy(pv_rx_buf++, &data, 4);
			}

			/* Clear IP_RX_WATERMARK Event in INTR register */
			/* Reset FIFO Read pointer for next iteration.*/
			fspi_writel(FSPI_INTR, FSPI_INTR_IPRXWA);
		}

		x_rem = x_size_rx % x_size_wm;

		if (x_rem != 0U) {
			/* Wait for data filled */
			while (!(fspi_readl(FSPI_IPRXFSTS) & FSPI_IPRXFSTS_FILL_MASK)) {
				PRA("0x%x", fspi_readl(FSPI_IPRXFSTS));
			}

			temp_size = 0;
			j = 0U;
			while (x_rem > 0U) {
				data = 0U;
				data =  fspi_readl(FSPI_RFDR + j);
#if FSPI_IPDATA_SWAP /* Just In case you want swap */
				data = bswap32(data);
#endif
				temp_size = (x_rem < 4) ? x_rem : 4;
				memcpy(pv_rx_buf++, &data, temp_size);
				x_rem -= temp_size;
			}
		}


		while (!(fspi_readl(FSPI_INTR) & FSPI_INTR_IPCMDDONE_MASK)) {
			PRA("0x%x", fspi_readl(FSPI_INTR));
		}

		/* Invalid the RX FIFO, to run next IP Command */
		fspi_writel(FSPI_IPRXFCR, FSPI_IPRXFCR_CLR);
		/* Clear IP Command Done flag in interrupt register*/
		fspi_writel(FSPI_INTR, FSPI_INTR_IPCMDDONE_MASK);

		/* Update remaining len, Increment x_addr read pointer. */
		x_len_bytes -= x_size_rx;
		x_addr += x_size_rx;
	}
	PR;
	return XSPI_SUCCESS;
}

void xspi_ip_write(uint32_t pc_wr_addr, uint32_t *pv_wr_buf, uint32_t ui_len)
{

	uint32_t x_iteration = 0U, x_rem = 0U;
	uint32_t x_size_tx = 0U, x_size_wm, temp_size;
	uint32_t i = 0U, j = 0U;
	uint32_t ui_data = 0U;
	uint32_t x_addr, x_len_bytes;


	x_size_wm = 8U;	/* Default TX WaterMark level: 8 Bytes. */
	x_addr = (uint32_t)pc_wr_addr;
	x_len_bytes = ui_len;
	VERBOSE("In func %s[%d] x_addr =0x%x xLen_bytes=%d\n",
			__func__, __LINE__, x_addr, x_len_bytes);

	while (x_len_bytes != 0U) {

		x_size_tx = (x_len_bytes >  FSPI_TX_IPBUF_SIZE) ?
				FSPI_TX_IPBUF_SIZE : x_len_bytes;

		/* IP Control Register0 - SF Address to be read */
		fspi_writel(FSPI_IPCR0, x_addr);
		INFO("In func %s[%d] x_addr =0x%x xLen_bytes=%d\n",
				__func__, __LINE__, x_addr, x_len_bytes);

		/*
		 * Fill TX FIFO's..
		 *
		 */

		x_iteration = x_size_tx / x_size_wm;
		for (i = 0U; i < x_iteration; i++) {

			/* Ensure TX FIFO Watermark Available */
			while ((fspi_readl(FSPI_INTR) & FSPI_INTR_IPTXWE_MASK) == 0)
				;


			/* Fill TxFIFO's ( upto watermark level) */
			for (j = 0U; j < x_size_wm; j += 4U) {
				memcpy(&ui_data, pv_wr_buf++,  4);
				/* Write TX FIFO Data Register */
				fspi_writel((FSPI_TFDR + j), ui_data);

			}

			/* Clear IP_TX_WATERMARK Event in INTR register */
			/* Reset the FIFO Write pointer for next iteration */
			fspi_writel(FSPI_INTR, FSPI_INTR_IPTXWE);
		}

		x_rem = x_size_tx % x_size_wm;

		if (x_rem != 0U) {
			/* Wait for TXFIFO empty */
			while (!(fspi_readl(FSPI_INTR) & FSPI_INTR_IPTXWE))
				;

			temp_size = 0U;
			j = 0U;
			while (x_rem > 0U) {
				ui_data = 0U;
				temp_size = (x_rem < 4U) ? x_rem : 4U;
				memcpy(&ui_data, pv_wr_buf++, temp_size);
				INFO("%d ---> pv_wr_buf=0x%p\n", __LINE__, pv_wr_buf);
				fspi_writel((FSPI_TFDR + j), ui_data);
				x_rem -= temp_size;
				j += 4U ; /* TODO: May not be needed*/
			}
			/* Clear IP_TX_WATERMARK Event in INTR register */
			/* Reset FIFO's Write pointer for next iteration.*/
			fspi_writel(FSPI_INTR, FSPI_INTR_IPTXWE);
		}

		/* IP Control Register1 - SEQID_WRITE operation, Size */
		fspi_writel(FSPI_IPCR1, (uint32_t)(FSPI_WRITE_SEQ_ID << FSPI_IPCR1_ISEQID_SHIFT) | (uint16_t) x_size_tx);
		/* Trigger IP Write Command */
		fspi_writel(FSPI_IPCMD, FSPI_IPCMD_TRG_MASK);

		/* Wait for IP Write command done */
		while (!(fspi_readl(FSPI_INTR) & FSPI_INTR_IPCMDDONE_MASK))
			;

		/* Invalidate TX FIFOs & acknowledge IP_CMD_DONE event */
		fspi_writel(FSPI_IPTXFCR, FSPI_IPTXFCR_CLR);
		fspi_writel(FSPI_INTR, FSPI_INTR_IPCMDDONE_MASK);

		/* for next iteration */
		x_len_bytes  -=  x_size_tx;
		x_addr += x_size_tx;
	}

}

int xspi_write(uint32_t pc_wr_addr, void *pv_wr_buf, uint32_t ui_len)
{

	uint32_t x_addr;
	uint32_t x_page1_len = 0U, x_page_l_len = 0U;
	uint32_t i, j = 0U;
	void *buf = pv_wr_buf;

	VERBOSE("\nIn func %s\n", __func__);

	x_addr = (uint32_t)(pc_wr_addr);
	if ((ui_len <= F_PAGE_256) && ((x_addr % F_PAGE_256) == 0)) {
		x_page1_len = ui_len;
		INFO("%d ---> x_page1_len=0x%x x_page_l_len =0x%x j=0x%x\n", __LINE__, x_page1_len, x_page_l_len, j);
	} else if ((ui_len <= F_PAGE_256) && ((x_addr % F_PAGE_256) != 0)) {
		x_page1_len = (F_PAGE_256 - (x_addr % F_PAGE_256));
		if (ui_len > x_page1_len) {
			x_page_l_len = (ui_len - x_page1_len) % F_PAGE_256;
		} else {
			x_page1_len = ui_len;
			x_page_l_len = 0;
		}
		j = 0U;
		INFO("%d 0x%x 0x%x\n", x_addr % F_PAGE_256, x_addr % F_PAGE_256, F_PAGE_256);
		INFO("%d ---> x_page1_len=0x%x x_page_l_len =0x%x j=0x%x\n", __LINE__, x_page1_len, x_page_l_len, j);
	} else if ((ui_len > F_PAGE_256) && ((x_addr % F_PAGE_256) == 0)) {
		j = ui_len / F_PAGE_256;
		x_page_l_len = ui_len % F_PAGE_256;
		INFO("%d ---> x_page1_len=0x%x x_page_l_len =0x%x j=0x%x\n", __LINE__, x_page1_len, x_page_l_len, j);
	} else if ((ui_len > F_PAGE_256) && ((x_addr % F_PAGE_256) != 0)) {
		x_page1_len = (F_PAGE_256 - (x_addr % F_PAGE_256));
		j = (ui_len - x_page1_len) / F_PAGE_256;
		x_page_l_len = (ui_len - x_page1_len) % F_PAGE_256;
		INFO("%d ---> x_page1_len=0x%x x_page_l_len =0x%x j=0x%x\n", __LINE__, x_page1_len, x_page_l_len, j);
	}

	if (x_page1_len != 0U) {
		xspi_wren(x_addr);
		xspi_ip_write(x_addr, (uint32_t *)buf, x_page1_len);
		while (is_flash_busy())
			;
		INFO("%d Initial pc_wr_addr=0x%x, Final x_addr=0x%x, Initial ui_len=0x%x Final ui_len=0x%x\n",
		     __LINE__, pc_wr_addr, x_addr, ui_len, (x_addr-pc_wr_addr));
		INFO("Initial Buf pv_wr_buf=%p, final Buf=%p\n", pv_wr_buf, buf);
		x_addr += x_page1_len;
		/* TODO What is buf start is not 4 aligned */
		buf = buf + x_page1_len;
	}

	for (i = 0U; i < j; i++) {
		INFO("In for loop Buf pv_wr_buf=%p, final Buf=%p x_addr=0x%x offset_buf %d.\n",
				pv_wr_buf, buf, x_addr, x_page1_len/4);
		xspi_wren(x_addr);
		xspi_ip_write(x_addr, (uint32_t *)buf, F_PAGE_256);
		while (is_flash_busy())
			;
		INFO("%d Initial pc_wr_addr=0x%x, Final x_addr=0x%x, Initial ui_len=0x%x Final ui_len=0x%x\n",
		     __LINE__, pc_wr_addr, x_addr, ui_len, (x_addr-pc_wr_addr));
		x_addr += F_PAGE_256;
		/* TODO What is buf start is not 4 aligned */
		buf = buf + F_PAGE_256;
		INFO("Initial Buf pv_wr_buf=%p, final Buf=%p\n", pv_wr_buf, buf);
	}

	if (x_page_l_len != 0U) {
		INFO("%d Initial Buf pv_wr_buf=%p, final Buf=%p x_page_l_len=0x%x\n", __LINE__, pv_wr_buf, buf, x_page_l_len);
		xspi_wren(x_addr);
		xspi_ip_write(x_addr, (uint32_t *)buf, x_page_l_len);
		while (is_flash_busy())
			;
		INFO("%d Initial pc_wr_addr=0x%x, Final x_addr=0x%x, Initial ui_len=0x%x Final ui_len=0x%x\n",
				__LINE__, pc_wr_addr, x_addr, ui_len, (x_addr-pc_wr_addr));
	}

	VERBOSE("Now calling func call Invalidate%s\n", __func__);
	fspi_ahb_invalidate();
	return XSPI_SUCCESS;
}

int xspi_wren(uint32_t pc_wr_addr)
{
	VERBOSE("In func %s Addr=0x%x\n", __func__, pc_wr_addr);

	fspi_writel(FSPI_IPTXFCR, FSPI_IPTXFCR_CLR);

	fspi_writel(FSPI_IPCR0, (uint32_t)pc_wr_addr);
	fspi_writel(FSPI_IPCR1, ((FSPI_WREN_SEQ_ID << FSPI_IPCR1_ISEQID_SHIFT) |  0));
	fspi_writel(FSPI_IPCMD, FSPI_IPCMD_TRG_MASK);

	while ((fspi_readl(FSPI_INTR) & FSPI_INTR_IPCMDDONE_MASK) == 0)
		;

	fspi_writel(FSPI_INTR, FSPI_INTR_IPCMDDONE_MASK);
	return XSPI_SUCCESS;
}

static void fspi_bbluk_er(void)
{
	VERBOSE("In func %s\n", __func__);
	fspi_writel(FSPI_IPCR0, 0x0);
	fspi_writel(FSPI_IPCR1, ((FSPI_BE_SEQ_ID << FSPI_IPCR1_ISEQID_SHIFT) | 20));
	fspi_writel(FSPI_IPCMD, FSPI_IPCMD_TRG_MASK);

	while ((fspi_readl(FSPI_INTR) & FSPI_INTR_IPCMDDONE_MASK) == 0)
		;
	fspi_writel(FSPI_INTR, FSPI_INTR_IPCMDDONE_MASK);

}

static void fspi_RDSR(uint32_t *rxbuf, const void *p_addr, uint32_t size)
{
	uint32_t iprxfcr = 0U;
	uint32_t data = 0U;

	iprxfcr = fspi_readl(FSPI_IPRXFCR);
	/* IP RX FIFO would be read by processor */
	iprxfcr = iprxfcr & (uint32_t)~FSPI_IPRXFCR_CLR;
	/* Invalid data entries in IP RX FIFO */
	iprxfcr = iprxfcr | FSPI_IPRXFCR_CLR;
	fspi_writel(FSPI_IPRXFCR, iprxfcr);

	fspi_writel(FSPI_IPCR0, (uintptr_t) p_addr);
	fspi_writel(FSPI_IPCR1,
		    (uint32_t) ((FSPI_RDSR_SEQ_ID << FSPI_IPCR1_ISEQID_SHIFT)
		    | (uint16_t) size));
	/* Trigger the command */
	fspi_writel(FSPI_IPCMD, FSPI_IPCMD_TRG_MASK);
	/* Wait for command done */
	while ((fspi_readl(FSPI_INTR) & FSPI_INTR_IPCMDDONE_MASK) == 0)
		;
	fspi_writel(FSPI_INTR, FSPI_INTR_IPCMDDONE_MASK);

	data = fspi_readl(FSPI_RFDR);
	memcpy(rxbuf, &data, size);

	/* Rx FIFO invalidation needs to be done prior w1c of INTR.IPRXWA bit */
	fspi_writel(FSPI_IPRXFCR, FSPI_IPRXFCR_CLR);
	fspi_writel(FSPI_INTR, FSPI_INTR_IPRXWA_MASK);
	fspi_writel(FSPI_INTR, FSPI_INTR_IPCMDDONE_MASK);

}

bool is_flash_busy(void)
{
#define FSPI_ONE_BYTE 1
	uint8_t data[4];

	VERBOSE("In func %s\n\n", __func__);
	fspi_RDSR((uint32_t *) data, 0, FSPI_ONE_BYTE);

	return !!((uint32_t) data[0] & FSPI_NOR_SR_WIP_MASK);
}

int xspi_bulk_erase(void)
{
	VERBOSE("In func %s\n", __func__);
	xspi_wren((uint32_t) 0x0);
	fspi_bbluk_er();
	while (is_flash_busy())
		;
	fspi_ahb_invalidate();
	return XSPI_SUCCESS;
}

static void fspi_sec_er(uint32_t pc_wr_addr)
{
	uint32_t x_addr;

	VERBOSE("In func %s\n", __func__);
	x_addr = (uint32_t)(pc_wr_addr);

	fspi_writel(FSPI_IPCR0, x_addr);
	INFO("In [%s][%d] Erase address 0x%x\n", __func__, __LINE__, (x_addr));
#if CONFIG_FSPI_ERASE_4K
	fspi_writel(FSPI_IPCR1, ((FSPI_4K_SEQ_ID << FSPI_IPCR1_ISEQID_SHIFT) | 0));
#else
	fspi_writel(FSPI_IPCR1, ((FSPI_SE_SEQ_ID << FSPI_IPCR1_ISEQID_SHIFT) | 0));
#endif
	fspi_writel(FSPI_IPCMD, FSPI_IPCMD_TRG_MASK);

	while ((fspi_readl(FSPI_INTR) & FSPI_INTR_IPCMDDONE_MASK) == 0) {
		PRA("0x%x", fspi_readl(FSPI_INTR));
	}
	fspi_writel(FSPI_INTR, FSPI_INTR_IPCMDDONE_MASK);
}

int xspi_sector_erase(uint32_t pc_wr_addr, uint32_t ui_len)
{
	uint32_t x_addr, x_len_bytes, i, num_sector = 0U;

	VERBOSE("In func %s\n", __func__);
	x_addr = (uint32_t)(pc_wr_addr);
	if ((x_addr % F_SECTOR_ERASE_SZ) != 0) {
		ERROR("!!! In func %s, unalinged start address can only be in multiples of 0x%x\n",
		      __func__, F_SECTOR_ERASE_SZ);
		return -XSPI_ERASE_FAIL;
	}

	x_len_bytes = ui_len * 1;
	if (x_len_bytes < F_SECTOR_ERASE_SZ) {
		ERROR("!!! In func %s, Less than 1 sector can only be in multiples of 0x%x\n",
				__func__, F_SECTOR_ERASE_SZ);
		return -XSPI_ERASE_FAIL;
	}

	num_sector = x_len_bytes/F_SECTOR_ERASE_SZ;
	num_sector += x_len_bytes % F_SECTOR_ERASE_SZ ? 1U : 0U;
	INFO("F_SECTOR_ERASE_SZ: 0x%08x, num_sector: %d\n", F_SECTOR_ERASE_SZ, num_sector);

	for (i = 0U; i < num_sector ; i++) {
		xspi_wren(x_addr + (F_SECTOR_ERASE_SZ * i));
		fspi_sec_er(x_addr + (F_SECTOR_ERASE_SZ * i));
		while (is_flash_busy())
			;
	}
	fspi_ahb_invalidate();
	return XSPI_SUCCESS;
}


__attribute__((unused)) static void  fspi_delay_ms(uint32_t x)
{
	volatile unsigned long  ul_count;

	for (ul_count = 0U; ul_count < (30U * x); ul_count++)
		;

}


#if defined(DEBUG_FLEXSPI)
static void fspi_dump_regs(void)
{
	uint32_t i;

	VERBOSE("\nRegisters Dump:\n");
	VERBOSE("Flexspi: Register FSPI_MCR0(0x%x) = 0x%08x\n", FSPI_MCR0, fspi_readl(FSPI_MCR0));
	VERBOSE("Flexspi: Register FSPI_MCR2(0x%x) = 0x%08x\n", FSPI_MCR2, fspi_readl(FSPI_MCR2));
	VERBOSE("Flexspi: Register FSPI_DLL_A_CR(0x%x) = 0x%08x\n", FSPI_DLLACR, fspi_readl(FSPI_DLLACR));
	VERBOSE("\n");

	for (i = 0U; i < 8U; i++) {
		VERBOSE("Flexspi: Register FSPI_AHBRX_BUF0CR0(0x%x) = 0x%08x\n", FSPI_AHBRX_BUF0CR0 + i * 4, fspi_readl((FSPI_AHBRX_BUF0CR0 + i * 4)));
	}
	VERBOSE("\n");

	VERBOSE("Flexspi: Register FSPI_AHBRX_BUF7CR0(0x%x) = 0x%08x\n", FSPI_AHBRX_BUF7CR0, fspi_readl(FSPI_AHBRX_BUF7CR0));
	VERBOSE("Flexspi: Register FSPI_AHB_CR(0x%x) \t  = 0x%08x\n", FSPI_AHBCR, fspi_readl(FSPI_AHBCR));
	VERBOSE("\n");

	for (i = 0U; i < 4U; i++) {
		VERBOSE("Flexspi: Register FSPI_FLSH_A1_CR2,(0x%x) = 0x%08x\n", FSPI_FLSHA1CR2 + i * 4, fspi_readl(FSPI_FLSHA1CR2 + i * 4));
	}
}
#endif

int fspi_init(uint32_t base_reg_addr, uint32_t flash_start_addr)
{
	uint32_t	mcrx;
	uint32_t	flash_size;

	if (fspi_base_reg_addr != 0U) {
		INFO("FSPI is already initialized.\n");
		return XSPI_SUCCESS;
	}

	fspi_base_reg_addr = base_reg_addr;
	fspi_flash_base_addr = flash_start_addr;

	INFO("Flexspi driver: Version v1.0\n");
	INFO("Flexspi: Default MCR0 = 0x%08x, before reset\n", fspi_readl(FSPI_MCR0));
	VERBOSE("Flexspi: Resetting controller...\n");

	/* Reset FlexSpi Controller */
	fspi_writel(FSPI_MCR0, FSPI_MCR0_SWRST);
	while ((fspi_readl(FSPI_MCR0) & FSPI_MCR0_SWRST))
		;  /* FSPI_MCR0_SWRESET_MASK */


	/* Disable Controller Module before programming its registersi, especially MCR0 (Master Control Register0) */
	fspi_MDIS(1);
	/*
	 * Program MCR0 with default values, AHB Timeout(0xff), IP Timeout(0xff).  {FSPI_MCR0- 0xFFFF0000}
	 */

	/* Timeout wait cycle for AHB command grant */
	mcrx = fspi_readl(FSPI_MCR0);
	mcrx |= (uint32_t)((FSPI_MAX_TIMEOUT_AHBCMD << FSPI_MCR0_AHBGRANTWAIT_SHIFT) & (FSPI_MCR0_AHBGRANTWAIT_MASK));

	/* Time out wait cycle for IP command grant*/
	mcrx |= (uint32_t) (FSPI_MAX_TIMEOUT_IPCMD << FSPI_MCR0_IPGRANTWAIT_SHIFT) & (FSPI_MCR0_IPGRANTWAIT_MASK);

	/* TODO why BE64 set BE32*/
	mcrx |= (uint32_t) (FSPI_ENDCFG_LE64 << FSPI_MCR0_ENDCFG_SHIFT) & FSPI_MCR0_ENDCFG_MASK;

	fspi_writel(FSPI_MCR0, mcrx);

	/* Reset the DLL register to default value */
	fspi_writel(FSPI_DLLACR, FSPI_DLLACR_OVRDEN);
	fspi_writel(FSPI_DLLBCR, FSPI_DLLBCR_OVRDEN);

#if ERRATA_FLASH_A050272	/* ERRATA DLL */
	for (uint8_t delay = 100U; delay > 0U; delay--)	{
		__asm__ volatile ("nop");
	}
#endif

	/* Configure flash control registers for different chip select */
	flash_size = (F_FLASH_SIZE_BYTES * FLASH_NUM) / FSPI_BYTES_PER_KBYTES;
	fspi_writel(FSPI_FLSHA1CR0, flash_size);
	fspi_writel(FSPI_FLSHA2CR0, 0U);
	fspi_writel(FSPI_FLSHB1CR0, 0U);
	fspi_writel(FSPI_FLSHB2CR0, 0U);

#if defined(CONFIG_FSPI_AHB)
	fspi_init_ahb();
#endif
	/* RE-Enable Controller Module */
	fspi_MDIS(0);
	INFO("Flexspi: After MCR0 = 0x%08x,\n", fspi_readl(FSPI_MCR0));
	fspi_setup_LUT();

	/* Dump of all registers, ensure controller not disabled anymore*/
#if defined(DEBUG_FLEXSPI)
	fspi_dump_regs();
#endif

	INFO("Flexspi: Init done!!\n");

#if DEBUG_FLEXSPI

	uint32_t xspi_addr = SZ_57M;

	/*
	 * Second argument of fspi_test is the size of buffer(s) passed
	 * to the function.
	 * SIZE_BUFFER defined in test_fspi.c is kept large enough to
	 * accommodate variety of sizes for regressive tests.
	 */
	fspi_test(xspi_addr, 0x40, 0);
	fspi_test(xspi_addr, 0x15, 2);
	fspi_test(xspi_addr, 0x80, 0);
	fspi_test(xspi_addr, 0x81, 0);
	fspi_test(xspi_addr, 0x79, 3);

	fspi_test(xspi_addr + 0x11, 0x15, 0);
	fspi_test(xspi_addr + 0x11, 0x40, 0);
	fspi_test(xspi_addr + 0xff, 0x40, 1);
	fspi_test(xspi_addr + 0x25, 0x81, 2);
	fspi_test(xspi_addr + 0xef, 0x6f, 3);

	fspi_test((xspi_addr - F_SECTOR_ERASE_SZ), 0x229, 0);
#endif

	return XSPI_SUCCESS;
}
