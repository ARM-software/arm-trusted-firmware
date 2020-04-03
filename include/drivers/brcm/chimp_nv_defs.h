/*
 * Copyright (c) 2016 - 2020, Broadcom
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef BNXNVM_DEFS_H
#define BNXNVM_DEFS_H

#if defined(__GNUC__)
	#define PACKED_STRUCT __packed
#else /* non-GCC compiler */

#ifndef DOS_DRIVERS
		#pragma pack(push)
		#pragma pack(1)
#endif
		#define PACKED_STRUCT
#endif

typedef uint32_t u32_t;
typedef uint8_t u8_t;
typedef uint16_t u16_t;

#define BNXNVM_DEFAULT_BLOCK_SIZE		4096
#define BNXNVM_UNUSED_BYTE_VALUE		0xff

#define NV_MAX_BLOCK_SIZE		16384

#define BITS_PER_BYTE		(8)
#define SIZEOF_IN_BITS(x)	(sizeof(x)*BITS_PER_BYTE)

/************************/
/* byte-swapping macros */
/************************/
#define BYTE_SWAP_16(x)	\
	((((u16_t)(x) & 0xff00) >> 8) | \
	(((u16_t)(x) & 0x00ff) << 8))
#define BYTE_SWAP_32(x)	\
	((((u32_t)(x) & 0xff000000) >> 24) | \
	(((u32_t)(x) & 0x00ff0000) >> 8)   | \
	(((u32_t)(x) & 0x0000ff00) << 8)   | \
	(((u32_t)(x) & 0x000000ff) << 24))

/* auto-detect integer size */
#define BYTE_SWAP_INT(x)	\
	(SIZEOF_IN_BITS(x) == 16 ? BYTE_SWAP_16(x) : \
		SIZEOF_IN_BITS(x) == 32 ? BYTE_SWAP_32(x) : (x))

/********************************/
/* Architecture-specific macros */
/********************************/
#ifdef __BIG_ENDIAN__	/* e.g. Motorola */

	#define BE_INT16(x)		(x)
	#define BE_INT32(x)		(x)
	#define BE_INT(x)		(x)
	#define LE_INT16(x)		BYTE_SWAP_16(x)
	#define LE_INT32(x)		BYTE_SWAP_32(x)
	#define LE_INT(x)		BYTE_SWAP_INT(x)

#else	/* Little Endian (e.g. Intel) */

	#define LE_INT16(x)		(x)
	#define LE_INT32(x)		(x)
	#define LE_INT(x)		(x)
	#define BE_INT16(x)		BYTE_SWAP_16(x)
	#define BE_INT32(x)		BYTE_SWAP_32(x)
	#define BE_INT(x)		BYTE_SWAP_INT(x)

#endif


enum {
	NV_OK = 0,
	NV_NOT_NVRAM,
	NV_BAD_MB,
	NV_BAD_DIR_HEADER,
	NV_BAD_DIR_ENTRY,
	NV_FW_NOT_FOUND,
};

typedef struct {
#define BNXNVM_MASTER_BLOCK_SIG	BE_INT32(0x424E5834)	/*"BNX4"*/
	/* Signature*/
	u32_t    sig;
	/* Length of Master Block Header, in bytes [32] */
	u32_t    length;
	/* Block size, in bytes [4096] */
	u32_t    block_size;
	/* Byte-offset to Directory Block (translated) */
	u32_t    directory_offset;
	/* Byte-offset to Block Redirection Table (non-translated) */
	u32_t    redirect_offset;
	/* Size, in bytes of Reserved Blocks region (at end of NVRAM) */
	u32_t    reserved_size;
	/*
	 * Size of NVRAM (in bytes) - may be used to
	 * override auto-detected size
	 */
	u32_t    nvram_size;
	/* CRC-32 (IEEE 802.3 compatible) of the above */
	u32_t    chksum;
} PACKED_STRUCT bnxnvm_master_block_header_t;

typedef struct {
#define BNXNVM_DIRECTORY_BLOCK_SIG BE_INT32(0x44697230)	/* "Dir0" */
	/* Signature */
	u32_t    sig;
	/* Length of Directory Header, in bytes [16] */
	u32_t    length;
	/* Number of Directory Entries */
	u32_t    entries;
	/* Length of each Directory Entry, in bytes [24] */
	u32_t    entry_length;
} PACKED_STRUCT bnxnvm_directory_block_header_t;

