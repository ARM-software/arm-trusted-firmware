#! /bin/bash

#
# This script generates the image file used in the ARM Trusted Firmware
# Translation Tables Library V2 Design document from the 'xlat_align.dia' file.
#

set -e

# Usage: generate_image <dia_filename> <layers> <image_filename>
function generate_image
{
	dia				\
		--show-layers=$2	\
		--filter=svg		\
		--export=$3		\
		$1

}

generate_image			\
	xlat_align.dia		\
	bg,translations		\
	xlat_align.svg

inkscape -z xlat_align.svg -e xlat_align.png -d 45
