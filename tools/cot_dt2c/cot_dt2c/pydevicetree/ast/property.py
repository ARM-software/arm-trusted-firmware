#!/usr/bin/env python3
# Copyright (c) 2019 SiFive Inc.
# SPDX-License-Identifier: Apache-2.0

from typing import List, Any, cast, Tuple, Optional, Iterable
from itertools import zip_longest

from cot_dt2c.pydevicetree.ast.helpers import wrapStrings, formatLevel

class PropertyValues:
    """PropertyValues is the parent class of all values which can be assigned to a Property

    Child classes include

        Bytestring
        CellArray
        StringList
    """
    def __init__(self, values: List[Any]):
        """Create a PropertyValue"""
        self.values = values

    def __repr__(self) -> str:
        return "<PropertyValues " + self.values.__repr__() + ">"

    def __str__(self) -> str:
        return self.to_dts()

    def __iter__(self):
        return iter(self.values)

    def __len__(self) -> int:
        return len(self.values)

    def to_dts(self, formatHex: bool = False) -> str:
        """Format the values in Devicetree Source format"""
        return ", ".join(wrapStrings(self.values, formatHex))

    def __getitem__(self, key) -> Any:
        return self.values[key]

    def __eq__(self, other) -> bool:
        if isinstance(other, PropertyValues):
            return self.values == other.values
        return self.values == other

class Bytestring(PropertyValues):
    """A Bytestring is a sequence of bytes

    In Devicetree, Bytestrings are represented as a sequence of two-digit hexadecimal integers,
    optionally space-separated, enclosed by square brackets:

        [de ad be eef]
    """
    def __init__(self, bytelist: List[int]):
        """Create a Bytestring object"""
        PropertyValues.__init__(self, cast(List[Any], bytearray(bytelist)))

    def __repr__(self) -> str:
        return "<Bytestring " + str(self.values) + ">"

    def to_dts(self, formatHex: bool = False) -> str:
        """Format the bytestring in Devicetree Source format"""
        return "[" + " ".join("%02x" % v for v in self.values) + "]"

class CellArray(PropertyValues):
    """A CellArray is an array of integer values

    CellArrays are commonly used as the value of Devicetree properties like `reg` and `interrupts`.
    The interpretation of each element of a CellArray is device-dependent. For example, the `reg`
    property encodes a CellArray as a list of tuples (base address, size), while the `interrupts`
    property encodes a CellArray as simply a list of interrupt line numbers.
    """
    def __init__(self, cells: List[Any]):
        """Create a CellArray object"""
        PropertyValues.__init__(self, cells)

    def __repr__(self) -> str:
        return "<CellArray " + self.values.__repr__() + ">"

    def to_dts(self, formatHex: bool = False) -> str:
        """Format the cell array in Devicetree Source format"""
        dtsValues = []
        for i in self.values:
            if not isinstance(i, OneString) and not isinstance(i, str):
                dtsValues.append(i)
        return "<" + " ".join(wrapStrings(dtsValues, formatHex)) + ">"

class RegArray(CellArray):
    """A RegArray is the CellArray assigned to the reg property"""
    def __init__(self, cells: List[int],
                 address_cells: int, size_cells: int,
                 names: Optional[List[str]] = None):
        """Create a RegArray from a list of ints"""
        # pylint: disable=too-many-locals
        CellArray.__init__(self, cells)
        self.address_cells = address_cells
        self.size_cells = size_cells

        self.tuples = [] # type: List[Tuple[int, int, Optional[str]]]

        group_size = self.address_cells + self.size_cells

        if len(cells) % group_size != 0:
            raise Exception("CellArray does not contain enough cells")

        grouped_cells = [cells[i:i+group_size] for i in range(0, len(cells), group_size)]

        if not names:
            names = []

        for group, name in zip_longest(grouped_cells, cast(Iterable[Any], names)):
            address = 0
            a_cells = list(reversed(group[:self.address_cells]))
            for a, i in zip(a_cells, range(len(a_cells))):
                address += (1 << (32 * i)) * a

            size = 0
            s_cells = list(reversed(group[self.address_cells:]))
            for s, i in zip(s_cells, range(len(s_cells))):
                size += (1 << (32 * i)) * s

            self.tuples.append(cast(Tuple[int, int, Optional[str]], tuple([address, size, name])))

    def get_by_name(self, name: str) -> Optional[Tuple[int, int]]:
        """Returns the (address, size) tuple with a given name"""
        for t in self.tuples:
            if t[2] == name:
                return cast(Tuple[int, int], tuple(t[:2]))
        return None

    def __repr__(self) -> str:
        return "<RegArray " + self.values.__repr__() + ">"

    def __iter__(self) -> Iterable[Tuple[int, int]]:
        return cast(Iterable[Tuple[int, int]], map(lambda t: tuple(t[:2]), self.tuples))

    def __len__(self) -> int:
        return len(self.tuples)

    def __getitem__(self, key) -> Optional[Tuple[int, int]]:
        return list(self.__iter__())[key]

