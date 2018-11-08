/*
 * Copyright (c) 2015-2018, Renesas Electronics Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/**
 * @file  emmc_std.h
 * @brief eMMC boot is expecting this header file
 *
 */

#ifndef EMMC_STD_H
#define EMMC_STD_H

#include "emmc_hal.h"

/* ************************ HEADER (INCLUDE) SECTION *********************** */

/* ***************** MACROS, CONSTANTS, COMPILATION FLAGS ****************** */
#ifndef FALSE
#define FALSE	0U
#endif
#ifndef TRUE
#define TRUE	1U
#endif

/** @brief 64bit registers
 **/
#define SETR_64(r, v)                   (*(volatile uint64_t *)(r) = (v))
#define GETR_64(r)                      (*(volatile uint64_t *)(r))

/** @brief 32bit registers
 **/
#define SETR_32(r, v)                   (*(volatile uint32_t *)(r) = (v))
#define GETR_32(r)                      (*(volatile uint32_t *)(r))

/** @brief 16bit registers
 */
#define SETR_16(r, v)                   (*(volatile uint16_t *)(r) = (v))
#define GETR_16(r)                      (*(volatile uint16_t *)(r))

/** @brief 8bit registers
 */
#define SETR_8(r, v)                    (*(volatile uint8_t *)(r) = (v))
#define GETR_8(r)                       (*(volatile uint8_t *)(r))

/** @brief CSD register Macros
 */
#define EMMC_GET_CID(x, y) (emmc_bit_field(mmc_drv_obj.cid_data, (x), (y)))

#define EMMC_CID_MID()			(EMMC_GET_CID(127, 120))
#define EMMC_CID_CBX()			(EMMC_GET_CID(113, 112))
#define EMMC_CID_OID()			(EMMC_GET_CID(111, 104))
#define EMMC_CID_PNM1()			(EMMC_GET_CID(103, 88))
#define EMMC_CID_PNM2()			(EMMC_GET_CID(87, 56))
#define EMMC_CID_PRV()			(EMMC_GET_CID(55, 48))
#define EMMC_CID_PSN()			(EMMC_GET_CID(47, 16))
#define EMMC_CID_MDT()			(EMMC_GET_CID(15, 8))
#define EMMC_CID_CRC()			(EMMC_GET_CID(7, 1))

/** @brief CSD register Macros
 */
#define EMMC_GET_CSD(x, y) (emmc_bit_field(mmc_drv_obj.csd_data, (x), (y)))

