NXP SoCs - Overview
=====================
.. section-numbering::
    :suffix: .

The QorIQ family of ARM based SoCs that are supported on TF-A are:

1. LX2160ARDB:
        Platform Name:

        a. lx2160ardb (Board details can be fetched from the link: `lx2160ardb`_)


Table of supported boot-modes by each platform & platform that needs FIP-DDR:
-----------------------------------------------------------------------------

+---+-----------------+-------+--------+-------+-------+-------+-------------+--------------+-----------------+
|   |     BOOT_MODE-->|  sd   |  qspi  |  nor  | nand  | emmc  | flexspi_nor | flexspi_nand | fip_ddr needed  |
|   |                 |       |        |       |       |       |             |              |                 |
|   |     PLAT        |       |        |       |       |       |             |              |                 |
+===+=================+=======+========+=======+=======+=======+=============+==============+=================+
| 1.| lx2160ardb      |  yes  |        |       |       |  yes  |   yes       |              |       yes       |
+---+-----------------+-------+--------+-------+-------+-------+-------------+--------------+-----------------+

Boot Sequence
-------------
::

+                           Secure World        |     Normal World
+ EL0                                           |
+                                               |
+ EL1                           BL32(Tee OS)    |     kernel
+                                ^ |            |       ^
+                                | |            |       |
+ EL2                            | |            |     BL33(u-boot)
+                                | |            |      ^
+                                | v            |     /
+ EL3        BootROM --> BL2 --> BL31 ---------------/
+

Boot Sequence with FIP-DDR
--------------------------
::

+                           Secure World        |     Normal World
+ EL0                                           |
+                                               |
+ EL1               fip-ddr     BL32(Tee OS)    |     kernel
+                     ^ |         ^ |           |       ^
+                     | |         | |           |       |
+ EL2                 | |         | |           |     BL33(u-boot)
+                     | |         | |           |      ^
+                     | v         | v           |     /
+ EL3     BootROM --> BL2 -----> BL31 ---------------/
+


How to build
=============

Code Locations
--------------

-  OP-TEE:
   `link <https://source.codeaurora.org/external/qoriq/qoriq-components/optee_os>`__

-  U-Boot:
   `link <https://source.codeaurora.org/external/qoriq/qoriq-components/u-boot>`__

-  RCW:
   `link <https://source.codeaurora.org/external/qoriq/qoriq-components/rcw>`__

-  ddr-phy-binary: Required by platforms that need fip-ddr.
   `link <https:://github.com/NXP/ddr-phy-binary>`__

-  cst: Required for TBBR.
   `link <https:://source.codeaurora.org/external/qoriq/qoriq-components/cst>`__

Build Procedure
---------------

-  Fetch all the above repositories into local host.

-  Prepare AARCH64 toolchain and set the environment variable "CROSS_COMPILE".

   .. code:: shell

       export CROSS_COMPILE=.../bin/aarch64-linux-gnu-

-  Build RCW. Refer README from the respective cloned folder for more details.

-  Build u-boot and OPTee firstly, and get binary images: u-boot.bin and tee.bin.
   For u-boot you can use the <platform>_tfa_defconfig for build.

-  Copy/clone the repo "ddr-phy-binary" to the tfa directory for platform needing ddr-fip.

-  Below are the steps to build TF-A images for the supported platforms.

Compilation steps without BL32
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

BUILD BL2:

-To compile
   .. code:: shell

       make PLAT=$PLAT \
       BOOT_MODE=<platform_supported_boot_mode> \
       RCW=$RCW_BIN \
       pbl

BUILD FIP:

   .. code:: shell

       make PLAT=$PLAT \
       BOOT_MODE=<platform_supported_boot_mode> \
       RCW=$RCW_BIN \
       BL33=$UBOOT_SECURE_BIN \
       pbl \
       fip

Compilation steps with BL32
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

BUILD BL2:

-To compile
   .. code:: shell

       make PLAT=$PLAT \
       BOOT_MODE=<platform_supported_boot_mode> \
       RCW=$RCW_BIN \
       BL32=$TEE_BIN SPD=opteed\
       pbl

BUILD FIP:

   .. code:: shell

       make PLAT=$PLAT \
       BOOT_MODE=<platform_supported_boot_mode> \
       RCW=$RCW_BIN \
       BL32=$TEE_BIN SPD=opteed\
       BL33=$UBOOT_SECURE_BIN \
       pbl \
       fip


BUILD fip-ddr (Mandatory for certain platforms, refer table above):
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

-To compile additional fip-ddr for selected platforms(Refer above table if the platform needs fip-ddr).
   .. code:: shell

	make PLAT=<platform_name> fip-ddr


Deploy ATF Images
=================

Note: The size in the standard uboot commands for copy to nor, qspi, nand or sd
should be modified based on the binary size of the image to be copied.

-  Deploy ATF images on flexspi-Nor flash Alt Bank from U-Boot prompt.
   --  Commands to flash images for bl2_xxx.pbl and fip.bin.

   .. code:: shell

        tftp 82000000  $path/bl2_flexspi_nor.pbl;
        i2c mw 66 50 20;sf probe 0:0; sf erase 0 +$filesize; sf write 0x82000000 0x0 $filesize;

        tftp 82000000  $path/fip.bin;
        i2c mw 66 50 20;sf probe 0:0; sf erase 0x100000 +$filesize; sf write 0x82000000 0x100000 $filesize;

   --  Next step is valid for platform where FIP-DDR is needed.

   .. code:: shell

        tftp 82000000  $path/ddr_fip.bin;
        i2c mw 66 50 20;sf probe 0:0; sf erase 0x800000 +$filesize; sf write 0x82000000 0x800000 $filesize;

   --  Then reset to alternate bank to boot up ATF.

   .. code:: shell

        qixisreset altbank;

-  Deploy ATF images on SD/eMMC from U-Boot prompt.
   -- file_size_in_block_sizeof_512 = (Size_of_bytes_tftp / 512)

   .. code:: shell

        mmc dev <idx>; (idx = 1 for eMMC; idx = 0 for SD)

        tftp 82000000  $path/bl2_<sd>_or_<emmc>.pbl;
        mmc write 82000000 8 <file_size_in_block_sizeof_512>;

        tftp 82000000  $path/fip.bin;
        mmc write 82000000 0x800 <file_size_in_block_sizeof_512>;

    --  Next step is valid for platform that needs FIP-DDR.

   .. code:: shell

        tftp 82000000  $path/ddr_fip.bin;
        mmc write 82000000 0x4000 <file_size_in_block_sizeof_512>;

   --  Then reset to sd/emmc to boot up ATF from sd/emmc as boot-source.

   .. code:: shell

        qixisreset <sd or emmc>;

Trusted Board Boot:
===================

For TBBR, the binary name changes:

+-------------+--------------------------+---------+-------------------+
|  Boot Type  |           BL2            |   FIP   |      FIP-DDR      |
+=============+==========================+=========+===================+
| Normal Boot |  bl2_<boot_mode>.pbl     | fip.bin | ddr_fip.bin       |
+-------------+--------------------------+---------+-------------------+
| TBBR Boot   |  bl2_<boot_mode>_sec.pbl | fip.bin | ddr_fip_sec.bin   |
+-------------+--------------------------+---------+-------------------+

Refer `nxp-ls-tbbr.rst`_ for detailed user steps.


.. _lx2160ardb: https://www.nxp.com/products/processors-and-microcontrollers/arm-processors/layerscape-communication-process/layerscape-lx2160a-multicore-communications-processor:LX2160A
.. _nxp-ls-tbbr.rst: ./nxp-ls-tbbr.rst
