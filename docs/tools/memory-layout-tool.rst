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

        poetry install --no-root

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

    $ poetry run memory symbols
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

The tool enables users to view static memory consumption. When the ``footprint``
command is provided, the script analyses the ELF binaries in the build path to
generate a table (per memory type), showing memory allocation and usage. This is
the default output generated by the tool.

.. code:: shell

    $ poetry run memory footprint
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

A hierarchical view of the memory layout can be produced by passing the ``tree``
command to the tool. This gives the start, end, and size of each module, their
ELF segments as well as sections.

.. code:: shell

    $ poetry run memory tree
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

    $ poetry run memory tree --depth 2
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

Memory Summary
~~~~~~~~~~~~~~

The tool provides a by-translation-unit summary of the sizes (``text``, ``bss``,
``data``) contributed by each translation unit or group of translation units.
For example, to print a table of an FVP build, with a path depth of 3:

.. code::

   $ poetry run memory summary build/fvp/debug/bl1/bl1.map  -d 3
    | Module                                 |         .text |       .data |          .bss |
    |----------------------------------------|---------------|-------------|---------------|
    | [fill]                                 |   3204(+3204) |       0(+0) |       97(+97) |
    | bl1/aem_generic.o                      |         0(+0) |       0(+0) |         0(+0) |
    | bl1/arm_bl1_fwu.o                      |     224(+224) |     80(+80) |         0(+0) |
    | bl1/arm_bl1_setup.o                    |     608(+608) |       0(+0) |       17(+17) |
    | bl1/arm_common.o                       |     116(+116) |       0(+0) |         0(+0) |
    | bl1/arm_console.o                      |     116(+116) |       0(+0) |       40(+40) |
    | bl1/arm_dev_rotpk.o                    |         0(+0) |       0(+0) |         0(+0) |
    | bl1/arm_dyn_cfg.o                      |     276(+276) |       0(+0) |   7184(+7184) |
    | bl1/arm_dyn_cfg_helpers.o              |     364(+364) |       0(+0) |         0(+0) |
    | bl1/arm_err.o                          |       12(+12) |       0(+0) |         0(+0) |
    | bl1/arm_fconf_io.o                     |         0(+0) |   952(+952) |         0(+0) |
    | bl1/arm_helpers.o                      |       44(+44) |       0(+0) |         0(+0) |
    | bl1/arm_io_storage.o                   |     480(+480) |       0(+0) |       32(+32) |
    | bl1/auth_mod.o                         |   1288(+1288) |       0(+0) |         0(+0) |
    | bl1/backtrace.o                        |     444(+444) |       0(+0) |         0(+0) |
    | bl1/bl1_arch_setup.o                   |       16(+16) |       0(+0) |         0(+0) |
    | bl1/bl1_context_mgmt.o                 |     340(+340) |       0(+0) |   1392(+1392) |
    | bl1/bl1_entrypoint.o                   |     236(+236) |       0(+0) |         0(+0) |
    | bl1/bl1_exceptions.o                   |   2240(+2240) |       0(+0) |         0(+0) |
    | bl1/bl1_fwu.o                          |   2188(+2188) |     44(+44) |         0(+0) |
    | bl1/bl1_main.o                         |     620(+620) |       0(+0) |         0(+0) |
    | bl1/bl_common.o                        |     772(+772) |       0(+0) |         4(+4) |
    | bl1/board_arm_helpers.o                |       44(+44) |       0(+0) |         0(+0) |
    | bl1/board_arm_trusted_boot.o           |       44(+44) |     16(+16) |         0(+0) |
    | bl1/cache_helpers.o                    |     112(+112) |       0(+0) |         0(+0) |
    | bl1/cci.o                              |     408(+408) |       0(+0) |       24(+24) |
    | bl1/context.o                          |     348(+348) |       0(+0) |         0(+0) |
    | bl1/context_mgmt.o                     |   1692(+1692) |       0(+0) |       48(+48) |
    | bl1/cortex_a35.o                       |       96(+96) |       0(+0) |         0(+0) |
    | bl1/cortex_a53.o                       |     248(+248) |       0(+0) |         0(+0) |
    | bl1/cortex_a57.o                       |     384(+384) |       0(+0) |         0(+0) |
    | bl1/cortex_a72.o                       |     356(+356) |       0(+0) |         0(+0) |
    | bl1/cortex_a73.o                       |     304(+304) |       0(+0) |         0(+0) |
    | bl1/cpu_helpers.o                      |     200(+200) |       0(+0) |         0(+0) |
    | bl1/crypto_mod.o                       |     380(+380) |       0(+0) |         0(+0) |
    | bl1/debug.o                            |     224(+224) |       0(+0) |         0(+0) |
    | bl1/delay_timer.o                      |       64(+64) |       0(+0) |         8(+8) |
    | bl1/enable_mmu.o                       |     112(+112) |       0(+0) |         0(+0) |
    | bl1/errata_report.o                    |     564(+564) |       0(+0) |         0(+0) |
    | bl1/fconf.o                            |     148(+148) |       0(+0) |         0(+0) |
    | bl1/fconf_dyn_cfg_getter.o             |     656(+656) |     32(+32) |     144(+144) |
    | bl1/fconf_tbbr_getter.o                |     332(+332) |       0(+0) |       24(+24) |
    | bl1/fdt_wrappers.o                     |     452(+452) |       0(+0) |         0(+0) |
    | bl1/fvp_bl1_setup.o                    |     168(+168) |       0(+0) |         0(+0) |
    | bl1/fvp_common.o                       |     512(+512) |       0(+0) |         8(+8) |
    | bl1/fvp_cpu_pwr.o                      |     136(+136) |       0(+0) |         0(+0) |
    | bl1/fvp_err.o                          |       44(+44) |       0(+0) |         0(+0) |
    | bl1/fvp_helpers.o                      |     148(+148) |       0(+0) |         0(+0) |
    | bl1/fvp_io_storage.o                   |     228(+228) |       0(+0) |       16(+16) |
    | bl1/fvp_trusted_boot.o                 |     292(+292) |       0(+0) |         0(+0) |
    | bl1/generic_delay_timer.o              |     136(+136) |       0(+0) |       16(+16) |
    | bl1/img_parser_mod.o                   |     588(+588) |       0(+0) |       20(+20) |
    | bl1/io_fip.o                           |   1332(+1332) |       0(+0) |     100(+100) |
    | bl1/io_memmap.o                        |     736(+736) |     16(+16) |       32(+32) |
    | bl1/io_semihosting.o                   |     648(+648) |     16(+16) |         0(+0) |
    | bl1/io_storage.o                       |   1268(+1268) |       0(+0) |     104(+104) |
    | bl1/mbedtls_common.o                   |     208(+208) |       0(+0) |         4(+4) |
    | bl1/mbedtls_crypto.o                   |     636(+636) |       0(+0) |         0(+0) |
    | bl1/mbedtls_x509_parser.o              |   1588(+1588) |       0(+0) |     120(+120) |
    | bl1/misc_helpers.o                     |     392(+392) |       0(+0) |         0(+0) |
    | bl1/multi_console.o                    |     528(+528) |       1(+1) |         8(+8) |
    | bl1/pl011_console.o                    |     308(+308) |       0(+0) |         0(+0) |
    | bl1/plat_bl1_common.o                  |     208(+208) |       0(+0) |         0(+0) |
    | bl1/plat_bl_common.o                   |       40(+40) |       0(+0) |         0(+0) |
    | bl1/plat_common.o                      |       48(+48) |       0(+0) |         8(+8) |
    | bl1/plat_log_common.o                  |       48(+48) |       0(+0) |         0(+0) |
    | bl1/plat_tbbr.o                        |     128(+128) |       0(+0) |         0(+0) |
    | bl1/platform_helpers.o                 |       12(+12) |       0(+0) |         0(+0) |
    | bl1/platform_up_stack.o                |       16(+16) |       0(+0) |         0(+0) |
    | bl1/semihosting.o                      |     352(+352) |       0(+0) |         0(+0) |
    | bl1/semihosting_call.o                 |         8(+8) |       0(+0) |         0(+0) |
    | bl1/smmu_v3.o                          |     296(+296) |       0(+0) |         0(+0) |
    | bl1/sp805.o                            |       64(+64) |       0(+0) |         0(+0) |
    | bl1/tbbr_cot_bl1.o                     |         0(+0) |     48(+48) |     156(+156) |
    | bl1/tbbr_cot_common.o                  |         0(+0) |   144(+144) |     306(+306) |
    | bl1/tbbr_img_desc.o                    |         0(+0) |   768(+768) |         0(+0) |
    | bl1/tf_log.o                           |     200(+200) |       4(+4) |         0(+0) |
    | bl1/xlat_tables_arch.o                 |     736(+736) |       0(+0) |         0(+0) |
    | bl1/xlat_tables_context.o              |     192(+192) |     96(+96) |   1296(+1296) |
    | bl1/xlat_tables_core.o                 |   2112(+2112) |       0(+0) |         0(+0) |
    | bl1/xlat_tables_utils.o                |         8(+8) |       0(+0) |         0(+0) |
    | lib/libc.a/assert.o                    |       48(+48) |       0(+0) |         0(+0) |
    | lib/libc.a/exit.o                      |       64(+64) |       0(+0) |         8(+8) |
    | lib/libc.a/memchr.o                    |       44(+44) |       0(+0) |         0(+0) |
    | lib/libc.a/memcmp.o                    |       52(+52) |       0(+0) |         0(+0) |
    | lib/libc.a/memcpy.o                    |       32(+32) |       0(+0) |         0(+0) |
    | lib/libc.a/memmove.o                   |       52(+52) |       0(+0) |         0(+0) |
    | lib/libc.a/memset.o                    |     140(+140) |       0(+0) |         0(+0) |
    | lib/libc.a/printf.o                    |   1532(+1532) |       0(+0) |         0(+0) |
    | lib/libc.a/snprintf.o                  |   1748(+1748) |       0(+0) |         0(+0) |
    | lib/libc.a/strcmp.o                    |       44(+44) |       0(+0) |         0(+0) |
    | lib/libc.a/strlen.o                    |       28(+28) |       0(+0) |         0(+0) |
    | lib/libfdt.a/fdt.o                     |   1460(+1460) |       0(+0) |         0(+0) |
    | lib/libfdt.a/fdt_ro.o                  |   1392(+1392) |       0(+0) |         0(+0) |
    | lib/libfdt.a/fdt_wip.o                 |     244(+244) |       0(+0) |         0(+0) |
    | lib/libmbedtls.a/asn1parse.o           |     956(+956) |       0(+0) |         0(+0) |
    | lib/libmbedtls.a/bignum.o              |   6796(+6796) |       0(+0) |         0(+0) |
    | lib/libmbedtls.a/bignum_core.o         |   3252(+3252) |       0(+0) |         0(+0) |
    | lib/libmbedtls.a/constant_time.o       |     280(+280) |       0(+0) |         8(+8) |
    | lib/libmbedtls.a/md.o                  |     504(+504) |       0(+0) |         0(+0) |
    | lib/libmbedtls.a/memory_buffer_alloc.o |   1264(+1264) |       0(+0) |       40(+40) |
    | lib/libmbedtls.a/oid.o                 |     752(+752) |       0(+0) |         0(+0) |
    | lib/libmbedtls.a/pk.o                  |     872(+872) |       0(+0) |         0(+0) |
    | lib/libmbedtls.a/pk_wrap.o             |     848(+848) |       0(+0) |         0(+0) |
    | lib/libmbedtls.a/pkparse.o             |     516(+516) |       0(+0) |         0(+0) |
    | lib/libmbedtls.a/platform.o            |       92(+92) |     24(+24) |         0(+0) |
    | lib/libmbedtls.a/platform_util.o       |       96(+96) |       0(+0) |         0(+0) |
    | lib/libmbedtls.a/rsa.o                 |   6588(+6588) |       0(+0) |         0(+0) |
    | lib/libmbedtls.a/rsa_alt_helpers.o     |   2340(+2340) |       0(+0) |         0(+0) |
    | lib/libmbedtls.a/sha256.o              |   1448(+1448) |       0(+0) |         0(+0) |
    | lib/libmbedtls.a/x509.o                |   1028(+1028) |       0(+0) |         0(+0) |
    | Subtotals                              | 69632(+69632) | 2241(+2241) | 11264(+11264) |
    Total Static RAM memory (data + bss): 13505(+13505) bytes
    Total Flash memory (text + data): 71873(+71873) bytes

