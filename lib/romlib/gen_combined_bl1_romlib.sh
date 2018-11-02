#!/bin/sh
# Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause

set -e

output="bl1_romlib.bin"

# Set trap for removing temporary file
trap 'r=$?;rm -f $bin_path/$$.tmp;exit $r' EXIT HUP QUIT INT TERM

# Read input parameters
for i
do
	case $i in
	-o)
		output=$2
		shift 2
		;;
	--)
		shift
		break
		;;
	-*)
		echo usage: gen_combined_bl1_romlib.sh [-o output] path_to_build_directory >&2
		;;
	esac
done


bin_path=$1
romlib_path=$1/romlib
bl1_file="$1/bl1/bl1.elf"
romlib_file="$1/romlib/romlib.elf"
bl1_end=""
romlib_begin=""

# Get address of __BL1_ROM_END__
bl1_end=`nm -a "$bl1_file" |
awk '$3 == "__BL1_ROM_END__" {print "0x"$1}'`

# Get start address of romlib "text" section
romlib_begin=`nm -a "$romlib_file" |
awk '$3 == ".text" {print "0x"$1}'`

# Character "U" will be read as "55" in hex when it is
# concatenated with bl1.bin. Generate combined BL1 and ROMLIB
# binary with filler bytes for juno
(cat $bin_path/bl1.bin
 yes U | sed $(($romlib_begin - $bl1_end))q | tr -d '\n'
 cat $bin_path/romlib/romlib.bin) > $bin_path/$$.tmp &&
mv $bin_path/$$.tmp $bin_path/$output
