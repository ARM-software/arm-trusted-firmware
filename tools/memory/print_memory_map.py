#!/usr/bin/env python3
#
# Copyright (c) 2019-2022, Arm Limited and Contributors. All rights reserved.
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
                '__RO_START__', '__RO_END_UNALIGNED__', '__RO_END__',
                '__TEXT_START__', '__TEXT_END__',
                '__TEXT_RESIDENT_START__', '__TEXT_RESIDENT_END__',
                '__RODATA_START__', '__RODATA_END__',
                '__DATA_START__', '__DATA_END__',
                '__STACKS_START__', '__STACKS_END__',
                '__BSS_START__', '__BSS_END__',
                '__COHERENT_RAM_START__', '__COHERENT_RAM_END__',
                '__CPU_OPS_START__', '__CPU_OPS_END__',
                '__FCONF_POPULATOR_START__', '__FCONF_POPULATOR_END__',
                '__GOT_START__', '__GOT_END__',
                '__PARSER_LIB_DESCS_START__', '__PARSER_LIB_DESCS_END__',
                '__PMF_TIMESTAMP_START__', '__PMF_TIMESTAMP_END__',
                '__PMF_SVC_DESCS_START__', '__PMF_SVC_DESCS_END__',
                '__RELA_START__', '__RELA_END__',
                '__RT_SVC_DESCS_START__', '__RT_SVC_DESCS_END__',
                '__BASE_XLAT_TABLE_START__', '__BASE_XLAT_TABLE_END__',
                '__XLAT_TABLE_START__', '__XLAT_TABLE_END__',
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

max_len = max(len(word) for word in blx_symbols) + 2
if (max_len % 2) != 0:
    max_len += 1

# Extract all the required symbols from the map files
for image in bl_images:
    file_path = os.path.join(build_dir, image, '{}.map'.format(image))
    if os.path.isfile(file_path):
        with open (file_path, 'rt') as mapfile:
            for line in mapfile:
                for symbol in blx_symbols:
                    skip_symbol = 0
                    # Regex to find symbol definition
                    line_pattern = re.compile(r"\b0x\w*\s*" + symbol + "\s= .")
                    match = line_pattern.search(line)
                    if match:
                        # Extract address from line
                        match = address_pattern.search(line)
                        if match:
                            if '_END__' in symbol:
                                sym_start = symbol.replace('_END__', '_START__')
                                if [match.group(0), sym_start, image] in address_list:
                                    address_list.remove([match.group(0), sym_start, image])
                                    skip_symbol = 1
                            if skip_symbol == 0:
                                address_list.append([match.group(0), symbol, image])

# Sort by address
address_list.sort(key=operator.itemgetter(0))

# Invert list for lower address at bottom
if inverted_print:
    address_list = reversed(address_list)

# Generate memory view
print(('{:-^%d}' % (max_len * 3 + 20 + 7)).format('Memory Map from: ' + build_dir))
for address in address_list:
    if "bl1" in address[2]:
        print(address[0], ('+{:-^%d}+ |{:^%d}| |{:^%d}|' % (max_len, max_len, max_len)).format(address[1], '', ''))
    elif "bl2" in address[2]:
        print(address[0], ('|{:^%d}| +{:-^%d}+ |{:^%d}|' % (max_len, max_len, max_len)).format('', address[1], ''))
    elif "bl31" in address[2]:
        print(address[0], ('|{:^%d}| |{:^%d}| +{:-^%d}+' % (max_len, max_len, max_len)).format('', '', address[1]))
    else:
        print(address[0], ('|{:^%d}| |{:^%d}| +{:-^%d}+' % (max_len, max_len, max_len)).format('', '', address[1]))

print(('{:^20}{:_^%d}   {:_^%d}   {:_^%d}' % (max_len, max_len, max_len)).format('', '', '', ''))
print(('{:^20}{:^%d}   {:^%d}   {:^%d}' % (max_len, max_len, max_len)).format('address', 'bl1', 'bl2', 'bl31'))
