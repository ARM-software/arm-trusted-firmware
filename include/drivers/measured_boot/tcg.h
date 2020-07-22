/*
 * Copyright (c) 2020, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef TCG_H
#define TCG_H

#include <stdint.h>

#define TCG_ID_EVENT_SIGNATURE_03	"Spec ID Event03"
#define TCG_STARTUP_LOCALITY_SIGNATURE	"StartupLocality"

#define TCG_SPEC_VERSION_MAJOR_TPM2   2
#define TCG_SPEC_VERSION_MINOR_TPM2   0
#define TCG_SPEC_ERRATA_TPM2          2

/*
 * Event types
 * Ref. Table 9 Events
 * TCG PC Client Platform Firmware Profile Specification.
 */
#define EV_PREBOOT_CERT				U(0x00000000)
#define EV_POST_CODE				U(0x00000001)
#define	EV_UNUSED				U(0x00000002)
#define EV_NO_ACTION				U(0x00000003)
#define EV_SEPARATOR				U(0x00000004)
#define EV_ACTION				U(0x00000005)
#define	EV_EVENT_TAG				U(0x00000006)
#define EV_S_CRTM_CONTENTS			U(0x00000007)
#define EV_S_CRTM_VERSION			U(0x00000008)
#define EV_CPU_MICROCODE			U(0x00000009)
#define EV_PLATFORM_CONFIG_FLAGS		U(0x0000000A)
#define EV_TABLE_OF_DEVICES			U(0x0000000B)
#define EV_COMPACT_HASH				U(0x0000000C)
#define	EV_IPL					U(0x0000000D)
#define	EV_IPL_PARTITION_DATA			U(0x0000000E)
#define EV_NONHOST_CODE				U(0x0000000F)
#define EV_NONHOST_CONFIG			U(0x00000010)
#define EV_NONHOST_INFO				U(0x00000011)
#define EV_OMIT_BOOT_DEVICE_EVENTS		U(0x00000012)
#define	EV_EFI_EVENT_BASE			U(0x80000000)
#define	EV_EFI_VARIABLE_DRIVER_CONFIG		U(0x80000001)
#define EV_EFI_VARIABLE_BOOT			U(0x80000002)
#define	EV_EFI_BOOT_SERVICES_APPLICATION	U(0x80000003)
#define	EV_EFI_BOOT_SERVICES_DRIVER		U(0x80000004)
#define	EV_EFI_RUNTIME_SERVICES_DRIVER		U(0x80000005)
#define	EV_EFI_GPT_EVENT			U(0x80000006)
#define	EV_EFI_ACTION				U(0x80000007)
#define	EV_EFI_PLATFORM_FIRMWARE_BLOB		U(0x80000008)
#define	EV_EFI_HANDOFF_TABLES			U(0x80000009)
#define	EV_EFI_HCRTM_EVENT			U(0x80000010)
#define	EV_EFI_VARIABLE_AUTHORITY		U(0x800000E0)

/*
 * TPM_ALG_ID constants.
 * Ref. Table 9 - Definition of (UINT16) TPM_ALG_ID Constants
 * Trusted Platform Module Library. Part 2: Structures
 */
#define TPM_ALG_SHA256		0x000B
#define TPM_ALG_SHA384		0x000C
#define TPM_ALG_SHA512		0x000D

/* TCG Platform Type */
#define PLATFORM_CLASS_CLIENT   0
#define PLATFORM_CLASS_SERVER   1

/* SHA digest sizes in bytes */
#define SHA1_DIGEST_SIZE	20
#define SHA256_DIGEST_SIZE	32
#define SHA384_DIGEST_SIZE	48
#define SHA512_DIGEST_SIZE	64

enum {
	/*
	 * SRTM, BIOS, Host Platform Extensions, Embedded
	 * Option ROMs and PI Drivers
	 */
	PCR_0 = 0,
	/* Host Platform Configuration */
	PCR_1,
	/* UEFI driver and application Code */
	PCR_2,
	/* UEFI driver and application Configuration and Data */
	PCR_3,
	/* UEFI Boot Manager Code (usually the MBR) and Boot Attempts */
	PCR_4,
	/*
	 * Boot Manager Code Configuration and Data (for use
	 * by the Boot Manager Code) and GPT/Partition Table
	 */
	PCR_5,
	/* Host Platform Manufacturer Specific */
	PCR_6,
	/* Secure Boot Policy */
	PCR_7,
	/* 8-15: Defined for use by the Static OS */
	PCR_8,
	/* Debug */
	PCR_16 = 16
};

#pragma pack(push, 1)

/*
 * PCR Event Header
 * TCG EFI Protocol Specification
 * 5.3 Event Log Header
 */
typedef struct {
	/* PCRIndex:
	 * The PCR Index to which this event is extended
	 */
	uint32_t	pcr_index;

	/* EventType:
	 * SHALL be an EV_NO_ACTION event
	 */
	uint32_t	event_type;

	/* SHALL be 20 Bytes of 0x00 */
	uint8_t		digest[SHA1_DIGEST_SIZE];

	/* The size of the event */
	uint32_t	event_size;

	/* SHALL be a TCG_EfiSpecIdEvent */
	uint8_t		event[];	/* [event_data_size] */
} tcg_pcr_event_t;

/*
 * Log Header Entry Data
 * Ref. Table 14 TCG_EfiSpecIdEventAlgorithmSize
 * TCG PC Client Platform Firmware Profile 9.4.5.1
 */
