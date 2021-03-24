/*
 * Copyright 2021 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <getopt.h>
#include <unistd.h>

#define NUM_MEM_BLOCK		1
#define FOUR_BYTE_ALIGN		4
#define EIGHT_BYTE_ALIGN	8
#define SIZE_TWO_PBL_CMD	24

/* Define for add_boot_ptr_cmd() */
#define BOOTPTR_ADDR 0x09570604
#define CSF_ADDR_SB 0x09ee0200
/* CCSR write command to address 0x1e00400 i.e BOOTLOCPTR */
#define BOOTPTR_ADDR_CH3 0x31e00400
/* Load CSF header command */
#define CSF_ADDR_SB_CH3 0x80220000

#define	MAND_ARG_MASK				0xFFF3
#define	ARG_INIT_MASK				0xFF00
#define RCW_FILE_NAME_ARG_MASK			0x0080
#define IN_FILE_NAME_ARG_MASK			0x0040
#define CHASSIS_ARG_MASK			0x0020
#define BOOT_SRC_ARG_MASK			0x0010
#define ENTRY_POINT_ADDR_ARG_MASK		0x0008
#define BL2_BIN_STRG_LOC_BOOT_SRC_ARG_MASK	0x0004
#define BL2_BIN_CPY_DEST_ADDR_ARG_MASK		0x0002
#define OP_FILE_NAME_ARG_MASK			0x0001

/* Define for add_cpy_cmd() */
#define OFFSET_MASK		        0x00ffffff
#define WRITE_CMD_BASE		    0x81000000
#define MAX_PBI_DATA_LEN_BYTE	64

/* 140 Bytes = Preamble + LOAD RCW command + RCW (128 bytes) + Checksum */
#define CHS3_CRC_PAYLOAD_START_OFFSET 140

#define PBI_CRC_POLYNOMIAL	0x04c11db7

typedef enum {
	CHASSIS_UNKNOWN,
	CHASSIS_2,
	CHASSIS_3,
	CHASSIS_3_2,
	CHASSIS_MAX    /* must be last item in list */
} chassis_t;

typedef enum {
	UNKNOWN_BOOT = 0,
	IFC_NOR_BOOT,
	IFC_NAND_BOOT,
	QSPI_BOOT,
	SD_BOOT,
	EMMC_BOOT,
	FLXSPI_NOR_BOOT,
	FLXSPI_NAND_BOOT,
	FLXSPI_NAND4K_BOOT,
	MAX_BOOT    /* must be last item in list */
} boot_src_t;

/* Base Addresses where PBL image is copied depending on the boot source.
 * Boot address map varies as per Chassis architecture.
 */
#define BASE_ADDR_UNDEFINED  0xFFFFFFFF
#define BASE_ADDR_QSPI       0x20000000
#define BASE_ADDR_SD         0x00001000
#define BASE_ADDR_IFC_NOR    0x30000000
#define BASE_ADDR_EMMC       0x00001000
#define BASE_ADDR_FLX_NOR    0x20000000
#define BASE_ADDR_NAND       0x20000000

uint32_t base_addr_ch3[MAX_BOOT] = {
	BASE_ADDR_UNDEFINED,
	BASE_ADDR_IFC_NOR,
	BASE_ADDR_UNDEFINED,	/*IFC NAND */
	BASE_ADDR_QSPI,
	BASE_ADDR_SD,
	BASE_ADDR_EMMC,
	BASE_ADDR_UNDEFINED,	/*FLXSPI NOR */
	BASE_ADDR_UNDEFINED,	/*FLXSPI NAND 2K */
	BASE_ADDR_UNDEFINED	/*FLXSPI NAND 4K */
};

uint32_t base_addr_ch32[MAX_BOOT] = {
	BASE_ADDR_UNDEFINED,
	BASE_ADDR_UNDEFINED,	/* IFC NOR */
	BASE_ADDR_UNDEFINED,	/* IFC NAND */
	BASE_ADDR_UNDEFINED,	/* QSPI */
	BASE_ADDR_SD,
	BASE_ADDR_EMMC,
	BASE_ADDR_FLX_NOR,
	BASE_ADDR_UNDEFINED,	/*FLXSPI NAND 2K */
	BASE_ADDR_UNDEFINED	/*FLXSPI NAND 4K */
};

/* for Chassis 3 */
uint32_t blk_cpy_hdr_map_ch3[] = {

	0,		    /* Unknown Boot Source */
	0x80000020,	/* NOR_BOOT */
	0x0,		/* NAND_BOOT */
	0x80000062,	/* QSPI_BOOT */
	0x80000040,	/* SD_BOOT */
	0x80000041,	/* EMMC_BOOT */
	0x0,		/* FLEXSPI NOR_BOOT */
	0x0,	/* FLEX SPI NAND2K BOOT */
	0x0,	/* CHASIS3_2_NAND4K_BOOT */
};

uint32_t blk_cpy_hdr_map_ch32[] = {
	0,		    /* Unknown Boot Source */
	0x0,		/* NOR_BOOT */
	0x0,		/* NAND_BOOT */
	0x0,		/* QSPI_BOOT */
	0x80000008,	/* SD_BOOT */
	0x80000009,	/* EMMC_BOOT */
	0x8000000F,	/* FLEXSPI NOR_BOOT */
	0x8000000C,	/* FLEX SPI NAND2K BOOT */
	0x8000000D,	/* CHASIS3_2_NAND4K_BOOT */
};

