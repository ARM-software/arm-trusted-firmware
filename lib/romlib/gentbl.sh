#!/bin/sh
# Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause

set -e

output=jmptbl.s
build=.

for i
do
	case $i in
	-o)
		output=$2
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
		echo usage: gentbl.sh [-o output] [-b dir] file ... >&2
		exit 1
		;;
	esac
done

tmp=`mktemp`
trap "rm -f $$.tmp" EXIT INT QUIT
rm -f $output

# Pre-process include files
awk '!/^$/ && !/[:blank:]*#.*/{
if (NF == 2 && $1 == "include") {
	while ((getline line < $2) > 0)
		if (line !~ /^$/ && line !~ /[:blank:]*#.*/)
			print line
		close($2)
} else
	print
}' "$@" |
awk -v OFS="\t" '
BEGIN{print "#index\tlib\tfunction\t[patch]"}
{print NR-1, $0}' | tee $build/jmptbl.i |
awk -v OFS="\n" '
BEGIN {print "\t.text",
             "\t.globl\tjmptbl",
             "jmptbl:"}
      {sub(/[:blank:]*#.*/,"")}
!/^$/ {if ($3 == "reserved")
		print "\t.word\t0x0"
	else
		print "\tb\t" $3}' > $$.tmp &&
mv $$.tmp $output
