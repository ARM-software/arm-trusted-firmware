/*
 * Copyright 2018-2019 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include <common/debug.h>
#include <io/io_block.h>
#include <lib/mmio.h>
#include <lib/xlat_tables/xlat_tables_v2.h>

#include <plat_common.h>
#include <platform_def.h>
#include "ifc.h"

/* Private structure for MMC driver data */
static struct nand_info nand_drv_data;

static int update_bbt(uint32_t idx,
			  uint32_t blk,
			  uint32_t *updated,
			  struct nand_info *nand);

/***************************************************************************
 *  Function	:	nand_wait()
 *  Arguments	:	void
 *  Return	:	0 or -1 on Error Code
 *  Description	:	It polls on OPC bit for NAND read operation completion
 *			and checks NAND Event and Error Status Register
 *			for any error during NAND read operation.
 * *************************************************************************/
int nand_wait(void)
{
	int timeout = 1;
	uint32_t  neesr;
	unsigned long start_time;

	start_time = get_timer_val(0);

	while (get_timer_val(start_time) < NAND_TIMEOUT_MS) {
		/* clear the OPC event */
		neesr = read_reg(NAND_EVTER_STAT);
		if (neesr & NAND_EVTER_STAT_OPC_DN) {
			write_reg(NAND_EVTER_STAT, neesr);
			timeout = 0;

			/* check for other errors */
			if (neesr & NAND_EVTER_STAT_FTOER) {
				ERROR("%s NAND_EVTER_STAT_FTOER occurs\n",
								__func__);
				return -1;
			} else if (neesr & NAND_EVTER_STAT_ECCER) {
				ERROR("%s NAND_EVTER_STAT_ECCER occurs\n",
								__func__);
				return -1;
			} else if (neesr & NAND_EVTER_STAT_DQSER) {
				ERROR("%s NAND_EVTER_STAT_DQSER occurs\n",
								__func__);
				return -1;
			}

			break;
		}
	}

	if (timeout) {
		ERROR("%s ERROR_NAND_TIMEOUT occurs\n", __func__);
		return -1;
	}

	return 0;
}

/**************************************************************************
 * Function	:	nand_get_port_size()
 * Arguments	:	none
 * Return	:	Port size or Error Code
 * Description	:	It reads port size from NAND CSPR register
 **************************************************************************/
static uint32_t nand_get_port_size(void)
{
	uint32_t port_size = 0;
	uint32_t cs_reg;
	uint32_t cur_cs;

	cur_cs = 0;
	cs_reg = CSPR(cur_cs);
	port_size = (read_reg(cs_reg) & CSPR_PS) >> CSPR_PS_SHIFT;
	switch (port_size) {
	case CSPR_PS_8:
		port_size = 8;
		break;
	case CSPR_PS_16:
		port_size = 16;
		break;
	case CSPR_PS_32:
		port_size = 32;
		break;
	default:
		port_size = 8;
	}

	return port_size;
}

/**************************************************************************
 * Function	:	nand_get_page_size()
 * Arguments	:	none
 * Return	:	Page size or Error Code
 * Description	:	It reads page size from NAND CSOR register
 **************************************************************************/
static uint32_t nand_get_page_size(void)
{
	uint32_t pg_size;
	uint32_t cs_reg;
	uint32_t cur_cs;

	cur_cs = 0;
	cs_reg = CSOR(cur_cs);
	pg_size = read_reg(cs_reg) & CSOR_NAND_PGS;
	switch (pg_size) {
	case CSOR_NAND_PGS_2K:
		pg_size = 2048;
		break;
	case CSOR_NAND_PGS_4K:
		pg_size = 4096;
		break;
	case CSOR_NAND_PGS_8K:
		pg_size = 8192;
		break;
	case CSOR_NAND_PGS_16K:
		pg_size = 16384;
		break;
	default:
		pg_size = 512;
	}

	return pg_size;
}

/**************************************************************************
 * Function	:	nand_get_pages_per_blk()
 * Arguments	:	none
 * Return	:	Pages_per_blk or Error Code
 * Description	:	It reads pages_per_blk from NAND CSOR register
 **************************************************************************/
