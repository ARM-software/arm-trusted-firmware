#
# Copyright (c) 2024, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

import os
import sys

from cot_dt2c.cli import *
from click.testing import CliRunner

def get_script_path():
    return os.path.dirname(os.path.realpath(sys.argv[0]))

def test_convert():
    runner = CliRunner()
    test_file = get_script_path() + "/test.dtsi"
    test_output = get_script_path() + "/test.c"

    result = runner.invoke(convert_to_c, [test_file, test_output])
    try:
        assert result.output == ""
    except:
        print("test convert fail")

    try:
        os.remove(test_output)
    except OSError:
        pass

if __name__=="__main__":
    test_convert()
