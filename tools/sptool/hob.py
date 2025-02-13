#!/usr/bin/python3
# Copyright (c) 2025, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause

import struct

EFI_HOB_HANDOFF_TABLE_VERSION = 0x000A

PAGE_SIZE_SHIFT = 12  # TODO assuming 4K page size

# HobType values of EFI_HOB_GENERIC_HEADER.

EFI_HOB_TYPE_HANDOFF = 0x0001
EFI_HOB_TYPE_MEMORY_ALLOCATION = 0x0002
EFI_HOB_TYPE_RESOURCE_DESCRIPTOR = 0x0003
EFI_HOB_TYPE_GUID_EXTENSION = 0x0004
EFI_HOB_TYPE_FV = 0x0005
EFI_HOB_TYPE_CPU = 0x0006
EFI_HOB_TYPE_MEMORY_POOL = 0x0007
EFI_HOB_TYPE_FV2 = 0x0009
EFI_HOB_TYPE_LOAD_PEIM_UNUSED = 0x000A
EFI_HOB_TYPE_UEFI_CAPSULE = 0x000B
EFI_HOB_TYPE_FV3 = 0x000C
EFI_HOB_TYPE_UNUSED = 0xFFFE
EFI_HOB_TYPE_END_OF_HOB_LIST = 0xFFFF

# GUID values
"""struct efi_guid {
         uint32_t time_low;
         uint16_t time_mid;
         uint16_t time_hi_and_version;
         uint8_t clock_seq_and_node[8];
}"""

MM_PEI_MMRAM_MEMORY_RESERVE_GUID = (
    0x0703F912,
    0xBF8D,
    0x4E2A,
    (0xBE, 0x07, 0xAB, 0x27, 0x25, 0x25, 0xC5, 0x92),
)
MM_NS_BUFFER_GUID = (
    0xF00497E3,
    0xBFA2,
    0x41A1,
    (0x9D, 0x29, 0x54, 0xC2, 0xE9, 0x37, 0x21, 0xC5),
)

# MMRAM states and capabilities
# See UEFI Platform Initialization Specification Version 1.8, IV-5.3.5
EFI_MMRAM_OPEN = 0x00000001
EFI_MMRAM_CLOSED = 0x00000002
EFI_MMRAM_LOCKED = 0x00000004
EFI_CACHEABLE = 0x00000008
EFI_ALLOCATED = 0x00000010
EFI_NEEDS_TESTING = 0x00000020
EFI_NEEDS_ECC_INITIALIZATION = 0x00000040

EFI_SMRAM_OPEN = EFI_MMRAM_OPEN
EFI_SMRAM_CLOSED = EFI_MMRAM_CLOSED
EFI_SMRAM_LOCKED = EFI_MMRAM_LOCKED

# EFI boot mode.
EFI_BOOT_WITH_FULL_CONFIGURATION = 0x00
EFI_BOOT_WITH_MINIMAL_CONFIGURATION = 0x01
EFI_BOOT_ASSUMING_NO_CONFIGURATION_CHANGES = 0x02
EFI_BOOT_WITH_FULL_CONFIGURATION_PLUS_DIAGNOSTICS = 0x03
EFI_BOOT_WITH_DEFAULT_SETTINGS = 0x04
EFI_BOOT_ON_S4_RESUME = 0x05
EFI_BOOT_ON_S5_RESUME = 0x06
EFI_BOOT_WITH_MFG_MODE_SETTINGS = 0x07
EFI_BOOT_ON_S2_RESUME = 0x10
EFI_BOOT_ON_S3_RESUME = 0x11
EFI_BOOT_ON_FLASH_UPDATE = 0x12
EFI_BOOT_IN_RECOVERY_MODE = 0x20

STMM_BOOT_MODE = EFI_BOOT_WITH_FULL_CONFIGURATION
STMM_MMRAM_REGION_STATE_DEFAULT = EFI_CACHEABLE | EFI_ALLOCATED
STMM_MMRAM_REGION_STATE_HEAP = EFI_CACHEABLE