static uint32_t nand_get_pages_per_blk(void)
{
	uint32_t pages_per_blk;
	uint32_t cs_reg;
	uint32_t cur_cs;

	cur_cs = 0;
	cs_reg = CSOR(cur_cs);
	pages_per_blk = (read_reg(cs_reg) & CSOR_NAND_PB);
	switch (pages_per_blk) {
	case CSOR_NAND_PB_32:
		pages_per_blk = 32;
		break;
	case CSOR_NAND_PB_64:
		pages_per_blk = 64;
		break;
	case CSOR_NAND_PB_128:
		pages_per_blk = 128;
		break;
	case CSOR_NAND_PB_256:
		pages_per_blk = 256;
		break;
	case CSOR_NAND_PB_512:
		pages_per_blk = 512;
		break;
	case CSOR_NAND_PB_1024:
		pages_per_blk = 1024;
		break;
	case CSOR_NAND_PB_2048:
		pages_per_blk = 2048;
		break;
	default:
		pages_per_blk = 0;
	}

	return pages_per_blk;
}

/**************************************************************************
 *  Function	:	get_page_index_width()
 *  Arguments	:	ppb - pages per block
 *  Return	:	log2(ppb) or Error Code
 *  Description	:	It returns log2(ppb) which is used to merge page
 *			number and block number in row address.
 **************************************************************************/
static uint32_t get_page_index_width(uint32_t ppb)
{
	switch (ppb) {
	case CSOR_NAND_PPB_32:
		return 5;
	case CSOR_NAND_PPB_64:
		return 6;
	case CSOR_NAND_PPB_128:
		return 7;
	case CSOR_NAND_PPB_256:
		return 8;
	case CSOR_NAND_PPB_512:
		return 9;
	case CSOR_NAND_PPB_1024:
		return 10;
	case CSOR_NAND_PPB_2048:
		return 11;
	default:
		return 5;
	}
}

/**************************************************************************
 * Function	:	nand_get_params()
 * Arguments	:	nand - nand structure pointer
 * Return	:	void
 * Description	:	This function initializes the nand structure by
 *			reading values from IFC registers.
 ***************************************************************************/
static void nand_get_params(struct nand_info *nand)
{
	nand->port_size = nand_get_port_size();

	nand->page_size = nand_get_page_size();

	/*
	 * Set Bad marker Location for LP / SP
	 * Small Page : 8 Bit	 : 0x5
	 * Small Page : 16 Bit	: 0xa
	 * Large Page : 8 /16 Bit : 0x0
	 */
	nand->bad_marker_loc = (nand->page_size == 512) ?
				((nand->port_size == 8) ? 0x5 : 0xa) : 0;

	/* NAND Blk serached count for incremental Bad block search cnt */
	nand->bbs = 0;

	/* pages per Block */
	nand->ppb = nand_get_pages_per_blk();

	/* Blk size */
	nand->blk_size = nand->page_size * nand->ppb;

	/* get_page_index_width */
	nand->pi_width = get_page_index_width(nand->ppb);

	/* bad block table init */
	nand->lgb = 0;
	nand->bbt_max = 0;
	nand->bzero_good = 0;
	memset(nand->bbt, EMPTY_VAL, BBT_SIZE * sizeof(nand->bbt[0]));
}

/**************************************************************************
 * Function	:	nand_init()
 * Arguments	:	nand_ptr - nand structure pointer
 *			a - dummy variable to keep same structure
 * Return	:	0 or Error Code
 * Description	:	It clears BOOT events, disable autoboot and
 *			initializes nand structure.
 **************************************************************************/
int nand_init(void *nand_ptr)
{
	uint32_t ncfgr = 0;
	struct nand_info *nand;

	nand = (struct nand_info *)nand_ptr;

	/* Get nand Parameters from IFC */
	nand_get_params(nand);

	/* Clear all errors */
	write_reg(NAND_EVTER_STAT, 0xffffffff);

	/*
	 * Disable autoboot in NCFGR. Mapping will change from
	 * physical to logical for SRAM buffer
	 */
	ncfgr = read_reg(NCFGR);
	write_reg(NCFGR, (ncfgr & ~NCFGR_BOOT));

	return 0;
}

/**************************************************************************
 * Function	:	nand_read_data()
 * Arguments	:	row_add - page address and block address
 *			col_add - offset within a page
 *			byte_cnt - number of bytes to be read
 *			data - pointer to destination location where data
 *			needs to write.
 *			main_spare - flag to indicate whether need to read
 *			from main region or spare region.
 *			nand - nand structure pointer
 * Return	:	0 or Error Code
 * Description	:	It programs nand registers and start read operation
 *			and finally write data to destination location from
 *			IFC internal buffer.
 ************************************************************************/
