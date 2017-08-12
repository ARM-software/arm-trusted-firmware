Trusted Firmware-A for Allwinner ARMv8 SoCs
===========================================

Trusted Firmware-A (TF-A) implements the EL3 firmware layer for Allwinner
SoCs with ARMv8 cores. Only BL31 is used to provide proper EL3 setup and
PSCI runtime services.
U-Boot's SPL acts as a loader, loading both BL31 and BL33 (typically U-Boot).
Loading is done from SD card, eMMC or SPI flash, also via an USB debug
interface (FEL).
BL31 lives in SRAM A2, which is documented to be accessible from secure
world only.

Current limitations:

-  Missing PMIC support

After building bl31.bin, the binary must be fed to the U-Boot build system
to include it in the FIT image that the SPL loader will process.
bl31.bin can be either copied (or sym-linked) into U-Boot's root directory,
or the environment variable BL31 must contain the binary's path.
See the respective `U-Boot documentation`_ for more details.

To build:

::

    make CROSS_COMPILE=aarch64-linux-gnu- PLAT=sun50i_a64 DEBUG=1 bl31

.. _U-Boot documentation: http://git.denx.de/?p=u-boot.git;f=board/sunxi/README.sunxi64;hb=HEAD
