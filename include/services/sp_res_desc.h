/*
 * Copyright (c) 2018, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SPM_RES_DESC_H
#define SPM_RES_DESC_H

#include <stdint.h>

#include <services/sp_res_desc_def.h>

/*******************************************************************************
 * Attribute Section
 ******************************************************************************/

struct sp_rd_sect_attribute {
	/*
	 * Version of the resource description.
	 */
	uint16_t version;

	/*
	 * Type of the Secure Partition:
	 * - bit[0]: SP Type
	 *   - b'0: UP SP
	 *   - b'1: MP SP
	 * If UP SP:
	 * - bit[1]: Type of UP SP
	 *   - b'0: Migratable UP SP
	 *   - b'1: Pinned UP SP
	 */
	uint16_t sp_type;

	/*
	 * If this is a Pinned UP SP, PE on which the Pinned UP SP will run.
	 */
	uint32_t pe_mpidr;

	/*
	 * Run-Time Exception Level:
	 * - 0: SEL0 SP
	 * - 1: SEL1 SP
	 */
	uint8_t runtime_el;

	/*
	 * Type of Execution:
	 * - 0: Init-time only
	 * - 1: Run-time Execution
	 */
	uint8_t exec_type;

	/*
	 * Expected behavior upon failure:
	 * - 0: Restartable
	 * - 1: One-Shot
	 */
	uint8_t panic_policy;

	/*
	 * Translation Granule to use in the SP translation regime:
	 * - 0: 4KB
	 * - 1: 16KB
	 * - 2: 64KB
	 */
	uint8_t xlat_granule;

	/*
	 * Size of the SP binary in bytes.
	 */
	uint32_t binary_size;

	/*
	 * - If SP is NOT PIE:
	 *   - VA Address where the SP expects to be loaded.
	 * - If SP is PIE:
	 *   - Ignored.
	 */
	uint64_t load_address;

	/*
	 * Initial execution address. This is a VA as the SP sees it.
	 */
	uint64_t entrypoint;
};

/*******************************************************************************
 * Memory Region Section
 ******************************************************************************/

struct sp_rd_sect_mem_region {
	/*
	 * Name of a Memory region, including null terminator. Reserved names:
	 * - "Client Shared Memory Region":
	 *   Memory region where memory shared by clients shall be mapped.
	 * - "Queue Memory Region":
	 *   Memory region shared with SPM for SP queue management.
	 */
	char name[RD_MEM_REGION_NAME_LEN];

	/*
	 * Memory Attributes:
	 * - bits[3:0]: Type of memory
	 *   - 0: Device
	 *   - 1: Code
	 *   - 2: Data
	 *   - 3: BSS
	 *   - 4: Read-only Data
	 *   - 5: SPM-to-SP Shared Memory Region
	 *   - 6: Client Shared Memory Region
	 *   - 7: Miscellaneous
	 * - If memory is { SPM-to-SP shared Memory, Client Shared Memory,
	 *   Miscellaneous }
	 *   - bits[4]: Position Independent
	 *     - b'0: Position Dependent
	 *     - b'1: Position Independent
	 */
	uint32_t attr;

	/*
	 * Base address of the memory region.
	 */
	uint64_t base;

	/*
	 * Size of the memory region.
	 */
	uint64_t size;

	/*
	 * Pointer to next memory region (or NULL if this is the last one).
	 */
	struct sp_rd_sect_mem_region *next;
};

/*******************************************************************************
 * Notification Section
 ******************************************************************************/

struct sp_rd_sect_notification {
	/*
	 * Notification attributes:
	 * - bit[31]: Notification Type
	 *   - b'0: Platform Notification
	 *   - b'1: Interrupt
	 * If Notification Type == Platform Notification
	 * - bits[15:0]: Implementation-defined Notification ID
	 * If Notification Type == Interrupt
	 * - bits[15:0]: IRQ number
	 * - bits[23:16]: Interrupt Priority
	 * - bit[24]: Trigger Type
	 *   - b'0: Edge Triggered
	 *   - b'1: Level Triggered
	 * - bit[25]: Trigger Level
	 *   - b'0: Falling or Low
	 *   - b'1: Rising or High
	 */
	uint32_t attr;

	/*
	 * Processing Element.
	 * If Notification Type == Interrupt && IRQ number is { SGI, LPI }
	 * - PE ID to which IRQ will be forwarded
	 */
	uint32_t pe;

	/*
	 * Pointer to next notification (or NULL if this is the last one).
	 */
	struct sp_rd_sect_notification *next;
};

/*******************************************************************************
 * Service Description Section
 ******************************************************************************/

struct sp_rd_sect_service {
	/*
	 * Service identifier.
	 */
	uint32_t uuid[4];

	/*
	 * Accessibility Options:
	 * - bit[0]: Accessibility by secure-world clients
	 *   - b'0: Not Accessible
	 *   - b'1: Accessible
	 * - bit[1]: Accessible by EL3
	 *   - b'0: Not Accessible
	 *   - b'1: Accessible
	 * - bit[2]: Accessible by normal-world clients
	 *   - b'0: Not Accessible
	 *   - b'1: Accessible
	 */
	uint8_t accessibility;

	/*
	 * Request type supported:
	 * - bit[0]: Blocking request
	 *   - b'0: Not Enable
	 *   - b'1: Enable
	 * - bit[1]: Non-blocking request
	 *   - b'0: Not Enable
	 *   - b'1: Enable
	 */
	uint8_t request_type;

	/*
	 * Maximum number of client connections that the service can support.
	 */
	uint16_t connection_quota;

	/*
	 * If the service requires secure world memory to be shared with its
	 * clients:
	 * - Maximum amount of secure world memory in bytes to reserve from the
	 *   secure world memory pool for the service.
	 */
	uint32_t secure_mem_size;

	/*
	 * Interrupt number used to notify the SP for the service.
	 * - Should also be enabled in the Notification Section.
	 */
	uint32_t interrupt_num;

	/*
	 * Pointer to next service (or NULL if this is the last one).
	 */
	struct sp_rd_sect_service *next;
};

/*******************************************************************************
 * Complete resource description struct
 ******************************************************************************/

struct sp_res_desc {

	/* Attribute Section */
	struct sp_rd_sect_attribute attribute;

	/* System Resource Section */
	struct sp_rd_sect_mem_region *mem_region;

	struct sp_rd_sect_notification *notification;

	/* Service Section */
	struct sp_rd_sect_service *service;
};

#endif /* SPM_RES_DESC_H */