static int nand_read_data(
		uint32_t row_add,
		uint32_t col_add,
		uint32_t byte_cnt,
		uint8_t *data,
		uint32_t main_spare,
		struct nand_info *nand)
{
	uint32_t base_addr, page_size_add_bits = 0;
	uint32_t page_add_in_actual, page_add;
	uint32_t sram_addr_calc;
	int ret;
	uint32_t col_val;

	base_addr = NXP_IFC_REGION_ADDR;

	 /* Programming MS bit to read from spare area. */
	col_val = (main_spare << NAND_COL_MS_SHIFT) | col_add;

	write_reg(NAND_BC, byte_cnt);

	write_reg(ROW0, row_add);
	write_reg(COL0, col_val);

	 /* Program FCR for small Page */
	if (nand->page_size == 512) {
		if (byte_cnt == 0 ||
			(byte_cnt != 0  && main_spare == 0 && col_add <= 255)) {
			write_reg(NAND_FCR0,
				  (NAND_CMD_READ0 << FCR_CMD0_SHIFT));
		} else if (main_spare == 0) {
			write_reg(NAND_FCR0,
				  (NAND_CMD_READ1 << FCR_CMD0_SHIFT));
		} else {
			write_reg(NAND_FCR0,
				  (NAND_CMD_READOOB << FCR_CMD0_SHIFT));
		}

	} else { /* Program FCR for Large Page */
		write_reg(NAND_FCR0, (NAND_CMD_READ0 << FCR_CMD0_SHIFT) |
			  (NAND_CMD_READSTART << FCR_CMD1_SHIFT));
	}
	if (nand->page_size == 512) {
		write_reg(NAND_FIR0, ((FIR_OP_CW0 << FIR_OP0_SHIFT) |
					  (FIR_OP_CA0 << FIR_OP1_SHIFT) |
					  (FIR_OP_RA0 << FIR_OP2_SHIFT) |
					  (FIR_OP_BTRD << FIR_OP3_SHIFT) |
					  (FIR_OP_NOP << FIR_OP4_SHIFT)));
		write_reg(NAND_FIR1, 0x00000000);
	} else {
		write_reg(NAND_FIR0, ((FIR_OP_CW0 << FIR_OP0_SHIFT) |
					 (FIR_OP_CA0 << FIR_OP1_SHIFT) |
					 (FIR_OP_RA0 << FIR_OP2_SHIFT) |
					 (FIR_OP_CMD1 << FIR_OP3_SHIFT) |
					 (FIR_OP_BTRD << FIR_OP4_SHIFT)));

		write_reg(NAND_FIR1, (FIR_OP_NOP << FIR_OP5_SHIFT));
	}
	write_reg(NANDSEQ_STRT, NAND_SEQ_STRT_FIR_STRT);

	ret = nand_wait();
	if (ret != 0)
		return ret;

	 /* calculate page_size_add_bits i.e bits in sram address
	  * corresponding to area  within a page for sram
	  */
	if (nand->page_size == 512)
		page_size_add_bits = 10;
	else if (nand->page_size == 2048)
		page_size_add_bits = 12;
	else if (nand->page_size == 4096)
		page_size_add_bits = 13;
	else if (nand->page_size == 8192)
		page_size_add_bits = 14;
	else if (nand->page_size == 16384)
		page_size_add_bits = 15;

	page_add = row_add;

	page_add_in_actual = (page_add << page_size_add_bits) & 0x0000FFFF;

	if (byte_cnt == 0)
		col_add = 0;

	 /* Calculate SRAM address for main and spare area */
	if (main_spare == 0)
		sram_addr_calc = base_addr | page_add_in_actual | col_add;
	else
		sram_addr_calc = base_addr | page_add_in_actual |
				 (col_add + nand->page_size);

	 /* Depending Byte_count copy full page or partial page from SRAM */
	if (byte_cnt == 0)
		memcpy(data, (void *)(uintptr_t)sram_addr_calc,
			nand->page_size);
	else
		memcpy(data, (void *)(uintptr_t)sram_addr_calc, byte_cnt);

	return 0;
}

/**************************************************************************
 * Function	:	nand_read()
 * Arguments	:	src_addr - address from where data to be read
 *			dst - address where data after reading need to write
 *			size - bytes of data to be read
 *			nand_ptr - nand structure pointer
 *  Return	:	0 or Error Code
 *  Description	:	It calculates page number and block number for read
 *			operation, updates bad block table, and finally
 *			writes data to destination location.
 **************************************************************************/
