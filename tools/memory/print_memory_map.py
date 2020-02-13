#!/usr/bin/env python3
#
# Copyright (c) 2019-2020, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

import re
import os
import sys
import operator

# List of folder/map to parse
bl_images = ['bl1', 'bl2', 'bl31']

# List of symbols to search for
blx_symbols = ['__BL1_RAM_START__', '__BL1_RAM_END__',
                '__BL2_END__',
                '__BL31_END__',
                '__TEXT_START__', '__TEXT_END__',
                '__RODATA_START__', '__RODATA_END__',
                '__DATA_START__', '__DATA_END__',
                '__STACKS_START__', '__STACKS_END__',
                '__BSS_END',
                '__COHERENT_RAM_START__', '__COHERENT_RAM_END__',
               ]

# Regex to extract address from map file
address_pattern = re.compile(r"\b0x\w*")

# List of found element: [address, symbol, file]
address_list = []

# Get the directory from command line or use a default one
inverted_print = True
if len(sys.argv) >= 2:
    build_dir = sys.argv[1]
    if len(sys.argv) >= 3:
        inverted_print = sys.argv[2] == '0'
else:
    build_dir = 'build/fvp/debug'

# Extract all the required symbols from the map files
for image in bl_images:
    file_path = os.path.join(build_dir, image, '{}.map'.format(image))
    if os.path.isfile(file_path):
        with open (file_path, 'rt') as mapfile:
            for line in mapfile:
                for symbol in blx_symbols:
                    # Regex to find symbol definition
                    line_pattern = re.compile(r"\b0x\w*\s*" + symbol + "\s= .")
                    match = line_pattern.search(line)
                    if match:
                        # Extract address from line
                        match = address_pattern.search(line)
                        if match:
                            address_list.append([match.group(0), symbol, image])

# Sort by address
address_list.sort(key=operator.itemgetter(0))

# Invert list for lower address at bottom
if inverted_print:
    address_list = reversed(address_list)

# Generate memory view
print('{:-^93}'.format('Memory Map from: ' + build_dir))
for address in address_list:
    if "bl1" in address[2]:
        print(address[0], '+{:-^22}+ |{:^22}| |{:^22}|'.format(address[1], '', ''))
    elif "bl2" in address[2]:
        print(address[0], '|{:^22}| +{:-^22}+ |{:^22}|'.format('', address[1], ''))
    elif "bl31" in address[2]:
        print(address[0], '|{:^22}| |{:^22}| +{:-^22}+'.format('', '', address[1]))
    else:
        print(address[0], '|{:^22}| |{:^22}| +{:-^22}+'.format('', '', address[1]))

print('{:^20}{:_^22}   {:_^22}   {:_^22}'.format('', '', '', ''))
print('{:^20}{:^22}   {:^22}   {:^22}'.format('address', 'bl1', 'bl2', 'bl31'))
