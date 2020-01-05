/*
 * Copyright (c) 2016 - 2020, Broadcom
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef CSL_SD_CMD_H
#define CSL_SD_CMD_H

#define SD_CMD_OK      0
#define SD_CMD_ERROR  -1

#define SD_CMD_ERR_NO_IO_FUNC           5
#define SD_CMD_ERR_INVALID_PARAMETER    6
#define SD_CMD_ERR_R1_ILLEGAL_COMMAND   7
#define SD_CMD_ERR_R1_COM_CRC_ERROR     8
#define SD_CMD_ERR_R1_FUNC_NUM_ERROR    9
#define SD_CMD_ERR_R1_ADDRESS_ERROR    10
#define SD_CMD_ERR_R1_PARAMETER_ERROR  11
#define SD_CMD_ERR_DATA_ERROR_TOKEN    12
#define SD_CMD_ERR_DATA_NOT_ACCEPTED   13
#define SD_CMD7_ARG_RCA_SHIFT          16

#define SD_CARD_STATUS_PENDING                 0x01
#define SD_CARD_STATUS_BUFFER_OVERFLOW         0x01
#define SD_CARD_STATUS_DEVICE_BUSY             0x02
#define SD_CARD_STATUS_UNSUCCESSFUL            0x03
#define SD_CARD_STATUS_NOT_IMPLEMENTED         0x04
#define SD_CARD_STATUS_ACCESS_VIOLATION        0x05
#define SD_CARD_STATUS_INVALID_HANDLE          0x06
#define SD_CARD_STATUS_INVALID_PARAMETER       0x07
#define SD_CARD_STATUS_NO_SUCH_DEVICE          0x08
#define SD_CARD_STATUS_INVALID_DEVICE_REQUEST  0x09
#define SD_CARD_STATUS_NO_MEMORY               0x0A
#define SD_CARD_STATUS_BUS_DRIVER_NOT_READY    0x0B
#define SD_CARD_STATUS_DATA_ERROR              0x0C
#define SD_CARD_STATUS_CRC_ERROR               0x0D
#define SD_CARD_STATUS_INSUFFICIENT_RESOURCES  0x0E
#define SD_CARD_STATUS_DEVICE_NOT_CONNECTED    0x10
#define SD_CARD_STATUS_DEVICE_REMOVED          0x11
#define SD_CARD_STATUS_DEVICE_NOT_RESPONDING   0x12
#define SD_CARD_STATUS_CANCELED                0x13
#define SD_CARD_STATUS_RESPONSE_TIMEOUT        0x14
#define SD_CARD_STATUS_DATA_TIMEOUT            0x15
#define SD_CARD_STATUS_DEVICE_RESPONSE_ERROR   0x16
#define SD_CARD_STATUS_DEVICE_UNSUPPORTED      0x17

/* Response structure */
struct sd_r2_resp {
	uint32_t rsp4;	/* 127:96 */
	uint32_t rsp3;	/* 95:64 */
	uint32_t rsp2;	/* 63:32 */
	uint32_t rsp1;	/* 31:0 */
};

struct sd_r3_resp {
	uint32_t ocr;
};

struct sd_r4_resp {
	uint8_t cardReady;
	uint8_t funcs;
	uint8_t memPresent;
	uint32_t ocr;
};

struct  sd_r5_resp {
	uint8_t data;
};

struct sd_r6_resp {
	uint16_t rca;
	uint16_t cardStatus;
};

struct sd_r7_resp {
	uint16_t rca;
};

struct sd_resp {
	uint8_t r1;
	uint32_t cardStatus;
	uint32_t rawData[4];
	union {
		struct sd_r2_resp r2;
		struct sd_r3_resp r3;
		struct sd_r4_resp r4;
		struct sd_r5_resp r5;
		struct sd_r6_resp r6;
		struct sd_r7_resp r7;
	} data;
};

