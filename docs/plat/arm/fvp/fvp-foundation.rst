Running on the Foundation FVP
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The following ``Foundation_Platform`` parameters should be used to boot Linux with
4 CPUs using the AArch64 build of TF-A.

.. code:: shell

    <path-to>/Foundation_Platform                   \
    --cores=4                                       \
    --arm-v8.0                                      \
    --secure-memory                                 \
    --visualization                                 \
    --gicv3                                         \
    --data="<path-to>/<bl1-binary>"@0x0             \
    --data="<path-to>/<FIP-binary>"@0x08000000      \
    --data="<path-to>/<kernel-binary>"@0x80080000   \
    --data="<path-to>/<ramdisk-binary>"@0x84000000

Notes:

-  BL1 is loaded at the start of the Trusted ROM.
-  The Firmware Image Package is loaded at the start of NOR FLASH0.
-  The firmware loads the FDT packaged in FIP to the DRAM. The FDT load address
   is specified via the ``load-address`` property in the ``hw-config`` node of
   `FW_CONFIG for FVP`_.
-  The default use-case for the Foundation FVP is to use the ``--gicv3`` option
   and enable the GICv3 device in the model. Note that without this option,
   the Foundation FVP defaults to legacy (Versatile Express) memory map which
   is not supported by TF-A.
-  In order for TF-A to run correctly on the Foundation FVP, the architecture
   versions must match. The Foundation FVP defaults to the highest v8.x
   version it supports but the default build for TF-A is for v8.0. To avoid
   issues either start the Foundation FVP to use v8.0 architecture using the
   ``--arm-v8.0`` option, or build TF-A with an appropriate value for
   ``ARM_ARCH_MINOR``.

--------------

*Copyright (c) 2019-2024, Arm Limited. All rights reserved.*

.. _FW_CONFIG for FVP: https://git.trustedfirmware.org/TF-A/trusted-firmware-a.git/tree/plat/arm/board/fvp/fdts/fvp_fw_config.dts
