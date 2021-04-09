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

Currently, the main difference between TC0 (TARGET_PLATFORM=0) and TC1
(TARGET_PLATFORM=1) platforms w.r.t to TF-A is the CPUs supported. TC0 has
support for Cortex A510, Cortex A710 and Cortex X2, while TC1 has support for
Cortex A510, Cortex Makalu and Cortex Makalu ELP Arm CPUs.


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

-  Obtain arm `toolchain <https://developer.arm.com/tools-and-software/open-source-software/developer-tools/gnu-toolchain/gnu-a/downloads>`_.
   Set the CROSS_COMPILE environment variable to point to the toolchain folder.

-  Build TF-A:

   .. code:: shell

      make PLAT=tc BL33=<path_to_uboot.bin> \
      SCP_BL2=<path_to_scp_ramfw.bin> TARGET_PLATFORM={0,1} all fip

   Enable TBBR by adding the following options to the make command:

   .. code:: shell

      MBEDTLS_DIR=<path_to_mbedtls_directory>  \
      TRUSTED_BOARD_BOOT=1 \
      GENERATE_COT=1 \
      ARM_ROTPK_LOCATION=devel_rsa  \
      ROT_KEY=plat/arm/board/common/rotpk/arm_rotprivk_rsa.pem

*Copyright (c) 2020-2021, Arm Limited. All rights reserved.*
