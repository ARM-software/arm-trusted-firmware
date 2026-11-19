#!/usr/bin/env python3
#
# Copyright (c) 2026, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

import argparse
import csv
import os
import pathlib
import re
import shutil

from math import isclose


def process_result(results, res):
    res_list = [int(i) for i in res.groups()[1:]]
    if res[1] not in results:
        results[res[1]] = [res_list]
    else:
        results[res[1]].append(res_list)

def process_lava_log(filename):
    results = {}
    with open(filename, "r") as f:
        version = r"<RT_INSTR:(\w+)\t(\d+)\t(\d+)\t(\d+)"
        ext_instr_row = version + r"\t(\d+)\t(\d+)"
        for line in f.readlines():
            p = version if "version" in line else ext_instr_row
            res = re.search(p, line)
            if res:
                process_result(results, res)
    return results

def parse_args():
    parser = argparse.ArgumentParser()
    parser.add_argument("lava_log", metavar="LAVA-output")
    parser.add_argument(
        "--data",
        type=pathlib.Path,
        metavar="base_dir",
        dest="data_base",
        required=True,
        help="Path to a data folder of CSVs. Will be used as a baseline and the data updated.",
    )
    parser.add_argument(
        "-t",
        "--tolerance",
        type=float,
        default=0.02,
        help="Allowed relative tolerance to print.",
    )
    return parser.parse_args()

def parse_int(number_str):
    return int(number_str.split(" ")[0])

def main():
    args = parse_args()
    new_tests = process_lava_log(args.lava_log)
    old_tests = {}
    previous_path = os.path.join(args.data_base, "previous")
    current_path = os.path.join(args.data_base, "current")

    format_delta = lambda x1, x2: f"{x2}" + (
        f" ({(x2 - x1) / x1:+.2%})"
        if not isclose(x1, x2, rel_tol=args.tolerance)
        else ""
    )

    # fetch the old data
    for test in new_tests.keys():
        with open(os.path.join(current_path, test + ".csv")) as file:
            old_tests[test] = [[parse_int(item) for item in line] for line in csv.reader(file, dialect=csv.unix_dialect)]


    # remove old files and make the current ones old
    shutil.rmtree(previous_path)
    os.rename(current_path, previous_path)
    os.mkdir(current_path)

    # fill out the new data
    for test, new_data in new_tests.items():
        data = [
            r[:2] + [format_delta(_c, c) for c, _c in zip(r[2:], _r[2:])]
            for r, _r in zip(new_data, old_tests[test])
        ]

        with open(os.path.join(current_path, test + ".csv"), "w", newline='') as file:
            writer = csv.writer(file, dialect=csv.unix_dialect)
            writer.writerows(data)

if __name__ == "__main__":
    main()
