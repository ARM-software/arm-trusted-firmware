Diphda Platform
==========================

Some of the features of the Diphda platform referenced in TF-A include:

- Cortex-A35 application processor (64-bit mode)
- Secure Enclave
- GIC-400
- Trusted Board Boot

Boot Sequence
-------------

The board boot relies on CoT (chain of trust). The trusted-firmware-a
BL2 is extracted from the FIP and verified by the Secure Enclave
processor. BL2 verification relies on the signature area at the
beginning of the BL2 image. This area is needed by the SecureEnclave
bootloader.

Then, the application processor is released from reset and starts by
executing BL2.

BL2 performs the actions described in the trusted-firmware-a TBB design
document.

Build Procedure (TF-A only)
~~~~~~~~~~~~~~~~~~~~~~~~~~~

-  Obtain AArch64 ELF bare-metal target `toolchain <https://developer.arm.com/tools-and-software/open-source-software/developer-tools/gnu-toolchain/gnu-a/downloads>`_.
   Set the CROSS_COMPILE environment variable to point to the toolchain folder.

-  Build TF-A:

   .. code:: shell

      make LD=aarch64-none-elf-ld \
      CC=aarch64-none-elf-gcc \
      V=1 \
      BUILD_BASE=<path to the build folder> \
      PLAT=diphda \
      SPD=spmd \
      SPMD_SPM_AT_SEL2=0 \
      DEBUG=1 \
      MBEDTLS_DIR=mbedtls \
      OPENSSL_DIR=<path to openssl usr folder> \
      RUNTIME_SYSROOT=<path to the sysroot> \
      ARCH=aarch64 \
      TARGET_PLATFORM=<fpga or fvp> \
      ENABLE_PIE=1 \
      BL2_AT_EL3=1 \
      CREATE_KEYS=1 \
      GENERATE_COT=1 \
      TRUSTED_BOARD_BOOT=1 \
      COT=tbbr \
      ARM_ROTPK_LOCATION=devel_rsa \
      ROT_KEY=plat/arm/board/common/rotpk/arm_rotprivk_rsa.pem \
      BL32=<path to optee binary> \
      BL33=<path to u-boot binary> \
      bl2

*Copyright (c) 2021, Arm Limited. All rights reserved.*
