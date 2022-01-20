/*
 * Copyright 2022 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef IFC_H
#define IFC_H

#include <endian.h>

#include <mmio.h>

#define NXP_IFC_RUN_TIME_ADDR	U(0x1000)

/* CPSR - Chip Select Property Register Offset */
#define EXT_CSPR(n)		(U(0x000C) + (n * 0xC))
#define CSPR(n)			(U(0x0010) + (n * 0xC))
#define CSOR(n)			(U(0x0130) + (n * 0xC))
#define EXT_CSOR(n)		(U(0x0134) + (n * 0xC))
#define IFC_AMASK_CS0		U(0x00A0)

/* NAND specific Registers Offset */
#define NCFGR			(NXP_IFC_RUN_TIME_ADDR + U(0x0000))
#define NAND_FCR0		(NXP_IFC_RUN_TIME_ADDR + U(0x0014))

#define ROW0			(NXP_IFC_RUN_TIME_ADDR + U(0x003C))
#define ROW1			(NXP_IFC_RUN_TIME_ADDR + U(0x004C))
#define COL0			(NXP_IFC_RUN_TIME_ADDR + U(0x0044))
#define COL1			(NXP_IFC_RUN_TIME_ADDR + U(0x0054))

#define NAND_BC			(NXP_IFC_RUN_TIME_ADDR + U(0x0108))
#define NAND_FIR0		(NXP_IFC_RUN_TIME_ADDR + U(0x0110))
#define NAND_FIR1		(NXP_IFC_RUN_TIME_ADDR + U(0x0114))
#define NAND_FIR2		(NXP_IFC_RUN_TIME_ADDR + U(0x0118))
#define NAND_CSEL		(NXP_IFC_RUN_TIME_ADDR + U(0x015C))
#define NANDSEQ_STRT		(NXP_IFC_RUN_TIME_ADDR + U(0x0164))
#define NAND_EVTER_STAT		(NXP_IFC_RUN_TIME_ADDR + U(0x016C))
#define NAND_AUTOBOOT_TRGR	(NXP_IFC_RUN_TIME_ADDR + U(0x0284))

/* Size of SRAM Buffer */
#define CSPR_PS			U(0x00000180)
#define CSPR_PS_SHIFT		7
#define CSPR_PS_8		0x1 // Port Size 8 bit
#define CSPR_PS_16		0x2 // Port Size 16 bit
#define CSPR_PS_32		0x3 // Port Size 32 bit

/* Chip Select Option Register NAND Machine */
#define CSOR_NAND_PGS		U(0x00380000)
#define CSOR_NAND_PGS_SHIFT	19
#define CSOR_NAND_PGS_512	U(0x00000000)
#define CSOR_NAND_PGS_2K	U(0x00080000)
#define CSOR_NAND_PGS_4K	U(0x00100000)
#define CSOR_NAND_PGS_8K	U(0x00180000)
#define CSOR_NAND_PGS_16K	U(0x00200000)


#define CSOR_NAND_PB			U(0x00000700)
#define CSOR_NAND_PB_32			U(0x00000000)
#define CSOR_NAND_PB_64			U(0x00000100)
#define CSOR_NAND_PB_128		U(0x00000200)
#define CSOR_NAND_PB_256		U(0x00000300)
#define CSOR_NAND_PB_512		U(0x00000400)
#define CSOR_NAND_PB_1024		U(0x00000500)
#define CSOR_NAND_PB_2048		U(0x00000600)
#define CSOR_NAND_PPB_32		32
#define CSOR_NAND_PPB_64		64
#define CSOR_NAND_PPB_128		128
#define CSOR_NAND_PPB_256		256
#define CSOR_NAND_PPB_512		512
#define CSOR_NAND_PPB_1024		1024
#define CSOR_NAND_PPB_2048		2048

/* NAND Chip select register */
#define NAND_CSEL_SHIFT			26
#define NAND_COL_MS_SHIFT		31