class RangeArray(CellArray):
    """A RangeArray is the CellArray assigned to the range property"""
    def __init__(self, cells: List[int], child_address_cells: int,
                 parent_address_cells: int, size_cells: int):
        """Create a RangeArray from a list of ints"""
        # pylint: disable=too-many-locals
        CellArray.__init__(self, cells)
        self.child_address_cells = child_address_cells
        self.parent_address_cells = parent_address_cells
        self.size_cells = size_cells

        self.tuples = [] # type: List[Tuple[int, int, int]]

        group_size = self.child_address_cells + self.parent_address_cells + self.size_cells

        if len(cells) % group_size != 0:
            raise Exception("CellArray does not contain enough cells")

        grouped_cells = [cells[i:i+group_size] for i in range(0, len(cells), group_size)]

        def sum_cells(cells: List[int]):
            value = 0
            for cell, index in zip(list(reversed(cells)), range(len(cells))):
                value += (1 << (32 * index)) * cell
            return value

        for group in grouped_cells:
            child_address = sum_cells(group[:self.child_address_cells])
            parent_address = sum_cells(group[self.child_address_cells: \
                                             self.child_address_cells + self.parent_address_cells])
            size = sum_cells(group[self.child_address_cells + self.parent_address_cells:])

            self.tuples.append(cast(Tuple[int, int, int],
                                    tuple([child_address, parent_address, size])))

    def __repr__(self) -> str:
        return "<RangeArray " + self.values.__repr__() + ">"

    def __iter__(self):
        return iter(self.tuples)

    def __len__(self) -> int:
        return len(self.tuples)

    def __getitem__(self, key) -> Any:
        return self.tuples[key]

class StringList(PropertyValues):
    """A StringList is a list of null-terminated strings

    The most common use of a StringList in Devicetree is to describe the `compatible` property.
    """
    def __init__(self, strings: List[str]):
        """Create a StringList object"""
        PropertyValues.__init__(self, strings)

    def __repr__(self) -> str:
        return "<StringList " + self.values.__repr__() + ">"

    def to_dts(self, formatHex: bool = False) -> str:
        """Format the list of strings in Devicetree Source format"""
        return ", ".join(wrapStrings(self.values))

class OneString(PropertyValues):
    def __init__(self, string: str):
        PropertyValues.__init__(self, string)

    def __repr__(self) -> str:
        return self.values.__repr__()

    def to_dts(self, formatHex: bool = False) -> str:
        return super().to_dts(formatHex)

class Property:
    """A Property is a key-value pair for a Devicetree Node

    Properties are used to describe Nodes in the tree. There are many common properties, like

        - compatible
        - reg
        - reg-names
        - ranges
        - interrupt-controller
        - interrupts
        - interrupt-parent
        - clocks
        - status

    Which might commonly describe many or all nodes in a tree, and there are device, vendor,
    operating system, runtime-specific properties.

    Properties can possess no value, conveing meaning solely by their presence:

        interrupt-controller;

    Properties can also possess values such as an array of cells, a list of strings, etc.

        reg = <0x10013000 0x1000>;
        compatible = "sifive,rocket0", "riscv";

    And properties can posses arbitrarily complex values, such as the following from the
    Devicetree specification:

        example = <0xf00f0000 19>, "a strange property format";
    """
    def __init__(self, name: str, values: PropertyValues):
        """Create a Property object"""
        self.name = name
        self.values = values

    def __repr__(self) -> str:
        return "<Property %s>" % self.name

    def __str__(self) -> str:
        return self.to_dts()

    @staticmethod
    def from_dts(dts: str) -> 'Property':
        """Parse a file and return a Devicetree object"""
        # pylint: disable=import-outside-toplevel,cyclic-import
        from pydevicetree.source import parseProperty
        return parseProperty(dts)

    def to_dts(self, level: int = 0) -> str:
        """Format the Property assignment in Devicetree Source format"""
        if self.name in ["reg", "ranges"]:
            value = self.values.to_dts(formatHex=True)
        else:
            value = self.values.to_dts(formatHex=False)

        if value != "":
            return formatLevel(level, "%s = %s;\n" % (self.name, value))
        if self.name == "ifdef":
            return ""
        return formatLevel(level, "%s;\n" % self.name)
