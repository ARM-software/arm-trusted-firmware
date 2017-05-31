/*
 * Copyright (c) 2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __UFS_H__
#define __UFS_H__

/* register map of UFSHCI */
/* Controller Capabilities */
#define CAP				0x00
#define CAP_NUTRS_MASK			0x1F

/* UFS Version */
#define VER				0x08
/* Host Controller Identification - Product ID */
#define HCDDID				0x10
/* Host Controller Identification Descriptor - Manufacturer ID */
#define HCPMID				0x14
/* Auto-Hibernate Idle Timer */
#define AHIT				0x18
/* Interrupt Status */
#define IS				0x20
/* Interrupt Enable */
#define IE				0x24
/* System Bus Fatal Error Status */
#define UFS_INT_SBFES			(1 << 17)
/* Host Controller Fatal Error Status */
#define UFS_INT_HCFES			(1 << 16)
/* UTP Error Status */
#define UFS_INT_UTPES			(1 << 12)
/* Device Fatal Error Status */
#define UFS_INT_DFES			(1 << 11)
/* UIC Command Completion Status */
#define UFS_INT_UCCS			(1 << 10)
/* UTP Task Management Request Completion Status */
#define UFS_INT_UTMRCS			(1 << 9)
/* UIC Link Startup Status */
#define UFS_INT_ULSS			(1 << 8)
/* UIC Link Lost Status */
#define UFS_INT_ULLS			(1 << 7)
/* UIC Hibernate Enter Status */
#define UFS_INT_UHES			(1 << 6)
/* UIC Hibernate Exit Status */
#define UFS_INT_UHXS			(1 << 5)
/* UIC Power Mode Status */
#define UFS_INT_UPMS			(1 << 4)
/* UIC Test Mode Status */
#define UFS_INT_UTMS			(1 << 3)
/* UIC Error */
#define UFS_INT_UE			(1 << 2)
/* UIC DME_ENDPOINTRESET Indication */
#define UFS_INT_UDEPRI			(1 << 1)
/* UTP Transfer Request Completion Status */
#define UFS_INT_UTRCS			(1 << 0)

/* Host Controller Status */
#define HCS				0x30
#define HCS_UPMCRS_MASK			(7 << 8)
#define HCS_PWR_LOCAL			(1 << 8)
#define HCS_UCRDY			(1 << 3)
#define HCS_UTMRLRDY			(1 << 2)
#define HCS_UTRLRDY			(1 << 1)
#define HCS_DP				(1 << 0)

/* Host Controller Enable */
#define HCE				0x34
#define HCE_ENABLE			1

/* Host UIC Error Code PHY Adapter Layer */
#define UECPA				0x38
/* Host UIC Error Code Data Link Layer */
#define UECDL				0x3C
/* Host UIC Error Code Network Layer */
#define UECN				0x40
/* Host UIC Error Code Transport Layer */
#define UECT				0x44
/* Host UIC Error Code */
#define UECDME				0x48
/* UTP Transfer Request Interrupt Aggregation Control Register */
#define UTRIACR				0x4C
#define UTRIACR_IAEN			(1 << 31)
#define UTRIACR_IAPWEN			(1 << 24)
#define UTRIACR_IASB			(1 << 20)
#define UTRIACR_CTR			(1 << 16)
#define UTRIACR_IACTH(x)		(((x) & 0x1F) << 8)
#define UTRIACR_IATOVAL(x)		((x) & 0xFF)

/* UTP Transfer Request List Base Address */
#define UTRLBA				0x50
/* UTP Transfer Request List Base Address Upper 32-bits */
#define UTRLBAU				0x54
/* UTP Transfer Request List Door Bell Register */
#define UTRLDBR				0x58
/* UTP Transfer Request List Clear Register */
#define UTRLCLR				0x5C
/* UTP Transfer Request List Run Stop Register */
#define UTRLRSR				0x60
#define UTMRLBA				0x70
#define UTMRLBAU			0x74
#define UTMRLDBR			0x78
#define UTMRLCLR			0x7C
#define UTMRLRSR			0x80
/* UIC Command */
#define UICCMD				0x90
/* UIC Command Argument 1 */
#define UCMDARG1			0x94
/* UIC Command Argument 2 */
#define UCMDARG2			0x98
/* UIC Command Argument 3 */
#define UCMDARG3			0x9C

