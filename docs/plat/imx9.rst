NXP i.MX 9 Series
==================

Building on the market-proven i.MX 6 and i.MX 8 series, i.MX 9 series applications
processors bring together higher performance applications cores, an independent
MCU-like real-time domain, Energy Flex architecture, state-of-the-art security
with EdgeLock® secure enclave and dedicated multi-sensory data processing engines
(graphics, image, display, audio and voice). The i.MX 9 series, part of the EdgeVerse™
edge computing platform, integrates hardware neural processing units across many
members of the series for acceleration of machine learning applications at the edge
`i.MX9 Applications Processors`_.

Boot Sequence
-------------

BootROM --> SPL --> BL31 --> BL33(u-boot) --> Linux kernel

How to build
------------

Build Procedure
~~~~~~~~~~~~~~~

-  Prepare AARCH64 toolchain.

- Get the ELE FW image from NXP linux SDK package

-  Build SPL and u-boot firstly, and get binary images: u-boot-spl.bin,
   u-boot.bin and dtb

-  Build TF-A

   Build bl31:

   .. code:: shell

       CROSS_COMPILE=aarch64-linux-gnu- make PLAT=<Target_SoC> bl31

   Target_SoC should be "imx93" for i.MX93 SoC.

Deploy TF-A Images
~~~~~~~~~~~~~~~~~~

TF-A binary(bl31.bin), u-boot-spl.bin u-boot.bin, ELE FW image are combined
together to generate a binary file called flash.bin, the imx-mkimage tool is
used to generate flash.bin, and flash.bin needs to be flashed into SD card
with certain offset for BOOT ROM.

Reference Documentation
~~~~~~~~~~~~~~~~~~~~~~~

Details on how to prepare, generate & deploy the boot image be found in following documents:

- i.MX Linux User's Guide
  `link <https://www.nxp.com/design/software/embedded-software/i-mx-software/embedded-linux-for-i-mx-applications-processors:IMXLINUX>`__
- i.MX Linux Reference Manual
  `link <https://www.nxp.com/design/software/embedded-software/i-mx-software/embedded-linux-for-i-mx-applications-processors:IMXLINUX>`__

.. _i.MX9 Applications Processors: https://www.nxp.com/products/processors-and-microcontrollers/arm-processors/i-mx-applications-processors/i-mx-9-processors:IMX9-PROCESSORS