#define EMMC_CSD_CSD_STRUCTURE()        (EMMC_GET_CSD(127, 126))
#define EMMC_CSD_SPEC_VARS()            (EMMC_GET_CSD(125, 122))
#define EMMC_CSD_TAAC()                 (EMMC_GET_CSD(119, 112))
#define EMMC_CSD_NSAC()                 (EMMC_GET_CSD(111, 104))
#define EMMC_CSD_TRAN_SPEED()           (EMMC_GET_CSD(103, 96))
#define EMMC_CSD_CCC()                  (EMMC_GET_CSD(95, 84))
#define EMMC_CSD_READ_BL_LEN()          (EMMC_GET_CSD(83, 80))
#define EMMC_CSD_READ_BL_PARTIAL()      (EMMC_GET_CSD(79, 79))
#define EMMC_CSD_WRITE_BLK_MISALIGN()   (EMMC_GET_CSD(78, 78))
#define EMMC_CSD_READ_BLK_MISALIGN()    (EMMC_GET_CSD(77, 77))
#define EMMC_CSD_DSR_IMP()              (EMMC_GET_CSD(76, 76))
#define EMMC_CSD_C_SIZE()               (EMMC_GET_CSD(73, 62))
#define EMMC_CSD_VDD_R_CURR_MIN()       (EMMC_GET_CSD(61, 59))
#define EMMC_CSD_VDD_R_CURR_MAX()       (EMMC_GET_CSD(58, 56))
#define EMMC_CSD_VDD_W_CURR_MIN()       (EMMC_GET_CSD(55, 53))
#define EMMC_CSD_VDD_W_CURR_MAX()       (EMMC_GET_CSD(52, 50))
#define EMMC_CSD_C_SIZE_MULT()          (EMMC_GET_CSD(49, 47))
#define EMMC_CSD_ERASE_GRP_SIZE()       (EMMC_GET_CSD(46, 42))
#define EMMC_CSD_ERASE_GRP_MULT()       (EMMC_GET_CSD(41, 37))
#define EMMC_CSD_WP_GRP_SIZE()          (EMMC_GET_CSD(36, 32))
#define EMMC_CSD_WP_GRP_ENABLE()        (EMMC_GET_CSD(31, 31))
#define EMMC_CSD_DEFALT_ECC()           (EMMC_GET_CSD(30, 29))
#define EMMC_CSD_R2W_FACTOR()           (EMMC_GET_CSD(28, 26))
#define EMMC_CSD_WRITE_BL_LEN()         (EMMC_GET_CSD(25, 22))
#define EMMC_CSD_WRITE_BL_PARTIAL()     (EMMC_GET_CSD(21, 21))
#define EMMC_CSD_CONTENT_PROT_APP()     (EMMC_GET_CSD(16, 16))
#define EMMC_CSD_FILE_FORMAT_GRP()      (EMMC_GET_CSD(15, 15))
#define EMMC_CSD_COPY()                 (EMMC_GET_CSD(14, 14))
#define EMMC_CSD_PERM_WRITE_PROTECT()   (EMMC_GET_CSD(13, 13))
#define EMMC_CSD_TMP_WRITE_PROTECT()    (EMMC_GET_CSD(12, 12))
#define EMMC_CSD_FILE_FORMAT()          (EMMC_GET_CSD(11, 10))
#define EMMC_CSD_ECC()                  (EMMC_GET_CSD(9, 8))
#define EMMC_CSD_CRC()                  (EMMC_GET_CSD(7, 1))

/** @brief for sector access
 */
#define EMMC_4B_BOUNDARY_CHECK_MASK         0x00000003
#define EMMC_SECTOR_SIZE_SHIFT              9U	/* 512 = 2^9 */
#define EMMC_SECTOR_SIZE                    512
#define EMMC_BLOCK_LENGTH                   512
#define EMMC_BLOCK_LENGTH_DW                128
#define EMMC_BUF_SIZE_SHIFT                 3U	/* 8byte = 2^3 */

/** @brief eMMC specification clock
 */
#define EMMC_CLOCK_SPEC_400K                400000UL	 /**< initialize clock 400KHz */
#define EMMC_CLOCK_SPEC_20M                 20000000UL	 /**< normal speed 20MHz */
#define EMMC_CLOCK_SPEC_26M                 26000000UL	 /**< high speed 26MHz */
#define EMMC_CLOCK_SPEC_52M                 52000000UL	 /**< high speed 52MHz */
#define EMMC_CLOCK_SPEC_100M                100000000UL	 /**< high speed 100MHz */

/** @brief EMMC driver error code. (extended HAL_MEMCARD_RETURN)
 */
typedef enum {
	EMMC_ERR = 0,				/**< unknown error */
	EMMC_SUCCESS,				/**< OK */
	EMMC_ERR_FROM_DMAC,			/**< DMAC allocation error */
	EMMC_ERR_FROM_DMAC_TRANSFER,		/**< DMAC transfer error */
	EMMC_ERR_CARD_STATUS_BIT,		/**< card status error. Non-masked error bit was set in the card status */
	EMMC_ERR_CMD_TIMEOUT,			/**< command timeout error */
	EMMC_ERR_DATA_TIMEOUT,			/**< data timeout error */
	EMMC_ERR_CMD_CRC,			/**< command CRC error */
	EMMC_ERR_DATA_CRC,			/**< data CRC error */
	EMMC_ERR_PARAM,				/**< parameter error */
	EMMC_ERR_RESPONSE,			/**< response error */
	EMMC_ERR_RESPONSE_BUSY,			/**< response busy error */
	EMMC_ERR_TRANSFER,			/**< data transfer error */
	EMMC_ERR_READ_SECTOR,			/**< read sector error */
	EMMC_ERR_WRITE_SECTOR,			/**< write sector error */
	EMMC_ERR_STATE,				/**< state error */
	EMMC_ERR_TIMEOUT,			/**< timeout error */
	EMMC_ERR_ILLEGAL_CARD,			/**< illegal card */
	EMMC_ERR_CARD_BUSY,			/**< Busy state */
	EMMC_ERR_CARD_STATE,			/**< card state error */
	EMMC_ERR_SET_TRACE,			/**< trace information error */
	EMMC_ERR_FROM_TIMER,			/**< Timer error */
	EMMC_ERR_FORCE_TERMINATE,		/**< Force terminate */
	EMMC_ERR_CARD_POWER,			/**< card power fail */
	EMMC_ERR_ERASE_SECTOR,			/**< erase sector error */
	EMMC_ERR_INFO2				    /**< exec cmd error info2 */
} EMMC_ERROR_CODE;

