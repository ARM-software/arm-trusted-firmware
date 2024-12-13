#
# Copyright (c) 2018-2024, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
# Platform specific romlib functions can be added or included here.
# The index in the output file will be generated cumulatively in the same
# order as it is given in this file.
# Output file can be found at: $BUILD_DIR/jmptbl.i
#
# Format:
# lib	function	[patch]
# Example:
# rom	rom_lib_init
# fdt	fdt_getprop_namelen	patch

rom     rom_lib_init
fdt     fdt_getprop
fdt     fdt_get_property
fdt     fdt_getprop_namelen
fdt     fdt_setprop_inplace
fdt     fdt_check_header
fdt     fdt_node_offset_by_compatible
fdt     fdt_setprop_inplace_namelen_partial
fdt     fdt_first_subnode
fdt     fdt_next_subnode
fdt     fdt_parent_offset
fdt     fdt_stringlist_search
fdt     fdt_get_alias_namelen
fdt     fdt_path_offset
fdt     fdt_path_offset_namelen
fdt     fdt_address_cells
fdt     fdt_size_cells
fdt     fdt_get_name
fdt     fdt_get_alias
fdt     fdt_node_offset_by_phandle
fdt     fdt_subnode_offset
fdt     fdt_add_subnode
c       exit
c       atexit
