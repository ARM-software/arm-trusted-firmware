#!/bin/sh
# Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause

set -e

output=jmptbl.s

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
		echo usage: gentbl.sh [-o output]  file ... >&2
		exit 1
		;;
	esac
done

tmp=`mktemp`
trap "rm -f $tmp" EXIT INT QUIT

rm -f $output

awk -v OFS="\n" '
BEGIN {print "\t.text",
             "\t.globl\tjmptbl",
             "jmptbl:"}
      {sub(/[:blank:]*#.*/,"")}
!/^$/ {print "\tb\t" $3}' "$@" > $tmp

mv $tmp $output
