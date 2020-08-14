#!/usr/bin/python3
# Copyright (c) 2020, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause

"""
This script is invoked by Make system and generates secure partition makefile.
It expects platform provided secure partition layout file which contains list
of Secure Partition Images and Partition manifests(PM).
Layout file can exist outside of TF-A tree and the paths of Image and PM files
must be relative to it.

This script parses the layout file and generates a make file which updates
FDT_SOURCES, FIP_ARGS, CRT_ARGS and SPTOOL_ARGS which are used in later build
steps.
This script also gets SP "uuid" from parsing its PM and converting it to a
standard format.

param1: Generated mk file "sp_gen.mk"
param2: "SP_LAYOUT_FILE", json file containing platform provided information
param3: plat out directory
param4: CoT parameter

Generated "sp_gen.mk" file contains triplet of following information for each
Secure Partition entry
    FDT_SOURCES +=  sp1.dts
    SPTOOL_ARGS += -i sp1.bin:sp1.dtb -o sp1.pkg
    FIP_ARGS += --blob uuid=XXXXX-XXX...,file=sp1.pkg
    CRT_ARGS += --sp-pkg1 sp1.pkg

A typical SP_LAYOUT_FILE file will look like
{
        "SP1" : {
                "image": "sp1.bin",
                "pm": "test/sp1.dts"
        },

        "SP2" : {
                "image": "sp2.bin",
                "pm": "test/sp2.dts"
        }

        ...
}

"""

import getopt
import json
import os
import re
import sys
import uuid

with open(sys.argv[2],'r') as in_file:
    data = json.load(in_file)
json_file = os.path.abspath(sys.argv[2])
json_dir = os.path.dirname(json_file)
gen_file = os.path.abspath(sys.argv[1])
out_dir = os.path.abspath(sys.argv[3])
dtb_dir = out_dir + "/fdts/"
MAX_SP = 8
dualroot = sys.argv[4].lower() == "dualroot"
split = int(MAX_SP / 2)
print(dtb_dir)
platform_count = 1
sip_count = 1

with open(gen_file, 'w') as out_file:
    for idx, key in enumerate(data.keys()):

        pkg_num = idx + 1

        if (pkg_num > MAX_SP):
            print("WARNING: Too many secure partitions\n")
            exit(-1)

        if dualroot:
            owner = data[key].get('owner')
            if owner == "Plat":
                if (platform_count > split):
                    print("WARNING: Maximum Secure partitions by Plat " +
                    "have been exceeded (" + str(split) + ")\n")
                    exit(-1)
                pkg_num = split + platform_count
                platform_count += 1
            elif (sip_count > split):
                print("WARNING: Maximum Secure partitions by SiP " +
                "have been exceeded (" + str(split) + ")\n")
                exit(-1)
            else:
                pkg_num = sip_count
                sip_count += 1

        """
        Append FDT_SOURCES
        """
        dts = os.path.join(json_dir, data[key]['pm'])
        dtb = dtb_dir + os.path.basename(data[key]['pm'][:-1] + "b")
        out_file.write("FDT_SOURCES += " + dts + "\n")

        """
        Update SPTOOL_ARGS
        """
        dst = out_dir + "/" + key + ".pkg"
        src = [ json_dir + "/" + data[key]['image'] , dtb  ]
        out_file.write("SPTOOL_ARGS += -i " + ":".join(src) + " -o " + dst + "\n")

        """
        Extract uuid from partition manifest
        """
        pm_file = open(dts)
        uuid_key = "uuid"

        for line in pm_file:
            if uuid_key in line:
                uuid_hex = re.findall(r'\<(.+?)\>', line)[0];

        # PM has uuid in format 0xABC... 0x... 0x... 0x...
        # Get rid of '0x' and spaces and convert to string of hex digits
        uuid_hex = uuid_hex.replace('0x','').replace(' ','')
        # make UUID from a string of hex digits
        uuid_std = uuid.UUID(uuid_hex)
        # convert UUID to a string of hex digits in standard form
        uuid_std = str(uuid_std)

        """
        Append FIP_ARGS
        """
        out_file.write("FIP_ARGS += --blob uuid=" + uuid_std + ",file=" + dst + "\n")

        """
        Append CRT_ARGS
        """

        out_file.write("CRT_ARGS += --sp-pkg" + str(pkg_num) + " " + dst + "\n")
        out_file.write("\n")
