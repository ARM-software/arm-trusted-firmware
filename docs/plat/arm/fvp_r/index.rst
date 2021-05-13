ARM V8-R64 Fixed Virtual Platform (FVP)
=======================================

Some of the features of Armv8-R AArch64 FVP platform referenced in Trusted
Boot R-class include:

- Secure World Support Only
- EL2 as Maximum EL support (No EL3)
- MPU Support only at EL2
- MPU or MMU Support at EL0/EL1
- AArch64 Support Only
- Trusted Board Boot

Further information on v8-R64 FVP is available at `info <https://developer.arm.com/documentation/ddi0600/latest/>`_

Boot Sequence
-------------

BL1 –> BL33

The execution begins from BL1 which loads the BL33 image, a boot-wrapped (bootloader + Operating System)
Operating System, from FIP to DRAM.

Build Procedure
~~~~~~~~~~~~~~~

-  Obtain arm `toolchain <https://developer.arm.com/tools-and-software/open-source-software/developer-tools/gnu-toolchain/gnu-a/downloads>`_.
   Set the CROSS_COMPILE environment variable to point to the toolchain folder.

-  Build TF-A:

   .. code:: shell

      make PLAT=fvp_r BL33=<path_to_os.bin> all fip

   Enable TBBR by adding the following options to the make command:

   .. code:: shell

      MBEDTLS_DIR=<path_to_mbedtls_directory>  \
      TRUSTED_BOARD_BOOT=1 \
      GENERATE_COT=1 \
      ARM_ROTPK_LOCATION=devel_rsa  \
      ROT_KEY=plat/arm/board/common/rotpk/arm_rotprivk_rsa.pem

*Copyright (c) 2021, Arm Limited. All rights reserved.*