typedef struct {
	/* Directory Entry Type (see enum bnxnvm_directory_type) */
	u16_t    type;
	/* Instance of this Directory Entry type (0-based) */
	u16_t    ordinal;
	/*
	 * Directory Entry Extension flags used to identify
	 * secondary instances of a type:ordinal combinations
	 */
	u16_t    ext;
	/* Directory Entry Attribute flags used to describe the item contents */
	u16_t    attr;
	/* Item location in NVRAM specified as offset (in bytes) */
	u32_t	 item_location;
	/*
	 * Length of NVRAM item in bytes
	 * (including padding - multiple of block size)
	 */
	u32_t    item_length;
	/* Length of item data in bytes (excluding padding) */
	u32_t    data_length;
	/*
	 * CRC-32 (IEEE 802.3 compatible) of item data
	 * (excluding padding) (optional)
	 */
	u32_t    data_chksum;
} PACKED_STRUCT bnxnvm_directory_entry_t;

enum bnxnvm_version_format {
	/* US-ASCII string (not necessarily null-terminated) */
	BNX_VERSION_FMT_ASCII				= 0,
	/* Each field 16-bits, displayed as unpadded decimal (e.g. "1.2.3.4") */
	BNX_VERSION_FMT_DEC				= 1,
	/* A single hexadecimal value, up to 64-bits (no dots) */
	BNX_VERSION_FMT_HEX				= 2,
	/* Multiple version values (three 8-bit version fields) */
	BNX_VERSION_FMT_MULTI				= 3
};

/* This structure definition must not change: */
typedef struct {
	u16_t	flags; /* bit-flags (defaults to 0x0000) */
	u8_t	version_format; /* enum bnxnvm_version_format */
	u8_t	version_length; /* in bytes */
	u8_t	version[16];		/* version value */
	u16_t	dir_type;		/* enum bnxnvm_directory_type */
	/* size of the entire trailer (to locate end of component data) */
	u16_t	trailer_length;
#define BNXNVM_COMPONENT_TRAILER_SIG BE_INT32(0x54726c72)	/* "Trlr" */
	u32_t	sig;
	u32_t	chksum;	/* CRC-32 of all bytes to this point */
} PACKED_STRUCT bnxnvm_component_trailer_base_t;

typedef struct {
	/*
	 * new trailer members (e.g. digital signature)
	 * go here (insert at top):
	 */
	u8_t rsa_sig[256]; /* 2048-bit RSA-encrypted SHA-256 hash */
	bnxnvm_component_trailer_base_t	base;
} PACKED_STRUCT bnxnvm_component_trailer_t;

#define BNX_MAX_LEN_DIR_NAME		12
#define BNX_MAX_LEN_DIR_DESC		50
/*********************************************************
 * NVRAM Directory Entry/Item Types, Names, and Descriptions
 *
 * If you see a name or description that needs improvement,
 * please correct it or raise for discussion.
 * When adding a new directory type, it would be appreciated
 * if you also updated ../../libs/nvm/bnxt_nvm_str.c.
 * DIR_NAME macros may contain up to 12 alpha-numeric
 * US-ASCII characters only, camelCase is preferred for clarity.
 * DIR_DESC macros may contain up to 50 US-ASCII characters
 * providing a verbose description of the directory type.
 */