/** @brief Function number */
#define EMMC_FUNCNO_NONE						0U
#define EMMC_FUNCNO_DRIVER_INIT						1U
#define EMMC_FUNCNO_CARD_POWER_ON					2U
#define EMMC_FUNCNO_MOUNT						3U
#define EMMC_FUNCNO_CARD_INIT						4U
#define EMMC_FUNCNO_HIGH_SPEED						5U
#define EMMC_FUNCNO_BUS_WIDTH						6U
#define EMMC_FUNCNO_MULTI_BOOT_SELECT_PARTITION				7U
#define EMMC_FUNCNO_MULTI_BOOT_READ_SECTOR				8U
#define EMMC_FUNCNO_TRANS_DATA_READ_SECTOR				9U
#define EMMC_FUNCNO_UBOOT_IMAGE_SELECT_PARTITION			10U
#define EMMC_FUNCNO_UBOOT_IMAGE_READ_SECTOR				11U
#define EMMC_FUNCNO_SET_CLOCK						12U
#define EMMC_FUNCNO_EXEC_CMD						13U
#define EMMC_FUNCNO_READ_SECTOR						14U
#define EMMC_FUNCNO_WRITE_SECTOR					15U
#define EMMC_FUNCNO_ERASE_SECTOR					16U
#define EMMC_FUNCNO_GET_PERTITION_ACCESS				17U
/** @brief Response
 */
/** R1 */
#define EMMC_R1_ERROR_MASK                      0xFDBFE080U	/* Type 'E' bit and bit14(must be 0). ignore bit22 */
#define EMMC_R1_ERROR_MASK_WITHOUT_CRC          (0xFD3FE080U)	/* Ignore bit23 (Not check CRC error) */
#define EMMC_R1_STATE_MASK                      0x00001E00U	/* [12:9] */
#define EMMC_R1_READY                           0x00000100U	/* bit8 */
#define EMMC_R1_STATE_SHIFT                     9

/** R4 */
#define EMMC_R4_RCA_MASK                        0xFFFF0000UL
#define EMMC_R4_STATUS                          0x00008000UL

/** CSD */
#define EMMC_TRANSPEED_FREQ_UNIT_MASK           0x07	/* bit[2:0] */
#define EMMC_TRANSPEED_FREQ_UNIT_SHIFT          0
#define EMMC_TRANSPEED_MULT_MASK                0x78	/* bit[6:3] */
#define EMMC_TRANSPEED_MULT_SHIFT               3

/** OCR */
#define EMMC_HOST_OCR_VALUE                     0x40FF8080
#define EMMC_OCR_STATUS_BIT                     0x80000000L	/* Card power up status bit */
#define EMMC_OCR_ACCESS_MODE_MASK               0x60000000L	/* bit[30:29] */
#define EMMC_OCR_ACCESS_MODE_SECT               0x40000000L
#define EMMC_OCR_ACCESS_MODE_BYTE               0x00000000L

