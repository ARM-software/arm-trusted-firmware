#!/bin/sh
#
# Copyright (c) 2016, ARM Limited and Contributors. All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
# Redistributions of source code must retain the above copyright notice, this
# list of conditions and the following disclaimer.
#
# Redistributions in binary form must reproduce the above copyright notice,
# this list of conditions and the following disclaimer in the documentation
# and/or other materials provided with the distribution.
#
# Neither the name of ARM nor the names of its contributors may be used
# to endorse or promote products derived from this software without specific
# prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
# LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
# CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
# SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
# INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
# CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
# ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
# POSSIBILITY OF SUCH DAMAGE.
#
# This script implements the old fip_create tool on top of
# the new fiptool.

usage() {
    cat << EOF
This tool is used to create a Firmware Image Package.

Usage:
	fip_create [options] FIP_FILENAME

Options:
	-h,--help: Print this help message and exit
	-d,--dump: Print contents of FIP after update
	-u,--unpack: Unpack images from an existing FIP
	-f,--force: Overwrite existing files when unpacking images

Components that can be added/updated:
	--scp-fwu-cfg FILENAME		SCP Firmware Updater Configuration FWU SCP_BL2U
	--ap-fwu-cfg FILENAME		AP Firmware Updater Configuration BL2U
	--fwu FILENAME			Firmware Updater NS_BL2U
	--fwu-cert FILENAME		Non-Trusted Firmware Updater certificate
	--tb-fw FILENAME		Trusted Boot Firmware BL2
	--scp-fw FILENAME		SCP Firmware SCP_BL2
	--soc-fw FILENAME		EL3 Runtime Firmware BL31
	--tos-fw FILENAME		Secure Payload BL32 (Trusted OS)
	--nt-fw FILENAME		Non-Trusted Firmware BL33
	--rot-cert FILENAME		Root Of Trust key certificate
	--trusted-key-cert FILENAME	Trusted key certificate
	--scp-fw-key-cert FILENAME	SCP Firmware key certificate
	--soc-fw-key-cert FILENAME	SoC Firmware key certificate
	--tos-fw-key-cert FILENAME	Trusted OS Firmware key certificate
	--nt-fw-key-cert FILENAME	Non-Trusted Firmware key certificate
	--tb-fw-cert FILENAME		Trusted Boot Firmware BL2 certificate
	--scp-fw-cert FILENAME		SCP Firmware content certificate
	--soc-fw-cert FILENAME		SoC Firmware content certificate
	--tos-fw-cert FILENAME		Trusted OS Firmware content certificate
	--nt-fw-cert FILENAME		Non-Trusted Firmware content certificate
EOF
    exit
}

echo "!! The fip_create tool is deprecated.  Use the new fiptool. !!"
basedir="$(dirname $0)/../fiptool"
fiptool_args=
while :; do
    case "$1" in
	-h | --help )
	    usage
	    break ;;
	-d | --dump )
	    fiptool_args="info $fiptool_args"
	    shift ;;
	-u | --unpack )
	    fiptool_args="unpack $fiptool_args"
	    shift ;;
	-f | --force )
	    fiptool_args="$fiptool_args --force"
	    shift ;;
	--scp-fwu-cfg | \
	    --ap-fwu-cfg | \
	    --fwu | \
	    --fwu-cert | \
	    --tb-fw | \
	    --scp-fw | \
	    --soc-fw | \
	    --tos-fw | \
	    --nt-fw | \
	    --rot-cert | \
	    --trusted-key-cert | \
	    --scp-fw-key-cert | \
	    --soc-fw-key-cert | \
	    --tos-fw-key-cert | \
	    --nt-fw-key-cert | \
	    --tb-fw-cert | \
	    --scp-fw-cert | \
	    --soc-fw-cert | \
	    --tos-fw-cert | \
	    --nt-fw-cert )
	    fiptool_args="$fiptool_args $1"
	    shift
	    if test -z $1; then
		usage
	    fi
	    fiptool_args="$fiptool_args $1"
	    shift ;;
	* )
	    break ;;
    esac
done

# expect a FIP filename
if test -z $1; then
    usage
fi

is_pack_cmd=1
for arg in $fiptool_args; do
    case "$arg" in
	unpack )
	    is_pack_cmd=0
	    break ;;
	info )
	    is_pack_cmd=0
	    break ;;
	* )
    esac
done

# if --unpack and --dump were not specified
# the default action is to pack
if test "$is_pack_cmd" -eq 1; then
    fiptool_args="update $fiptool_args"
fi

# append FIP filename
fiptool_args="$fiptool_args $1"
echo "Invoking fiptool with args: $fiptool_args"
"$basedir/fiptool" $fiptool_args