A delta between two images can be generated by passing the ``--old`` option with
a path to the previous map file.

For example:

.. code::

    $ poetry run memory summary ../maps/fvp-tbb-mbedtls/bl1.map --old ../maps/fvp-tbb-mbedtls/bl1.map.old -d 1
    | Module    |         .text |    .data |         .bss |
    |-----------|---------------|----------|--------------|
    | [fill]    |    780(-2424) |    0(+0) |    321(+224) |
    | bl1       |   32024(+108) | 2217(+0) |    11111(+0) |
    | lib       | 45020(+10508) |   24(+0) |  1880(+1824) |
    | Subtotals |  77824(+8192) | 2241(+0) | 13312(+2048) |
    Total Static RAM memory (data + bss): 15553(+2048) bytes
    Total Flash memory (text + data): 80065(+8192) bytes

Note that since the old map file includes the required suffix, specifying the
``--old`` argument is optional here.

Under some circumstances, some executables are padded to meet certain
alignments, such as a 4KB page boundary, and excluding that padding can provide
more helpful diffs. Taking the last example, and adding the ``-e`` argument
yields such a summary:

.. code::

    $ poetry run memory summary ../maps/fvp-tbb-mbedtls/bl1.map --old ../maps/fvp-tbb-mbedtls/bl1.map.old  -d 1 -e
    | Module    |         .text |    .data |         .bss |
    |-----------|---------------|----------|--------------|
    | bl1       |   32024(+108) | 2217(+0) |    11111(+0) |
    | lib       | 45020(+10508) |   24(+0) |  1880(+1824) |
    | Subtotals | 77044(+10616) | 2241(+0) | 12991(+1824) |
    Total Static RAM memory (data + bss): 15232(+1824) bytes
    Total Flash memory (text + data): 79285(+10616) bytes

--------------

*Copyright (c) 2023-2025, Arm Limited. All rights reserved.*

.. _Poetry: https://python-poetry.org/docs/