/* FCR - Flash Command Register */
#define FCR_CMD0			U(0xFF000000)
#define FCR_CMD0_SHIFT			24
#define FCR_CMD1			U(0x00FF0000)
#define FCR_CMD1_SHIFT			16
#define FCR_CMD2			U(0x0000FF00)
#define FCR_CMD2_SHIFT			8
#define FCR_CMD3			U(0x000000FF)
#define FCR_CMD3_SHIFT			0

/* FIR - Flash Instruction Register Opcode */
#define FIR_OP0				U(0xFC000000)
#define FIR_OP0_SHIFT			26
#define FIR_OP1				U(0x03F00000)
#define FIR_OP1_SHIFT			20
#define FIR_OP2				U(0x000FC000)
#define FIR_OP2_SHIFT			14
#define FIR_OP3				U(0x00003F00)
#define FIR_OP3_SHIFT			8
#define FIR_OP4				U(0x000000FC)
#define FIR_OP4_SHIFT			2
#define FIR_OP5				U(0xFC000000)
#define FIR_OP5_SHIFT			26
#define FIR_OP6				U(0x03F00000)
#define FIR_OP6_SHIFT			20

/* Instruction Opcode - 6 bits */
#define FIR_OP_NOP			0x00
#define FIR_OP_CA0			0x01 /* Issue current column address */
#define FIR_OP_CA1			0x02 /* Issue current column address */
#define FIR_OP_RA0			0x05 /* Issue current column address */
#define FIR_OP_RA1			0x06 /* Issue current column address */
#define FIR_OP_CMD0			0x09 /* Issue command from FCR[CMD0] */
#define FIR_OP_CMD1			0x0a /* Issue command from FCR[CMD1] */
#define FIR_OP_CMD2			0x0b /* Issue command from FCR[CMD2] */
#define FIR_OP_CMD3			0x0c /* Issue command from FCR[CMD3] */
#define FIR_OP_CW0			0x11 /* Wait then issue FCR[CMD0] */
#define FIR_OP_CW1			0x12 /* Wait then issue FCR[CMD1] */
#define FIR_OP_CW2			0x13 /* Wait then issue FCR[CMD1] */
#define FIR_OP_CW3			0x14 /* Wait then issue FCR[CMD1] */
#define FIR_OP_WBCD			0x19 /* Wait then read FBCR bytes */
#define FIR_OP_RBCD			0x1a /* Wait then read 1 or 2 bytes */
#define FIR_OP_BTRD			0x1b /* Wait then read 1 or 2 bytes */
#define FIR_OP_RDSTAT			0x1c /* Wait then read 1 or 2 bytes */
#define FIR_OP_NWAIT			0x1d /* Wait then read 1 or 2 bytes */
#define FIR_OP_WFR			0x1e /* Wait then read 1 or 2 bytes */

#define NAND_SEQ_STRT_FIR_STRT		U(0x80000000)
#define NAND_SEQ_STRT_FIR_STRT_SHIFT	31

#define NAND_EVTER_STAT_FTOER		U(0x08000000)
#define NAND_EVTER_STAT_WPER		U(0x04000000)
#define NAND_EVTER_STAT_ECCER		U(0x02000000)
#define NAND_EVTER_STAT_DQSER		U(0x01000000)
#define NAND_EVTER_STAT_RCW_DN		U(0x00008000)
#define NAND_EVTER_STAT_BOOT_DN		U(0x00004000)
#define NAND_EVTER_STAT_RCW_DN		U(0x00008000)
#define NAND_EVTER_STAT_OPC_DN		U(0x80000000)
#define NAND_EVTER_STAT_BBI_SRCH_SEL	U(0x00000800)
#define NCFGR_BOOT			U(0x80000000)
#define NAND_AUTOBOOT_TRGR_RCW_LD	U(0x80000000)
#define NAND_AUTOBOOT_TRGR_BOOT_LD	U(0x20000000)

