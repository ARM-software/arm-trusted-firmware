#
# Copyright (c) 2024, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

import sys
from cot_dt2c.cot_parser import COT

def generateMain(input, output=None):
    cot = COT(input, output)
    cot.generate_c_file()

def validateMain(input):
    cot = COT(input)
    if not cot.validate_nodes():
        print("not a valid CoT DT file")

def visualizeMain(input):
    cot = COT(input)
    cot.tree_visualization()

if __name__=="__main__":
    if (len(sys.argv) < 2):
        print("usage: python3 " + sys.argv[0] + " [dtsi file path] [optional output c file path]")
        exit()
    if len(sys.argv) == 3:
        generateMain(sys.argv[1], sys.argv[2])
    if len(sys.argv) == 2:
        validateMain(sys.argv[1])
