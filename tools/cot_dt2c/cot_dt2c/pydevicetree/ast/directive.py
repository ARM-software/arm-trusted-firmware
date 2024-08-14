#!/usr/bin/env python3
# Copyright (c) 2019 SiFive Inc.
# SPDX-License-Identifier: Apache-2.0

from typing import Any

from cot_dt2c.pydevicetree.ast.helpers import formatLevel, wrapStrings

class Directive:
    """Represents a Devicetree directive

    Directives in Devicetree source are statements of the form

        /directive-name/ [option1 [option2 [...]]];

    Common directive examples include:

        /dts-v1/;
        /include/ "overlay.dtsi";
        /delete-node/ &uart0;
        /delete-property/ status;

    Their semantic meaning depends on the directive name, their location in the Devicetree,
    and their options.
    """
    def __init__(self, directive: str, option: Any = None):
        """Create a directive object"""
        self.directive = directive
        self.option = option

    def __repr__(self) -> str:
        return "<Directive %s>" % self.directive

    def __str__(self) -> str:
        return self.to_dts()

    def to_dts(self, level: int = 0) -> str:
        """Format the Directive in Devicetree Source format"""
        if isinstance(self.option, list):
            return formatLevel(level, "%s %s;\n" % (self.directive,
                                                    wrapStrings(self.option)))
        if isinstance(self.option, str):
            if self.directive == "/include/":
                return formatLevel(level, "%s \"%s\"\n" % (self.directive, self.option))
            return formatLevel(level, "%s \"%s\";\n" % (self.directive, self.option))
        return formatLevel(level, "%s;\n" % self.directive)