/* ECC ERROR STATUS Registers */
#define NAND_RCW_LD			U(0x80000000)
#define NAND_BOOT_LD			U(0x20000000)

/*Other Temp Defines */
/*256 bad Blocks supported */
#define BBT_SIZE			256

/*Standard NAND flash commands */
#define NAND_CMD_READ0			0
#define NAND_CMD_READ1			1
#define NAND_CMD_READOOB		0x50

/*Extended commands for large page devices */
#define NAND_CMD_READSTART		0x30

#define NAND_TIMEOUT_MS			40

#define EMPTY_VAL_CHECK			U(0xFFFFFFFF)
#define EMPTY_VAL			0xFF


#define MAIN				0
#define SPARE				1

#define GOOD_BLK			1
#define BAD_BLK				0
#define DIV_2				2

#define ATTRIBUTE_PGSZ			0xa
#define ATTRIBUTE_PPB			0xb

#define CSPR_PORT_SIZE_8		(0x1 << 7)
#define CSPR_PORT_SIZE_16		(0x2 << 7)
#define CSPR_PORT_SIZE_32		(0x3 << 7)

/* NAND specific */
#define RCW_SRC_NAND_PORT_MASK		U(0x00000080)

#define NAND_DEFAULT_CSPR		U(0x00000053)
#define NAND_DEFAULT_CSOR		U(0x0180C00C)
#define NAND_DEFAULT_EXT_CSPR		U(0x00000000)
#define NAND_DEFAULT_EXT_CSOR		U(0x00000000)
#define NAND_DEFAULT_FTIM0		U(0x181c0c10)
#define NAND_DEFAULT_FTIM1		U(0x5454141e)
#define NAND_DEFAULT_FTIM2		U(0x03808034)
#define NAND_DEFAULT_FTIM3		U(0x2c000000)

#define NAND_CSOR_ECC_MODE_DISABLE	U(0x00000000)
#define NAND_CSOR_ECC_MODE0		U(0x84000000)
#define NAND_CSOR_ECC_MODE1		U(0x94000000)
#define NAND_CSOR_ECC_MODE2		U(0xa4000000)
#define NAND_CSOR_ECC_MODE3		U(0xb4000000)
#define NAND_CSOR_PAGE_SIZE_2K		(0x1 << 19)
#define NAND_CSOR_PAGE_SIZE_4K		(0x2 << 19)
#define NAND_CSOR_PAGE_SIZE_8K		(0x3 << 19)
#define NAND_CSOR_PAGE_SIZE_16K		(0x4 << 19)
#define NAND_CSOR_PPB_64		(0x1 << 8)
#define NAND_CSOR_PPB_128		(0x2 << 8)
#define NAND_CSOR_PPB_256		(0x3 << 8)
#define NAND_CSOR_PPB_512		(0x4 << 8)

/* BBI INDICATOR for NAND_2K(CFG_RCW_SRC[1]) for
 * devices greater than 2K page size(CFG_RCW_SRC[3])
 */
#define RCW_SRC_NAND_BBI_MASK		U(0x00000008)
#define RCW_SRC_NAND_BBI_MASK_NAND_2K	U(0x00000002)
#define NAND_BBI_ONFI_2K		(0x1 << 1)
#define NAND_BBI_ONFI			(0x1 << 3)

#define RCW_SRC_NAND_PAGE_MASK		U(0x00000070)
#define RCW_SRC_NAND_PAGE_MASK_NAND_2K	U(0x0000000C)
#define NAND_2K_XXX			0x00
#define NAND_2K_64			0x04
#define NAND_2K_128			0x08
#define NAND_4K_128			0x10
#define NAND_4K_256			0x20
#define NAND_4K_512			0x30
#define NAND_8K_128			0x40
#define NAND_8K_256			0x50
#define NAND_8K_512			0x60
#define NAND_16K_512			0x70
#define BLOCK_LEN_2K			2048