struct sd_card_info {
	uint32_t type;	/* card type SD, MMC or SDIO */
	uint64_t size;	/* card size */
	uint32_t speed;	/* card speed */
	uint32_t voltage;	/* voltage supported */
	uint32_t mId;	/* manufacturer ID */
	uint32_t oId;	/* OEM ID */
	uint32_t classes;	/* card class */
	uint32_t name1;	/* product name part 1 */
	uint32_t name2;	/* product name part 2 */
	uint32_t revision;	/* revison */
	uint32_t sn;	/* serial number */
	uint32_t numIoFuns;	/* total I/O function number */
	uint32_t maxRdBlkLen;	/* max read block length */
	uint32_t maxWtBlkLen;	/* max write block length */
	uint32_t blkMode;	/* sdio card block mode support */
	uint32_t f0Cis;	/* sdio card block mode support */
	uint32_t f1Cis;	/* sdio card block mode support */

	uint8_t partRead;	/* partial block read allowed */
	uint8_t partWrite;	/* partial block write allowed */
	uint8_t dsr;	/* card DSR */
	uint8_t rdCurMin;	/* min current for read */
	uint8_t rdCurMax;	/* max current for read */
	uint8_t wtCurMin;	/* min current for write */
	uint8_t wtCurMax;	/* max current for write */
	uint8_t erase;	/* erase enable */
	uint8_t eraseSecSize;	/* erase sector size */
	uint8_t proGrpSize;	/* write protection group size */
	uint8_t protect;	/* permanent write protection or not */
	uint8_t tmpProt;	/* temp write protection or not */
	uint8_t wtSpeed;	/* write speed relatively to read */
	uint8_t version;	/* card version 0:1.0 - 1.01, 1:1.10, 2:2.0 */
	uint8_t eraseState;	/* if the data will be 0 or 1 after erase */
	uint8_t bus;	/* data with supported */
	uint8_t security;	/* security support 0, 2:1.01 3:2.0 */
	uint8_t format;	/* file format */
	uint8_t fileGrp;	/* file group */
	char pwd[20];	/* password */
};

struct sd_handle {
	struct sd_dev *device;
	struct sd_card_info *card;
};

int sd_cmd0(struct sd_handle *handle);
int sd_cmd1(struct sd_handle *handle, uint32_t initOcr, uint32_t *ocr);
int sd_cmd2(struct sd_handle *handle);
int sd_cmd3(struct sd_handle *handle);
int sd_cmd7(struct sd_handle *handle, uint32_t rca);
int sd_cmd9(struct sd_handle *handle, struct sd_card_data *card);
int sd_cmd13(struct sd_handle *handle, uint32_t *status);
int sd_cmd16(struct sd_handle *handle, uint32_t blockLen);
int sd_cmd17(struct sd_handle *handle,
	     uint32_t addr, uint32_t len, uint8_t *buffer);
int sd_cmd18(struct sd_handle *handle,
	     uint32_t addr, uint32_t len, uint8_t *buffer);
#ifdef INCLUDE_EMMC_DRIVER_WRITE_CODE
int sd_cmd24(struct sd_handle *handle,
	     uint32_t addr, uint32_t len, uint8_t *buffer);
int sd_cmd25(struct sd_handle *handle,
	     uint32_t addr, uint32_t len, uint8_t *buffer);
#endif
#ifdef INCLUDE_EMMC_DRIVER_ERASE_CODE
int sd_cmd35(struct sd_handle *handle, uint32_t start);
int sd_cmd36(struct sd_handle *handle, uint32_t end);
int sd_cmd38(struct sd_handle *handle);
#endif
int mmc_cmd6(struct sd_handle *handle, uint32_t argument);
int mmc_cmd8(struct sd_handle *handle, uint8_t *extCsdReg);

int send_cmd(struct sd_handle *handle, uint32_t cmdIndex,
	     uint32_t argument, uint32_t options, struct sd_resp *resp);
#endif /* CSL_SD_CMD_H */
