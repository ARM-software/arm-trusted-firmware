#!/usr/bin/python3
# Copyright (c) 2020-2024, Arm Limited. All rights reserved.
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
param5: Generated dts file "sp_list_fragment.dts"

Generated "sp_gen.mk" file contains triplet of following information for each
Secure Partition entry
    FDT_SOURCES +=  sp1.dts
    SPTOOL_ARGS += -i sp1.bin:sp1.dtb -o sp1.pkg
    FIP_ARGS += --blob uuid=XXXXX-XXX...,file=sp1.pkg
    CRT_ARGS += --sp-pkg1 sp1.pkg

It populates the number of SP in the defined macro 'NUM_SP'
    $(eval $(call add_define_val,NUM_SP,{len(sp_layout.keys())}))

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

def get_size(sp_node):
    if not "size" in sp_node:
        print("WARNING: default image size 0x100000")
        return 0x100000

    # Try if it was a decimal value.
    try:
        return int(sp_node["size"])
    except ValueError:
        print("WARNING: trying to parse base 16 size")
        # Try if it is of base 16
        return int(sp_node["size"], 16)

def get_sp_pkg(sp, args :dict):
    check_out_dir(args)
    return os.path.join(args["out_dir"], f"{sp}.pkg")

def is_line_in_sp_gen(line, args :dict):
    with open(args["sp_gen_mk"], "r") as f:
        sppkg_rule = [l for l in f if line in l]
    return len(sppkg_rule) != 0

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

def get_uuid(sp_layout, sp, args :dict):
    ''' Helper to fetch uuid from pm file listed in sp_layout.json'''
    if "uuid" in sp_layout[sp]:
        # Extract the UUID from the JSON file if the SP entry has a 'uuid' field
        uuid_std = uuid.UUID(sp_layout[sp]['uuid'])
    else:
        with open(get_sp_manifest_full_path(sp_layout[sp], args), "r") as pm_f:
            uuid_lines = [l for l in pm_f if 'uuid' in l]
        assert(len(uuid_lines) == 1)
        # The uuid field in SP manifest is the little endian representation
        # mapped to arguments as described in SMCCC section 5.3.
        # Convert each unsigned integer value to a big endian representation
        # required by fiptool.
        uuid_parsed = re.findall("0x([0-9a-f]+)", uuid_lines[0])
        y = list(map(bytearray.fromhex, uuid_parsed))
        z = [int.from_bytes(i, byteorder='little', signed=False) for i in y]
        uuid_std = uuid.UUID(f'{z[0]:08x}{z[1]:08x}{z[2]:08x}{z[3]:08x}')
    return uuid_std

def get_load_address(sp_layout, sp, args :dict):
    ''' Helper to fetch load-address from pm file listed in sp_layout.json'''
    with open(get_sp_manifest_full_path(sp_layout[sp], args), "r") as pm_f:
        load_address_lines = [l for l in pm_f if 'load-address' in l]

    if len(load_address_lines) != 1:
        return None

    load_address_parsed = re.search("(0x[0-9a-f]+)", load_address_lines[0])
    return load_address_parsed.group(0)

@SpSetupActions.sp_action(global_action=True)
def check_max_sps(sp_layout, _, args :dict):
    ''' Check validate the maximum number of SPs is respected. '''
    if len(sp_layout.keys()) > MAX_SP:
        raise Exception(f"Too many SPs in SP layout file. Max: {MAX_SP}")
    return args

@SpSetupActions.sp_action(global_action=True)
def count_sps(sp_layout, _, args :dict):
    ''' Count number of SP and put in NUM_SP '''
    write_to_sp_mk_gen(f"$(eval $(call add_define_val,NUM_SP,{len(sp_layout.keys())}))", args)
    return args

@SpSetupActions.sp_action
def gen_fdt_sources(sp_layout, sp, args :dict):
    ''' Generate FDT_SOURCES values for a given SP. '''
    manifest_path = get_sp_manifest_full_path(sp_layout[sp], args)
    write_to_sp_mk_gen(f"FDT_SOURCES += {manifest_path}", args)
    return args

def generate_sp_pkg(sp_node, pkg, sp_img, sp_dtb):
    ''' Generates the rule in case SP is to be generated in an SP Pkg. '''
    pm_offset = get_pm_offset(sp_node)
    sptool_args = f" --pm-offset {pm_offset}" if pm_offset is not None else ""
    image_offset = get_image_offset(sp_node)
    sptool_args += f" --img-offset {image_offset}" if image_offset is not None else ""
    sptool_args += f" -o {pkg}"
    return f'''
{pkg}: {sp_dtb} {sp_img}
\t$(Q)echo Generating {pkg}
\t$(Q)$(PYTHON) $(SPTOOL)  -i {sp_img}:{sp_dtb} {sptool_args}
'''

