TF-A Memory Layout Tool
=======================

TF-A's memory layout tool is a Python script for analyzing the virtual
memory layout of TF-A builds.

Prerequisites
~~~~~~~~~~~~~

#. Python (3.8 or later)
#. `Poetry`_ Python package manager

Getting Started
~~~~~~~~~~~~~~~

#. Install Poetry

    .. code:: shell

        curl -sSL https://install.python-poetry.org | python3 -

#. Install the required packages

    .. code:: shell

        poetry install --with memory

#. Verify that the tool runs in the installed virtual environment

    .. code:: shell

        poetry run memory --help

Symbol Virtual Map
~~~~~~~~~~~~~~~~~~

The tool can be used to generate a visualisation of the symbol table. By
default, it prints the symbols representing the start and end address of the
main memory regions in an ELF file (i.e. text, bss, rodata) but can be modified
to print any set of symbols.

.. code:: shell

    $ poetry run memory -s
    build-path: build/fvp/release
    Virtual Address Map:
               +------------__BL1_RAM_END__------------+---------------------------------------+
               +---------__COHERENT_RAM_END__----------+                                       |
               +--------__COHERENT_RAM_START__---------+                                       |
    0x0403b000 +----------__XLAT_TABLE_END__-----------+                                       |
    0x04036000 +---------__XLAT_TABLE_START__----------+                                       |
               +--------__BASE_XLAT_TABLE_END__--------+                                       |
    0x04035600 +--------------__BSS_END__--------------+                                       |
               +-------__BASE_XLAT_TABLE_START__-------+                                       |
               +-----__PMF_PERCPU_TIMESTAMP_END__------+                                       |
               +---------__PMF_TIMESTAMP_END__---------+                                       |
    0x04035400 +--------__PMF_TIMESTAMP_START__--------+                                       |
               +-------------__BSS_START__-------------+                                       |
    0x04034a00 +------------__STACKS_END__-------------+                                       |
    0x04034500 +-----------__STACKS_START__------------+                                       |
    0x040344c5 +-----------__DATA_RAM_END__------------+                                       |
               +-----------__BL1_RAM_START__-----------+                                       |
    0x04034000 +----------__DATA_RAM_START__-----------+                                       |
               |                                       +---------__COHERENT_RAM_END__----------+
               |                                       +--------__COHERENT_RAM_START__---------+
    0x0402e000 |                                       +----------__XLAT_TABLE_END__-----------+
    0x04029000 |                                       +---------__XLAT_TABLE_START__----------+
               |                                       +--------__BASE_XLAT_TABLE_END__--------+
    0x04028800 |                                       +--------------__BSS_END__--------------+
               |                                       +-------__BASE_XLAT_TABLE_START__-------+
               |                                       +-----__PMF_PERCPU_TIMESTAMP_END__------+
               |                                       +---------__PMF_TIMESTAMP_END__---------+
    0x04028580 |                                       +--------__PMF_TIMESTAMP_START__--------+
    0x04028000 |                                       +-------------__BSS_START__-------------+
    0x04027e40 |                                       +------------__STACKS_END__-------------+
    0x04027840 |                                       +-----------__STACKS_START__------------+
    0x04027000 |                                       +------------__RODATA_END__-------------+
               |                                       +------------__CPU_OPS_END__------------+
               |                                       +-----------__CPU_OPS_START__-----------+
               |                                       +--------__FCONF_POPULATOR_END__--------+
               |                                       +--------------__GOT_END__--------------+
               |                                       +-------------__GOT_START__-------------+
               |                                       +---------__PMF_SVC_DESCS_END__---------+
    0x04026c10 |                                       +--------__PMF_SVC_DESCS_START__--------+
    0x04026bf8 |                                       +-------__FCONF_POPULATOR_START__-------+
               |                                       +-----------__RODATA_START__------------+
    0x04026000 |                                       +-------------__TEXT_END__--------------+
    0x04021000 |                                       +------------__TEXT_START__-------------+
    0x000062b5 +------------__BL1_ROM_END__------------+                                       |
    0x00005df0 +----------__DATA_ROM_START__-----------+                                       |
               +------------__CPU_OPS_END__------------+                                       |
               +--------------__GOT_END__--------------+                                       |
               +-------------__GOT_START__-------------+                                       |
    0x00005de8 +------------__RODATA_END__-------------+                                       |
               +-----------__CPU_OPS_START__-----------+                                       |
               +--------__FCONF_POPULATOR_END__--------+                                       |
               +---------__PMF_SVC_DESCS_END__---------+                                       |
    0x00005c98 +--------__PMF_SVC_DESCS_START__--------+                                       |
    0x00005c80 +-------__FCONF_POPULATOR_START__-------+                                       |
               +-----------__RODATA_START__------------+                                       |
    0x00005000 +-------------__TEXT_END__--------------+                                       |
    0x00000000 +------------__TEXT_START__-------------+---------------------------------------+

Addresses are displayed in hexadecimal by default but can be printed in decimal
instead with the ``-d`` option.

Because of the length of many of the symbols, the tool defaults to a text width
of 120 chars. This can be increased if needed with the ``-w`` option.

For more detailed help instructions, run:

.. code:: shell

    poetry run memory --help

Memory Footprint
~~~~~~~~~~~~~~~~