enum bnxnvm_directory_type {
	/* 0x00 Unused directory entry, available for use */
		BNX_DIR_TYPE_UNUSED				= 0,
#define BNX_DIR_NAME_UNUSED				"unused"
#define BNX_DIR_DESC_UNUSED				"Deleted directory entry, available for reuse"
	/* 0x01 Package installation log */
	BNX_DIR_TYPE_PKG_LOG			= 1,
#define BNX_DIR_NAME_PKG_LOG			"pkgLog"
#define BNX_DIR_DESC_PKG_LOG			"Package Installation Log"
	BNX_DIR_TYPE_CHIMP_PATCH        = 3,
#define BNX_DIR_NAME_CHIMP_PATCH		"chimpPatch"
#define BNX_DIR_DESC_CHIMP_PATCH		"ChiMP Patch Firmware"
	/* 0x04 ChiMP firmware: Boot Code phase 1 */
	BNX_DIR_TYPE_BOOTCODE			= 4,
#define BNX_DIR_NAME_BOOTCODE			"chimpBoot"
#define BNX_DIR_DESC_BOOTCODE			"Chip Management Processor Boot Firmware"
	/* 0x05 VPD data block */
	BNX_DIR_TYPE_VPD				= 5,
#define BNX_DIR_NAME_VPD				"VPD"
#define BNX_DIR_DESC_VPD				"Vital Product Data"
	/* 0x06 Exp ROM MBA */
	BNX_DIR_TYPE_EXP_ROM_MBA		= 6,
#define BNX_DIR_NAME_EXP_ROM_MBA		"MBA"
#define BNX_DIR_DESC_EXP_ROM_MBA		"Multiple Boot Agent Expansion ROM"
	BNX_DIR_TYPE_AVS		= 7,	/* 0x07 AVS FW */
#define BNX_DIR_NAME_AVS				"AVS"
#define BNX_DIR_DESC_AVS				"Adaptive Voltage Scaling Firmware"
	BNX_DIR_TYPE_PCIE	= 8,	/* 0x08 PCIE FW */
#define BNX_DIR_NAME_PCIE				"PCIEucode"
#define BNX_DIR_DESC_PCIE				"PCIe Microcode"
	BNX_DIR_TYPE_PORT_MACRO			= 9,	/* 0x09 PORT MACRO FW */
#define BNX_DIR_NAME_PORT_MACRO			"portMacro"
#define BNX_DIR_DESC_PORT_MACRO			"Port Macro Firmware"
	BNX_DIR_TYPE_APE_FW		= 10,		/* 0x0A APE Firmware */
#define BNX_DIR_NAME_APE_FW				"apeFW"
#define BNX_DIR_DESC_APE_FW			"Application Processing Engine Firmware"
	/* 0x0B Patch firmware executed by APE ROM */
	BNX_DIR_TYPE_APE_PATCH		= 11,
#define BNX_DIR_NAME_APE_PATCH			"apePatch"
#define BNX_DIR_DESC_APE_PATCH			"APE Patch Firmware"
	BNX_DIR_TYPE_KONG_FW		= 12,	/* 0x0C Kong Firmware */
#define BNX_DIR_NAME_KONG_FW		"kongFW"
#define BNX_DIR_DESC_KONG_FW		"Kong Firmware"
	/* 0x0D Patch firmware executed by Kong ROM */
	BNX_DIR_TYPE_KONG_PATCH		= 13,
#define BNX_DIR_NAME_KONG_PATCH			"kongPatch"
#define BNX_DIR_DESC_KONG_PATCH			"Kong Patch Firmware"
	BNX_DIR_TYPE_BONO_FW		= 14,	/* 0x0E Bono Firmware */
#define BNX_DIR_NAME_BONO_FW		"bonoFW"
#define BNX_DIR_DESC_BONO_FW		"Bono Firmware"
	/* 0x0F Patch firmware executed by Bono ROM */
	BNX_DIR_TYPE_BONO_PATCH		= 15,
#define BNX_DIR_NAME_BONO_PATCH			"bonoPatch"
#define BNX_DIR_DESC_BONO_PATCH			"Bono Patch Firmware"
	BNX_DIR_TYPE_TANG_FW		= 16,	/* 0x10 Tang firmware */
#define BNX_DIR_NAME_TANG_FW			"tangFW"
#define BNX_DIR_DESC_TANG_FW			"Tang Firmware"
	/* 0x11 Patch firmware executed by Tang ROM */
	BNX_DIR_TYPE_TANG_PATCH		= 17,
#define BNX_DIR_NAME_TANG_PATCH			"tangPatch"
#define BNX_DIR_DESC_TANG_PATCH			"Tang Patch Firmware"
	/* 0x12 ChiMP firmware: Boot Code phase 2 (loaded by phase 1) */
	BNX_DIR_TYPE_BOOTCODE_2		= 18,
#define BNX_DIR_NAME_BOOTCODE_2			"chimpHWRM"
#define BNX_DIR_DESC_BOOTCODE_2			"ChiMP Hardware Resource Manager Firmware"
	BNX_DIR_TYPE_CCM	= 19,	/* 0x13 CCM ROM binary */
#define	BNX_DIR_NAME_CCM	"CCM"
#define BNX_DIR_DESC_CCM	"Comprehensive Configuration Management"
	/* 0x14 PCI-IDs, PCI-related configuration properties */
	BNX_DIR_TYPE_PCI_CFG	= 20,
#define BNX_DIR_NAME_PCI_CFG		"pciCFG"
#define BNX_DIR_DESC_PCI_CFG		"PCIe Configuration Data"