char *boot_src_string[] = {
	"UNKNOWN_BOOT",
	"IFC_NOR_BOOT",
	"IFC_NAND_BOOT",
	"QSPI_BOOT",
	"SD_BOOT",
	"EMMC_BOOT",
	"FLXSPI_NOR_BOOT",
	"FLXSPI_NAND_BOOT",
	"FLXSPI_NAND4K_BOOT",
};

enum stop_command {
	STOP_COMMAND = 0,
	CRC_STOP_COMMAND
};

/* Structure will get populated in the main function
 * as part of parsing the command line arguments.
 * All member parameters are mandatory except:
 *	-ep
 *	-src_addr
 */
struct pbl_image {
	char *rcw_nm;		/* Input RCW File */
	char *sec_imgnm;	/* Input BL2 binary */
	char *imagefile;	/* Generated output file */
	boot_src_t boot_src;	/* Boot Source - QSPI, SD, NOR, NAND etc */
	uint32_t src_addr;	/* Source Address */
	uint32_t addr;		/* Load address */
	uint32_t ep;		/* Entry point <opt> default is load address */
	chassis_t chassis;	/* Chassis type */
} pblimg;

#define SUCCESS			 0
#define FAILURE			-1
#define CRC_STOP_CMD_ARM	0x08610040
#define CRC_STOP_CMD_ARM_CH3	0x808f0000
#define STOP_CMD_ARM_CH3	0x80ff0000
#define BYTE_SWAP_32(word)	((((word) & 0xff000000) >> 24)|	\
				(((word) & 0x00ff0000) >>  8) |	\
				(((word) & 0x0000ff00) <<  8) |	\
				(((word) & 0x000000ff) << 24))

#define PBI_LEN_MASK	0xFFF00000
#define PBI_LEN_SHIFT	20
#define NUM_RCW_WORD	35
#define PBI_LEN_ADD		6

#define MAX_CRC_ENTRIES 256

/* SoC numeric identifier */
#define SOC_LS1012 1012
#define SOC_LS1023 1023
#define SOC_LS1026 1026
#define SOC_LS1028 1028
#define SOC_LS1043 1043
#define SOC_LS1046 1046
#define SOC_LS1088 1088
#define SOC_LS2080 2080
#define SOC_LS2088 2088
#define SOC_LX2160 2160

static uint32_t pbl_size;
bool sb_flag;

/***************************************************************************
 * Description	:	CRC32 Lookup Table
 ***************************************************************************/
