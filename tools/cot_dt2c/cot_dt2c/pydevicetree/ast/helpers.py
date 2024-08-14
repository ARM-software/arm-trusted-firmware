#!/usr/bin/env python3
# Copyright (c) 2019 SiFive Inc.
# SPDX-License-Identifier: Apache-2.0

from typing import List, Any

from cot_dt2c.pydevicetree.ast.reference import Reference

def formatLevel(level: int, s: str) -> str:
    """Helper to indent a string with a number of tabs"""
    return "\t" * level + s

def wrapStrings(values: List[Any], formatHex: bool = False) -> List[Any]:
    """Helper to wrap strings in quotes where appropriate"""
    wrapped = []
    for v in values:
        if isinstance(v, Reference):
            wrapped.append(v.to_dts())
        elif isinstance(v, str):
            wrapped.append("\"%s\"" % v)
        elif isinstance(v, int):
            if formatHex:
                wrapped.append("0x%x" % v)
            else:
                wrapped.append(str(v))
        else:
            wrapped.append(str(v))
    return wrapped
