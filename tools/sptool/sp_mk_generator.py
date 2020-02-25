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
FDT_SOURCES, FIP_ARGS and SPTOOL_ARGS which are used in later build steps.
This script also gets SP "uuid" from parsing its PM and converting it to a
standard format.

param1: Generated mk file "sp_gen.mk"
param2: "SP_LAYOUT_FILE", json file containing platform provided information
param3: plat out directory

Generated "sp_gen.mk" file contains triplet of following information for each
Secure Partition entry
    FDT_SOURCES +=  sp1.dts
    SPTOOL_ARGS += -i sp1.bin:sp1.dtb -o sp1.pkg
    FIP_ARGS += --blob uuid=XXXXX-XXX...,file=sp1.pkg

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
gen_file = sys.argv[1]
out_dir = sys.argv[3][2:]
dtb_dir = out_dir + "/fdts/"
print(dtb_dir)

with open(gen_file, 'w') as out_file:
    for key in data.keys():

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
        key = "uuid"

        for line in pm_file:
            if key in line:
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
        out_file.write("\n")