#define RCW_SRC_NAND_ECC_MASK		U(0x00000007)
#define RCW_SRC_NAND_ECC_MASK_NAND_2K	U(0x00000001)
#define NAND_ECC_DISABLE		0x0
#define NAND_ECC_4_520			0x1
#define NAND_ECC_8_528			0x5
#define NAND_ECC_24_1K			0x6
#define NAND_ECC_40_1K			0x7

#define NAND_SPARE_2K			U(0x00000040)
#define NAND_SPARE_4K_ECC_M0		U(0x00000080)
#define NAND_SPARE_4K_ECC_M1		U(0x000000D2)
#define NAND_SPARE_4K_ECC_M2		U(0x000000B0)
#define NAND_SPARE_4K_ECC_M3		U(0x00000120)
#define NAND_SPARE_8K_ECC_M0		U(0x00000088)
#define NAND_SPARE_8K_ECC_M1		U(0x00000108)
#define NAND_SPARE_8K_ECC_M2		U(0x00000158)
#define NAND_SPARE_8K_ECC_M3		U(0x00000238)
#define NAND_SPARE_16K_ECC_M0		U(0x00000108)
#define NAND_SPARE_16K_ECC_M1		U(0x00000208)
#define NAND_SPARE_16K_ECC_M2		U(0x000002A8)
#define NAND_SPARE_16K_ECC_M3		U(0x00000468)

struct nand_info {
	uintptr_t ifc_register_addr;
	uintptr_t ifc_region_addr;
	uint32_t page_size;
	uint32_t port_size;
	uint32_t blk_size;
	uint32_t ppb;
	uint32_t pi_width;	/* Bits Required to index a page in block */
	uint32_t ral;
	uint32_t ibr_flow;
	uint32_t bbt[BBT_SIZE];
	uint32_t lgb;		/* Last Good Block */
	uint32_t bbt_max;	/* Total entries in bbt */
	uint32_t bzero_good;
	uint8_t bbs;
	uint8_t bad_marker_loc;
	uint8_t onfi_dev_flag;
	uint8_t init_time_boot_flag;
	uint8_t *buf;
};

struct ifc_regs {
	uint32_t ext_cspr;
	uint32_t cspr;
	uint32_t csor;
	uint32_t ext_csor;
};

struct sec_nand_info {
	uint32_t cspr_port_size;
	uint32_t csor_ecc_mode;
	uint32_t csor_page_size;
	uint32_t csor_ppb;
	uint32_t ext_csor_spare_size;
	uint32_t onfi_flag;
};

struct sec_nor_info {
	uint32_t cspr_port_size;
	uint32_t csor_nor_mode;
	uint32_t csor_adm_shift;
	uint32_t port_size;
	uint32_t addr_bits;
};

enum ifc_chip_sel {
	IFC_CS0,
	IFC_CS1,
	IFC_CS2,
	IFC_CS3,
	IFC_CS4,
	IFC_CS5,
	IFC_CS6,
	IFC_CS7,
};

enum ifc_ftims {
	IFC_FTIM0,
	IFC_FTIM1,
	IFC_FTIM2,
	IFC_FTIM3,
};

#ifdef NXP_IFC_BE
#define nand_in32(a)		bswap32(mmio_read_32((uintptr_t)a))
#define nand_out32(a, v)	mmio_write_32((uintptr_t)a, bswap32(v))
#else
#define nand_in32(a)		mmio_read_32((uintptr_t)a)
#define nand_out32(a, v)	mmio_write_32((uintptr_t)a, v)
#endif

/* Read Write on IFC registers */
static inline void write_reg(struct nand_info *nand, uint32_t reg, uint32_t val)
{
	nand_out32(nand->ifc_register_addr + reg, val);
}

static inline uint32_t read_reg(struct nand_info *nand, uint32_t reg)
{
	return nand_in32(nand->ifc_register_addr + reg);
}

#endif /* IFC_H */