The tool enables users to view static memory consumption. When the options
``-f``, or ``--footprint`` are provided, the script analyses the ELF binaries in
the build path to generate a table (per memory type), showing memory allocation
and usage. This is the default output generated by the tool.

.. code:: shell

    $ poetry run memory -f
    build-path: build/fvp/release
    +----------------------------------------------------------------------------+
    |                         Memory Usage (bytes) [RAM]                         |
    +-----------+------------+------------+------------+------------+------------+
    | Component |   Start    |   Limit    |    Size    |    Free    |   Total    |
    +-----------+------------+------------+------------+------------+------------+
    |    BL1    |    4034000 |    4040000 |       7000 |       5000 |       c000 |
    |    BL2    |    4021000 |    4034000 |       d000 |       6000 |      13000 |
    |    BL2U   |    4021000 |    4034000 |       a000 |       9000 |      13000 |
    |    BL31   |    4003000 |    4040000 |      1e000 |      1f000 |      3d000 |
    +-----------+------------+------------+------------+------------+------------+

    +----------------------------------------------------------------------------+
    |                         Memory Usage (bytes) [ROM]                         |
    +-----------+------------+------------+------------+------------+------------+
    | Component |   Start    |   Limit    |    Size    |    Free    |   Total    |
    +-----------+------------+------------+------------+------------+------------+
    |    BL1    |          0 |    4000000 |       5df0 |    3ffa210 |    4000000 |
    +-----------+------------+------------+------------+------------+------------+

The script relies on symbols in the symbol table to determine the start, end,
and limit addresses of each bootloader stage.

Memory Tree
~~~~~~~~~~~

A hierarchical view of the memory layout can be produced by passing the option
``-t`` or ``--tree`` to the tool. This gives the start, end, and size of each
module, their ELF segments as well as sections.

.. code:: shell

    $ poetry run memory -t
    build-path: build/fvp/release
    name                                       start        end       size
    bl1                                            0    400c000    400c000
    ├── 00                                         0       5de0       5de0
    │   ├── .text                                  0       5000       5000
    │   └── .rodata                             5000       5de0        de0
    ├── 01                                   4034000    40344c5        4c5
    │   └── .data                            4034000    40344c5        4c5
    ├── 02                                   4034500    4034a00        500
    │   └── .stacks                          4034500    4034a00        500
    ├── 04                                   4034a00    4035600        c00
    │   └── .bss                             4034a00    4035600        c00
    └── 03                                   4036000    403b000       5000
        └── .xlat_table                      4036000    403b000       5000
    bl2                                      4021000    4034000      13000
    ├── 00                                   4021000    4027000       6000
    │   ├── .text                            4021000    4026000       5000
    │   └── .rodata                          4026000    4027000       1000
    └── 01                                   4027000    402e000       7000
        ├── .data                            4027000    4027809        809
        ├── .stacks                          4027840    4027e40        600
        ├── .bss                             4028000    4028800        800
        └── .xlat_table                      4029000    402e000       5000
    bl2u                                     4021000    4034000      13000
    ├── 00                                   4021000    4025000       4000
    │   ├── .text                            4021000    4024000       3000
    │   └── .rodata                          4024000    4025000       1000
    └── 01                                   4025000    402b000       6000
        ├── .data                            4025000    4025065         65
        ├── .stacks                          4025080    4025480        400
        ├── .bss                             4025600    4025c00        600
        └── .xlat_table                      4026000    402b000       5000
    bl31                                     4003000    4040000      3d000
    ├── 02                                  ffe00000   ffe03000       3000
    │   └── .el3_tzc_dram                   ffe00000   ffe03000       3000
    ├── 00                                   4003000    4010000       d000
    │   └── .text                            4003000    4010000       d000
    └── 01                                   4010000    4021000      11000
        ├── .rodata                          4010000    4012000       2000
        ├── .data                            4012000    401219d        19d
        ├── .stacks                          40121c0    40161c0       4000
        ├── .bss                             4016200    4018c00       2a00
        ├── .xlat_table                      4019000    4020000       7000
        └── .coherent_ram                    4020000    4021000       1000


The granularity of this view can be modified with the ``--depth`` option. For
instance, if you only require the tree up to the level showing segment data,
you can specify the depth with:

.. code::

    $ poetry run memory -t --depth 2
    build-path: build/fvp/release
    name                          start        end       size
    bl1                               0    400c000    400c000
    ├── 00                            0       5df0       5df0
    ├── 01                      4034000    40344c5        4c5
    ├── 02                      4034500    4034a00        500
    ├── 04                      4034a00    4035600        c00
    └── 03                      4036000    403b000       5000
    bl2                         4021000    4034000      13000
    ├── 00                      4021000    4027000       6000
    └── 01                      4027000    402e000       7000
    bl2u                        4021000    4034000      13000
    ├── 00                      4021000    4025000       4000
    └── 01                      4025000    402b000       6000
    bl31                        4003000    4040000      3d000
    ├── 02                     ffe00000   ffe03000       3000
    ├── 00                      4003000    4010000       d000
    └── 01                      4010000    4021000      11000

--------------

*Copyright (c) 2023, Arm Limited. All rights reserved.*

.. _Poetry: https://python-poetry.org/docs/
