TC Total Compute Platform
==========================

Some of the features of TC platform referenced in TF-A include:

- A `System Control Processor <https://github.com/ARM-software/SCP-firmware>`_
  to abstract power and system management tasks away from application
  processors. The RAM firmware for SCP is included in the TF-A FIP and is
  loaded by AP BL2 from FIP in flash to SRAM for copying by SCP (SCP has access
  to AP SRAM).
- GICv4
- Trusted Board Boot
- SCMI
- MHUv2

The TF-A build is specified by the option `TARGET_PLATFORM` which represents
the Total Compute platform number. The platforms support the CPU variants
listed as below:

-  TC0 has support for Cortex A510, Cortex A710 and Cortex X2. (Note TC0 is now deprecated)
-  TC1 has support for Cortex A510, Cortex A715 and Cortex X3. (Note TC1 is now deprecated)
-  TC2 has support for Cortex A520, Cortex A720 and Cortex x4. (Note TC2 is now deprecated)
-  TC3 has support for Cortex A520, Cortex A725 and Cortex x925.


Boot Sequence
-------------

The execution begins from SCP_BL1. SCP_BL1 powers up the AP which starts
executing AP_BL1 and then executes AP_BL2 which loads the SCP_BL2 from
FIP to SRAM. The SCP has access to AP SRAM. The address and size of SCP_BL2
is communicated to SCP using SDS. SCP copies SCP_BL2 from SRAM to its own
RAM and starts executing it. The AP then continues executing the rest of TF-A
stages including BL31 runtime stage and hands off executing to
Non-secure world (u-boot).

Build Procedure (TF-A only)
~~~~~~~~~~~~~~~~~~~~~~~~~~~

-  Obtain `Arm toolchain`_ and set the CROSS_COMPILE environment variable to
   point to the toolchain folder.

-  Build TF-A:

   .. code:: shell

      make PLAT=tc BL33=<path_to_uboot.bin> \
      SCP_BL2=<path_to_scp_ramfw.bin> TARGET_PLATFORM={3} all fip

   Enable TBBR by adding the following options to the make command:

   .. code:: shell

      MBEDTLS_DIR=<path_to_mbedtls_directory>  \
      TRUSTED_BOARD_BOOT=1 \
      GENERATE_COT=1 \
      ARM_ROTPK_LOCATION=devel_rsa  \
      ROT_KEY=plat/arm/board/common/rotpk/arm_rotprivk_rsa.pem

--------------

*Copyright (c) 2020-2023, Arm Limited. All rights reserved.*

.. _Arm Toolchain: https://developer.arm.com/tools-and-software/open-source-software/developer-tools/gnu-toolchain/downloads