int nand_read(uint32_t src_addr, uintptr_t dst, uint32_t size)
{
	uint32_t log_blk = 0;
	uint32_t pg_no = 0;
	uint32_t col_off = 0;
	uint32_t row_off = 0;
	uint32_t byte_cnt = 0;
	uint32_t read_cnt = 0;
	uint32_t i = 0;
	uint32_t updated = 0;

	int ret = 0;
	uint8_t *out = (uint8_t *)dst;

	uint32_t pblk;

	struct nand_info *nand = &nand_drv_data;

	 /* loop till size */
	while (size) {
		log_blk = (src_addr / nand->blk_size);
		pg_no = ((src_addr - (log_blk * nand->blk_size)) /
					 nand->page_size);
		pblk = log_blk;

		for (i = 0; i <= nand->bbt_max; i++) {
			if (nand->bbt[i] == EMPTY_VAL) {
				ret = update_bbt(i, pblk, &updated, nand);

				if (ret != 0)
					return ret;
				 /*
				  * if table not updated and we reached
				  * end of table
				  */
				if (!updated)
					break;
			}

			if (pblk < nand->bbt[i])
				break;
			else if (pblk >= nand->bbt[i])
				pblk++;
		}

		col_off = (src_addr % nand->page_size);
		if (col_off) {
			if ((col_off + size) < nand->page_size)
				byte_cnt = size;
			else
				byte_cnt = nand->page_size - col_off;

			row_off = (pblk << nand->pi_width) | pg_no;

			ret = nand_read_data(
					row_off,
					col_off,
					byte_cnt, out, MAIN, nand);

			if (ret != 0)
				return ret;
		} else {
			 /*
			  * fullpage/Partial Page
			  * if byte_cnt = 0 full page
			  * else partial page
			  */
			if (size < nand->page_size) {
				byte_cnt = size;
				read_cnt = size;
			} else	{
				byte_cnt = nand->page_size;
				read_cnt = 0;
			}
			row_off = (pblk << nand->pi_width) | pg_no;

			ret = nand_read_data(
					row_off,
					0,
					read_cnt, out, MAIN, nand);

			if (ret != 0) {
				ERROR("Error from nand-read_data %d\n", ret);
				return ret;
			}
		}
		src_addr += byte_cnt;
		out += byte_cnt;
		size -= byte_cnt;
	}
	return 0;
}

/**************************************************************************
 * Function	:	isgoodblock()
 * Arguments	:	blk - block number
 *			gb - pointer to location containing good block number
 *			nand - nand structure pointer
 * Return	:	0 or Error Code
 * Description	:	It reads page 0 and page 1(NON-ONFI)/last page(ONFI)
 *			if both are good then it stores block as good block
 **************************************************************************/
static int isgoodblock(uint32_t blk, uint32_t *gb, struct nand_info *nand)
{
	uint8_t buf[2];
	int ret;
	uint32_t row_add;

	*gb = 0;

	 /* read Page 0 of blk */
	ret = nand_read_data(
			blk << nand->pi_width,
			nand->bad_marker_loc,
			0x2, buf, 1, nand);

	if (ret != 0)
		return ret;

	 /* For ONFI devices check Page 0 and Last page of block for
	  * Bad Marker and for NON-ONFI Page 0 and 1 for Bad Marker
	  */
	row_add = (blk << nand->pi_width);
	if (nand->port_size == 8) {
		/* port size is 8 Bit
		 * check if page 0 has 0xff
		 */
		if (buf[0] == 0xff) {
			 /* check page 1 */
			if (nand->onfi_dev_flag)
				ret =  nand_read_data(
						row_add | (nand->ppb - 1),
						nand->bad_marker_loc,
						0x2, buf, SPARE, nand);
			else
				ret =  nand_read_data(
						row_add | 1,
						nand->bad_marker_loc,
						0x2, buf, SPARE, nand);

			if (ret != 0)
				return ret;

			if (buf[0] == 0xff)
				*gb = GOOD_BLK;
			else
				*gb = BAD_BLK;
		} else {/* no, so it is bad blk */
			*gb = BAD_BLK;
		}
	} else {
		/* Port size 16-Bit
		 * check if page 0 has 0xffff
		 */
		if ((buf[0] == 0xff) &&
			(buf[1] == 0xff)) {
			 /* check page 1 for 0xffff */
			if (nand->onfi_dev_flag) {
				ret =  nand_read_data(
						row_add | (nand->ppb - 1),
						nand->bad_marker_loc,
						0x2, buf, SPARE, nand);
			} else {
				ret =  nand_read_data(
						row_add | 1,
						nand->bad_marker_loc,
						0x2, buf, SPARE, nand);
			}

			if (ret != 0)
				return ret;

			if ((buf[0] == 0xff) &&
				(buf[1] == 0xff)) {
				*gb = GOOD_BLK;
			} else {
				*gb = BAD_BLK;
			}
		} else { /* no, so it is goodblk */
			*gb = BAD_BLK;
		}
	}
	return 0;
}