#define UFS_BLOCK_SHIFT			12		/* 4KB */
#define UFS_BLOCK_SIZE			(1 << UFS_BLOCK_SHIFT)
#define UFS_BLOCK_MASK			(UFS_BLOCK_SIZE - 1)
#define UFS_MAX_LUNS			8

/* UTP Transfer Request Descriptor */
/* Command Type */
#define CT_UFS_STORAGE			1
#define CT_SCSI				0

/* Data Direction */
#define DD_OUT				2		/* Device --> Host */
#define DD_IN				1		/* Host --> Device */
#define DD_NO_DATA_TRANSFER		0

#define UTP_TRD_SIZE			32

/* Transaction Type */
#define TRANS_TYPE_HD			(1 << 7)	/* E2ECRC */
#define TRANS_TYPE_DD			(1 << 6)
#define TRANS_TYPE_CODE_MASK		0x3F
#define QUERY_RESPONSE_UPIU		(0x36 << 0)
#define READY_TO_TRANSACTION_UPIU	(0x31 << 0)
#define DATA_IN_UPIU			(0x22 << 0)
#define RESPONSE_UPIU			(0x21 << 0)
#define NOP_IN_UPIU			(0x20 << 0)
#define QUERY_REQUEST_UPIU		(0x16 << 0)
#define DATA_OUT_UPIU			(0x02 << 0)
#define CMD_UPIU			(0x01 << 0)
#define NOP_OUT_UPIU			(0x00 << 0)

#define OCS_SUCCESS			0x0
#define OCS_INVALID_FUNC_ATTRIBUTE	0x1
#define OCS_MISMATCH_REQUEST_SIZE	0x2
#define OCS_MISMATCH_RESPONSE_SIZE	0x3
#define OCS_PEER_COMMUNICATION_FAILURE	0x4
#define OCS_ABORTED			0x5
#define OCS_FATAL_ERROR			0x6
#define OCS_MASK			0xF

/* UIC Command */
#define DME_GET				0x01
#define DME_SET				0x02
#define DME_PEER_GET			0x03
#define DME_PEER_SET			0x04
#define DME_POWERON			0x10
#define DME_POWEROFF			0x11
#define DME_ENABLE			0x12
#define DME_RESET			0x14
#define DME_ENDPOINTRESET		0x15
#define DME_LINKSTARTUP			0x16
#define DME_HIBERNATE_ENTER		0x17
#define DME_HIBERNATE_EXIT		0x18
#define DME_TEST_MODE			0x1A

#define GEN_SELECTOR_IDX(x)		((x) & 0xFFFF)

#define CONFIG_RESULT_CODE_MASK		0xFF

#define CDBCMD_TEST_UNIT_READY		0x00
#define CDBCMD_READ_6			0x08
#define CDBCMD_WRITE_6			0x0A
#define CDBCMD_START_STOP_UNIT		0x1B
#define CDBCMD_READ_CAPACITY_10		0x25
#define CDBCMD_READ_10			0x28
#define CDBCMD_WRITE_10			0x2A
#define CDBCMD_READ_16			0x88
#define CDBCMD_WRITE_16			0x8A
#define CDBCMD_READ_CAPACITY_16		0x9E
#define CDBCMD_REPORT_LUNS		0xA0

#define UPIU_FLAGS_R			(1 << 6)
#define UPIU_FLAGS_W			(1 << 5)
#define UPIU_FLAGS_ATTR_MASK		(3 << 0)
#define UPIU_FLAGS_ATTR_S		(0 << 0)	/* Simple */
#define UPIU_FLAGS_ATTR_O		(1 << 0)	/* Ordered */
#define UPIU_FLAGS_ATTR_HQ		(2 << 0)	/* Head of Queue */
#define UPIU_FLAGS_ATTR_ACA		(3 << 0)
#define UPIU_FLAGS_O			(1 << 6)
#define UPIU_FLAGS_U			(1 << 5)
#define UPIU_FLAGS_D			(1 << 4)

