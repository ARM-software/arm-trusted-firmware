#!/bin/bash

#
# Copyright (c) 2025, Qualcomm Technologies, Inc. and/or its subsidiaries.
# SPDX-License-Identifier: BSD-3-Clause
#

${CROSS_COMPILE}objcopy -I binary -B aarch64 -O elf64-littleaarch64 ${1} fip.o
${CROSS_COMPILE}ld fip.o -o fip_unsigned.elf -EL -T $(dirname "$0")/fip-elf.lds --defsym=ELFENTRY=${2} -Ttext=${2}
rm -f fip.o

if [ ! -d "${PWD}/qtestsign" ]; then
	git clone https://github.com/msm8916-mainline/qtestsign.git ${PWD}/qtestsign
fi

${PWD}/qtestsign/qtestsign.py -v6 aboot -o fip.elf fip_unsigned.elf
rm -f fip_unsigned.elf