static uint32_t crc32_lookup[] = {
	 0x00000000, 0x77073096, 0xEE0E612C, 0x990951BA,
	 0x076DC419, 0x706AF48F, 0xE963A535, 0x9E6495A3,
	 0x0EDB8832, 0x79DCB8A4, 0xE0D5E91E, 0x97D2D988,
	 0x09B64C2B, 0x7EB17CBD, 0xE7B82D07, 0x90BF1D91,
	 0x1DB71064, 0x6AB020F2, 0xF3B97148, 0x84BE41DE,
	 0x1ADAD47D, 0x6DDDE4EB, 0xF4D4B551, 0x83D385C7,
	 0x136C9856, 0x646BA8C0, 0xFD62F97A, 0x8A65C9EC,
	 0x14015C4F, 0x63066CD9, 0xFA0F3D63, 0x8D080DF5,
	 0x3B6E20C8, 0x4C69105E, 0xD56041E4, 0xA2677172,
	 0x3C03E4D1, 0x4B04D447, 0xD20D85FD, 0xA50AB56B,
	 0x35B5A8FA, 0x42B2986C, 0xDBBBC9D6, 0xACBCF940,
	 0x32D86CE3, 0x45DF5C75, 0xDCD60DCF, 0xABD13D59,
	 0x26D930AC, 0x51DE003A, 0xC8D75180, 0xBFD06116,
	 0x21B4F4B5, 0x56B3C423, 0xCFBA9599, 0xB8BDA50F,
	 0x2802B89E, 0x5F058808, 0xC60CD9B2, 0xB10BE924,
	 0x2F6F7C87, 0x58684C11, 0xC1611DAB, 0xB6662D3D,
	 0x76DC4190, 0x01DB7106, 0x98D220BC, 0xEFD5102A,
	 0x71B18589, 0x06B6B51F, 0x9FBFE4A5, 0xE8B8D433,
	 0x7807C9A2, 0x0F00F934, 0x9609A88E, 0xE10E9818,
	 0x7F6A0DBB, 0x086D3D2D, 0x91646C97, 0xE6635C01,
	 0x6B6B51F4, 0x1C6C6162, 0x856530D8, 0xF262004E,
	 0x6C0695ED, 0x1B01A57B, 0x8208F4C1, 0xF50FC457,
	 0x65B0D9C6, 0x12B7E950, 0x8BBEB8EA, 0xFCB9887C,
	 0x62DD1DDF, 0x15DA2D49, 0x8CD37CF3, 0xFBD44C65,
	 0x4DB26158, 0x3AB551CE, 0xA3BC0074, 0xD4BB30E2,
	 0x4ADFA541, 0x3DD895D7, 0xA4D1C46D, 0xD3D6F4FB,
	 0x4369E96A, 0x346ED9FC, 0xAD678846, 0xDA60B8D0,
	 0x44042D73, 0x33031DE5, 0xAA0A4C5F, 0xDD0D7CC9,
	 0x5005713C, 0x270241AA, 0xBE0B1010, 0xC90C2086,
	 0x5768B525, 0x206F85B3, 0xB966D409, 0xCE61E49F,
	 0x5EDEF90E, 0x29D9C998, 0xB0D09822, 0xC7D7A8B4,
	 0x59B33D17, 0x2EB40D81, 0xB7BD5C3B, 0xC0BA6CAD,
	 0xEDB88320, 0x9ABFB3B6, 0x03B6E20C, 0x74B1D29A,
	 0xEAD54739, 0x9DD277AF, 0x04DB2615, 0x73DC1683,
	 0xE3630B12, 0x94643B84, 0x0D6D6A3E, 0x7A6A5AA8,
	 0xE40ECF0B, 0x9309FF9D, 0x0A00AE27, 0x7D079EB1,
	 0xF00F9344, 0x8708A3D2, 0x1E01F268, 0x6906C2FE,
	 0xF762575D, 0x806567CB, 0x196C3671, 0x6E6B06E7,
	 0xFED41B76, 0x89D32BE0, 0x10DA7A5A, 0x67DD4ACC,
	 0xF9B9DF6F, 0x8EBEEFF9, 0x17B7BE43, 0x60B08ED5,
	 0xD6D6A3E8, 0xA1D1937E, 0x38D8C2C4, 0x4FDFF252,
	 0xD1BB67F1, 0xA6BC5767, 0x3FB506DD, 0x48B2364B,
	 0xD80D2BDA, 0xAF0A1B4C, 0x36034AF6, 0x41047A60,
	 0xDF60EFC3, 0xA867DF55, 0x316E8EEF, 0x4669BE79,
	 0xCB61B38C, 0xBC66831A, 0x256FD2A0, 0x5268E236,
	 0xCC0C7795, 0xBB0B4703, 0x220216B9, 0x5505262F,
	 0xC5BA3BBE, 0xB2BD0B28, 0x2BB45A92, 0x5CB36A04,
	 0xC2D7FFA7, 0xB5D0CF31, 0x2CD99E8B, 0x5BDEAE1D,
	 0x9B64C2B0, 0xEC63F226, 0x756AA39C, 0x026D930A,
	 0x9C0906A9, 0xEB0E363F, 0x72076785, 0x05005713,
	 0x95BF4A82, 0xE2B87A14, 0x7BB12BAE, 0x0CB61B38,
	 0x92D28E9B, 0xE5D5BE0D, 0x7CDCEFB7, 0x0BDBDF21,
	 0x86D3D2D4, 0xF1D4E242, 0x68DDB3F8, 0x1FDA836E,
	 0x81BE16CD, 0xF6B9265B, 0x6FB077E1, 0x18B74777,
	 0x88085AE6, 0xFF0F6A70, 0x66063BCA, 0x11010B5C,
	 0x8F659EFF, 0xF862AE69, 0x616BFFD3, 0x166CCF45,
	 0xA00AE278, 0xD70DD2EE, 0x4E048354, 0x3903B3C2,
	 0xA7672661, 0xD06016F7, 0x4969474D, 0x3E6E77DB,
	 0xAED16A4A, 0xD9D65ADC, 0x40DF0B66, 0x37D83BF0,
	 0xA9BCAE53, 0xDEBB9EC5, 0x47B2CF7F, 0x30B5FFE9,
	 0xBDBDF21C, 0xCABAC28A, 0x53B39330, 0x24B4A3A6,
	 0xBAD03605, 0xCDD70693, 0x54DE5729, 0x23D967BF,
	 0xB3667A2E, 0xC4614AB8, 0x5D681B02, 0x2A6F2B94,
	 0xB40BBE37, 0xC30C8EA1, 0x5A05DF1B, 0x2D02EF8D
	};


static void print_usage(void)
{
	printf("\nCorrect Usage of Tool is:\n");
	printf("\n ./create_pbl [options] (mentioned below):\n\n");
	printf("\t-r  <RCW file-name>     - name of RCW binary file.\n");
	printf("\t-i  <BL2 Bin file-name> - file to be added to rcw file.\n");
	printf("\t-c  <Number>            - Chassis Architecture (=2 or =3\n");
	printf("\t                          or =4 for 3.2).\n");
	printf("\t-b  <qspi/nor/nand/sd>  - Boot source.\n");
	printf("\t-d  <Address>           - Destination address where BL2\n");
	printf("\t                          image is to be copied\n");
	printf("\t-o  <output filename>	  - Name of PBL image generated\n");
	printf("\t                          as an output of the tool.\n");
	printf("\t-f  <Address>           - BL2 image Src Offset\n");
	printf("\t                          on Boot Source for block copy.\n");
	printf("\t                          command for chassis >=3.)\n");
	printf("\t-e  <Address>           - [Optional] Entry Point Address\n");
	printf("\t                          of the BL2.bin\n");
	printf("\t-s  Secure Boot.\n");
	printf("\t-h  Help.\n");
	printf("\n\n");
	exit(0);

}

