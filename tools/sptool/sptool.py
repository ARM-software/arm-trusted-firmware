#!/usr/bin/python3
# Copyright (c) 2022, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause

#
# Copyright 2022 The Hafnium Authors.
#
# Use of this source code is governed by a BSD-style
# license that can be found in the LICENSE file or at
# https://opensource.org/licenses/BSD-3-Clause.

"""
Script which generates a Secure Partition package.
https://trustedfirmware-a.readthedocs.io/en/latest/components/secure-partition-manager.html#secure-partition-packages
"""

import argparse
from collections import namedtuple
import sys
from shutil import copyfileobj
import os

HF_PAGE_SIZE = 0x1000 # bytes
HEADER_ELEMENT_BYTES = 4 # bytes
MANIFEST_IMAGE_SPLITTER=':'
PM_OFFSET_DEFAULT = "0x1000"
IMG_OFFSET_DEFAULT = "0x4000"

def split_dtb_bin(i : str):
    return i.split(MANIFEST_IMAGE_SPLITTER)

def align_to_page(n):
    return HF_PAGE_SIZE * \
          (round(n / HF_PAGE_SIZE) + \
           (1 if n % HF_PAGE_SIZE else 0))

def to_bytes(value):
    return int(value).to_bytes(HEADER_ELEMENT_BYTES, 'little')

class SpPkg:
    def __init__(self, pm_path : str, img_path : str, pm_offset: int,
                 img_offset: int):
        if not os.path.isfile(pm_path) or not os.path.isfile(img_path):
            raise Exception(f"Parameters should be path.  \
                              manifest: {pm_path}; img: {img_path}")
        self.pm_path = pm_path
        self.img_path = img_path
        self._SpPkgHeader = namedtuple("SpPkgHeader",
                             ("magic", "version",
                              "pm_offset", "pm_size",
                              "img_offset", "img_size"))

        if pm_offset >= img_offset:
            raise ValueError("pm_offset must be smaller than img_offset")

        is_hfpage_aligned = lambda val : val % HF_PAGE_SIZE == 0
        if not is_hfpage_aligned(pm_offset) or not is_hfpage_aligned(img_offset):
           raise ValueError(f"Offsets provided need to be page aligned: pm-{pm_offset}, img-{img_offset}")

        if img_offset - pm_offset < self.pm_size:
            raise ValueError(f"pm_offset and img_offset do not fit the specified file:{pm_path})")

        self.pm_offset = pm_offset
        self.img_offset = img_offset

    def __str__(self):
        return \
        f'''--SP package Info--
        header:{self.header}
        pm: {self.pm_path}
        img: {self.img_path}
        '''

    @property
    def magic(self):
        return "SPKG".encode()

    @property
    def version(self):
        return 0x2

    @property
    def pm_size(self):
        return os.path.getsize(self.pm_path)

    @property
    def img_size(self):
        return os.path.getsize(self.img_path)

    @property
    def header(self):
        return self._SpPkgHeader(
                self.magic,
                self.version,
                self.pm_offset,
                self.pm_size,
                self.img_offset,
                self.img_size)

    @property
    def header_size(self):
        return len(self._SpPkgHeader._fields)

    def generate(self, f_out : str):
        with open(f_out, "wb+") as output:
            for h in self.header:
                to_write = h if type(h) is bytes else to_bytes(h)
                output.write(to_write)
            output.seek(self.pm_offset)
            with open(self.pm_path, "rb") as pm:
                copyfileobj(pm, output)
            output.seek(self.img_offset)
            with open(self.img_path, "rb") as img:
                copyfileobj(img, output)

def Main():
    parser = argparse.ArgumentParser()
    parser.add_argument("-i", required=True,
                        help="path to partition's image and manifest separated by a colon.")
    parser.add_argument("--pm-offset", required=False, default=PM_OFFSET_DEFAULT,
                        help="set partitition manifest offset.")
    parser.add_argument("--img-offset", required=False, default=IMG_OFFSET_DEFAULT,
                        help="set partition image offset.")
    parser.add_argument("-o", required=True, help="set output file path.")
    parser.add_argument("-v", required=False, action="store_true",
                        help="print package information.")
    args = parser.parse_args()

    if not os.path.exists(os.path.dirname(args.o)):
        raise Exception("Provide a valid output file path!\n")

    image_path, manifest_path = split_dtb_bin(args.i)
    pm_offset = int(args.pm_offset, 0)
    img_offset = int(args.img_offset, 0)
    pkg = SpPkg(manifest_path, image_path, pm_offset, img_offset)
    pkg.generate(args.o)

    if args.v is True:
        print(pkg)

    return 0

if __name__ == "__main__":
    sys.exit(Main())
