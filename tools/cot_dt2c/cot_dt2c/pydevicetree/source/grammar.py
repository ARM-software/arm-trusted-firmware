#!/usr/bin/env python3
# Copyright (c) 2019 SiFive Inc.
# SPDX-License-Identifier: Apache-2.0

import os
import sys

import pyparsing as p # type: ignore

ENV_CACHE_OPTION = "PYDEVICETREE_CACHE_SIZE_BOUND"

cache_bound = None
if ENV_CACHE_OPTION in os.environ:
    option = os.environ[ENV_CACHE_OPTION]
    if option != "None":
        try:
            cache_bound = int(option)
        except ValueError:
            print("%s requires a valid integer" % ENV_CACHE_OPTION, file=sys.stderr)
p.ParserElement.enablePackrat(cache_bound)

node_name = p.Word(p.alphanums + ",.-+_") ^ p.Literal("/")
integer = p.pyparsing_common.integer ^ (p.Literal("0x").suppress() + p.pyparsing_common.hex_integer)
unit_address = p.pyparsing_common.hex_integer
unit_addresses = p.delimitedList(unit_address("address"), delim=",")
node_handle = node_name("node_name") + p.Optional(p.Literal("@") + unit_addresses)
property_name = p.Word(p.alphanums + ",.-_+?#")
label = p.Word(p.alphanums + "_").setResultsName("label")
label_creation = p.Combine(label + p.Literal(":"))
string = p.QuotedString(quoteChar='"')
stringlist = p.delimitedList(string)
node_path = p.Combine(p.Literal("/") + \
        p.delimitedList(node_handle, delim="/", combine=True)).setResultsName("path")
path_reference = p.Literal("&{").suppress() + node_path + p.Literal("}").suppress()
label_reference = p.Literal("&").suppress() + label
label_raw = p.Word(p.alphanums + "_")
reference = path_reference ^ label_reference ^ label_raw
include_directive = p.Literal("/include/") + p.QuotedString(quoteChar='"')
generic_directive = p.QuotedString(quoteChar="/", unquoteResults=False) + \
        p.Optional(string ^ property_name ^ node_name ^ reference ^ (integer * 2)) + \
        p.Literal(";").suppress()
directive = include_directive ^ generic_directive

operator = p.oneOf("~ ! * / + - << >> < <= > >= == != & ^ | && ||")
arith_expr = p.Forward()
ternary_element = arith_expr ^ integer
ternary_expr = ternary_element + p.Literal("?") + ternary_element + p.Literal(":") + ternary_element
arith_expr = p.nestedExpr(content=(p.OneOrMore(operator ^ integer) ^ ternary_expr))
arth_str = p.Forward()
arith_str_expr = p.nestedExpr(content=(p.OneOrMore(operator ^ integer ^ label_raw ^ p.Literal(",")) ^ ternary_expr))

label_list = p.OneOrMore(p.Combine(label + p.Literal("\n")))

cell_array = p.Literal("<").suppress() + \
        p.ZeroOrMore(integer ^ arith_expr ^ arith_str_expr ^ label_list ^ string ^ reference ^ label_creation.suppress()) + \
        p.Literal(">").suppress()
bytestring = p.Literal("[").suppress() + \
        (p.OneOrMore(p.Word(p.hexnums, exact=2) ^ label_creation.suppress())) + \
        p.Literal("]").suppress()
property_values = p.Forward()
property_values = p.delimitedList(property_values ^ cell_array ^ bytestring ^ stringlist ^ \
                                  reference ^ label_raw)
property_assignment = property_name("property_name") + p.Optional(p.Literal("=").suppress() + \
        (property_values)).setResultsName("value") + p.Optional(p.Literal(";").suppress())

ifdef_label = p.ZeroOrMore(p.Word(p.alphanums + " _|//*=/(/)"))
ifdef_define = p.Combine(p.Keyword("#if") + ifdef_label)
ifdef_end = p.Combine(p.Keyword("#endif") + ifdef_label)
ifdef_define_values = p.Forward()
ifdef_define_values = p.ZeroOrMore(ifdef_define)
ifdef_end_values = p.Forward()
ifdef_end_values = p.ZeroOrMore(ifdef_end)

node_opener = ifdef_define_values + p.Optional(label_creation) + node_handle + p.Literal("{").suppress()
node_reference_opener = reference + p.Literal("{").suppress()
node_closer = p.Literal("}").suppress() + p.Literal(";").suppress() + ifdef_end_values
node_definition = p.Forward()
# pylint: disable=expression-not-assigned
node_definition << (node_opener ^ node_reference_opener) + \
        p.ZeroOrMore(property_assignment ^ directive ^ node_definition ^ ifdef_define ^ ifdef_end) + \
        node_closer

devicetree = p.ZeroOrMore(directive ^ node_definition)

devicetree.ignore(p.cStyleComment)
devicetree.ignore("//" + p.SkipTo(p.lineEnd))
devicetree.ignore("#include" + p.SkipTo(p.lineEnd))
devicetree.ignore("#define" + p.SkipTo(p.lineEnd))
devicetree.ignore("#else" + p.SkipTo(p.lineEnd))
devicetree.ignore("#error" + p.SkipTo(p.lineEnd))
devicetree.ignore("#ifndef" + p.SkipTo(p.lineEnd))

if __name__ == "__main__":
    if len(sys.argv) > 1:
        devicetree.parseFile(sys.argv[1]).pprint()
