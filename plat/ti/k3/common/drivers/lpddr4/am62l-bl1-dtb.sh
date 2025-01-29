#!/bin/bash
dtb_offset=$(readelf -s $1 | grep dtb_array | tr -s " " | cut -d " " -f 3 | cut -c12-16)
dtb_seek=$(echo "ibase=16; $dtb_offset" | bc)
dd if=$2 of=$3 bs=1 seek=$dtb_seek conv=notrunc