/** EXT_CSD */
#define EMMC_EXT_CSD_S_CMD_SET                      504
#define EMMC_EXT_CSD_INI_TIMEOUT_AP                 241
#define EMMC_EXT_CSD_PWR_CL_DDR_52_360              239
#define EMMC_EXT_CSD_PWR_CL_DDR_52_195              238
#define EMMC_EXT_CSD_MIN_PERF_DDR_W_8_52            235
#define EMMC_EXT_CSD_MIN_PERF_DDR_R_8_52            234
#define EMMC_EXT_CSD_TRIM_MULT                      232
#define EMMC_EXT_CSD_SEC_FEATURE_SUPPORT            231
#define EMMC_EXT_CSD_SEC_ERASE_MULT                 229
#define EMMC_EXT_CSD_BOOT_INFO                      228
#define EMMC_EXT_CSD_BOOT_SIZE_MULTI                226
#define EMMC_EXT_CSD_ACC_SIZE                       225
#define EMMC_EXT_CSD_HC_ERASE_GRP_SIZE              224
#define EMMC_EXT_CSD_ERASE_TIMEOUT_MULT             223
#define EMMC_EXT_CSD_PEL_WR_SEC_C                   222
#define EMMC_EXT_CSD_HC_WP_GRP_SIZE                 221
#define EMMC_EXT_CSD_S_C_VCC                        220
#define EMMC_EXT_CSD_S_C_VCCQ                       219
#define EMMC_EXT_CSD_S_A_TIMEOUT                    217
#define EMMC_EXT_CSD_SEC_COUNT                      215
#define EMMC_EXT_CSD_MIN_PERF_W_8_52                210
#define EMMC_EXT_CSD_MIN_PERF_R_8_52                209
#define EMMC_EXT_CSD_MIN_PERF_W_8_26_4_52           208
#define EMMC_EXT_CSD_MIN_PERF_R_8_26_4_52           207
#define EMMC_EXT_CSD_MIN_PERF_W_4_26                206
#define EMMC_EXT_CSD_MIN_PERF_R_4_26                205
#define EMMC_EXT_CSD_PWR_CL_26_360                  203
#define EMMC_EXT_CSD_PWR_CL_52_360                  202
#define EMMC_EXT_CSD_PWR_CL_26_195                  201
#define EMMC_EXT_CSD_PWR_CL_52_195                  200
#define EMMC_EXT_CSD_CARD_TYPE                      196
#define EMMC_EXT_CSD_CSD_STRUCTURE                  194
#define EMMC_EXT_CSD_EXT_CSD_REV                    192
#define EMMC_EXT_CSD_CMD_SET                        191
#define EMMC_EXT_CSD_CMD_SET_REV                    189
#define EMMC_EXT_CSD_POWER_CLASS                    187
#define EMMC_EXT_CSD_HS_TIMING                      185
#define EMMC_EXT_CSD_BUS_WIDTH                      183
#define EMMC_EXT_CSD_ERASED_MEM_CONT                181
#define EMMC_EXT_CSD_PARTITION_CONFIG               179
#define EMMC_EXT_CSD_BOOT_CONFIG_PROT               178
#define EMMC_EXT_CSD_BOOT_BUS_WIDTH                 177
#define EMMC_EXT_CSD_ERASE_GROUP_DEF                175
#define EMMC_EXT_CSD_BOOT_WP                        173
#define EMMC_EXT_CSD_USER_WP                        171
#define EMMC_EXT_CSD_FW_CONFIG                      169
#define EMMC_EXT_CSD_RPMB_SIZE_MULT                 168
#define EMMC_EXT_CSD_RST_n_FUNCTION                 162
#define EMMC_EXT_CSD_PARTITIONING_SUPPORT           160
#define EMMC_EXT_CSD_MAX_ENH_SIZE_MULT              159
#define EMMC_EXT_CSD_PARTITIONS_ATTRIBUTE           156
#define EMMC_EXT_CSD_PARTITION_SETTING_COMPLETED    155
#define EMMC_EXT_CSD_GP_SIZE_MULT                   154
#define EMMC_EXT_CSD_ENH_SIZE_MULT                  142
#define EMMC_EXT_CSD_ENH_START_ADDR                 139
#define EMMC_EXT_CSD_SEC_BAD_BLK_MGMNT              134