def generate_tl_pkg(sp_node, pkg, sp_img, sp_dtb, hob_path = None):
    ''' Generate make rules for a Transfer List type package. '''
    # TE Type for the FF-A manifest.
    TE_FFA_MANIFEST = 0x106
    # TE Type for the SP binary.
    TE_SP_BINARY = 0x103
    # TE Type for the HOB List.
    TE_HOB_LIST = 0x3
    tlc_add_hob = f"\t$(Q)poetry run tlc add --entry {TE_HOB_LIST} {hob_path} {pkg}" if hob_path is not None else ""
    return f'''
{pkg}: {sp_dtb} {sp_img}
\t$(Q)echo Generating {pkg}
\t$(Q)$(TLCTOOL) create --size {get_size(sp_node)} --entry {TE_FFA_MANIFEST} {sp_dtb} {pkg} --align 12
\t$(Q)$(TLCTOOL) add --entry {TE_SP_BINARY} {sp_img} {pkg}
'''

@SpSetupActions.sp_action
def gen_partition_pkg(sp_layout, sp, args :dict):
    ''' Generate Sp Pkgs rules. '''
    pkg = get_sp_pkg(sp, args)

    sp_dtb_name = os.path.basename(get_file_from_layout(sp_layout[sp]["pm"]))[:-1] + "b"
    sp_dtb = os.path.join(args["out_dir"], f"fdts/{sp_dtb_name}")
    sp_img = get_sp_img_full_path(sp_layout[sp], args)

    # Do not generate rule if already there.
    if is_line_in_sp_gen(f'{pkg}:', args):
        return args

    # This should include all packages of all kinds.
    write_to_sp_mk_gen(f"SP_PKGS += {pkg}\n", args)
    package_type = sp_layout[sp]["package"] if "package" in sp_layout[sp] else "sp_pkg"

    if package_type == "sp_pkg":
        partition_pkg_rule = generate_sp_pkg(sp_layout[sp], pkg, sp_img, sp_dtb)
    elif package_type == "tl_pkg":
        partition_pkg_rule = generate_tl_pkg(sp_layout[sp], pkg, sp_img, sp_dtb)
    else:
        raise ValueError(f"Specified invalid pkg type {package_type}")

    write_to_sp_mk_gen(partition_pkg_rule, args)
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
    uuid_std = get_uuid(sp_layout, sp, args)
    write_to_sp_mk_gen(f"FIP_ARGS += --blob uuid={str(uuid_std)},file={get_sp_pkg(sp, args)}\n", args)
    return args

@SpSetupActions.sp_action
def gen_fconf_fragment(sp_layout, sp, args: dict):
    ''' Generate the fconf fragment file'''
    with open(args["fconf_fragment"], "a") as f:
        uuid = get_uuid(sp_layout, sp, args)
        owner = "Plat" if sp_layout[sp].get("owner") == "Plat" else "SiP"

        if "physical-load-address" in sp_layout[sp].keys():
            load_address = sp_layout[sp]["physical-load-address"]
        else:
            load_address = get_load_address(sp_layout, sp, args)

        if load_address is not None:
            f.write(
f'''\
{sp} {{
    uuid = "{uuid}";
    load-address = <{load_address}>;
    owner = "{owner}";
}};

''')
        else:
            print("Warning: No load-address was found in the SP manifest.")

    return args

def init_sp_actions(sys):
    # Initialize arguments for the SP actions framework
    args = {}
    args["sp_gen_mk"] = os.path.abspath(sys.argv[1])
    sp_layout_file = os.path.abspath(sys.argv[2])
    args["sp_layout_dir"] = os.path.dirname(sp_layout_file)
    args["out_dir"] = os.path.abspath(sys.argv[3])
    args["dualroot"] = sys.argv[4] == "dualroot"
    args["fconf_fragment"] = os.path.abspath(sys.argv[5])


    with open(sp_layout_file) as json_file:
        sp_layout = json.load(json_file)
    #Clear content of file "sp_gen.mk".
    with open(args["sp_gen_mk"], "w"):
        None
    #Clear content of file "fconf_fragment".
    with open(args["fconf_fragment"], "w"):
        None

    return args, sp_layout

if __name__ == "__main__":
    args, sp_layout = init_sp_actions(sys)
    SpSetupActions.run_actions(sp_layout, args)
