#!/usr/bin/env python3
# Copyright (c) 2019, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause

"""
This module contains a set of classes and a runner that can generate code for the romlib module
based on the templates in the 'templates' directory.
"""

import argparse
import os
import re
import subprocess
import string
import sys

class IndexFileParser:
    """
    Parses the contents of the index file into the items and dependencies variables. It
    also resolves included files in the index files recursively with circular inclusion detection.
    """

    def __init__(self):
        self.items = []
        self.dependencies = {}
        self.include_chain = []

    def add_dependency(self, parent, dependency):
        """ Adds a dependency into the dependencies variable. """
        if parent in self.dependencies:
            self.dependencies[parent].append(dependency)
        else:
            self.dependencies[parent] = [dependency]

    def get_dependencies(self, parent):
        """ Gets all the recursive dependencies of a parent file. """
        parent = os.path.normpath(parent)
        if parent in self.dependencies:
            direct_deps = self.dependencies[parent]
            deps = direct_deps
            for direct_dep in direct_deps:
                deps += self.get_dependencies(direct_dep)
            return deps

        return []

    def parse(self, file_name):
        """ Opens and parses index file. """
        file_name = os.path.normpath(file_name)

        if file_name not in self.include_chain:
            self.include_chain.append(file_name)
            self.dependencies[file_name] = []
        else:
            raise Exception("Circular dependency detected: " + file_name)

        with open(file_name, "r") as index_file:
            for line in index_file.readlines():
                line_elements = line.split()

                if line.startswith("#") or not line_elements:
                    # Comment or empty line
                    continue

                if line_elements[0] == "reserved":
                    # Reserved slot in the jump table
                    self.items.append({"type": "reserved"})
                elif line_elements[0] == "include" and len(line_elements) > 1:
                    # Include other index file
                    included_file = os.path.normpath(line_elements[1])
                    self.add_dependency(file_name, included_file)
                    self.parse(included_file)
                elif len(line_elements) > 1:
                    # Library function
                    library_name = line_elements[0]
                    function_name = line_elements[1]
                    patch = bool(len(line_elements) > 2 and line_elements[2] == "patch")

                    self.items.append({"type": "function", "library_name": library_name,
                                       "function_name": function_name, "patch": patch})
                else:
                    raise Exception("Invalid line: '" + line + "'")

        self.include_chain.pop()

class RomlibApplication:
    """ Base class of romlib applications. """
    TEMPLATE_DIR = os.path.dirname(os.path.realpath(__file__)) + "/templates/"

    def __init__(self, prog):
        self.args = argparse.ArgumentParser(prog=prog, description=self.__doc__)
        self.config = None

    def parse_arguments(self, argv):
        """ Parses the arguments that should come from the command line arguments. """
        self.config = self.args.parse_args(argv)

    def build_template(self, name, mapping=None, remove_comment=False):
        """
        Loads a template and builds it with the defined mapping. Template paths are always relative
        to this script.
        """

        with open(self.TEMPLATE_DIR + name, "r") as template_file:
            if remove_comment:
                # Removing copyright comment to make the generated code more readable when the
                # template is inserted multiple times into the output.
                template_lines = template_file.readlines()
                end_of_comment_line = 0
                for index, line in enumerate(template_lines):
                    if line.find("*/") != -1:
                        end_of_comment_line = index
                        break
                template_data = "".join(template_lines[end_of_comment_line + 1:])
            else:
                template_data = template_file.read()

            template = string.Template(template_data)
            return template.substitute(mapping)

class IndexPreprocessor(RomlibApplication):
    """ Removes empty and comment lines from the index file and resolves includes. """

    def __init__(self, prog):
        RomlibApplication.__init__(self, prog)

        self.args.add_argument("-o", "--output", help="Output file", metavar="output",
                               default="jmpvar.s")
        self.args.add_argument("--deps", help="Dependency file")
        self.args.add_argument("file", help="Input file")

    def main(self):
        """
        After parsing the input index file it generates a clean output with all includes resolved.
        Using --deps option it also outputs the dependencies in makefile format like gcc's with -M.
        """

        index_file_parser = IndexFileParser()
        index_file_parser.parse(self.config.file)

        with open(self.config.output, "w") as output_file:
            for item in index_file_parser.items:
                if item["type"] == "function":
                    patch = "\tpatch" if item["patch"] else ""
                    output_file.write(
                        item["library_name"] + "\t" + item["function_name"] + patch + "\n")
                else:
                    output_file.write("reserved\n")

        if self.config.deps:
            with open(self.config.deps, "w") as deps_file:
                deps = [self.config.file] + index_file_parser.get_dependencies(self.config.file)
                deps_file.write(self.config.output + ": " + " \\\n".join(deps) + "\n")

