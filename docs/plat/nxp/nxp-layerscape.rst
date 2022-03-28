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

- LS1028ARDB Board:

The LS1028A reference design board (RDB) is a computing, evaluation,
and development platform that supports industrial IoT applications, human
machine interface solutions, and industrial networking.

Details about LS1028A RDB board can be found at `ls1028ardb`_.

3. LS1043A

- SoC Overview:

The Layerscape LS1043A processor is NXP's first quad-core, 64-bit Arm®-based
processor for embedded networking. The LS1023A (two core version) and the
LS1043A (four core version) deliver greater than 10 Gbps of performance
in a flexible I/O package supporting fanless designs. This SoC is a
purpose-built solution for small-form-factor networking and industrial
applications with BOM optimizations for economic low layer PCB, lower cost
power supply and single clock design. The new 0.9V versions of the LS1043A
and LS1023A deliver addition power savings for applications such as Wireless
LAN and to Power over Ethernet systems.

Details about LS1043A can be found at `ls1043a`_.

- LS1043ARDB Board:

The LS1043A reference design board (RDB) is a computing, evaluation, and
development platform that supports the Layerscape LS1043A architecture
processor. The LS1043A-RDB can help shorten your time to market by providing
the following features:

Memory subsystem:
	* 2GByte DDR4 SDRAM (32bit bus)
	* 128 Mbyte NOR flash single-chip memory
	* 512 Mbyte NAND flash
	* 16 Mbyte high-speed SPI flash
	* SD connector to interface with the SD memory card

Ethernet:
	* XFI 10G port
	* QSGMII with 4x 1G ports
	* Two RGMII ports

PCIe:
	* PCIe2 (Lanes C) to mini-PCIe slot
	* PCIe3 (Lanes D) to PCIe slot

USB 3.0: two super speed USB 3.0 type A ports

UART: supports two UARTs up to 115200 bps for console

Details about LS1043A RDB board can be found at `ls1043ardb`_.

4. LS1046A

- SoC Overview:

The LS1046A is a cost-effective, power-efficient, and highly integrated
system-on-chip (SoC) design that extends the reach of the NXP value-performance
line of QorIQ communications processors. Featuring power-efficient 64-bit
Arm Cortex-A72 cores with ECC-protected L1 and L2 cache memories for high
reliability, running up to 1.8 GHz.

Details about LS1046A can be found at `ls1046a`_.

- LS1046ARDB Board:

The LS1046A reference design board (RDB) is a high-performance computing,
evaluation, and development platform that supports the Layerscape LS1046A
architecture processor. The LS1046ARDB board supports the Layerscape LS1046A
processor and is optimized to support the DDR4 memory and a full complement
of high-speed SerDes ports.

Details about LS1046A RDB board can be found at `ls1046ardb`_.

- LS1046AFRWY Board:

The LS1046A Freeway board (FRWY) is a high-performance computing, evaluation,
and development platform that supports the LS1046A architecture processor
capable of support more than 32,000 CoreMark performance. The FRWY-LS1046A
board supports the LS1046A processor, onboard DDR4 memory, multiple Gigabit
Ethernet, USB3.0 and M2_Type_E interfaces for Wi-Fi, FRWY-LS1046A-AC includes
the Wi-Fi card.

Details about LS1046A FRWY board can be found at `ls1046afrwy`_.

5. LS1088A

- SoC Overview:

The LS1088A family of multicore communications processors combines up to and eight
Arm Cortex-A53 cores with the advanced, high-performance data path and network
peripheral interfaces required for wireless access points, networking infrastructure,
intelligent edge access, including virtual customer premise equipment (vCPE) and
high-performance industrial applications.

Details about LS1088A can be found at `ls1088a`_.

- LS1088ARDB Board:

The LS1088A reference design board provides a comprehensive platform that
enables design and evaluation of the product (LS1088A processor). This RDB
comes pre-loaded with a board support package (BSP) based on a standard
Linux kernel.

