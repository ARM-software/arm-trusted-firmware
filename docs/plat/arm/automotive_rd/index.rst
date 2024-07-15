RD-1 AE (Kronos) Platform
=========================

Some of the features of the RD-1 AE platform referenced in TF-A include:

- Neoverse-V3AE, Arm9.2-A application processor (64-bit mode)
- A GICv4-compatible GIC-720AE

Further information on RD1-AE is available at `rd1ae`_

Boot Sequence
-------------

The boot process starts from RSE (Runtime Security Engine) that loads the
Application Processor (AP) BL2 image and signals the System Control Processor (SCP)
to power up the AP. The AP then runs AP BL2

The primary compute boot flow follows the following steps:

1. AP BL2:

   * Performs the actions described in the `Trusted Board Boot (TBB)`_ document.
   * Copies the AP BL31 image from Secure Flash to Secure SRAM
   * Copies the AP BL32 (OP-TEE) image from Secure Flash to Secure DRAM
   * Copies the AP BL33 (U-Boot) image from Secure Flash to Normal DRAM
   * Transfers the execution to AP BL31

2. AP BL31:

   * Initializes Trusted Firmware-A Services
   * Transfers the execution to AP BL32 and then transfers the execution to AP BL33
   * During runtime, acts as the Secure Monitor, handling SMC calls,
     and context switching between secure and non-secure worlds.

3. AP BL32:

   * Initializes OP-TEE environment
   * Initializes Secure Partitions
   * Transfers the execution back to AP BL31
   * During runtime, it facilitates secure communication between the
     normal world environment (e.g. Linux) and the Trusted Execution Environment.

Build Procedure (TF-A only)
~~~~~~~~~~~~~~~~~~~~~~~~~~~

-  Obtain `Arm toolchain`_ and set the CROSS_COMPILE environment variable to
   point to the toolchain folder.

-  Build TF-A:

   .. code:: shell

      make \
      PLAT=rd1ae \
      MBEDTLS_DIR=<mbedtls_dir> \
      ARCH=aarch64 \
      CREATE_KEYS=1 \
      GENERATE_COT=1 \
      TRUSTED_BOARD_BOOT=1 \
      COT=tbbr \
      ARM_ROTPK_LOCATION=devel_rsa \
      ROT_KEY=plat/arm/board/common/rotpk/arm_rotprivk_rsa.pem \
      BL32=<path to optee binary> \
      SPD=spmd \
      SPMD_SPM_AT_SEL2=0 \
      BL33=<path to u-boot binary> \

*Copyright (c) 2024, Arm Limited. All rights reserved.*

.. _Arm Toolchain: https://developer.arm.com/tools-and-software/open-source-software/developer-tools/gnu-toolchain/downloads
.. _rd1ae: https://developer.arm.com/Tools%20and%20Software/Arm%20Reference%20Design-1%20AE
.. _Trusted Board Boot (TBB): https://trustedfirmware-a.readthedocs.io/en/latest/design/trusted-board-boot.html
