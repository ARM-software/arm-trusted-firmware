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
	--bti=*)
		enable_bti=$(echo $1 | sed 's/--bti=\(.*\)/\1/')
		shift 1
		;;
	--asflags=*)
		asflags=$(echo $1 | sed 's/--asflags=\(.*\)/\1/')
		shift 1
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

awk -v BTI=$enable_bti '
{sub(/[:blank:]*#.*/,"")}
!/^$/ && $NF != "patch" && $NF != "reserved" {
		if (BTI == 1)
			print $1*8, $2, $3
		else
			print $1*4, $2, $3}' "$@" |
while read idx lib sym
do
	file=$build/${lib}_$sym

	cat <<EOF > $file.s
	.globl	$sym
$sym:
EOF
if [ $enable_bti = 1 ]
then
	echo "\tbti\tjc" >> $file.s
fi
	cat <<EOF >> $file.s
	ldr	x17, =jmptbl
	mov	x16, #$idx
	ldr	x17, [x17]
	add	x16, x16, x17
	br	x16
EOF

	${CROSS_COMPILE}as ${asflags} -o $file.o $file.s
done

${CROSS_COMPILE}ar -rc $out $build/*.o