	BNX_DIR_TYPE_TSCF_UCODE		= 21,	/* 0x15 TSCF micro-code */
#define BNX_DIR_NAME_TSCF_UCODE		"PHYucode"
#define BNX_DIR_DESC_TSCF_UCODE		"Falcon PHY Microcode"
	BNX_DIR_TYPE_ISCSI_BOOT		= 22,	/* 0x16 iSCSI Boot */
#define BNX_DIR_NAME_ISCSI_BOOT			"iSCSIboot"
#define BNX_DIR_DESC_ISCSI_BOOT			"iSCSI Boot Software Initiator"
	/* 0x18 iSCSI Boot IPV6 - ***DEPRECATED*** */
	BNX_DIR_TYPE_ISCSI_BOOT_IPV6	= 24,
	/* 0x19 iSCSI Boot IPV4N6 - ***DEPRECATED*** */
	BNX_DIR_TYPE_ISCSI_BOOT_IPV4N6	= 25,
	BNX_DIR_TYPE_ISCSI_BOOT_CFG	= 26,	/* 0x1a iSCSI Boot CFG v6 */
#define BNX_DIR_NAME_ISCSI_BOOT_CFG		"iSCSIcfg"
#define BNX_DIR_DESC_ISCSI_BOOT_CFG		"iSCSI Boot Configuration Data"
	BNX_DIR_TYPE_EXT_PHY		= 27,	/* 0x1b External PHY FW */
#define BNX_DIR_NAME_EXT_PHY			"extPHYfw"
#define BNX_DIR_DESC_EXT_PHY			"External PHY Firmware"
	BNX_DIR_TYPE_MODULES_PN	= 28,	/* 0x1c Modules PartNum list */
#define BNX_DIR_NAME_MODULES_PN			"modPartNums"
#define BNX_DIR_DESC_MODULES_PN			"Optical Modules Part Number List"
	BNX_DIR_TYPE_SHARED_CFG	= 40,	/* 0x28 shared configuration block */
#define BNX_DIR_NAME_SHARED_CFG			"sharedCFG"
#define BNX_DIR_DESC_SHARED_CFG			"Shared Configuration Data"
	BNX_DIR_TYPE_PORT_CFG	= 41,	/* 0x29 port configuration block */
#define BNX_DIR_NAME_PORT_CFG			"portCFG"
#define BNX_DIR_DESC_PORT_CFG			"Port Configuration Data"
	BNX_DIR_TYPE_FUNC_CFG	= 42,	/* 0x2A func configuration block */
#define BNX_DIR_NAME_FUNC_CFG			"funcCFG"
#define BNX_DIR_DESC_FUNC_CFG			"Function Configuration Data"

	/* Management Firmware (TruManage) related dir entries*/
	/* 0x30 Management firmware configuration (see BMCFG library)*/
	BNX_DIR_TYPE_MGMT_CFG			= 48,
#define BNX_DIR_NAME_MGMT_CFG			"mgmtCFG"
#define BNX_DIR_DESC_MGMT_CFG			"Out-of-band Management Configuration Data"
	BNX_DIR_TYPE_MGMT_DATA	= 49,	/* 0x31 "Opaque Management Data" */
#define BNX_DIR_NAME_MGMT_DATA			"mgmtData"
#define BNX_DIR_DESC_MGMT_DATA			"Out-of-band Management Data"
	BNX_DIR_TYPE_MGMT_WEB_DATA = 50,	/* 0x32 "Web GUI" file data */
#define BNX_DIR_NAME_MGMT_WEB_DATA		"webData"
#define BNX_DIR_DESC_MGMT_WEB_DATA		"Out-of-band Management Web Data"
	/* 0x33 "Web GUI" file metadata */
	BNX_DIR_TYPE_MGMT_WEB_META = 51,
#define BNX_DIR_NAME_MGMT_WEB_META		"webMeta"
#define BNX_DIR_DESC_MGMT_WEB_META		"Out-of-band Management Web Metadata"
	/* 0x34 Management firmware Event Log (a.k.a. "SEL") */
	BNX_DIR_TYPE_MGMT_EVENT_LOG	= 52,
#define BNX_DIR_NAME_MGMT_EVENT_LOG		"eventLog"
#define BNX_DIR_DESC_MGMT_EVENT_LOG		"Out-of-band Management Event Log"
	/* 0x35 Management firmware Audit Log */
	BNX_DIR_TYPE_MGMT_AUDIT_LOG	= 53
#define BNX_DIR_NAME_MGMT_AUDIT_LOG		"auditLog"
#define BNX_DIR_DESC_MGMT_AUDIT_LOG		"Out-of-band Management Audit Log"

};

/* For backwards compatibility only, may be removed later */
#define BNX_DIR_TYPE_ISCSI_BOOT_CFG6 BNX_DIR_TYPE_ISCSI_BOOT_CFG

/* Firmware NVM items of "APE BIN" format are identified with
 * the following macro:
 */
