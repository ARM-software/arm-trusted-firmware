Library at ROM
==============

This document provides an overview of the "library at ROM" implementation in
Trusted Firmware-A (TF-A).

Introduction
~~~~~~~~~~~~

The "library at ROM" feature allows platforms to build a library of functions to
be placed in ROM. This reduces SRAM usage by utilising the available space in
ROM. The "library at ROM" contains a jump table with the list of functions that
are placed in ROM. The capabilities of the "library at ROM" are:

1. Functions can be from one or several libraries.

2. Functions can be patched after they have been programmed into ROM.

3. Platform-specific libraries can be placed in ROM.

4. Functions can be accessed by one or more BL images.

Index file
~~~~~~~~~~

.. image:: ../resources/diagrams/romlib_design.png
    :width: 600

Library at ROM is described by an index file with the list of functions to be
placed in ROM. The index file is platform specific and its format is:

::

    lib function    [patch]

    lib      -- Name of the library the function belongs to
    function -- Name of the function to be placed in library at ROM
    [patch]  -- Option to patch the function

It is also possible to insert reserved spaces in the list by using the keyword
"reserved" rather than the "lib" and "function" names as shown below:

::

    reserved

The reserved spaces can be used to add more functions in the future without
affecting the order and location of functions already existing in the jump
table. Also, for additional flexibility and modularity, the index file can
include other index files.

For an index file example, refer to ``lib/romlib/jmptbl.i``.

Wrapper functions
~~~~~~~~~~~~~~~~~

.. image:: ../resources/diagrams/romlib_wrapper.png
    :width: 600

When invoking a function of the "library at ROM", the calling sequence is as
follows:

BL image --> wrapper function --> jump table entry --> library at ROM

The index file is used to create a jump table which is placed in ROM. Then, the
wrappers refer to the jump table to call the "library at ROM" functions. The
wrappers essentially contain a branch instruction to the jump table entry
corresponding to the original function. Finally, the original function in the BL
image(s) is replaced with the wrapper function.

The "library at ROM" contains a necessary init function that initialises the
global variables defined by the functions inside "library at ROM".

Script
~~~~~~

There is a ``romlib_generate.py`` Python script that generates the necessary
files for the "library at ROM" to work. It implements multiple functions:

1. ``romlib_generate.py gentbl [args]`` - Generates the jump table by parsing
   the index file.

2. ``romlib_generator.py genvar [args]`` - Generates the jump table global
   variable (**not** the jump table itself) with the absolute address in ROM.
   This global variable is, basically, a pointer to the jump table.

3. ``romlib_generator.py genwrappers [args]`` - Generates a wrapper function for
   each entry in the index file except for the ones that contain the keyword
   ``patch``. The generated wrapper file is called ``<fn_name>.s``.

4. ``romlib_generator.py pre [args]`` - Preprocesses the index file which means
   it resolves all the include commands in the file recursively. It can also
   generate a dependency file of the included index files which can be directly
   used in makefiles.

Each ``romlib_generate.py`` function has its own manual which is accessible by
runing ``romlib_generator.py [function] --help``.

``romlib_generate.py`` requires Python 3 environment.


Patching of functions in library at ROM
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The ``romlib_generator.py genwrappers`` does not generate wrappers for the
entries in the index file that contain the keyword ``patch``. Thus, it allows
calling the function from the actual library by breaking the link to the
"library at ROM" version of this function.

The calling sequence for a patched function is as follows:

BL image --> function

Memory impact
~~~~~~~~~~~~~

Using library at ROM will modify the memory layout of the BL images:

- The ROM library needs a page aligned RAM section to hold the RW data. This
  section is defined by the ROMLIB_RW_BASE and ROMLIB_RW_END macros.
  On Arm platforms a section of 1 page (0x1000) is allocated at the top of SRAM.
  This will have for effect to shift down all the BL images by 1 page.

- Depending on the functions moved to the ROM library, the size of the BL images
  will be reduced.
  For example: moving MbedTLS function into the ROM library reduces BL1 and
  BL2, but not BL31.

- This change in BL images size can be taken into consideration to optimize the
  memory layout when defining the BLx_BASE macros.

Build library at ROM
~~~~~~~~~~~~~~~~~~~~~

The environment variable ``CROSS_COMPILE`` must be set appropriately. Refer to
:ref:`Performing an Initial Build` for more information about setting this
variable.

In the below example the usage of ROMLIB together with mbed TLS is demonstrated
to showcase the benefits of library at ROM - it's not mandatory.

.. code:: shell

    make PLAT=fvp                                                   \
    MBEDTLS_DIR=</path/to/mbedtls/>                                 \
    TRUSTED_BOARD_BOOT=1 GENERATE_COT=1                             \
    ARM_ROTPK_LOCATION=devel_rsa                                    \
    ROT_KEY=plat/arm/board/common/rotpk/arm_rotprivk_rsa.pem        \
    BL33=</path/to/bl33.bin>                                        \
    USE_ROMLIB=1                                                    \
    all fip

--------------

*Copyright (c) 2019, Arm Limited. All rights reserved.*
