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

--------------

*Copyright (c) 2023, Arm Limited. All rights reserved.*

.. _Poetry: https://python-poetry.org/docs/