class TableGenerator(RomlibApplication):
    """ Generates the jump table by parsing the index file. """

    def __init__(self, prog):
        RomlibApplication.__init__(self, prog)

        self.args.add_argument("-o", "--output", help="Output file", metavar="output",
                               default="jmpvar.s")
        self.args.add_argument("--bti", help="Branch Target Identification", type=int)
        self.args.add_argument("file", help="Input file")

    def main(self):
        """
        Inserts the jmptbl definition and the jump entries into the output file. Also can insert
        BTI related code before entries if --bti option set. It can output a dependency file of the
        included index files. This can be directly included in makefiles.
        """

        index_file_parser = IndexFileParser()
        index_file_parser.parse(self.config.file)

        with open(self.config.output, "w") as output_file:
            output_file.write(self.build_template("jmptbl_header.S"))
            bti = "_bti" if self.config.bti == 1 else ""

            for item in index_file_parser.items:
                template_name = "jmptbl_entry_" + item["type"] + bti + ".S"
                output_file.write(self.build_template(template_name, item, True))

class WrapperGenerator(RomlibApplication):
    """
    Generates a wrapper function for each entry in the index file except for the ones that contain
    the keyword patch. The generated wrapper file is called <lib>_<fn_name>.s.
    """

    def __init__(self, prog):
        RomlibApplication.__init__(self, prog)

        self.args.add_argument("-b", help="Build directory", default=".", metavar="build")
        self.args.add_argument("--bti", help="Branch Target Identification", type=int)
        self.args.add_argument("--list", help="Only list assembly files", action="store_true")
        self.args.add_argument("file", help="Input file")

    def main(self):
        """
        Iterates through the items in the parsed index file and builds the template for each entry.
        """

        index_file_parser = IndexFileParser()
        index_file_parser.parse(self.config.file)

        bti = "_bti" if self.config.bti == 1 else ""
        function_offset = 0
        files = []

        for item_index in range(0, len(index_file_parser.items)):
            item = index_file_parser.items[item_index]

            if item["type"] == "reserved" or item["patch"]:
                continue

            asm = self.config.b + "/" + item["function_name"] + ".s"
            if self.config.list:
                # Only listing files
                files.append(asm)
            else:
                with open(asm, "w") as asm_file:
                    # The jump instruction is 4 bytes but BTI requires and extra instruction so
                    # this makes it 8 bytes per entry.
                    function_offset = item_index * (8 if self.config.bti else 4)

                    item["function_offset"] = function_offset
                    asm_file.write(self.build_template("wrapper" + bti + ".S", item))

        if self.config.list:
            print(" ".join(files))

class VariableGenerator(RomlibApplication):
    """ Generates the jump table global variable with the absolute address in ROM. """

    def __init__(self, prog):
        RomlibApplication.__init__(self, prog)

        self.args.add_argument("-o", "--output", help="Output file", metavar="output",
                               default="jmpvar.s")
        self.args.add_argument("file", help="Input file")

    def main(self):
        """
        Runs nm -a command on the input file and inserts the address of the .text section into the
        template as the ROM address of the jmp_table.
        """
        symbols = subprocess.check_output(["nm", "-a", self.config.file])

        matching_symbol = re.search("([0-9A-Fa-f]+) . \\.text", str(symbols))
        if not matching_symbol:
            raise Exception("No '.text' section was found in %s" % self.config.file)

        mapping = {"jmptbl_address": matching_symbol.group(1)}

        with open(self.config.output, "w") as output_file:
            output_file.write(self.build_template("jmptbl_glob_var.S", mapping))

if __name__ == "__main__":
    APPS = {"genvar": VariableGenerator, "pre": IndexPreprocessor,
            "gentbl": TableGenerator, "genwrappers": WrapperGenerator}

    if len(sys.argv) < 2 or sys.argv[1] not in APPS:
        print("usage: romlib_generator.py [%s] [args]" % "|".join(APPS.keys()), file=sys.stderr)
        sys.exit(1)

    APP = APPS[sys.argv[1]]("romlib_generator.py " + sys.argv[1])
    APP.parse_arguments(sys.argv[2:])
    try:
        APP.main()
        sys.exit(0)
    except FileNotFoundError as file_not_found_error:
        print(file_not_found_error, file=sys.stderr)
    except subprocess.CalledProcessError as called_process_error:
        print(called_process_error.output, file=sys.stderr)

    sys.exit(1)
