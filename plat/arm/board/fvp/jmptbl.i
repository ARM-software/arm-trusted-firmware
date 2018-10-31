#
# Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
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

include ../../lib/romlib/jmptbl.i