#define QUERY_FUNC_STD_READ		0x01
#define QUERY_FUNC_STD_WRITE		0x81

#define QUERY_NOP			0x00
#define QUERY_READ_DESC			0x01
#define QUERY_WRITE_DESC		0x02
#define QUERY_READ_ATTR			0x03
#define QUERY_WRITE_ATTR		0x04
#define QUERY_READ_FLAG			0x05
#define QUERY_SET_FLAG			0x06
#define QUERY_CLEAR_FLAG		0x07
#define QUERY_TOGGLE_FLAG		0x08

#define RW_WITHOUT_CACHE		0x18

#define DESC_TYPE_DEVICE		0x00
#define DESC_TYPE_CONFIGURATION		0x01
#define DESC_TYPE_UNIT			0x02
#define DESC_TYPE_INTERCONNECT		0x04
#define DESC_TYPE_STRING		0x05

#define ATTR_CUR_PWR_MODE		0x02	/* bCurrentPowerMode */
#define ATTR_ACTIVECC			0x03	/* bActiveICCLevel */

#define DEVICE_DESCRIPTOR_LEN		0x40
#define UNIT_DESCRIPTOR_LEN		0x23

#define QUERY_RESP_SUCCESS		0x00
#define QUERY_RESP_OPCODE		0xFE
#define QUERY_RESP_GENERAL_FAIL		0xFF

#define SENSE_KEY_NO_SENSE		0x00
#define SENSE_KEY_RECOVERED_ERROR	0x01
#define SENSE_KEY_NOT_READY		0x02
#define SENSE_KEY_MEDIUM_ERROR		0x03
#define SENSE_KEY_HARDWARE_ERROR	0x04
#define SENSE_KEY_ILLEGAL_REQUEST	0x05
#define SENSE_KEY_UNIT_ATTENTION	0x06
#define SENSE_KEY_DATA_PROTECT		0x07
#define SENSE_KEY_BLANK_CHECK		0x08
#define SENSE_KEY_VENDOR_SPECIFIC	0x09
#define SENSE_KEY_COPY_ABORTED		0x0A
#define SENSE_KEY_ABORTED_COMMAND	0x0B
#define SENSE_KEY_VOLUME_OVERFLOW	0x0D
#define SENSE_KEY_MISCOMPARE		0x0E

#define SENSE_DATA_VALID		0x70
#define SENSE_DATA_LENGTH		18

#define READ_CAPACITY_LENGTH		8

#define FLAG_DEVICE_INIT		0x01

/* UFS Driver Flags */
#define UFS_FLAGS_SKIPINIT		(1 << 0)

typedef struct sense_data {
	uint8_t		resp_code : 7;
	uint8_t		valid : 1;
	uint8_t		reserved0;
	uint8_t		sense_key : 4;
	uint8_t		reserved1 : 1;
	uint8_t		ili : 1;
	uint8_t		eom : 1;
	uint8_t		file_mark : 1;
	uint8_t		info[4];
	uint8_t		asl;
	uint8_t		cmd_spec_len[4];
	uint8_t		asc;
	uint8_t		ascq;
	uint8_t		fruc;
	uint8_t		sense_key_spec0 : 7;
	uint8_t		sksv : 1;
	uint8_t		sense_key_spec1;
	uint8_t		sense_key_spec2;
} sense_data_t;

/* UTP Transfer Request Descriptor */
typedef struct utrd_header {
	uint32_t	reserved0 : 24;
	uint32_t	i : 1;		/* interrupt */
	uint32_t	dd : 2;		/* data direction */
	uint32_t	reserved1 : 1;
	uint32_t	ct : 4;		/* command type */
	uint32_t	reserved2;
	uint32_t	ocs : 8;	/* Overall Command Status */
	uint32_t	reserved3 : 24;
	uint32_t	reserved4;
	uint32_t	ucdba;		/* aligned to 128-byte */
	uint32_t	ucdbau;		/* Upper 32-bits */
	uint32_t	rul : 16;	/* Response UPIU Length */
	uint32_t	ruo : 16;	/* Response UPIU Offset */
	uint32_t	prdtl : 16;	/* PRDT Length */
	uint32_t	prdto : 16;	/* PRDT Offset */
} utrd_header_t;	/* 8 words with little endian */

