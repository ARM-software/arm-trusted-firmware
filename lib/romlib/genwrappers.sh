#!/bin/sh
# Copyright (c) 2018-2019, ARM Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause

set -e

build=.
out=output.a

for i
do
	case $i in
	-o)
		out=$2
		shift 2
		;;
	-b)
		build=$2
		shift 2
		;;
	--)
		shift
		break
		;;
	-*)
		echo usage: genwrappers.sh [-o output] [-b dir] file ... >&2
		exit 1
		;;
	esac
done

awk  '{sub(/[:blank:]*#.*/,"")}
!/^$/ && $NF != "patch" && $NF != "reserved" {print $1*4, $2, $3}' "$@" |
while read idx lib sym
do
	file=$build/${lib}_$sym

	cat <<EOF > $file.s
	.globl	$sym
$sym:
	ldr	x17, =jmptbl
	ldr	x17, [x17]
	mov	x16, #$idx
	add	x16, x16, x17
	br	x16
EOF

	${CROSS_COMPILE}as -o $file.o $file.s
done

${CROSS_COMPILE}ar -rc $out $build/*.o