/***************************************************************************
 * Function	:	crypto_calculate_checksum()
 * Arguments	:	data - Pointer to FILE
 *			num - Number of 32 bit words for checksum
 * Return	:	Checksum Value
 * Description	:	Calculate Checksum over the data
 ***************************************************************************/
uint32_t crypto_calculate_checksum(FILE *fp_rcw_pbi_op, uint32_t num)
{
	uint32_t i;
	uint64_t sum = 0;
	uint32_t word;

	fseek(fp_rcw_pbi_op, 0L, SEEK_SET);
	for (i = 0; i < num ; i++) {
		if ((fread(&word, sizeof(word), NUM_MEM_BLOCK, fp_rcw_pbi_op))
			< NUM_MEM_BLOCK) {
			printf("%s: Error reading word.\n", __func__);
			return FAILURE;
		}
		sum = sum + word;
		sum = sum & 0xFFFFFFFF;
	}
	return (uint32_t)sum;
}

/***************************************************************************
 * Function	:	add_pbi_stop_cmd
 * Arguments	:	fp_rcw_pbi_op - output rcw_pbi file pointer
 * Return	:	SUCCESS or FAILURE
 * Description	:	This function insert pbi stop command.
 ***************************************************************************/
int add_pbi_stop_cmd(FILE *fp_rcw_pbi_op, enum stop_command flag)
{
	int ret = FAILURE;
	int32_t pbi_stop_cmd;
	uint32_t pbi_crc = 0xffffffff, i, j, c;
	uint32_t crc_table[MAX_CRC_ENTRIES];
	uint8_t data;

	switch (pblimg.chassis) {
	case CHASSIS_2:
		pbi_stop_cmd = BYTE_SWAP_32(CRC_STOP_CMD_ARM);
		break;
	case CHASSIS_3:
	case CHASSIS_3_2:
		/* Based on flag add the corresponsding cmd
		 * -- stop cmd or stop with CRC cmd
		 */
		if (flag == CRC_STOP_COMMAND) {
			pbi_stop_cmd = CRC_STOP_CMD_ARM_CH3;
		} else {
			pbi_stop_cmd = STOP_CMD_ARM_CH3;
		}
		break;
	case CHASSIS_UNKNOWN:
	case CHASSIS_MAX:
	default:
		printf("Internal Error: Invalid Chassis val = %d.\n",
			pblimg.chassis);
		goto pbi_stop_err;
	}

	if (fwrite(&pbi_stop_cmd, sizeof(pbi_stop_cmd), NUM_MEM_BLOCK,
			fp_rcw_pbi_op) != NUM_MEM_BLOCK) {
		printf("%s: Error in Writing PBI STOP CMD\n", __func__);
		goto pbi_stop_err;
	}

	if (flag == CRC_STOP_COMMAND) {
		for (i = 0; i < MAX_CRC_ENTRIES; i++) {
			c = i << 24;
			for (j = 0; j < 8; j++) {
				c = (c & 0x80000000) ?
					PBI_CRC_POLYNOMIAL ^ (c << 1) : c << 1;
			}

			crc_table[i] = c;
		}
	}

	switch (pblimg.chassis) {
	case CHASSIS_2:
		/* Chassis 2: CRC is calculated on  RCW + PBL cmd.*/
		fseek(fp_rcw_pbi_op, 0L, SEEK_SET);
		break;
	case CHASSIS_3:
	case CHASSIS_3_2:
		/* Chassis 3: CRC is calculated on  PBL cmd only. */
		fseek(fp_rcw_pbi_op, CHS3_CRC_PAYLOAD_START_OFFSET, SEEK_SET);
		break;
	case CHASSIS_UNKNOWN:
	case CHASSIS_MAX:
		printf("%s: Unknown Chassis.\n", __func__);
		goto pbi_stop_err;
	}

	while ((fread(&data, sizeof(data), NUM_MEM_BLOCK, fp_rcw_pbi_op))
		== NUM_MEM_BLOCK) {
		if (flag == CRC_STOP_COMMAND) {
			if (pblimg.chassis == CHASSIS_2) {
				pbi_crc = crc_table
					  [((pbi_crc >> 24) ^ (data)) & 0xff] ^
					  (pbi_crc << 8);
			} else {
				pbi_crc =  (pbi_crc >> 8) ^
					   crc32_lookup[((pbi_crc) ^
							   (data)) & 0xff];
			}
		}
	}

	switch (pblimg.chassis) {
	case CHASSIS_2:
		pbi_crc = BYTE_SWAP_32(pbi_crc);
		break;
	case CHASSIS_3:
	case CHASSIS_3_2:
		if (flag == CRC_STOP_COMMAND) {
			pbi_crc = pbi_crc ^ 0xFFFFFFFF;
		} else {
			pbi_crc = 0x00000000;
		}
		break;
	case CHASSIS_UNKNOWN:
	case CHASSIS_MAX:
		printf("%s: Unknown Chassis.\n", __func__);
		goto pbi_stop_err;
	}

	if (fwrite(&pbi_crc, sizeof(pbi_crc), NUM_MEM_BLOCK, fp_rcw_pbi_op)
		!= NUM_MEM_BLOCK) {
		printf("%s: Error in Writing PBI PBI CRC\n", __func__);
		goto pbi_stop_err;
	}
	ret = SUCCESS;

pbi_stop_err:
	return ret;
}

