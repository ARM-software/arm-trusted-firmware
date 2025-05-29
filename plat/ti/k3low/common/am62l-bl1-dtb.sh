#!/bin/bash
dtb_array_addr=$(readelf -s $1 | grep -w dtb_array | tr -s " " | cut -d " " -f 3)
dtb_array_end_addr=$(readelf -s $1 | grep -w dtb_array_end | tr -s " " | cut -d " " -f 3)
text_addr=$(readelf -S $1 | grep "\.text" | tr -s " " | cut -d " " -f 6)
dtb_seek=$(printf "%d" $((0x$dtb_array_addr - 0x$text_addr)))
dtb_array_size=$(printf "%d" $((0x$dtb_array_end_addr - 0x$dtb_array_addr)))
dtb_file_size=$(stat -c%s $2)
if [ "$dtb_array_size -ge $dtb_file_size" ]; then
	dd if=$2 of=$3 bs=1 seek=$dtb_seek conv=notrunc status=none
else
	exit 1
fi