/**************************************************************************
 * Function	:	update_bbt()
 * Arguments	:	idx - index of entry in bbt table
 *			blk - block from which data to be read
 *			updated - flag to indicate if table is updated or not
 *			nand - nand structure pointer
 * Return	:	0 or Error Code
 * Description	:	It scans all blocks from last good block till blk
 *			and update bbt for each of them.
 *			Returns 0 if table is not updated.
 *			It updates bbt in incremental manner only.
 ***************************************************************************/
static int update_bbt(uint32_t idx, uint32_t blk,
			   uint32_t *updated,  struct nand_info *nand)
{
	uint32_t sblk; /* search block */
	uint32_t lgb;
	int ret;

	if (nand->bzero_good && blk == 0)
		return 0;

	 /* special case for lgb == 0
	  * if blk <= lgb retrun
	  */
	if (nand->lgb != 0 && blk <= nand->lgb)
		return 0;

	*updated = 0;

	 /* if blk is more than lgb, iterate from lgb till a good block
	  * is found for blk
	  */

	if (nand->lgb < blk)
		sblk = nand->lgb;
	else   /* this is when lgb = 0 */
		sblk = blk;


	lgb = nand->lgb;

	 /* loop from blk to find a good block */
	while (1) {
		while (lgb <= sblk) {
			uint32_t gb = 0;

			ret =  isgoodblock(lgb, &gb, nand);
			if (ret != 0)
				return ret;

			 /* special case block 0 is good then set this flag */
			if (lgb == 0 && gb == 1)
				nand->bzero_good = 1;

			if (!gb) {
				if (idx >= BBT_SIZE) {
					ERROR("NAND BBT Table full\n");
					return -1;
				}
				*updated = 1;
				nand->bbt[idx] = lgb;
				idx++;
				blk++;
				sblk++;
				if (idx > nand->bbt_max)
					nand->bbt_max = idx;
			}
			lgb++;
		}
		 /* the access block found */
		if (sblk == blk) {
			 /* when good block found update lgb */
			nand->lgb =  blk;
			break;
		}
		sblk++;
	}

	return 0;
}

int nand_drv_init(void)
{
	struct nand_info *nand = NULL;
	int ret;

	nand = &nand_drv_data;
	memset(nand, 0, sizeof(struct nand_info));

	INFO("nand_init\n");
	ret = nand_init(nand);
	return ret;
}

static size_t ifc_nand_read(int lba, uintptr_t buf, size_t size)
{
	int ret;
	uint32_t page_size = nand_get_page_size();
	uint32_t src_addr;

	src_addr = lba * page_size;
	ret = nand_read(src_addr, buf, size);
	return ret ? 0 : size;
}

static struct io_block_dev_spec ifc_nand_spec = {
	.buffer = {
		.offset = NXP_SD_BLOCK_BUF_ADDR,
		.length = NXP_SD_BLOCK_BUF_SIZE,
	},
	.ops = {
		.read = ifc_nand_read,
	},
	/*
	 * Default block size assumed as 2K
	 * Would be updated based on actual size
	 */
	.block_size = 2048,
};

int ifc_nand_io_setup(void)
{
	uintptr_t block_dev_spec;
	int ret;

	ret = nand_drv_init();
	if (ret)
		return ret;

	ifc_nand_spec.block_size = nand_get_page_size();

	INFO("Page size is %ld\n", ifc_nand_spec.block_size);

	block_dev_spec = (uintptr_t)&ifc_nand_spec;

	/* Adding NAND SRAM< Buffer in XLAT Table */
	mmap_add_region(NXP_IFC_REGION_ADDR, NXP_IFC_REGION_ADDR,
			NXP_IFC_SRAM_BUFFER_SIZE, MT_DEVICE | MT_RW);

	return plat_io_block_setup(PLAT_FIP_OFFSET, block_dev_spec);
}

