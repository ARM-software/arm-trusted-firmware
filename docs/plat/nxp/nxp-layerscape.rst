NXP SoCs - Overview
=====================
.. section-numbering::
    :suffix: .

The QorIQ family of ARM based SoCs that are supported on TF-A are:

1. LX2160A

- SoC Overview:

The LX2160A multicore processor, the highest-performance member of the
Layerscape family, combines FinFET process technology's low power and
sixteen Arm® Cortex®-A72 cores with datapath acceleration optimized for
L2/3 packet processing, together with security offload, robust traffic
management and quality of service.

Details about LX2160A can be found at `lx2160a`_.

- LX2160ARDB Board:

The LX2160A reference design board provides a comprehensive platform
that enables design and evaluation of the LX2160A or LX2162A processors. It
comes preloaded with a board support package (BSP) based on a standard Linux
kernel.

Board details can be fetched from the link: `lx2160ardb`_.

2. LS1028A

- SoC Overview:

The Layerscape LS1028A applications processor for industrial and
automotive includes a time-sensitive networking (TSN) -enabled Ethernet
switch and Ethernet controllers to support converged IT and OT networks.
Two powerful 64-bit Arm®v8 cores support real-time processing for
industrial control and virtual machines for edge computing in the IoT.
The integrated GPU and LCD controller enable Human-Machine Interface
(HMI) systems with next-generation interfaces.

Details about LS1028A can be found at `ls1028a`_.

- LS1028ARDB Boards:

The LS1028A reference design board (RDB) is a computing, evaluation,
and development platform that supports industrial IoT applications, human
machine interface solutions, and industrial networking.

Details about LS1028A RDB board can be found at `ls1028ardb`_.

Table of supported boot-modes by each platform & platform that needs FIP-DDR:
-----------------------------------------------------------------------------

+---------------------+---------------------------------------------------------------------+-----------------+
|                     |                            BOOT_MODE                                |                 |
|       PLAT          +-------+--------+-------+-------+-------+-------------+--------------+ fip_ddr_needed  |
|                     |  sd   |  qspi  |  nor  | nand  | emmc  | flexspi_nor | flexspi_nand |                 |
+=====================+=======+========+=======+=======+=======+=============+==============+=================+
|     lx2160ardb      |  yes  |        |       |       |  yes  |   yes       |              |       yes       |
+---------------------+-------+--------+-------+-------+-------+-------------+--------------+-----------------+
|     ls1028ardb      |  yes  |        |       |       |  yes  |   yes       |              |       no        |
+---------------------+-------+--------+-------+-------+-------+-------------+--------------+-----------------+


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

DDR Memory Layout
--------------------------

NXP Platforms divide DRAM into banks:

- DRAM0 Bank:  Maximum size of this bank is fixed to 2GB, DRAM0 size is defined in platform_def.h if it is less than 2GB.

- DRAM1 ~ DRAMn Bank:  Greater than 2GB belongs to DRAM1 and following banks, and size of DRAMn Bank varies for one platform to others.

The following diagram is default DRAM0 memory layout in which secure memory is at top of DRAM0.

::

  high  +---------------------------------------------+
        |                                             |
        |   Secure EL1 Payload Shared Memory (2 MB)   |
        |                                             |
        +---------------------------------------------+
        |                                             |
        |            Secure Memory (64 MB)            |
        |                                             |
        +---------------------------------------------+
        |                                             |
        |             Non Secure Memory               |
        |                                             |
  low   +---------------------------------------------+

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


.. _lx2160a: https://www.nxp.com/products/processors-and-microcontrollers/arm-processors/layerscape-processors/layerscape-lx2160a-lx2120a-lx2080a-processors:LX2160A
.. _lx2160ardb: https://www.nxp.com/products/processors-and-microcontrollers/arm-processors/layerscape-communication-process/layerscape-lx2160a-multicore-communications-processor:LX2160A
.. _ls1028a: https://www.nxp.com/products/processors-and-microcontrollers/arm-processors/layerscape-processors/layerscape-1028a-applications-processor:LS1028A
.. _ls1028ardb: https://www.nxp.com/design/qoriq-developer-resources/layerscape-ls1028a-reference-design-board:LS1028ARDB
.. _nxp-ls-tbbr.rst: ./nxp-ls-tbbr.rst
