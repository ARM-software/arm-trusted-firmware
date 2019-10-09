Socionext UniPhier
==================

Socionext UniPhier Armv8-A SoCs use Trusted Firmware-A (TF-A) as the secure
world firmware, supporting BL2 and BL31.

UniPhier SoC family implements its internal boot ROM, which loads 64KB [1]_
image from a non-volatile storage to the on-chip SRAM, and jumps over to it.
TF-A provides a special mode, BL2-AT-EL3, which enables BL2 to execute at EL3.
It is useful for platforms with non-TF-A boot ROM, like UniPhier. Here, a
problem is BL2 does not fit in the 64KB limit if
:ref:`Trusted Board Boot (TBB) <Trusted Board Boot>` is enabled.
To solve this issue, Socionext provides a first stage loader called
`UniPhier BL`_. This loader runs in the on-chip SRAM, initializes the DRAM,
expands BL2 there, and hands the control over to it. Therefore, all images
of TF-A run in DRAM.

The UniPhier platform works with/without TBB. See below for the build process
of each case. The image authentication for the UniPhier platform fully
complies with the Trusted Board Boot Requirements (TBBR) specification.

The UniPhier BL does not implement the authentication functionality, that is,
it can not verify the BL2 image by itself. Instead, the UniPhier BL assures
the BL2 validity in a different way; BL2 is GZIP-compressed and appended to
the UniPhier BL. The concatenation of the UniPhier BL and the compressed BL2
fits in the 64KB limit. The concatenated image is loaded by the internal boot
ROM (and verified if the chip fuses are blown).


Boot Flow
---------

1. The Boot ROM

   This is hard-wired ROM, so never corrupted. It loads the UniPhier BL (with
   compressed-BL2 appended) into the on-chip SRAM. If the SoC fuses are blown,
   the image is verified by the SoC's own method.

2. UniPhier BL

   This runs in the on-chip SRAM. After the minimum SoC initialization and DRAM
   setup, it decompresses the appended BL2 image into the DRAM, then jumps to
   the BL2 entry.

3. BL2 (at EL3)

   This runs in the DRAM. It extracts more images such as BL31, BL33 (optionally
   SCP_BL2, BL32 as well) from Firmware Image Package (FIP). If TBB is enabled,
   they are all authenticated by the standard mechanism of TF-A.
   After loading all the images, it jumps to the BL31 entry.

4. BL31, BL32, and BL33

   They all run in the DRAM. See :ref:`Firmware Design` for details.


Basic Build
-----------

BL2 must be compressed for the reason above. The UniPhier's platform makefile
provides a build target ``bl2_gzip`` for this.

For a non-secure boot loader (aka BL33), U-Boot is well supported for UniPhier
SoCs. The U-Boot image (``u-boot.bin``) must be built in advance. For the build
procedure of U-Boot, refer to the document in the `U-Boot`_ project.

To build minimum functionality for UniPhier (without TBB)::

    make CROSS_COMPILE=<gcc-prefix> PLAT=uniphier BL33=<path-to-BL33> bl2_gzip fip

Output images:

- ``bl2.bin.gz``
- ``fip.bin``


Optional features
-----------------

- Trusted Board Boot

  `mbed TLS`_ is needed as the cryptographic and image parser modules.
  Refer to the :ref:`User Guide` for the appropriate version of mbed TLS.

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
.. _UniPhier BL: https://github.com/uniphier/uniphier-bl
.. _U-Boot: https://www.denx.de/wiki/U-Boot
.. _mbed TLS: https://tls.mbed.org/