"""`struct` python module allows user to specify endianness.
We are expecting FVP or STMM platform as target and that they will be
little-endian. See `struct` python module documentation if other endianness is
needed."""
ENDIANNESS = "<"


def struct_pack_with_endianness(format_str, *args):
    return struct.pack((ENDIANNESS + format_str), *args)


def struct_calcsize_with_endianness(format_str):
    return struct.calcsize(ENDIANNESS + format_str)


# Helper for fdt node property parsing
def get_integer_property_value(fdt_node, name):
    if fdt_node.exist_property(name):
        p = fdt_node.get_property(name)

        # <u32> Device Tree value
        if len(p) == 1:
            return p.value
        # <u64> Device Tree value represented as two 32-bit values
        if len(p) == 2:
            msb = p[0]
            lsb = p[1]
            return lsb | (msb << 32)
    return None


class EfiGuid:
    """Class representing EFI GUID (Globally Unique Identifier) as described by
    the UEFI Specification v2.10"""

    def __init__(self, time_low, time_mid, time_hi_and_version, clock_seq_and_node):
        self.time_low = time_low
        self.time_mid = time_mid
        self.time_hi_and_version = time_hi_and_version
        self.clock_seq_and_node = clock_seq_and_node
        self.format_str = "IHH8B"

    def pack(self):
        return struct_pack_with_endianness(
            self.format_str,
            self.time_low,
            self.time_mid,
            self.time_hi_and_version,
            *self.clock_seq_and_node,
        )

    def __str__(self):
        return f"{hex(self.time_low)}, {hex(self.time_mid)}, \
    {hex(self.time_hi_and_version)}, {[hex(i) for i in self.clock_seq_and_node]}"


class HobGenericHeader:
    """Class representing the Hob Generic Header data type as described
    in the UEFI Platform Initialization Specification version 1.8.

    Each HOB is required to contain this header specifying the type and length
    of the HOB.
    """

    def __init__(self, hob_type, hob_length):
        self.format_str = "HHI"
        self.hob_type = hob_type
        self.hob_length = struct_calcsize_with_endianness(self.format_str) + hob_length
        self.reserved = 0

    def pack(self):
        return struct_pack_with_endianness(
            self.format_str, self.hob_type, self.hob_length, self.reserved
        )

    def __str__(self):
        return f"Hob Type: {self.hob_type} Hob Length: {self.hob_length}"


class HobGuid:
    """Class representing the Guid Extension HOB as described in the UEFI
    Platform Initialization Specification version 1.8.

    Allows the production of HOBs whose types are not defined by the
    specification by generating a GUID for the HOB entry."""

    def __init__(self, name: EfiGuid, data_format_str, data):
        hob_length = struct_calcsize_with_endianness(
            name.format_str
        ) + struct_calcsize_with_endianness(data_format_str)
        self.header = HobGenericHeader(EFI_HOB_TYPE_GUID_EXTENSION, hob_length)
        self.name = name
        self.data = data
        self.data_format_str = data_format_str
        self.format_str = (
            self.header.format_str + self.name.format_str + data_format_str
        )

    def pack(self):
        return (
            self.header.pack()
            + self.name.pack()
            + struct_pack_with_endianness(self.data_format_str, *self.data)
        )

    def __str__(self):
        return f"Header: {self.header}\n Name: {self.name}\n Data: {self.data}"