#define BNX_DIR_TYPE_IS_APE_BIN_FMT(type)\
	((type) == BNX_DIR_TYPE_CHIMP_PATCH	\
		|| (type) == BNX_DIR_TYPE_BOOTCODE		\
		|| (type) == BNX_DIR_TYPE_BOOTCODE_2	\
		|| (type) == BNX_DIR_TYPE_APE_FW		\
		|| (type) == BNX_DIR_TYPE_APE_PATCH		\
		|| (type) == BNX_DIR_TYPE_TANG_FW		\
		|| (type) == BNX_DIR_TYPE_TANG_PATCH	\
		|| (type) == BNX_DIR_TYPE_KONG_FW		\
		|| (type) == BNX_DIR_TYPE_KONG_PATCH	\
		|| (type) == BNX_DIR_TYPE_BONO_FW		\
		|| (type) == BNX_DIR_TYPE_BONO_PATCH	\
	)

/* Other (non APE BIN) executable NVM items are identified with
 * the following macro:
 */
#define BNX_DIR_TYPE_IS_OTHER_EXEC(type)\
	((type) == BNX_DIR_TYPE_AVS	\
		|| (type) == BNX_DIR_TYPE_EXP_ROM_MBA	\
		|| (type) == BNX_DIR_TYPE_PCIE			\
		|| (type) == BNX_DIR_TYPE_TSCF_UCODE	\
		|| (type) == BNX_DIR_TYPE_EXT_PHY		\
		|| (type) == BNX_DIR_TYPE_CCM			\
		|| (type) == BNX_DIR_TYPE_ISCSI_BOOT	\
	)

/* Executable NVM items (e.g. microcode, firmware, software) identified
 * with the following macro
 */
#define BNX_DIR_TYPE_IS_EXECUTABLE(type)	\
	(BNX_DIR_TYPE_IS_APE_BIN_FMT(type) \
	|| BNX_DIR_TYPE_IS_OTHER_EXEC(type))

#define BNX_DIR_ORDINAL_FIRST	0	/* Ordinals are 0-based */

/* No extension flags for this directory entry */
#define BNX_DIR_EXT_NONE	0
/* Directory entry is inactive (not used, not hidden,
 * not available for reuse)
 */
#define BNX_DIR_EXT_INACTIVE	(1 << 0)
/* Directory content is a temporary staging location for
 * updating the primary (non-update) directory entry contents
 * (e.g. performing a secure firmware update)
 */
#define BNX_DIR_EXT_UPDATE	(1 << 1)

/* No attribute flags set for this directory entry */
#define BNX_DIR_ATTR_NONE	0
/* Directory entry checksum of contents is purposely incorrect */
#define BNX_DIR_ATTR_NO_CHKSUM	(1 << 0)
/* Directory contents are in the form of a property-stream
 * (e.g. configuration properties)
 */
#define BNX_DIR_ATTR_PROP_STREAM	(1 << 1)
/* Directory content (e.g. iSCSI boot) supports IPv4 */
#define BNX_DIR_ATTR_IPv4	(1 << 2)
/* Directory content (e.g. iSCSI boot) supports IPv6 */
#define BNX_DIR_ATTR_IPv6	(1 << 3)
/* Directory content includes standard NVM component trailer
 * (bnxnvm_component_trailer_t)
 */
#define BNX_DIR_ATTR_TRAILER	(1 << 4)

/* Index of tab-delimited fields in each package log
 * (BNX_DIR_TYPE_PKG_LOG) record (\n-terminated line):
 */
enum bnxnvm_pkglog_field_index {
	/* Package installation date/time in ISO-8601 format */
	BNX_PKG_LOG_FIELD_IDX_INSTALLED_TIMESTAMP	= 0,
	/* Installed package description (from package header) or "N/A" */
	BNX_PKG_LOG_FIELD_IDX_PKG_DESCRIPTION	= 1,
	/* Installed package version string (from package header) or "N/A" */
	BNX_PKG_LOG_FIELD_IDX_PKG_VERSION	= 2,
	/* Installed package creation/modification timestamp (ISO-8601) */
	BNX_PKG_LOG_FIELD_IDX_PKG_TIMESTAMP = 3,
	/* Installed package checksum in hexadecimal (CRC-32) or "N/A" */
	BNX_PKG_LOG_FIELD_IDX_PKG_CHECKSUM	= 4,
	/* Total number of packaged items applied in this installation */
	BNX_PKG_LOG_FIELD_IDX_INSTALLED_ITEMS	= 5,
	/* Hexadecimal bit-mask identifying which items were installed */
	BNX_PKG_LOG_FIELD_IDX_INSTALLED_MASK	= 6
};

#if !defined(__GNUC__)
#ifndef DOS_DRIVERS
	#pragma pack(pop)		/* original packing */
#endif
#endif

#endif /* Don't add anything after this line */
