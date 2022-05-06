#!/usr/bin/python3
# Copyright (c) 2020-2022, Arm Limited. All rights reserved.
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
If the SP entry in the layout file has a "uuid" field the scripts gets the UUID
from there, otherwise it parses the associated partition manifest and extracts
the UUID from there.

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
                "pm": "test/sp2.dts",
                "uuid": "1b1820fe-48f7-4175-8999-d51da00b7c9f"
        }

        ...
}

"""
import json
import os
import re
import sys
import uuid
from spactions import SpSetupActions

MAX_SP = 8
UUID_LEN = 4

# Some helper functions to access args propagated to the action functions in
# SpSetupActions framework.
def check_sp_mk_gen(args :dict):
    if "sp_gen_mk" not in args.keys():
        raise Exception(f"Path to file sp_gen.mk needs to be in 'args'.")

def check_out_dir(args :dict):
    if "out_dir" not in args.keys() or not os.path.isdir(args["out_dir"]):
        raise Exception("Define output folder with \'out_dir\' key.")

def check_sp_layout_dir(args :dict):
    if "sp_layout_dir" not in args.keys() or not os.path.isdir(args["sp_layout_dir"]):
        raise Exception("Define output folder with \'sp_layout_dir\' key.")

def write_to_sp_mk_gen(content, args :dict):
    check_sp_mk_gen(args)
    with open(args["sp_gen_mk"], "a") as f:
        f.write(f"{content}\n")

def get_sp_manifest_full_path(sp_node, args :dict):
    check_sp_layout_dir(args)
    return os.path.join(args["sp_layout_dir"], get_file_from_layout(sp_node["pm"]))

def get_sp_img_full_path(sp_node, args :dict):
    check_sp_layout_dir(args)
    return os.path.join(args["sp_layout_dir"], get_file_from_layout(sp_node["image"]))

def get_sp_pkg(sp, args :dict):
    check_out_dir(args)
    return os.path.join(args["out_dir"], f"{sp}.pkg")

def is_line_in_sp_gen(line, args :dict):
    with open(args["sp_gen_mk"], "r") as f:
        sppkg_rule = [l for l in f if line in l]
    return len(sppkg_rule) is not 0

def get_file_from_layout(node):
    ''' Helper to fetch a file path from sp_layout.json. '''
    if type(node) is dict and "file" in node.keys():
        return node["file"]
    return node

def get_offset_from_layout(node):
    ''' Helper to fetch an offset from sp_layout.json. '''
    if type(node) is dict and "offset" in node.keys():
        return int(node["offset"], 0)
    return None

def get_image_offset(node):
    ''' Helper to fetch image offset from sp_layout.json '''
    return get_offset_from_layout(node["image"])

def get_pm_offset(node):
    ''' Helper to fetch pm offset from sp_layout.json '''
    return get_offset_from_layout(node["pm"])

@SpSetupActions.sp_action(global_action=True)
def check_max_sps(sp_layout, _, args :dict):
    ''' Check validate the maximum number of SPs is respected. '''
    if len(sp_layout.keys()) > MAX_SP:
        raise Exception(f"Too many SPs in SP layout file. Max: {MAX_SP}")
    return args

@SpSetupActions.sp_action
def gen_fdt_sources(sp_layout, sp, args :dict):
    ''' Generate FDT_SOURCES values for a given SP. '''
    manifest_path = get_sp_manifest_full_path(sp_layout[sp], args)
    write_to_sp_mk_gen(f"FDT_SOURCES += {manifest_path}", args)
    return args

@SpSetupActions.sp_action
def gen_sptool_args(sp_layout, sp, args :dict):
    ''' Generate Sp Pkgs rules. '''
    sp_pkg = get_sp_pkg(sp, args)
    sp_dtb_name = os.path.basename(get_file_from_layout(sp_layout[sp]["pm"]))[:-1] + "b"
    sp_dtb = os.path.join(args["out_dir"], f"fdts/{sp_dtb_name}")

    # Do not generate rule if already there.
    if is_line_in_sp_gen(f'{sp_pkg}:', args):
        return args
    write_to_sp_mk_gen(f"SP_PKGS += {sp_pkg}\n", args)

    sptool_args = f" -i {get_sp_img_full_path(sp_layout[sp], args)}:{sp_dtb}"
    pm_offset = get_pm_offset(sp_layout[sp])
    sptool_args += f" --pm-offset {pm_offset}" if pm_offset is not None else ""
    image_offset = get_image_offset(sp_layout[sp])
    sptool_args += f" --img-offset {image_offset}" if image_offset is not None else ""
    sptool_args += f" -o {sp_pkg}"
    sppkg_rule = f'''
{sp_pkg}:
\t$(Q)echo Generating {sp_pkg}
\t$(Q)$(PYTHON) $(SPTOOL) {sptool_args}
'''
    write_to_sp_mk_gen(sppkg_rule, args)
    return args

@SpSetupActions.sp_action(global_action=True, exec_order=1)
def check_dualroot(sp_layout, _, args :dict):
    ''' Validate the amount of SPs from SiP and Platform owners. '''
    if not args.get("dualroot"):
        return args
    args["split"] =  int(MAX_SP / 2)
    owners = [sp_layout[sp].get("owner") for sp in sp_layout]
    args["plat_max_count"] = owners.count("Plat")
    # If it is owned by the platform owner, it is assigned to the SiP.
    args["sip_max_count"] = len(sp_layout.keys()) - args["plat_max_count"]
    if  args["sip_max_count"] > args["split"] or args["sip_max_count"] > args["split"]:
        print(f"WARN: SiP Secure Partitions should not be more than {args['split']}")
    # Counters for gen_crt_args.
    args["sip_count"] = 1
    args["plat_count"] = 1
    return args

@SpSetupActions.sp_action
def gen_crt_args(sp_layout, sp, args :dict):
    ''' Append CRT_ARGS. '''
    # If "dualroot" is configured, 'sp_pkg_idx' depends on whether the SP is owned
    # by the "SiP" or the "Plat".
    if args.get("dualroot"):
        # If the owner is not specified as "Plat", default to "SiP".
        if sp_layout[sp].get("owner") == "Plat":
            if args["plat_count"] > args["plat_max_count"]:
                raise ValueError("plat_count can't surpass plat_max_count in args.")
            sp_pkg_idx = args["plat_count"] + args["split"]
            args["plat_count"] += 1
        else:
            if args["sip_count"] > args["sip_max_count"]:
                raise ValueError("sip_count can't surpass sip_max_count in args.")
            sp_pkg_idx = args["sip_count"]
            args["sip_count"] += 1
    else:
        sp_pkg_idx = [k for k in sp_layout.keys()].index(sp) + 1
    write_to_sp_mk_gen(f"CRT_ARGS += --sp-pkg{sp_pkg_idx} {get_sp_pkg(sp, args)}\n", args)
    return args

@SpSetupActions.sp_action
def gen_fiptool_args(sp_layout, sp, args :dict):
    ''' Generate arguments for the FIP Tool. '''
    if "uuid" in sp_layout[sp]:
        # Extract the UUID from the JSON file if the SP entry has a 'uuid' field
        uuid_std = uuid.UUID(data[key]['uuid'])
    else:
        with open(get_sp_manifest_full_path(sp_layout[sp], args), "r") as pm_f:
            uuid_lines = [l for l in pm_f if 'uuid' in l]
        assert(len(uuid_lines) is 1)
        # The uuid field in SP manifest is the little endian representation
        # mapped to arguments as described in SMCCC section 5.3.
        # Convert each unsigned integer value to a big endian representation
        # required by fiptool.
        uuid_parsed = re.findall("0x([0-9a-f]+)", uuid_lines[0])
        y = list(map(bytearray.fromhex, uuid_parsed))
        z = [int.from_bytes(i, byteorder='little', signed=False) for i in y]
        uuid_std = uuid.UUID(f'{z[0]:08x}{z[1]:08x}{z[2]:08x}{z[3]:08x}')
    write_to_sp_mk_gen(f"FIP_ARGS += --blob uuid={str(uuid_std)},file={get_sp_pkg(sp, args)}\n", args)
    return args

def init_sp_actions(sys):
    sp_layout_file = os.path.abspath(sys.argv[2])
    with open(sp_layout_file) as json_file:
        sp_layout = json.load(json_file)
    # Initialize arguments for the SP actions framework
    args = {}
    args["sp_gen_mk"] = os.path.abspath(sys.argv[1])
    args["sp_layout_dir"] = os.path.dirname(sp_layout_file)
    args["out_dir"] = os.path.abspath(sys.argv[3])
    args["dualroot"] = sys.argv[4] == "dualroot"
    #Clear content of file "sp_gen.mk".
    with open(args["sp_gen_mk"], "w"):
        None
    return args, sp_layout

if __name__ == "__main__":
    args, sp_layout = init_sp_actions(sys)
    SpSetupActions.run_actions(sp_layout, args)
