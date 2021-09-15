FF-A manifest binding to device tree
========================================

This document defines the nodes and properties used to define a partition,
according to the FF-A specification.

Version 1.0
-----------

Partition Properties
^^^^^^^^^^^^^^^^^^^^

- compatible [mandatory]
   - value type: <string>
   - Must be the string "arm,ffa-manifest-X.Y" which specifies the major and
     minor versions of the device tree binding for the FFA manifest represented
     by this node. The minor number is incremented if the binding changes in a
     backwards compatible manner.

      - X is an integer representing the major version number of this document.
      - Y is an integer representing the minor version number of this document.

- ffa-version [mandatory]
   - value type: <u32>
   - Must be two 16 bits values (X, Y), concatenated as 31:16 -> X,
     15:0 -> Y, where:

      - X is the major version of FF-A expected by the partition at the FFA
        instance it will execute.
      - Y is the minor version of FF-A expected by the partition at the FFA
        instance it will execute.

- uuid [mandatory]
   - value type: <prop-encoded-array>
   - An array consisting of 4 <u32> values, identifying the UUID of the service
     implemented by this partition. The UUID format is described in RFC 4122.

- id
   - value type: <u32>
   - Pre-allocated partition ID.

- auxiliary-id
   - value type: <u32>
   - Pre-allocated ID that could be used in memory management transactions.

- description
   - value type: <string>
   - Name of the partition e.g. for debugging purposes.

- execution-ctx-count [mandatory]
   - value type: <u32>
   - Number of vCPUs that a VM or SP wants to instantiate.

      - In the absence of virtualization, this is the number of execution
        contexts that a partition implements.
      - If value of this field = 1 and number of PEs > 1 then the partition is
        treated as UP & migrate capable.
      - If the value of this field > 1 then the partition is treated as a MP
        capable partition irrespective of the number of PEs.

- exception-level [mandatory]
   - value type: <u32>
   - The target exception level for the partition:

      - 0x0: EL1
      - 0x1: S_EL0
      - 0x2: S_EL1

- execution-state [mandatory]
   - value type: <u32>
   - The target execution state of the partition:

      - 0: AArch64
      - 1: AArch32

- load-address
   - value type: <u64>
   - Physical base address of the partition in memory. Absence of this field
     indicates that the partition is position independent and can be loaded at
     any address chosen at boot time.

- entrypoint-offset
   - value type: <u64>
   - Offset from the base of the partition's binary image to the entry point of
     the partition. Absence of this field indicates that the entry point is at
     offset 0x0 from the base of the partition's binary.

- xlat-granule [mandatory]
   - value type: <u32>
   - Translation granule used with the partition:

      - 0x0: 4k
      - 0x1: 16k
      - 0x2: 64k

- boot-order
   - value type: <u32>
   - A unique number amongst all partitions that specifies if this partition
     must be booted before others. The partition with the smaller number will be
     booted first.

- rx-tx-buffer
   - value type: "memory-regions" node
   - Specific "memory-regions" nodes that describe the RX/TX buffers expected
     by the partition.
     The "compatible" must be the string "arm,ffa-manifest-rx_tx-buffer".

- messaging-method [mandatory]
   - value type: <u8>
   - Specifies which messaging methods are supported by the partition, set bit
     means the feature is supported, clear bit - not supported:

      - Bit[0]: partition can receive direct requests if set
      - Bit[1]: partition can send direct requests if set
      - Bit[2]: partition can send and receive indirect messages

- managed-exit
   - value type: <empty>
   - Specifies if managed exit is supported.

- has-primary-scheduler
   - value type: <empty>
   - Presence of this field indicates that the partition implements the primary
     scheduler. If so, run-time EL must be EL1.

- run-time-model
   - value type: <u32>
   - Run time model that the SPM must enforce for this SP:

      - 0x0: Run to completion
      - 0x1: Preemptible

- time-slice-mem
   - value type: <empty>
   - Presence of this field indicates that the partition doesn't expect the
     partition manager to time slice long running memory management functions.

- gp-register-num
   - value type: <u32>
   - Presence of this field indicates that the partition expects the
     ffa_init_info structure to be passed in via the specified general purpose
     register.
     The field specifies the general purpose register number but not its width.
     The width is derived from the partition's execution state, as specified in
     the partition properties. For example, if the number value is 1 then the
     general-purpose register used will be x1 in AArch64 state and w1 in AArch32
     state.

- stream-endpoint-ids
   - value type: <prop-encoded-array>
   - List of <u32> tuples, identifying the IDs this partition is acting as
     proxy for.

Memory Regions
--------------

- compatible [mandatory]
   - value type: <string>
   - Must be the string "arm,ffa-manifest-memory-regions".

- description
   - value type: <string>
   - Name of the memory region e.g. for debugging purposes.

- pages-count [mandatory]
   - value type: <u32>
   - Count of pages of memory region as a multiple of the translation granule
     size

- attributes [mandatory]
   - value type: <u32>
   - Mapping modes: ORed to get required permission

      - 0x1: Read
      - 0x2: Write
      - 0x4: Execute

- base-address
   - value type: <u64>
   - Base address of the region. The address must be aligned to the translation
     granule size.
     The address given may be a Physical Address (PA), Virtual Address (VA), or
     Intermediate Physical Address (IPA). Refer to the FFA specification for
     more information on the restrictions around the address type.
     If the base address is omitted then the partition manager must map a memory
     region of the specified size into the partition's translation regime and
     then communicate the region properties (including the base address chosen
     by the partition manager) to the partition.

Device Regions
--------------

- compatible [mandatory]
   - value type: <string>
   - Must be the string "arm,ffa-manifest-device-regions".

- description
   - value type: <string>
   - Name of the device region e.g. for debugging purposes.

- reg [mandatory]
   - value type: <prop-encoded-array>
   - A (address, num-pages) pair describing the device, where:

      - address: The physical base address <u64> value of the device MMIO
        region.
      - num-pages: The <u32> number of pages of the region. The total size of
        the region is this value multiplied by the translation granule size.

- attributes [mandatory]
   - value type: <u32>
   - Mapping modes: ORed to get required permission

     - 0x1: Read
     - 0x2: Write
     - 0x4: Execute

- smmu-id
   - value type: <u32>
   - On systems with multiple System Memory Management Units (SMMUs) this
     identifier is used to inform the partition manager which SMMU the device is
     upstream of. If the field is omitted then it is assumed that the device is
     not upstream of any SMMU.

- stream-ids
   - value type: <prop-encoded-array>
   - A list of (id, mem-manage) pair, where:

      - id: A unique <u32> value amongst all devices assigned to the partition.

- interrupts [mandatory]
   - value type: <prop-encoded-array>
   - A list of (id, attributes) pair describing the device interrupts, where:

      - id: The <u32> interrupt IDs.
      - attributes: A <u32> value,
        containing the attributes for each interrupt ID:

         - Interrupt type: SPI, PPI, SGI
         - Interrupt configuration: Edge triggered, Level triggered
         - Interrupt security state: Secure, Non-secure
         - Interrupt priority value
         - Target execution context/vCPU for each SPI

- exclusive-access
   - value type: <empty>
   - Presence of this field implies that this endpoint must be granted exclusive
     access and ownership of this device's MMIO region.

--------------

*Copyright (c) 2019-2021, Arm Limited and Contributors. All rights reserved.*
