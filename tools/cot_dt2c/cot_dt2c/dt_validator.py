#
# Copyright (c) 2024, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

import sys
from os import path, walk, mkdir
import subprocess
from pydevicetree import Devicetree

class bcolors:
    HEADER = '\033[95m'
    OKBLUE = '\033[94m'
    OKCYAN = '\033[96m'
    OKGREEN = '\033[92m'
    WARNING = '\033[93m'
    FAIL = '\033[91m'
    ENDC = '\033[0m'
    BOLD = '\033[1m'
    UNDERLINE = '\033[4m'

class DTTree:
    def __init__(self, input):
        self.input = input
        self.test_dir = "./tmp"
        self.logging_file = self.test_dir + "/result.log"

    def dtValidate(self):
        subprocess.run(["rm", "-rf", self.test_dir])

        if not path.exists(self.test_dir):
            mkdir(self.test_dir)

        if path.isfile(self.input):
            self.dtValidateFile(self.input, printInfo=True)
            return

        if path.isdir(self.input):
            self.dtValidateFiles()
            return

    def dtValidateFile(self, input, printInfo=False):
        valid, tree = self.dtParseFile(input, printInfo)

        if not valid:
            return False

        if input.rfind("/") != -1:
            filename = self.test_dir + input[input.rfind("/"):]
        else:
            filename = self.test_dir + "/" + input

        f = open(filename, "w+")
        if "/dts-v1/;" not in str(tree):
            f.write("/dts-v1/;\n\n")
        f.write(str(tree))
        f.close()

        if str(tree) == "":
            return valid

        return valid

    def dtParseFile(self, input, printInfo=False):
        with open(input, 'r') as f:
            contents = f.read()

        pos = contents.find("/ {")
        if pos != -1:
            contents = contents[pos:]

        try:
            tree = Devicetree.parseStr(contents)
            if printInfo:
                print(bcolors.OKGREEN + "{} parse tree successfully".format(input) + bcolors.ENDC)
        except Exception as e:
            if printInfo:
                print(bcolors.FAIL + "{} parse tree failed:\t{}".format(input, str(e)) + bcolors.ENDC)
            else:
                f = open(self.logging_file, "a")
                f.write("=====================================================================================\n")
                f.write("{} result:\n".format(input))
                f.write("{} INVALID:\t{}\n".format(input, str(e)))
                f.close()
            return False, None

        return True, tree

    def dtValidateFiles(self):
        f = []
        for (dirpath, dirnames, filenames) in walk(self.input):
            f.extend(filenames)

        allFile = len(f)
        dtsiFile = 0
        validFile = 0
        invalidFile = 0

        for i in f:
            if (".dtsi" in i or ".dts" in i) and "cot" not in i and "fw-config" not in i:
                dtsiFile += 1
                valid = True

                if self.input[-1] == "/":
                    valid = self.dtValidateFile(self.input + i)
                else:
                    valid = self.dtValidateFile(self.input + "/" + i)

                if valid:
                    validFile += 1
                else:
                    invalidFile += 1

        print("=====================================================")
        print("Total File: " + str(allFile))
        print("Total DT File: " + str(dtsiFile))
        print("Total Valid File: " + str(validFile))
        print("Total Invalid File: " + str(invalidFile))

def dtValidatorMain(input):
    dt = DTTree(input)
    dt.dtValidate()

if __name__=="__main__":
    if (len(sys.argv) < 2):
        print("usage: python3 " + sys.argv[0] + " [dtsi file path] or [dtsi folder path]")
        exit()
    if len(sys.argv) == 2:
        dtValidatorMain(sys.argv[1])