/*
 * Returns:
 *     File size in bytes, on Success.
 *     FAILURE, on failure.
 */
int get_filesize(const char *c)
{
	FILE *fp;
	int ret = FAILURE;

	fp = fopen(c, "rb");
	if (fp == NULL) {
		fprintf(stderr, "%s: Error in opening the file: %s\n",
			__func__, c);
		goto filesize_err;
	}

	fseek(fp, 0L, SEEK_END);
	ret = ftell(fp);
	fclose(fp);

filesize_err:
	return ret;
}

/***************************************************************************
 * Function	:	get_bootptr
 * Arguments	:	fp_rcw_pbi_op - Pointer to output file
 * Return	:	SUCCESS or FAILURE
 * Description	:	Add bootptr pbi command to output file
 ***************************************************************************/
int add_boot_ptr_cmd(FILE *fp_rcw_pbi_op)
{
	uint32_t bootptr_addr;
	int ret = FAILURE;

	switch (pblimg.chassis) {
	case CHASSIS_2:
		if (sb_flag == true)
			bootptr_addr = BYTE_SWAP_32(CSF_ADDR_SB);
		else
			bootptr_addr = BYTE_SWAP_32(BOOTPTR_ADDR);
		pblimg.ep    = BYTE_SWAP_32(pblimg.ep);
		break;
	case CHASSIS_3:
	case CHASSIS_3_2:
		if (sb_flag == true)
			bootptr_addr = CSF_ADDR_SB_CH3;
		else
			bootptr_addr = BOOTPTR_ADDR_CH3;
		break;
	case CHASSIS_UNKNOWN:
	case CHASSIS_MAX:
	default:
		printf("Internal Error: Invalid Chassis val = %d.\n",
			pblimg.chassis);
		goto bootptr_err;
	}

	if (fwrite(&bootptr_addr, sizeof(bootptr_addr), NUM_MEM_BLOCK,
		fp_rcw_pbi_op) != NUM_MEM_BLOCK) {
		printf("%s: Error in Writing PBI Words:[%d].\n",
			 __func__, ret);
		goto bootptr_err;
	}

	if (pblimg.ep != 0) {
		if (fwrite(&pblimg.ep, sizeof(pblimg.ep), NUM_MEM_BLOCK,
			fp_rcw_pbi_op) != NUM_MEM_BLOCK) {
			printf("%s: Error in Writing PBI Words\n", __func__);
			goto bootptr_err;
		}
	}

	printf("\nBoot Location Pointer= %x\n", BYTE_SWAP_32(pblimg.ep));
	ret = SUCCESS;

bootptr_err:
	return ret;
}

/***************************************************************************
 * Function	:	add_blk_cpy_cmd
 * Arguments	:	pbi_word - pointer to pbi commands
 *			args - Command  line args flag.
 * Return	:	SUCCESS or FAILURE
 * Description	:	Add pbi commands for block copy cmd in pbi_words
 ***************************************************************************/
int add_blk_cpy_cmd(FILE *fp_rcw_pbi_op, uint16_t args)
{
	uint32_t blk_cpy_hdr;
	uint32_t file_size, new_file_size;
	uint32_t align = 4;
	int ret = FAILURE;
	int num_pad_bytes = 0;

	if ((args & BL2_BIN_STRG_LOC_BOOT_SRC_ARG_MASK) == 0) {
		printf("ERROR: Offset not specified for Block Copy Cmd.\n");
		printf("\tSee Usage and use -f option\n");
		goto blk_copy_err;
	}

	switch (pblimg.chassis) {
	case CHASSIS_3:
		/* Block copy command */
		blk_cpy_hdr = blk_cpy_hdr_map_ch3[pblimg.boot_src];
		pblimg.src_addr += base_addr_ch3[pblimg.boot_src];
		break;
	case CHASSIS_3_2:
		/* Block copy command */
		blk_cpy_hdr = blk_cpy_hdr_map_ch32[pblimg.boot_src];
		pblimg.src_addr += base_addr_ch32[pblimg.boot_src];
		break;
	default:
		printf("%s: Error invalid chassis type for this command.\n",
				__func__);
		goto blk_copy_err;
	}

	file_size = get_filesize(pblimg.sec_imgnm);
	if (file_size > 0) {
		new_file_size = (file_size + (file_size % align));

		/* Add Block copy command */
		if (fwrite(&blk_cpy_hdr, sizeof(blk_cpy_hdr), NUM_MEM_BLOCK,
			fp_rcw_pbi_op) != NUM_MEM_BLOCK) {
			printf("%s: Error writing blk_cpy_hdr to the file.\n",
				 __func__);
			goto blk_copy_err;
		}

		if ((args & BL2_BIN_STRG_LOC_BOOT_SRC_ARG_MASK) == 0)
			num_pad_bytes = pblimg.src_addr % 4;

		/* Add Src address word */
		if (fwrite(&pblimg.src_addr + num_pad_bytes,
			   sizeof(pblimg.src_addr), NUM_MEM_BLOCK,
			   fp_rcw_pbi_op) != NUM_MEM_BLOCK) {
			printf("%s: Error writing BLK SRC Addr to the file.\n",
				 __func__);
			goto blk_copy_err;
		}

		/* Add Dest address word */
		if (fwrite(&pblimg.addr, sizeof(pblimg.addr),
			NUM_MEM_BLOCK, fp_rcw_pbi_op) != NUM_MEM_BLOCK) {
			printf("%s: Error writing DST Addr to the file.\n",
			__func__);
			goto blk_copy_err;
		}

		/* Add size */
		if (fwrite(&new_file_size, sizeof(new_file_size),
			NUM_MEM_BLOCK, fp_rcw_pbi_op) != NUM_MEM_BLOCK) {
			printf("%s: Error writing size to the file.\n",
				__func__);
			goto blk_copy_err;
		}
	}

	ret = SUCCESS;

blk_copy_err:
	return ret;
}

