Allwinner ARMv8 SoCs
====================

Trusted Firmware-A (TF-A) implements the EL3 firmware layer for Allwinner
SoCs with ARMv8 cores. Only BL31 is used to provide proper EL3 setup and
PSCI runtime services.

Building TF-A
-------------

There is one build target per supported SoC:

+------+-------------------+
| SoC  | TF-A build target |
+======+===================+
| A64  | sun50i_a64        |
+------+-------------------+
| H5   | sun50i_a64        |
+------+-------------------+
| H6   | sun50i_h6         |
+------+-------------------+
| H616 | sun50i_h616       |
+------+-------------------+
| H313 | sun50i_h616       |
+------+-------------------+
| R329 | sun50i_r329       |
+------+-------------------+

To build with the default settings for a particular SoC:

.. code:: shell

    make CROSS_COMPILE=aarch64-linux-gnu- PLAT=<build target> DEBUG=1

So for instance to build for a board with the Allwinner A64 SoC::

    make CROSS_COMPILE=aarch64-linux-gnu- PLAT=sun50i_a64 DEBUG=1

Platform-specific build options
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The default build options should generate a working firmware image. There are
some build options that allow to fine-tune the firmware, or to disable support
for optional features.

-  ``SUNXI_PSCI_USE_NATIVE`` : Support direct control of the CPU cores powerdown
   and powerup sequence by BL31. This requires either support for a code snippet
   to be loaded into the ARISC SCP (A64, H5), or the power sequence control
   registers to be programmed directly (H6, H616). This supports only basic
   control, like core on/off and system off/reset.
   This option defaults to 1. If an active SCP supporting the SCPI protocol
   is detected at runtime, this control scheme will be ignored, and SCPI
   will be used instead, unless support has been explicitly disabled.

-  ``SUNXI_PSCI_USE_SCPI`` : Support control of the CPU cores powerdown and
   powerup sequence by talking to the SCP processor via the SCPI protocol.
   This allows more advanced power saving techniques, like suspend to RAM.
   This option defaults to 1 on SoCs that feature an SCP. If no SCP firmware
   using the SCPI protocol is detected, the native sequence will be used
   instead. If both native and SCPI methods are included, SCPI will be favoured
   if SCP support is detected.

-  ``SUNXI_SETUP_REGULATORS`` : On SoCs that typically ship with a PMIC
   power management controller, BL31 tries to set up all needed power rails,
   programming them to their respective voltages. That allows bootloader
   software like U-Boot to ignore power control via the PMIC.
   This setting defaults to 1. In some situations that enables too many
   regulators, or some regulators need to be enabled in a very specific
   sequence. To avoid problems with those boards, ``SUNXI_SETUP_REGULATORS``
   can bet set to ``0`` on the build command line, to skip the PMIC setup
   entirely. Any bootloader or OS would need to setup the PMIC on its own then.

Installation
------------

U-Boot's SPL acts as a loader, loading both BL31 and BL33 (typically U-Boot).
Loading is done from SD card, eMMC or SPI flash, also via an USB debug
interface (FEL).

After building bl31.bin, the binary must be fed to the U-Boot build system
to include it in the FIT image that the SPL loader will process.
bl31.bin can be either copied (or sym-linked) into U-Boot's root directory,
or the environment variable BL31 must contain the binary's path.
See the respective `U-Boot documentation`_ for more details.

.. _U-Boot documentation: https://gitlab.denx.de/u-boot/u-boot/-/blob/master/board/sunxi/README.sunxi64

Memory layout
-------------

A64, H5 and H6 SoCs
~~~~~~~~~~~~~~~~~~~

BL31 lives in SRAM A2, which is documented to be accessible from secure
world only. Since this SRAM region is very limited (48 KB), we take
several measures to reduce memory consumption. One of them is to confine
BL31 to only 28 bits of virtual address space, which reduces the number
of required page tables (each occupying 4KB of memory).
The mapping we use on those SoCs is as follows:

::

   0 64K         16M             1GB         1G+160M     physical address
   +-+------+-+---+------+--...---+-------+----+------+----------
   |B|      |S|///|      |//...///|       |////|      |
   |R| SRAM |C|///| dev  |//...///| (sec) |////| BL33 |  DRAM ...
   |O|      |P|///| MMIO |//...///| DRAM  |////|      |
   |M|      | |///|      |//...///| (32M) |////|      |
   +-+------+-+---+------+--...---+-------+----+------+----------
   | |      | |   |      |       /       /   /      /
   | |      | |   |      |      /       /  /      /
   | |      | |   |      |     /       / /      /
   | |      | |   |      |    /       //      /
   | |      | |   |      |   /       /      /
   +-+------+-+---+------+--+-------+------+
   |B|      |S|///|      |//|       |      |
   |R| SRAM |C|///| dev  |//|  sec  | BL33 |
   |O|      |P|///| MMIO |//| DRAM  |      |
   |M|      | |///|      |//|       |      |
   +-+------+-+---+------+--+-------+------+
   0 64K         16M       160M    192M  256M             virtual address


H616 SoC
~~~~~~~~

The H616 lacks the secure SRAM region present on the other SoCs, also
lacks the "ARISC" management processor (SCP) we use. BL31 thus needs to
run from DRAM, which prevents our compressed virtual memory map described
above. Since running in DRAM also lifts the restriction of the limited
SRAM size, we use the normal 1:1 mapping with 32 bits worth of virtual
address space. So the virtual addresses used in BL31 match the physical
addresses as presented above.

Trusted OS dispatcher
---------------------

One can boot Trusted OS(OP-TEE OS, bl32 image) along side bl31 image on Allwinner A64.

In order to include the 'opteed' dispatcher in the image, pass 'SPD=opteed' on the command line
while compiling the bl31 image and make sure the loader (SPL) loads the Trusted OS binary to
the beginning of DRAM (0x40000000).