/* UTP Task Management Request Descriptor */
typedef struct utp_utmrd {
	/* 4 words with little endian */
	uint32_t	reserved0 : 24;
	uint32_t	i : 1;		/* interrupt */
	uint32_t	reserved1 : 7;
	uint32_t	reserved2;
	uint32_t	ocs : 8;	/* Overall Command Status */
	uint32_t	reserved3 : 24;
	uint32_t	reserved4;

	/* followed by 8 words UPIU with big endian */

	/* followed by 8 words Response UPIU with big endian */
} utp_utmrd_t;

/* NOP OUT UPIU */
typedef struct nop_out_upiu {
	uint8_t		trans_type;
	uint8_t		flags;
	uint8_t		reserved0;
	uint8_t		task_tag;
	uint8_t		reserved1;
	uint8_t		reserved2;
	uint8_t		reserved3;
	uint8_t		reserved4;
	uint8_t		total_ehs_len;
	uint8_t		reserved5;
	uint16_t	data_segment_len;
	uint32_t	reserved6;
	uint32_t	reserved7;
	uint32_t	reserved8;
	uint32_t	reserved9;
	uint32_t	reserved10;
	uint32_t	e2ecrc;
} nop_out_upiu_t;	/* 36 bytes with big endian */

/* NOP IN UPIU */
typedef struct nop_in_upiu {
	uint8_t		trans_type;
	uint8_t		flags;
	uint8_t		reserved0;
	uint8_t		task_tag;
	uint8_t		reserved1;
	uint8_t		reserved2;
	uint8_t		response;
	uint8_t		reserved3;
	uint8_t		total_ehs_len;
	uint8_t		dev_info;
	uint16_t	data_segment_len;
	uint32_t	reserved4;
	uint32_t	reserved5;
	uint32_t	reserved6;
	uint32_t	reserved7;
	uint32_t	reserved8;
	uint32_t	e2ecrc;
} nop_in_upiu_t;	/* 36 bytes with big endian */

/* Command UPIU */
typedef struct cmd_upiu {
	uint8_t		trans_type;
	uint8_t		flags;
	uint8_t		lun;
	uint8_t		task_tag;
	uint8_t		cmd_set_type;
	uint8_t		reserved0;
	uint8_t		reserved1;
	uint8_t		reserved2;
	uint8_t		total_ehs_len;
	uint8_t		reserved3;
	uint16_t	data_segment_len;
	uint32_t	exp_data_trans_len;
	/*
	 * A CDB has a fixed length of 16bytes or a variable length
	 * of between 12 and 260 bytes
	 */
	uint8_t		cdb[16];	/* little endian */
} cmd_upiu_t;	/* 32 bytes with big endian except for cdb[] */

typedef struct query_desc {
	uint8_t		opcode;
	uint8_t		idn;
	uint8_t		index;
	uint8_t		selector;
	uint8_t		reserved0[2];
	uint16_t	length;
	uint32_t	reserved2[2];
} query_desc_t;		/* 16 bytes with big endian */

typedef struct query_flag {
	uint8_t		opcode;
	uint8_t		idn;
	uint8_t		index;
	uint8_t		selector;
	uint8_t		reserved0[7];
	uint8_t		value;
	uint32_t	reserved8;
} query_flag_t;		/* 16 bytes with big endian */

typedef struct query_attr {
	uint8_t		opcode;
	uint8_t		idn;
	uint8_t		index;
	uint8_t		selector;
	uint8_t		reserved0[4];
	uint32_t	value;	/* little endian */
	uint32_t	reserved4;
} query_attr_t;		/* 16 bytes with big endian except for value */

