BaseV2 Platform
===============

Some of the features of the BaseV2 platform referenced in TF-A include:

- A System Control Processor running `SCP-firmware`_ for power and system
  management.
- A Runtime Security Engine (RSE).
- GICv4.
- Trusted Board Boot.
- SCMI.
- MHUv3.

The BaseV2 TF-A currently supports only the ``AEMvA`` CPU.

Build Procedure (TF-A only)
---------------------------

-  Obtain `Arm toolchain`_ and set the CROSS_COMPILE environment variable to
   point to the toolchain path.

      export CROSS_COMPILE=<path_to_toolchain>/bin/aarch64-none-elf-

-  Build TF-A:

   .. code:: shell

      make PLAT=basev2 \
      ARM_GPT_SUPPORT=1 \
      BL33=<path_to_uboot.bin> \
      CSS_LOAD_SCP_IMAGES=0 \
      BASE_TARGET_DISTRO=buildroot \
      all fip

   Enable Trusted Board Boot by adding the following options to the make
   command:

   .. code:: shell

      TRUSTED_BOARD_BOOT=1 \
      GENERATE_COT=1 \
      ARM_ROTPK_LOCATION=devel_ecdsa \
      ROT_KEY=plat/arm/board/common/rotpk/arm_rotprivk_ecdsa.pem \
      KEY_ALG=ecdsa

--------------

*Copyright (c) 2026, Arm Limited. All rights reserved.*

.. _SCP-firmware: https://github.com/ARM-software/SCP-firmware
.. _Arm toolchain: https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads
