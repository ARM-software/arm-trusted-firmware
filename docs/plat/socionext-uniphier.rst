ARM Trusted Firmware for Socionext UniPhier SoCs
================================================


Socionext UniPhier ARMv8-A SoCs use ARM Trusted Firmware as the secure world
firmware, supporting BL1, BL2, and BL31.

UniPhier SoC family implements its internal boot ROM, so BL1 is used as pseudo
ROM (i.e. runs in RAM). The internal boot ROM loads 64KB [1]_ image from a
non-volatile storage to the on-chip SRAM. Unfortunately, BL1 does not fit in
the 64KB limit if `Trusted Board Boot`_ (TBB) is enabled. To solve this problem,
Socionext provides a first stage loader called `UniPhier BL`_. This loader runs
in the on-chip SRAM, initializes the DRAM, expands BL1 there, and hands the
control over to it. Therefore, all images of ARM Trusted Firmware run in DRAM.

The UniPhier platform works with/without TBB. See below for the build process
of each case. The image authentication for the UniPhier platform fully
complies with the Trusted Board Boot Requirements (TBBR) specification.

The UniPhier BL does not implement the authentication functionality, that is,
it can not verify the BL1 image by itself. Instead, the UniPhier BL assures
the BL1 validity in a different way; BL1 is GZIP-compressed and appended to
the UniPhier BL. The concatenation of the UniPhier BL and the compressed BL1
fits in the 64KB limit. The concatenated image is loaded by the boot ROM
(and verified if the chip fuses are blown).


Boot Flow
---------

1. The Boot ROM

   This is hard-wired ROM, so never corrupted. It loads the UniPhier BL (with
   compressed-BL1 appended) into the on-chip SRAM. If the SoC fuses are blown,
   the image is verified by the SoC's own method.

2. UniPhier BL

   This runs in the on-chip SRAM. After the minimum SoC initialization and DRAM
   setup, it decompresses the appended BL1 image into the DRAM, then jumps to
   the BL1 entry.

3. BL1

   This runs in the DRAM. It extracts BL2 from FIP (Firmware Image Package).
   If TBB is enabled, the BL2 is authenticated by the standard mechanism of ARM
   Trusted Firmware.

4. BL2, BL31, and more

   They all run in the DRAM, and are authenticated by the standard mechanism if
   TBB is enabled. See `Firmware Design`_ for details.


Basic Build
-----------

BL1 must be compressed for the reason above. The UniPhier's platform makefile
provides a build target ``bl1_gzip`` for this.

For a non-secure boot loader (aka BL33), U-Boot is well supported for UniPhier
SoCs. The U-Boot image (``u-boot.bin``) must be built in advance. For the build
procedure of U-Boot, refer to the document in the `U-Boot`_ project.

To build minimum functionality for UniPhier (without TBB)::

    make CROSS_COMPILE=<gcc-prefix> PLAT=uniphier BL33=<path-to-BL33> bl1_gzip fip

Output images:

- ``bl1.bin.gzip``
- ``fip.bin``


Optional features
-----------------

- Trusted Board Boot

  `mbed TLS`_ is needed as the cryptographic and image parser modules.
  Refer to the `User Guide`_ for the appropriate version of mbed TLS.

  To enable TBB, add the following options to the build command::

      TRUSTED_BOARD_BOOT=1 GENERATE_COT=1 MBEDTLS_DIR=<path-to-mbedtls>

- System Control Processor (SCP)

  If desired, FIP can include an SCP BL2 image. If BL2 finds an SCP BL2 image
  in FIP, BL2 loads it into DRAM and kicks the SCP. Most of UniPhier boards
  still work without SCP, but SCP provides better power management support.

  To include SCP BL2, add the following option to the build command::

      SCP_BL2=<path-to-SCP>

- BL32 (Secure Payload)

  To enable BL32, add the following options to the build command::

      SPD=<spd> BL32=<path-to-BL32>

  If you use TSP for BL32, ``BL32=<path-to-BL32>`` is not required. Just add the
  following::

      SPD=tspd


.. [1] Some SoCs can load 80KB, but the software implementation must be aligned
   to the lowest common denominator.
.. _Trusted Board Boot: ../trusted-board-boot.rst
.. _UniPhier BL: https://github.com/uniphier/uniphier-bl
.. _Firmware Design: ../firmware-design.rst
.. _U-Boot: https://www.denx.de/wiki/U-Boot
.. _mbed TLS: https://tls.mbed.org/
.. _User Guide: ../user-guide.rst