/***************************************************************************
 * Function	:	add_cpy_cmd
 * Arguments	:	pbi_word - pointer to pbi commands
 * Return	:	SUCCESS or FAILURE
 * Description	:	Append pbi commands for copying BL2 image to the
 *			load address stored in pbl_image.addr
 ***************************************************************************/
int add_cpy_cmd(FILE *fp_rcw_pbi_op)
{
	uint32_t ALTCBAR_ADDRESS = BYTE_SWAP_32(0x09570158);
	uint32_t WAIT_CMD_WRITE_ADDRESS = BYTE_SWAP_32(0x096100c0);
	uint32_t WAIT_CMD = BYTE_SWAP_32(0x000FFFFF);
	int file_size;
	uint32_t pbi_cmd, altcbar;
	uint8_t pbi_data[MAX_PBI_DATA_LEN_BYTE];
	uint32_t dst_offset;
	FILE *fp_img = NULL;
	int ret = FAILURE;

	altcbar = pblimg.addr;
	dst_offset = pblimg.addr;
	fp_img = fopen(pblimg.sec_imgnm, "rb");
	if (fp_img == NULL) {
		printf("%s: Error in opening the file: %s\n", __func__,
		      pblimg.sec_imgnm);
		goto add_cpy_err;
	}
	file_size = get_filesize(pblimg.sec_imgnm);
	altcbar = 0xfff00000 & altcbar;
	altcbar = BYTE_SWAP_32(altcbar >> 16);
	if (fwrite(&ALTCBAR_ADDRESS, sizeof(ALTCBAR_ADDRESS), NUM_MEM_BLOCK,
		fp_rcw_pbi_op) != NUM_MEM_BLOCK) {
		printf("%s: Error in writing address of ALTCFG CMD.\n",
			 __func__);
		goto add_cpy_err;
	}
	if (fwrite(&altcbar, sizeof(altcbar), NUM_MEM_BLOCK, fp_rcw_pbi_op)
		!= NUM_MEM_BLOCK) {
		printf("%s: Error in writing ALTCFG CMD.\n", __func__);
		goto add_cpy_err;
	}
	if (fwrite(&WAIT_CMD_WRITE_ADDRESS, sizeof(WAIT_CMD_WRITE_ADDRESS),
		NUM_MEM_BLOCK, fp_rcw_pbi_op) != NUM_MEM_BLOCK) {
		printf("%s: Error in writing address of WAIT_CMD.\n",
			__func__);
		goto add_cpy_err;
	}
	if (fwrite(&WAIT_CMD, sizeof(WAIT_CMD), NUM_MEM_BLOCK, fp_rcw_pbi_op)
		!= NUM_MEM_BLOCK) {
		printf("%s: Error in writing WAIT_CMD.\n", __func__);
		goto add_cpy_err;
	}
	do {
		memset(pbi_data, 0, MAX_PBI_DATA_LEN_BYTE);

		ret = fread(&pbi_data, MAX_PBI_DATA_LEN_BYTE,
				NUM_MEM_BLOCK, fp_img);
		if ((ret != NUM_MEM_BLOCK) && (!feof(fp_img))) {
			printf("%s: Error writing ALTCFG Word: [%d].\n",
				__func__, ret);
			goto add_cpy_err;
		}

		dst_offset &= OFFSET_MASK;
		pbi_cmd = WRITE_CMD_BASE | dst_offset;
		pbi_cmd = BYTE_SWAP_32(pbi_cmd);
		if (fwrite(&pbi_cmd, sizeof(pbi_cmd), NUM_MEM_BLOCK,
			fp_rcw_pbi_op) != NUM_MEM_BLOCK) {
			printf("%s: Error writing ALTCFG Word write cmd.\n",
				 __func__);
			goto add_cpy_err;
		}
		if (fwrite(&pbi_data,  MAX_PBI_DATA_LEN_BYTE, NUM_MEM_BLOCK,
			fp_rcw_pbi_op) != NUM_MEM_BLOCK) {
			printf("%s: Error writing ALTCFG_Word.\n", __func__);
			goto add_cpy_err;
		}
		dst_offset += MAX_PBI_DATA_LEN_BYTE;
		file_size -= MAX_PBI_DATA_LEN_BYTE;
	} while (!feof(fp_img));

	ret = SUCCESS;

add_cpy_err:
	if (fp_img != NULL) {
		fclose(fp_img);
	}
	return ret;
}

