/*
 * Copyright (c) 2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */
#ifndef MPINFO_H
#define MPINFO_H

#include <stdbool.h>
#include <stdint.h>
#include <lib/utils_def.h>

/*
 * Value used in the NumberProcessors parameter of the GetProcessorInfo function
 */
#define CPU_V2_EXTENDED_TOPOLOGY  UL(1 << 24)

/*
 * This bit is used in the StatusFlag field of EFI_PROCESSOR_INFORMATION and
 * indicates whether the processor is playing the role of BSP. If the bit is 1,
 * then the processor is BSP. Otherwise, it is AP.
 */
#define PROCESSOR_AS_BSP_BIT  UL(1 << 0)

/*
 * This bit is used in the StatusFlag field of EFI_PROCESSOR_INFORMATION and
 * indicates whether the processor is enabled. If the bit is 1, then the
 * processor is enabled. Otherwise, it is disabled.
 */
#define PROCESSOR_ENABLED_BIT  UL(1 << 1)

/*
 * This bit is used in the StatusFlag field of EFI_PROCESSOR_INFORMATION and
 * indicates whether the processor is healthy. If the bit is 1, then the
 * processor is healthy. Otherwise, some fault has been detected for the processor.
 */
#define PROCESSOR_HEALTH_STATUS_BIT  UL(1 << 2)

/*
 * Structure that describes the physical location of a logical CPU.
 */
struct efi_cpu_physical_location {
	uint32_t package;
	uint32_t core;
	uint32_t thread;
};

/*
 * Structure that defines the 6-level physical location of the processor
 */
struct efi_cpu_physical_location2 {
	uint32_t package;
	uint32_t module;
	uint32_t tile;
	uint32_t die;
	uint32_t core;
	uint32_t thread;
};

union extended_processor_information {
	/*
	 * The 6-level physical location of the processor, including the
	 * physical package number that identifies the cartridge, the physical
	 * module number within package, the physical tile number within the module,
	 * the physical die number within the tile, the physical core number within
	 * package, and logical thread number within core.
	 */
	struct efi_cpu_physical_location2 location2;
};

/*
 * Structure that describes information about a logical CPU.
 */
struct efi_processor_information {
	/*
	 * The unique processor ID determined by system hardware.
	 */
	uint64_t processor_id;

	/*
	 * Flags indicating if the processor is BSP or AP, if the processor is enabled
	 * or disabled, and if the processor is healthy. Bits 3..31 are reserved and
	 * must be 0.
	 *
	 * <pre>
	 * BSP  ENABLED  HEALTH  Description
	 * ===  =======  ======  ===================================================
	 * 0      0       0     Unhealthy Disabled AP.
	 * 0      0       1     Healthy Disabled AP.
	 * 0      1       0     Unhealthy Enabled AP.
	 * 0      1       1     Healthy Enabled AP.
	 * 1      0       0     Invalid. The BSP can never be in the disabled state.
	 * 1      0       1     Invalid. The BSP can never be in the disabled state.
	 * 1      1       0     Unhealthy Enabled BSP.
	 * 1      1       1     Healthy Enabled BSP.
	 * </pre>
	 */
	uint32_t status_flags;

	/*
	 * The physical location of the processor, including the physical package number
	 * that identifies the cartridge, the physical core number within package, and
	 * logical thread number within core.
	 */
	struct efi_cpu_physical_location location;

	/*
	 * The extended information of the processor. This field is filled only when
	 * CPU_V2_EXTENDED_TOPOLOGY is set in parameter ProcessorNumber.
	 */
	union extended_processor_information extended_information;
};

struct efi_mp_information_hob_data {
	uint64_t number_of_processors;
	uint64_t number_of_enabled_processors;
	struct efi_processor_information processor_info[];
};

#endif /* MPINFO_H */
