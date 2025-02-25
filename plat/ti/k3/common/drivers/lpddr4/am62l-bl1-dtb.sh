#!/bin/bash
dtb_array_addr=$(readelf -s $1 | grep dtb_array | tr -s " " | cut -d " " -f 3)
text_addr=$(readelf -S $1 | grep "\.text" | tr -s " " | cut -d " " -f 6)
dtb_seek=$(printf "%d" $((0x$dtb_array_addr - 0x$text_addr)))
dd if=$2 of=$3 bs=1 seek=$dtb_seek conv=notrunc status=none