#define EMMC_EXT_CSD_CARD_TYPE_26MHZ                0x01
#define EMMC_EXT_CSD_CARD_TYPE_52MHZ                0x02
#define EMMC_EXT_CSD_CARD_TYPE_DDR_52MHZ_12V        0x04
#define EMMC_EXT_CSD_CARD_TYPE_DDR_52MHZ_18V        0x08
#define EMMC_EXT_CSD_CARD_TYPE_52MHZ_MASK           0x0e

/** SWITCH (CMD6) argument */
#define	EXTCSD_ACCESS_BYTE	(BIT25|BIT24)
#define	EXTCSD_SET_BITS		BIT24

#define	HS_TIMING_ADD		(185<<16)	/* H'b9 */
#define	HS_TIMING_1			(1<<8)
#define	HS_TIMING_HS200		(2<<8)
#define	HS_TIMING_HS400		(3<<8)

#define	BUS_WIDTH_ADD		(183<<16)	/* H'b7 */
#define	BUS_WIDTH_1			(0<<8)
#define	BUS_WIDTH_4			(1<<8)
#define	BUS_WIDTH_8			(2<<8)
#define	BUS_WIDTH_4DDR		(5<<8)
#define	BUS_WIDTH_8DDR		(6<<8)

#define EMMC_SWITCH_HS_TIMING           (EXTCSD_ACCESS_BYTE|HS_TIMING_ADD|HS_TIMING_1)		/**< H'03b90100 */
#define	EMMC_SWITCH_HS_TIMING_OFF	    (EXTCSD_ACCESS_BYTE|HS_TIMING_ADD)					/**< H'03b90000 */

#define EMMC_SWITCH_BUS_WIDTH_1         (EXTCSD_ACCESS_BYTE|BUS_WIDTH_ADD|BUS_WIDTH_1)		/**< H'03b70000 */
#define EMMC_SWITCH_BUS_WIDTH_4         (EXTCSD_ACCESS_BYTE|BUS_WIDTH_ADD|BUS_WIDTH_4)		/**< H'03b70100 */
#define EMMC_SWITCH_BUS_WIDTH_8         (EXTCSD_ACCESS_BYTE|BUS_WIDTH_ADD|BUS_WIDTH_8)		/**< H'03b70200 */
#define	EMMC_SWITCH_BUS_WIDTH_4DDR      (EXTCSD_ACCESS_BYTE|BUS_WIDTH_ADD|BUS_WIDTH_4DDR)	/**< H'03b70500 */
#define	EMMC_SWITCH_BUS_WIDTH_8DDR      (EXTCSD_ACCESS_BYTE|BUS_WIDTH_ADD|BUS_WIDTH_8DDR)	/**< H'03b70600 */
#define EMMC_SWITCH_PARTITION_CONFIG    0x03B30000UL	/**< Partition config = 0x00 */

#define TIMING_HIGH_SPEED					1UL
#define EMMC_BOOT_PARTITION_EN_MASK	0x38U
#define EMMC_BOOT_PARTITION_EN_SHIFT	3U

/** Bus width */
#define EMMC_BUSWIDTH_1BIT              CE_CMD_SET_DATW_1BIT
#define EMMC_BUSWIDTH_4BIT              CE_CMD_SET_DATW_4BIT
#define EMMC_BUSWIDTH_8BIT              CE_CMD_SET_DATW_8BIT

/** for st_mmc_base */
#define EMMC_MAX_RESPONSE_LENGTH        17
#define EMMC_MAX_CID_LENGTH             16
#define EMMC_MAX_CSD_LENGTH             16
#define EMMC_MAX_EXT_CSD_LENGTH         512U
#define EMMC_RES_REG_ALIGNED            4U
#define EMMC_BUF_REG_ALIGNED            8U

/** @brief for TAAC mask
 */
#define TAAC_TIME_UNIT_MASK         (0x07)
#define TAAC_MULTIPLIER_FACTOR_MASK (0x0F)

/* ********************** STRUCTURES, TYPE DEFINITIONS ********************* */

/** @brief Partition id
 */