class HandoffInfoTable:
    """Class representing the Handoff Info Table HOB (also known as PHIT HOB)
    as described in the UEFI Platform Initialization Specification version 1.8.

    Must be the first HOB in the HOB list. Contains general state
    information.

    For an SP, the range `memory_bottom` to `memory_top` will be the memory
    range for the SP starting at the load address. `free_memory_bottom` to
    `free_memory_top` indicates space where more HOB's could be added to the
    HOB List."""

    def __init__(self, memory_base, memory_size, free_memory_base, free_memory_size):
        # header,uint32t,uint32t, uint64_t * 5
        self.format_str = "II5Q"
        hob_length = struct_calcsize_with_endianness(self.format_str)
        self.header = HobGenericHeader(EFI_HOB_TYPE_HANDOFF, hob_length)
        self.version = EFI_HOB_HANDOFF_TABLE_VERSION
        self.boot_mode = STMM_BOOT_MODE
        self.memory_top = memory_base + memory_size
        self.memory_bottom = memory_base
        self.free_memory_top = free_memory_base + free_memory_size
        self.free_memory_bottom = free_memory_base + self.header.hob_length
        self.hob_end = None

    def set_hob_end_addr(self, hob_end_addr):
        self.hob_end = hob_end_addr

    def set_free_memory_bottom_addr(self, addr):
        self.free_memory_bottom = addr

    def pack(self):
        return self.header.pack() + struct_pack_with_endianness(
            self.format_str,
            self.version,
            self.boot_mode,
            self.memory_top,
            self.memory_bottom,
            self.free_memory_top,
            self.free_memory_bottom,
            self.hob_end,
        )


class FirmwareVolumeHob:
    """Class representing the Firmware Volume HOB type as described in the
    UEFI Platform Initialization Specification version 1.8.

    For an SP this will detail where the SP binary is located.
    """

    def __init__(self, base_address, img_offset, img_size):
        # header, uint64_t, uint64_t
        self.data_format_str = "2Q"
        hob_length = struct_calcsize_with_endianness(self.data_format_str)
        self.header = HobGenericHeader(EFI_HOB_TYPE_FV, hob_length)
        self.format_str = self.header.format_str + self.data_format_str
        self.base_address = base_address + img_offset
        self.length = img_size - img_offset

    def pack(self):
        return self.header.pack() + struct_pack_with_endianness(
            self.data_format_str, self.base_address, self.length
        )


class EndOfHobListHob:
    """Class representing the End of HOB List HOB type as described in the
    UEFI Platform Initialization Specification version 1.8.

    Must be the last entry in a HOB list.
    """

    def __init__(self):
        self.header = HobGenericHeader(EFI_HOB_TYPE_END_OF_HOB_LIST, 0)
        self.format_str = ""

    def pack(self):
        return self.header.pack()


class HobList:
    """Class representing a HOB (Handoff Block list) based on the UEFI Platform
    Initialization Sepcification version 1.8"""

    def __init__(self, phit: HandoffInfoTable):
        if phit is None:
            raise Exception("HobList must be initialized with valid PHIT HOB")
        final_hob = EndOfHobListHob()
        phit.hob_end = phit.free_memory_bottom
        phit.free_memory_bottom += final_hob.header.hob_length
        self.hob_list = [phit, final_hob]

    def add(self, hob):
        if hob is not None:
            if hob.header.hob_length > (
                self.get_phit().free_memory_top - self.get_phit().free_memory_bottom
            ):
                raise MemoryError(
                    f"Cannot add HOB of length {hob.header.hob_length}. \
                    Resulting table size would exceed max table size of \
                    {self.max_size}. Current table size: {self.size}."
                )
            self.hob_list.insert(-1, hob)
            self.get_phit().hob_end += hob.header.hob_length
            self.get_phit().free_memory_bottom += hob.header.hob_length

    def get_list(self):
        return self.hob_list

    def get_phit(self):
        if self.hob_list is not None:
            if type(self.hob_list[0]) is not HandoffInfoTable:
                raise Exception("First hob in list must be of type PHIT")
            return self.hob_list[0]


def generate_mmram_desc(base_addr, page_count, granule, region_state):
    physical_size = page_count << (PAGE_SIZE_SHIFT + (granule << 1))
    physical_start = base_addr
    cpu_start = base_addr

    return ("4Q", (physical_start, cpu_start, physical_size, region_state))