typedef struct {
	/* Algorithm ID (hashAlg) of the Hash used by BIOS */
	uint16_t	algorithm_id;

	/* The size of the digest produced by the implemented Hash algorithm */
	uint16_t	digest_size;
} id_event_algorithm_size_t;

/*
 * TCG_EfiSpecIdEvent structure
 * Ref. Table 15 TCG_EfiSpecIdEvent
 * TCG PC Client Platform Firmware Profile 9.4.5.1
 */
typedef struct {
	/*
	 * The NUL-terminated ASCII string "Spec ID Event03".
	 * SHALL be set to {0x53, 0x70, 0x65, 0x63, 0x20, 0x49, 0x44,
	 * 0x20, 0x45, 0x76, 0x65, 0x6e, 0x74, 0x30, 0x33, 0x00}.
	 */
	uint8_t		signature[16];

	/*
	 * The value for the Platform Class.
	 * The enumeration is defined in the TCG ACPI Specification Client
	 * Common Header.
	 */
	uint32_t	platform_class;

	/*
	 * The PC Client Platform Profile Specification minor version number
	 * this BIOS supports.
	 * Any BIOS supporting this version (2.0) MUST set this value to 0x00.
	 */
	uint8_t		spec_version_minor;

	/*
	 * The PC Client Platform Profile Specification major version number
	 * this BIOS supports.
	 * Any BIOS supporting this version (2.0) MUST set this value to 0x02.
	 */
	uint8_t		spec_version_major;

	/*
	 * The PC Client Platform Profile Specification errata version number
	 * this BIOS supports.
	 * Any BIOS supporting this version (2.0) MUST set this value to 0x02.
	 */
	uint8_t		spec_errata;

	/*
	 * Specifies the size of the UINTN fields used in various data
	 * structures used in this specification.
	 * 0x01 indicates UINT32 and 0x02 indicates UINT64.
	 */
	uint8_t		uintn_size;

	/*
	 * The number of Hash algorithms in the digestSizes field.
	 * This field MUST be set to a value of 0x01 or greater.
	 */
	uint32_t	number_of_algorithms;

	/*
	 * Each TCG_EfiSpecIdEventAlgorithmSize SHALL contain an algorithmId
	 * and digestSize for each hash algorithm used in the TCG_PCR_EVENT2
	 * structure, the first of which is a Hash algorithmID and the second
	 * is the size of the respective digest.
	 */
	id_event_algorithm_size_t    digest_size[]; /* number_of_algorithms */
} id_event_struct_header_t;

typedef struct {
	/*
	 * Size in bytes of the VendorInfo field.
	 * Maximum value MUST be FFh bytes.
	 */
	uint8_t		vendor_info_size;

	/*
	 * Provided for use by Platform Firmware implementer. The value might
	 * be used, for example, to provide more detailed information about the
	 * specific BIOS such as BIOS revision numbers, etc. The values within
	 * this field are not standardized and are implementer-specific.
	 * Platform-specific or -unique information MUST NOT be provided in
	 * this field.
	 *
	 */
	uint8_t		vendor_info[];	/* [vendorInfoSize] */
} id_event_struct_data_t;

typedef struct {
	id_event_struct_header_t	struct_header;
	id_event_struct_data_t		struct_data;
} id_event_struct_t;

typedef struct {
	tcg_pcr_event_t			header;
	id_event_struct_header_t	struct_header;
} id_event_headers_t;

/* TPMT_HA Structure */
typedef struct {
	/* Selector of the hash contained in the digest that implies
	 * the size of the digest
	 */
	uint16_t	algorithm_id;	/* AlgorithmId */

	/* Digest, depends on AlgorithmId */
	uint8_t		digest[];	/* Digest[] */
} tpmt_ha;

/*
 * TPML_DIGEST_VALUES Structure
 */
typedef struct {
	/* The number of digests in the list */
	uint32_t	count;			/* Count */

	/* The list of tagged digests, as sent to the TPM as part of a
	 * TPM2_PCR_Extend or as received from a TPM2_PCR_Event command
	 */
	tpmt_ha		digests[];		/* Digests[Count] */
} tpml_digest_values;

/*
 * TCG_PCR_EVENT2 header
 */
typedef struct {
	 /* The PCR Index to which this event was extended */
	uint32_t		pcr_index;	/* PCRIndex */

	/* Type of event */
	uint32_t		event_type;	/* EventType */

	/* Digests:
	 * A counted list of tagged digests, which contain the digest of
	 * the event data (or external data) for all active PCR banks
	 */
	tpml_digest_values	digests;	/* Digests */
} event2_header_t;

typedef struct event2_data {
	/* The size of the event data */
	uint32_t		event_size;	/* EventSize */

	/* The data of the event */
	uint8_t			event[];	/* Event[EventSize] */
} event2_data_t;

/*
 * Startup Locality Event
 * Ref. TCG PC Client Platform Firmware Profile 9.4.5.3
 */
typedef struct {
	/*
	 * The NUL-terminated ASCII string "StartupLocality" SHALL be
	 * set to {0x53 0x74 0x61 0x72 0x74 0x75 0x70 0x4C 0x6F 0x63
	 * 0x61 0x6C 0x69 0x74 0x79 0x00}
	 */
	uint8_t		signature[16];

	/* The Locality Indicator which sent the TPM2_Startup command */
	uint8_t		startup_locality;
} startup_locality_event_t;

#pragma pack(pop)

#endif /* TCG_H */