typedef enum {
	PARTITION_ID_USER = 0x0,    /**< User Area */
	PARTITION_ID_BOOT_1 = 0x1,  /**< boot partition 1 */
	PARTITION_ID_BOOT_2 = 0x2,  /**< boot partition 2 */
	PARTITION_ID_RPMB = 0x3,    /**< Replay Protected Memory Block */
	PARTITION_ID_GP_1 = 0x4,    /**< General Purpose partition 1 */
	PARTITION_ID_GP_2 = 0x5,    /**< General Purpose partition 2 */
	PARTITION_ID_GP_3 = 0x6,    /**< General Purpose partition 3 */
	PARTITION_ID_GP_4 = 0x7,    /**< General Purpose partition 4 */
	PARTITION_ID_MASK = 0x7	    /**< [2:0] */
} EMMC_PARTITION_ID;

/** @brief card state in R1 response [12:9]
 */
typedef enum {
	EMMC_R1_STATE_IDLE = 0,
	EMMC_R1_STATE_READY,
	EMMC_R1_STATE_IDENT,
	EMMC_R1_STATE_STBY,
	EMMC_R1_STATE_TRAN,
	EMMC_R1_STATE_DATA,
	EMMC_R1_STATE_RCV,
	EMMC_R1_STATE_PRG,
	EMMC_R1_STATE_DIS,
	EMMC_R1_STATE_BTST,
	EMMC_R1_STATE_SLEP
} EMMC_R1_STATE;

typedef enum {
	ESTATE_BEGIN = 0,
	ESTATE_ISSUE_CMD,
	ESTATE_NON_RESP_CMD,
	ESTATE_RCV_RESP,
	ESTATE_RCV_RESPONSE_BUSY,
	ESTATE_CHECK_RESPONSE_COMPLETE,
	ESTATE_DATA_TRANSFER,
	ESTATE_DATA_TRANSFER_COMPLETE,
	ESTATE_ACCESS_END,
	ESTATE_TRANSFER_ERROR,
	ESTATE_ERROR,
	ESTATE_END
} EMMC_INT_STATE;

/** @brief eMMC boot driver error information
 */
typedef struct {
	uint16_t num;		  /**< error no */
	uint16_t code;		  /**< error code */
	volatile uint32_t info1;  /**< SD_INFO1 register value. (hardware dependence) */
	volatile uint32_t info2;  /**< SD_INFO2 register value. (hardware dependence) */
	volatile uint32_t status1;/**< SD_ERR_STS1 register value. (hardware dependence) */
	volatile uint32_t status2;/**< SD_ERR_STS2 register value. (hardware dependence) */
	volatile uint32_t dm_info1;/**< DM_CM_INFO1 register value. (hardware dependence) */
	volatile uint32_t dm_info2;/**< DM_CM_INFO2 register value. (hardware dependence) */
} st_error_info;

/** @brief Command information
 */
typedef struct {
	HAL_MEMCARD_COMMAND cmd;	/**< Command information */
	uint32_t arg;			  /**< argument */
	HAL_MEMCARD_OPERATION dir;	/**< direction */
	uint32_t hw;			  /**< H/W dependence. SD_CMD register value. */
} st_command_info;

/** @brief MMC driver base
 */