def generate_stmm_region_descriptor(base_addr, physical_size):
    region_state = STMM_MMRAM_REGION_STATE_DEFAULT
    physical_start = base_addr
    cpu_start = base_addr
    return ("4Q", (physical_start, cpu_start, physical_size, region_state))


def generate_ns_buffer_guid(mmram_desc):
    return HobGuid(EfiGuid(*MM_NS_BUFFER_GUID), *mmram_desc)


def generate_pei_mmram_memory_reserve_guid(regions):
    # uint32t n_reserved regions, 4 bytes for padding so that array is aligned,
    # array of mmram descriptors
    format_str = "I4x"
    data = [len(regions)]
    for desc_format_str, mmram_desc in regions:
        format_str += desc_format_str
        data.extend(mmram_desc)
    guid_data = (format_str, data)
    return HobGuid(EfiGuid(*MM_PEI_MMRAM_MEMORY_RESERVE_GUID), *guid_data)


def generate_hob_from_fdt_node(sp_fdt, hob_offset, hob_size=None):
    """Create a HOB list binary from an SP FDT."""
    fv_hob = None
    ns_buffer_hob = None
    mmram_reserve_hob = None
    shared_buf_hob = None

    load_address = get_integer_property_value(sp_fdt, "load-address")
    img_size = get_integer_property_value(sp_fdt, "image-size")
    entrypoint_offset = get_integer_property_value(sp_fdt, "entrypoint-offset")

    if entrypoint_offset is None:
        entrypoint_offset = 0x0
    if hob_offset is None:
        hob_offset = 0x0
    if img_size is None:
        img_size = 0x0

    regions = []

    # StMM requires the first memory region described in the
    # MM_PEI_MMRAM_MEMORY_RESERVE_GUID describe the full partition layout.
    regions.append(generate_stmm_region_descriptor(load_address, img_size))

    if sp_fdt.exist_node("memory-regions"):
        if sp_fdt.exist_property("xlat-granule"):
            granule = int(sp_fdt.get_property("xlat-granule").value)
        else:
            # Default granule to 4K
            granule = 0
        memory_regions = sp_fdt.get_node("memory-regions")
        for node in memory_regions.nodes:
            base_addr = get_integer_property_value(node, "base-address")
            page_count = get_integer_property_value(node, "pages-count")

            if base_addr is None:
                offset = get_integer_property_value(
                    node, "load-address-relative-offset"
                )
                if offset is None:
                    # Cannot create memory descriptor without base address, so skip
                    # node if base address cannot be defined
                    continue
                else:
                    base_addr = load_address + offset

            if node.name.strip() == "heap":
                region_state = STMM_MMRAM_REGION_STATE_HEAP
            else:
                region_state = STMM_MMRAM_REGION_STATE_DEFAULT

            mmram_desc = generate_mmram_desc(
                base_addr, page_count, granule, region_state
            )

            if node.name.strip() == "ns_comm_buffer":
                ns_buffer_hob = generate_ns_buffer_guid(mmram_desc)

            regions.append(mmram_desc)

        mmram_reserve_hob = generate_pei_mmram_memory_reserve_guid(regions)

    fv_hob = FirmwareVolumeHob(load_address, entrypoint_offset, img_size)
    hob_list_base = load_address + hob_offset

    # TODO assuming default of 1 page allocated for HOB List
    if hob_size is not None:
        max_table_size = hob_size
    else:
        max_table_size = 1 << PAGE_SIZE_SHIFT
    phit = HandoffInfoTable(
        load_address, entrypoint_offset + img_size, hob_list_base, max_table_size
    )

    # Create a HobList containing only PHIT and EndofHobList HOBs.
    hob_list = HobList(phit)

    # Add HOBs to HOB list
    if fv_hob is not None:
        hob_list.add(fv_hob)
    if ns_buffer_hob is not None:
        hob_list.add(ns_buffer_hob)
    if mmram_reserve_hob is not None:
        hob_list.add(mmram_reserve_hob)
    if shared_buf_hob is not None:
        hob_list.add(shared_buf_hob)

    return hob_list
