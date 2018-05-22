#!/bin/sh
# Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause

set -e

output=jmpvar.s
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
		echo usage: genvar.sh [-o output] file... >&2
		;;
	esac
done

tmp=`mktemp`
trap "rm -f $tmp" EXIT INT QUIT

nm -a "$@" |
awk -v OFS="\n" '
$3 == ".text" {print "\t.data",
                     "\t.globl\tjmptbl",
                     "\t.align\t4",
                     "jmptbl:\t.quad\t0x" $1}' > $tmp

mv $tmp $output