Details about LS1088A RDB board can be found at `ls1088ardb`_.

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
|     ls1043ardb      |  yes  |        |  yes  |  yes  |       |             |              |       no        |
+---------------------+-------+--------+-------+-------+-------+-------------+--------------+-----------------+
|     ls1046ardb      |  yes  |  yes   |       |       |  yes  |             |              |       no        |
+---------------------+-------+--------+-------+-------+-------+-------------+--------------+-----------------+
|     ls1046afrwy     |  yes  |  yes   |       |       |       |             |              |       no        |
+---------------------+-------+--------+-------+-------+-------+-------------+--------------+-----------------+
|     ls1088ardb      |  yes  |  yes   |       |       |       |             |              |       no        |
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

-  Deploy ATF images on flexspi-Nor or QSPI flash Alt Bank from U-Boot prompt.

   --  Commands to flash images for bl2_xxx.pbl and fip.bin

   Notes: ls1028ardb has no flexspi-Nor Alt Bank, so use "sf probe 0:0" for current bank.

   .. code:: shell

        tftp 82000000  $path/bl2_xxx.pbl;

        i2c mw 66 50 20;sf probe 0:1; sf erase 0 +$filesize; sf write 0x82000000 0x0 $filesize;

        tftp 82000000  $path/fip.bin;
        i2c mw 66 50 20;sf probe 0:1; sf erase 0x100000 +$filesize; sf write 0x82000000 0x100000 $filesize;

   --  Next step is valid for platform where FIP-DDR is needed.

   .. code:: shell

        tftp 82000000  $path/ddr_fip.bin;
        i2c mw 66 50 20;sf probe 0:1; sf erase 0x800000 +$filesize; sf write 0x82000000 0x800000 $filesize;

   --  Then reset to alternate bank to boot up ATF.

   Command for lx2160a, ls1088a and ls1028a platforms:

   .. code:: shell

        qixisreset altbank;

   Command for ls1046a platforms:

   .. code:: shell

        cpld reset altbank;

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

   Command for lx2160A, ls1088a and ls1028a platforms:

   .. code:: shell

        qixisreset <sd or emmc>;

   Command for ls1043a and ls1046a platform:

   .. code:: shell

        cpld reset <sd or emmc>;

-  Deploy ATF images on IFC nor flash from U-Boot prompt.

   .. code:: shell

        tftp 82000000  $path/bl2_nor.pbl;
	protect off 64000000 +$filesize; erase 64000000 +$filesize; cp.b 82000000 64000000 $filesize;

        tftp 82000000  $path/fip.bin;
	protect off 64100000 +$filesize; erase 64100000 +$filesize; cp.b 82000000 64100000 $filesize;

   --  Then reset to alternate bank to boot up ATF.

   Command for ls1043a platform:

   .. code:: shell

        cpld reset altbank;

-  Deploy ATF images on IFC nand flash from U-Boot prompt.

   .. code:: shell

        tftp 82000000  $path/bl2_nand.pbl;
	nand erase 0x0 $filesize; nand write 82000000 0x0 $filesize;

        tftp 82000000  $path/fip.bin;
	nand erase 0x100000 $filesize;nand write 82000000 0x100000 $filesize;

   --  Then reset to nand flash to boot up ATF.

   Command for ls1043a platform:

   .. code:: shell

        cpld reset nand;



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
.. _ls1043a: https://www.nxp.com/products/processors-and-microcontrollers/arm-processors/layerscape-processors/layerscape-1043a-and-1023a-processors:LS1043A
.. _ls1043ardb: https://www.nxp.com/design/qoriq-developer-resources/layerscape-ls1043a-reference-design-board:LS1043A-RDB
.. _ls1046a: https://www.nxp.com/products/processors-and-microcontrollers/arm-processors/layerscape-processors/layerscape-1046a-and-1026a-processors:LS1046A
.. _ls1046ardb: https://www.nxp.com/design/qoriq-developer-resources/layerscape-ls1046a-reference-design-board:LS1046A-RDB
.. _ls1046afrwy: https://www.nxp.com/design/qoriq-developer-resources/ls1046a-freeway-board:FRWY-LS1046A
.. _ls1088a: https://www.nxp.com/products/processors-and-microcontrollers/arm-processors/layerscape-processors/layerscape-1088a-and-1048a-processor:LS1088A
.. _ls1088ardb: https://www.nxp.com/design/qoriq-developer-resources/layerscape-ls1088a-reference-design-board:LS1088A-RDB
.. _nxp-ls-tbbr.rst: ./nxp-ls-tbbr.rst