typedef struct {
	st_error_info error_info;	/**< error information */
	st_command_info cmd_info;	/**< command information */

	/* for data transfer */
	uint32_t *buff_address_virtual;	   /**< Dest or Src buff */
	uint32_t *buff_address_physical;   /**< Dest or Src buff */
	HAL_MEMCARD_DATA_WIDTH bus_width;
					/**< bus width */
	uint32_t trans_size;		  /**< transfer size for this command */
	uint32_t remain_size;		  /**< remain size for this command */
	uint32_t response_length;	  /**< response length for this command */
	uint32_t sector_size;		   /**< sector_size */

	/* clock */
	uint32_t base_clock;		  /**< MMC host controller clock */
	uint32_t max_freq;		  /**< Max frequency (Card Spec)[Hz]. It changes dynamically by CSD and EXT_CSD. */
	uint32_t request_freq;		  /**< request freq [Hz] (400K, 26MHz, 52MHz, etc) */
	uint32_t current_freq;		  /**< current MMC clock[Hz] (the closest frequency supported by HW) */

	/* state flag */
	HAL_MEMCARD_PRESENCE_STATUS card_present;
						/**< presence status of the memory card */
	uint32_t card_power_enable;		  /**< True : Power ON */
	uint32_t clock_enable;			  /**< True : Clock ON */
	uint32_t initialize;			  /**< True : initialize complete. */
	uint32_t access_mode;			  /**< True : sector access, FALSE : byte access */
	uint32_t mount;				  /**< True : mount complete. */
	uint32_t selected;			  /**< True : selected card. */
	HAL_MEMCARD_DATA_TRANSFER_MODE transfer_mode;
						    /**< 0: DMA, 1:PIO */
	uint32_t image_num;			  /**< loaded ISSW image No. ISSW have copy image. */
	EMMC_R1_STATE current_state;		/**< card state */
	volatile uint32_t during_cmd_processing;  /**< True : during command processing */
	volatile uint32_t during_transfer;	  /**< True : during transfer */
	volatile uint32_t during_dma_transfer;	  /**< True : during transfer (DMA)*/
	volatile uint32_t dma_error_flag;	  /**< True : occurred DMAC error */
	volatile uint32_t force_terminate;	  /**< force terminate flag */
	volatile uint32_t state_machine_blocking; /**< state machine blocking flag : True or False */
	volatile uint32_t get_partition_access_flag;
						  /**< True : get partition access processing */

	EMMC_PARTITION_ID boot_partition_en;	/**< Boot partition */
	EMMC_PARTITION_ID partition_access;	/**< Current access partition */

	/* timeout */
	uint32_t hs_timing;			/**< high speed */

	/* timeout */
	uint32_t data_timeout;			  /**< read and write data timeout.*/

	/* retry */
	uint32_t retries_after_fail;  /**< how many times to try after fail, for instance sending command */

	/* interrupt */
	volatile uint32_t int_event1; /**< interrupt SD_INFO1 Event */
	volatile uint32_t int_event2;	  /**< interrupt SD_INFO2 Event */
	volatile uint32_t dm_event1;  /**< interrupt DM_CM_INFO1 Event */
	volatile uint32_t dm_event2;	  /**< interrupt DM_CM_INFO2 Event */

	/* response */
	uint32_t *response;	      /**< pointer to buffer for executing command. */
	uint32_t r1_card_status;      /**< R1 response data */
	uint32_t r3_ocr;	      /**< R3 response data */
	uint32_t r4_resp;	      /**< R4 response data */
	uint32_t r5_resp;	      /**< R5 response data */

	uint32_t low_clock_mode_enable;
				      /**< True : clock mode is low. (MMC clock = Max26MHz) */
	uint32_t reserved2;
	uint32_t reserved3;
	uint32_t reserved4;

	/* CSD registers (4byte align) */
	uint8_t csd_data[EMMC_MAX_CSD_LENGTH]		      /**< CSD */
	    __attribute__ ((aligned(EMMC_RES_REG_ALIGNED)));
	/* CID registers (4byte align) */
	uint8_t cid_data[EMMC_MAX_CID_LENGTH]		      /**< CID */
	    __attribute__ ((aligned(EMMC_RES_REG_ALIGNED)));
	/* EXT CSD registers (8byte align) */
	uint8_t ext_csd_data[EMMC_MAX_EXT_CSD_LENGTH]	      /**< EXT_CSD */
	    __attribute__ ((aligned(EMMC_BUF_REG_ALIGNED)));
	/* Response registers (4byte align) */
	uint8_t response_data[EMMC_MAX_RESPONSE_LENGTH]	      /**< other response */
	    __attribute__ ((aligned(EMMC_RES_REG_ALIGNED)));
} st_mmc_base;

typedef int (*func) (void);

/* ********************** DECLARATION OF EXTERNAL DATA ********************* */

/* ************************** FUNCTION PROTOTYPES ************************** */
uint32_t emmc_get_csd_time(void);

#define MMC_DEBUG
/* ********************************* CODE ********************************** */

/* ******************************** END ************************************ */
#endif /* EMMC_STD_H */