int main(int argc, char **argv)
{
	FILE *file = NULL;
	char *ptr;
	int opt;
	int tmp;
	uint16_t args = ARG_INIT_MASK;
	FILE *fp_rcw_pbi_ip = NULL, *fp_rcw_pbi_op = NULL;
	uint32_t word, word_1;
	int ret = FAILURE;
	bool bootptr_flag = false;
	enum stop_command flag_stop_cmd = CRC_STOP_COMMAND;

	/* Initializing the global structure to zero. */
	memset(&pblimg, 0x0, sizeof(struct pbl_image));

	while ((opt = getopt(argc, argv,
			     ":b:f:r:i:e:d:c:o:h:s")) != -1) {
		switch (opt) {
		case 'd':
			pblimg.addr = strtoull(optarg, &ptr, 16);
			if (*ptr != 0) {
				fprintf(stderr, "CMD Error: invalid load or destination address %s\n", optarg);
				goto exit_main;
			}
			args |= BL2_BIN_CPY_DEST_ADDR_ARG_MASK;
			break;
		case 'r':
			pblimg.rcw_nm = optarg;
			file = fopen(pblimg.rcw_nm, "r");
			if (file == NULL) {
				printf("CMD Error: Opening the RCW File.\n");
				goto exit_main;
			} else {
				args |= RCW_FILE_NAME_ARG_MASK;
				fclose(file);
			}
			break;
		case 'e':
			bootptr_flag = true;
			pblimg.ep = strtoull(optarg, &ptr, 16);
			if (*ptr != 0) {
				fprintf(stderr,
				"CMD Error: Invalid entry point %s\n", optarg);
				goto exit_main;
			}
			break;
		case 'h':
			print_usage();
			break;
		case 'i':
			pblimg.sec_imgnm = optarg;
			file = fopen(pblimg.sec_imgnm, "r");
			if (file == NULL) {
				printf("CMD Error: Opening Input file.\n");
				goto exit_main;
			} else {
				args |= IN_FILE_NAME_ARG_MASK;
				fclose(file);
			}
			break;
		case 'c':
			tmp = atoi(optarg);
			switch (tmp) {
			case SOC_LS1012:
			case SOC_LS1023:
			case SOC_LS1026:
			case SOC_LS1043:
			case SOC_LS1046:
				pblimg.chassis = CHASSIS_2;
				break;
			case SOC_LS1088:
			case SOC_LS2080:
			case SOC_LS2088:
				pblimg.chassis = CHASSIS_3;
				break;
			case SOC_LS1028:
			case SOC_LX2160:
				pblimg.chassis = CHASSIS_3_2;
				break;
			default:
			printf("CMD Error: Invalid SoC Val = %d.\n", tmp);
				goto exit_main;
			}

			args |= CHASSIS_ARG_MASK;
			break;
		case 'o':
			pblimg.imagefile = optarg;
			args |= OP_FILE_NAME_ARG_MASK;
			break;
		case 's':
			sb_flag = true;
			break;
		case 'b':
			if (strcmp(optarg, "qspi") == 0) {
				pblimg.boot_src = QSPI_BOOT;
			} else if (strcmp(optarg, "nor") == 0) {
				pblimg.boot_src = IFC_NOR_BOOT;
			} else if (strcmp(optarg, "nand") == 0) {
				pblimg.boot_src = IFC_NAND_BOOT;
			} else if (strcmp(optarg, "sd") == 0) {
				pblimg.boot_src = SD_BOOT;
			} else if (strcmp(optarg, "emmc") == 0) {
				pblimg.boot_src = EMMC_BOOT;
			} else if (strcmp(optarg, "flexspi_nor") == 0) {
				pblimg.boot_src = FLXSPI_NOR_BOOT;
			} else if (strcmp(optarg, "flexspi_nand") == 0) {
				pblimg.boot_src = FLXSPI_NAND_BOOT;
			} else if (strcmp(optarg, "flexspi_nand2k") == 0) {
				pblimg.boot_src = FLXSPI_NAND4K_BOOT;
			} else {
				printf("CMD Error: Invalid boot source.\n");
				goto exit_main;
			}
			args |= BOOT_SRC_ARG_MASK;
			break;
		case 'f':
			pblimg.src_addr = strtoull(optarg, &ptr, 16);
			if (*ptr != 0) {
				fprintf(stderr,
				"CMD Error: Invalid src offset %s\n", optarg);
				goto exit_main;
			}
			args |= BL2_BIN_STRG_LOC_BOOT_SRC_ARG_MASK;
			break;
		default:
			/* issue a warning and skip the unknown arg */
			printf("Cmd Warning: Invalid Arg = %c.\n", opt);
		}
	}

	if ((args & MAND_ARG_MASK) != MAND_ARG_MASK) {
		print_usage();
	}

	fp_rcw_pbi_ip = fopen(pblimg.rcw_nm, "rb");
	if (fp_rcw_pbi_ip == NULL) {
		printf("%s: Error in opening the rcw file: %s\n",
			__func__, pblimg.rcw_nm);
		goto exit_main;
	}

	fp_rcw_pbi_op = fopen(pblimg.imagefile, "wb+");
	if (fp_rcw_pbi_op == NULL) {
		printf("%s: Error opening the input file: %s\n",
			__func__, pblimg.imagefile);
		goto exit_main;
	}

	printf("\nInput Boot Source: %s\n", boot_src_string[pblimg.boot_src]);
	printf("Input RCW File: %s\n", pblimg.rcw_nm);
	printf("Input BL2 Binary File: %s\n", pblimg.sec_imgnm);
	printf("Input load address for BL2 Binary File: 0x%x\n", pblimg.addr);

	printf("Chassis Type: %d\n", pblimg.chassis);
	switch (pblimg.chassis) {
	case CHASSIS_2:
		if (fread(&word, sizeof(word), NUM_MEM_BLOCK, fp_rcw_pbi_ip)
			!= NUM_MEM_BLOCK) {
			printf("%s: Error in reading word from the rcw file.\n",
				__func__);
			goto exit_main;
		}
		while (BYTE_SWAP_32(word) != 0x08610040) {
			if (BYTE_SWAP_32(word) == 0x09550000
				|| BYTE_SWAP_32(word) == 0x000f400c) {
				break;
			}
			if (fwrite(&word, sizeof(word), NUM_MEM_BLOCK,
				fp_rcw_pbi_op) != NUM_MEM_BLOCK) {
				printf("%s: [CH2] Error in Writing PBI Words\n",
				__func__);
				goto exit_main;
			}
			if (fread(&word, sizeof(word), NUM_MEM_BLOCK,
				fp_rcw_pbi_ip) != NUM_MEM_BLOCK) {
				printf("%s: [CH2] Error in Reading PBI Words\n",
					__func__);
				goto exit_main;
			}
		}

		if (bootptr_flag == true) {
			/* Add command to set boot_loc ptr */
			ret = add_boot_ptr_cmd(fp_rcw_pbi_op);
			if (ret != SUCCESS) {
				goto exit_main;
			}
		}

		/* Write acs write commands to output file */
		ret = add_cpy_cmd(fp_rcw_pbi_op);
		if (ret != SUCCESS) {
			goto exit_main;
		}

		/* Add stop command after adding pbi commands
		 * For Chasis 2.0 platforms it is always CRC &
		 * Stop command
		 */
		flag_stop_cmd = CRC_STOP_COMMAND;
		ret = add_pbi_stop_cmd(fp_rcw_pbi_op, flag_stop_cmd);
		if (ret != SUCCESS) {
			goto exit_main;
		}

	break;

	case CHASSIS_3:
	case CHASSIS_3_2:
		if (fread(&word, sizeof(word), NUM_MEM_BLOCK, fp_rcw_pbi_ip)
			!= NUM_MEM_BLOCK) {
			printf("%s: Error reading PBI Cmd.\n", __func__);
			goto exit_main;
		}
		while (word != 0x808f0000 && word != 0x80ff0000) {
			pbl_size++;
			/* 11th words in RCW has PBL length. Update it
			 * with new length. 2 comamnds get added
			 * Block copy + CCSR Write/CSF header write
			 */
			if (pbl_size == 11) {
				word_1 = (word & PBI_LEN_MASK)
					+ (PBI_LEN_ADD << 20);
				word = word & ~PBI_LEN_MASK;
				word = word | word_1;
			}
			/* Update the CRC command */
			/* Check load command..
			 * add a check if command is Stop with CRC
			 * or stop without checksum
			 */
			if (pbl_size == 35) {
				word = crypto_calculate_checksum(fp_rcw_pbi_op,
						NUM_RCW_WORD - 1);
				if (word == FAILURE) {
					goto exit_main;
				}
			}
			if (fwrite(&word, sizeof(word),	NUM_MEM_BLOCK,
				fp_rcw_pbi_op) != NUM_MEM_BLOCK) {
				printf("%s: [CH3] Error in Writing PBI Words\n",
					__func__);
				goto exit_main;
			}
			if (fread(&word, sizeof(word), NUM_MEM_BLOCK,
				fp_rcw_pbi_ip) != NUM_MEM_BLOCK) {
				printf("%s: [CH3] Error in Reading PBI Words\n",
					 __func__);
				goto exit_main;
			}

			if (word == CRC_STOP_CMD_ARM_CH3) {
				flag_stop_cmd = CRC_STOP_COMMAND;
			} else if (word == STOP_CMD_ARM_CH3) {
				flag_stop_cmd = STOP_COMMAND;
			}
		}
		if (bootptr_flag == true) {
			/* Add command to set boot_loc ptr */
			ret = add_boot_ptr_cmd(fp_rcw_pbi_op);
			if (ret != SUCCESS) {
				printf("%s: add_boot_ptr_cmd return failure.\n",
					__func__);
				goto exit_main;
			}
		}

		/* Write acs write commands to output file */
		ret = add_blk_cpy_cmd(fp_rcw_pbi_op, args);
		if (ret != SUCCESS) {
			printf("%s: Function add_blk_cpy_cmd return failure.\n",
				 __func__);
			goto exit_main;
		}

		/* Add stop command after adding pbi commands */
		ret = add_pbi_stop_cmd(fp_rcw_pbi_op, flag_stop_cmd);
		if (ret != SUCCESS) {
			goto exit_main;
		}

	break;

	default:
		printf("%s: Unknown chassis type.\n",
				__func__);
	}

	if (ret == SUCCESS) {
		printf("Output file successfully created with name: %s\n\n",
			   pblimg.imagefile);
	}

exit_main:
	if (fp_rcw_pbi_op != NULL) {
		fclose(fp_rcw_pbi_op);
	}
	if (fp_rcw_pbi_ip != NULL) {
		fclose(fp_rcw_pbi_ip);
	}

	return ret;
}
