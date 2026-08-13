#
# Copyright (c) 2020, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

LINUX_SOURCE=$1
TFA_SOURCE=$2
DTS_LINUX64_JUNO_R1="$LINUX_SOURCE/arch/arm64/boot/dts/arm/juno-r1.dts"
DTS_LINUX64_SUN50I_A64_PINE64_PLUS="$LINUX_SOURCE/arch/arm64/boot/dts/allwinner/sun50i-a64-pine64-plus.dts"
DTS_LINUX32_ATLAS7_EVB="$LINUX_SOURCE/arch/arm/boot/dts/atlas7-evb.dts"
DTS_LINUX32_BCM2837_RPI_3_B_PLUS="$LINUX_SOURCE/arch/arm/boot/dts/bcm2837-rpi-3-b-plus.dts"
DTS_TEST="test.dts"
DTS_FVP_BASE_GICV3_PSCI="$TFA_SOURCE/fdts/fvp-base-gicv3-psci.dts"

process_linux_dts()
{
	# DTS file path in first argument
	# DTB file in 2nd argument
	# Use global LINUX_SOURCE
	DTS_FILE=$1
	DTB_FILE=$2

	cpp -I $LINUX_SOURCE/include -x assembler-with-cpp -o $DTS_FILE.preprocessed $DTS_FILE
	sed -i -e '/stdc-predef.h/d' $DTS_FILE.preprocessed -e 's/1,pci-domain/linux,pci-domain/g' $DTS_FILE.preprocessed
	dtc -O dtb -o $DTB_FILE -i $DTS_FILE -b 0 $DTS_FILE.preprocessed
	rm $DTS_FILE.preprocessed
}

process_tfa_dts()
{
	# DTS file path in first argument
	# DTB file in 2nd argument
	DTS_FILE=$1
	DTB_FILE=$2
	cpp -I $TFA_SOURCE/include -x assembler-with-cpp -o $DTS_FILE.preprocessed $DTS_FILE
	sed -i -e '/stdc-predef.h/d' $DTS_FILE.preprocessed -e 's/1,pci-domain/linux,pci-domain/g' $DTS_FILE.preprocessed
	dtc -O dtb -o $DTB_FILE -i $DTS_FILE -b 0 $DTS_FILE.preprocessed
	rm $DTS_FILE.preprocessed
}

process_dts()
{
	# DTS file path in first argument
	# DTB file in 2nd argument
	DTS_FILE=$1
	DTB_FILE=$2
	dtc -o $DTB_FILE $DTS_FILE
}

# basic argument checks
if [[ ( "$1" == "-h" ) || ( "$1" == "--help" ) ]]; then
	echo "Usage: $0 [OPTION] path/to/linux/source path/to/tfa/source"
	echo ""
	echo "  This script will build the necessary DTB files from their"
	echo "  sources in the Linux kernel and TFA repositories. Provide the"
	echo "  path to the Linux source tree as the first argument to this"
	echo "  script and the path to the TFA source tree as the second"
	echo "  argument."
	echo ""
	echo "  -h, --help      print this help text"
	exit 0
fi
if [[ ( -z "$1" ) || ( -z "$2" ) ]]; then
	echo "Invalid arguments, use -h/--help for more info."
	exit 1
fi

# create a folder for new DTB files to be placed in
mkdir dtb

# generate linux DTB files
process_linux_dts $DTS_LINUX64_JUNO_R1 dtb/juno-r1.dtb
process_linux_dts $DTS_LINUX64_SUN50I_A64_PINE64_PLUS dtb/sun50i-a64-pine64-plus.dtb
process_linux_dts $DTS_LINUX32_ATLAS7_EVB dtb/atlas7-evb.dtb
process_linux_dts $DTS_LINUX32_BCM2837_RPI_3_B_PLUS dtb/bcm2837-rpi-3-b-plus.dtb

# generate TFA DTB file
process_tfa_dts $DTS_FVP_BASE_GICV3_PSCI dtb/fvp-base-gicv3-psci.dtb

# generate test DTB file
process_dts $DTS_TEST dtb/test.dtb