/* Query Request UPIU */
typedef struct query_upiu {
	uint8_t		trans_type;
	uint8_t		flags;
	uint8_t		reserved0;
	uint8_t		task_tag;
	uint8_t		reserved1;
	uint8_t		query_func;
	uint8_t		reserved2;
	uint8_t		reserved3;
	uint8_t		total_ehs_len;
	uint8_t		reserved4;
	uint16_t	data_segment_len;
	/* Transaction Specific Fields */
	union {
		query_desc_t	desc;
		query_flag_t	flag;
		query_attr_t	attr;
	} ts;
	uint32_t	reserved5;
} query_upiu_t; /* 32 bytes with big endian */

/* Query Response UPIU */
typedef struct query_resp_upiu {
	uint8_t		trans_type;
	uint8_t		flags;
	uint8_t		reserved0;
	uint8_t		task_tag;
	uint8_t		reserved1;
	uint8_t		query_func;
	uint8_t		query_resp;
	uint8_t		reserved2;
	uint8_t		total_ehs_len;
	uint8_t		dev_info;
	uint16_t	data_segment_len;
	union {
		query_desc_t	desc;
		query_flag_t	flag;
		query_attr_t	attr;
	} ts;
	uint32_t	reserved3;
} query_resp_upiu_t;	/* 32 bytes with big endian */

/* Response UPIU */
typedef struct resp_upiu {
	uint8_t		trans_type;
	uint8_t		flags;
	uint8_t		lun;
	uint8_t		task_tag;
	uint8_t		cmd_set_type;
	uint8_t		reserved0;
	uint8_t		reserved1;
	uint8_t		status;
	uint8_t		total_ehs_len;
	uint8_t		dev_info;
	uint16_t	data_segment_len;
	uint32_t	res_trans_cnt;	/* Residual Transfer Count */
	uint32_t	reserved2[4];
	uint16_t	sense_data_len;
	union {
		uint8_t		sense_data[18];
		sense_data_t	sense;
	} sd;
} resp_upiu_t;		/* 52 bytes with big endian */

typedef struct cmd_info {
	uintptr_t	buf;
	size_t		length;
	int		lba;
	uint8_t		op;
	uint8_t		direction;
	uint8_t		lun;
} cmd_info_t;

typedef struct utp_utrd {
	uintptr_t	header;		/* utrd_header_t */
	uintptr_t	upiu;
	uintptr_t	resp_upiu;
	uintptr_t	prdt;
	size_t		size_upiu;
	size_t		size_resp_upiu;
	size_t		size_prdt;
	int		task_tag;
} utp_utrd_t;

/* Physical Region Description Table */
typedef struct prdt {
	uint32_t	dba;		/* Data Base Address */
	uint32_t	dbau;		/* Data Base Address Upper 32-bits */
	uint32_t	reserved0;
	uint32_t	dbc : 18;	/* Data Byte Count */
	uint32_t	reserved1 : 14;
} prdt_t;

typedef struct uic_cmd {
	uint32_t	op;
	uint32_t	arg1;
	uint32_t	arg2;
	uint32_t	arg3;
} uic_cmd_t;

typedef struct ufs_params {
	uintptr_t	reg_base;
	uintptr_t	desc_base;
	size_t		desc_size;
	unsigned long	flags;
} ufs_params_t;

typedef struct ufs_ops {
	int		(*phy_init)(ufs_params_t *params);
	int		(*phy_set_pwr_mode)(ufs_params_t *params);
} ufs_ops_t;

int ufshc_send_uic_cmd(uintptr_t base, uic_cmd_t *cmd);
int ufshc_dme_get(unsigned int attr, unsigned int idx, unsigned int *val);
int ufshc_dme_set(unsigned int attr, unsigned int idx, unsigned int val);

unsigned int ufs_read_attr(int idn);
void ufs_write_attr(int idn, unsigned int value);
unsigned int ufs_read_flag(int idn);
void ufs_set_flag(int idn);
void ufs_clear_flag(int idn);
void ufs_read_desc(int idn, int index, uintptr_t buf, size_t size);
void ufs_write_desc(int idn, int index, uintptr_t buf, size_t size);
size_t ufs_read_blocks(int lun, int lba, uintptr_t buf, size_t size);
size_t ufs_write_blocks(int lun, int lba, const uintptr_t buf, size_t size);
int ufs_init(const ufs_ops_t *ops, ufs_params_t *params);

#endif /* __UFS_H__ */
