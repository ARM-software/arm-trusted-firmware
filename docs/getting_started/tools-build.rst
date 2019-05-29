Building Supporting Tools
=========================

Building and using the FIP tool
-------------------------------

Firmware Image Package (FIP) is a packaging format used by TF-A to package
firmware images in a single binary. The number and type of images that should
be packed in a FIP is platform specific and may include TF-A images and other
firmware images required by the platform. For example, most platforms require
a BL33 image which corresponds to the normal world bootloader (e.g. UEFI or
U-Boot).

The TF-A build system provides the make target ``fip`` to create a FIP file
for the specified platform using the FIP creation tool included in the TF-A
project. Examples below show how to build a FIP file for FVP, packaging TF-A
and BL33 images.

For AArch64:

.. code:: shell

    make PLAT=fvp BL33=<path-to>/bl33.bin fip

For AArch32:

.. code:: shell

    make PLAT=fvp ARCH=aarch32 AARCH32_SP=sp_min BL33=<path-to>/bl33.bin fip

The resulting FIP may be found in:

::

    build/fvp/<build-type>/fip.bin

For advanced operations on FIP files, it is also possible to independently build
the tool and create or modify FIPs using this tool. To do this, follow these
steps:

It is recommended to remove old artifacts before building the tool:

.. code:: shell

    make -C tools/fiptool clean

Build the tool:

.. code:: shell

    make [DEBUG=1] [V=1] fiptool

The tool binary can be located in:

::

    ./tools/fiptool/fiptool

Invoking the tool with ``help`` will print a help message with all available
options.

Example 1: create a new Firmware package ``fip.bin`` that contains BL2 and BL31:

.. code:: shell

    ./tools/fiptool/fiptool create \
        --tb-fw build/<platform>/<build-type>/bl2.bin \
        --soc-fw build/<platform>/<build-type>/bl31.bin \
        fip.bin

Example 2: view the contents of an existing Firmware package:

.. code:: shell

    ./tools/fiptool/fiptool info <path-to>/fip.bin

Example 3: update the entries of an existing Firmware package:

.. code:: shell

    # Change the BL2 from Debug to Release version
    ./tools/fiptool/fiptool update \
        --tb-fw build/<platform>/release/bl2.bin \
        build/<platform>/debug/fip.bin

Example 4: unpack all entries from an existing Firmware package:

.. code:: shell

    # Images will be unpacked to the working directory
    ./tools/fiptool/fiptool unpack <path-to>/fip.bin

Example 5: remove an entry from an existing Firmware package:

.. code:: shell

    ./tools/fiptool/fiptool remove \
        --tb-fw build/<platform>/debug/fip.bin

Note that if the destination FIP file exists, the create, update and
remove operations will automatically overwrite it.

The unpack operation will fail if the images already exist at the
destination. In that case, use -f or --force to continue.

More information about FIP can be found in the :ref:`Firmware Design` document.

.. _tools_build_cert_create:

Building the Certificate Generation Tool
----------------------------------------

The ``cert_create`` tool is built as part of the TF-A build process when the
``fip`` make target is specified and TBB is enabled (as described in the
previous section), but it can also be built separately with the following
command:

.. code:: shell

    make PLAT=<platform> [DEBUG=1] [V=1] certtool

For platforms that require their own IDs in certificate files, the generic
'cert_create' tool can be built with the following command. Note that the target
platform must define its IDs within a ``platform_oid.h`` header file for the
build to succeed.

.. code:: shell

    make PLAT=<platform> USE_TBBR_DEFS=0 [DEBUG=1] [V=1] certtool

``DEBUG=1`` builds the tool in debug mode. ``V=1`` makes the build process more
verbose. The following command should be used to obtain help about the tool:

.. code:: shell

    ./tools/cert_create/cert_create -h

--------------

*Copyright (c) 2019, Arm Limited. All rights reserved.*
