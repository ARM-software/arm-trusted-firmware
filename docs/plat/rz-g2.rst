Renesas RZ/G
============

The "RZ/G" Family of high-end 64-bit Arm®-based microprocessors (MPUs)
enables the solutions required for the smart society of the future.
Through a variety of Arm Cortex®-A53 and A57-based devices, engineers can
easily implement high-resolution human machine interfaces (HMI), embedded
vision, embedded artificial intelligence (e-AI) and real-time control and
industrial ethernet connectivity.

The scalable RZ/G hardware platform and flexible software platform
cover the full product range, from the premium class to the entry
level. Plug-ins are available for multiple open-source software tools.


Renesas RZ/G2 reference platforms:
----------------------------------

+--------------+----------------------------------------------------------------------------------+
| Board        |      Details                                                                     |
+==============+===============+==================================================================+
| hihope-rzg2h | "96 boards" compatible board from Hoperun equipped with Renesas RZ/G2H SoC       |
|              +----------------------------------------------------------------------------------+
|              | http://hihope.org/product/musashi                                                |
+--------------+----------------------------------------------------------------------------------+
| hihope-rzg2m | "96 boards" compatible board from Hoperun equipped with Renesas RZ/G2M SoC       |
|              +----------------------------------------------------------------------------------+
|              | http://hihope.org/product/musashi                                                |
+--------------+----------------------------------------------------------------------------------+
| hihope-rzg2n | "96 boards" compatible board from Hoperun equipped with Renesas RZ/G2N SoC       |
|              +----------------------------------------------------------------------------------+
|              | http://hihope.org/product/musashi                                                |
+--------------+----------------------------------------------------------------------------------+
| ek874        | "96 boards" compatible board from Silicon Linux equipped with Renesas RZ/G2E SoC |
|              +----------------------------------------------------------------------------------+
|              | https://www.si-linux.co.jp/index.php?CAT%2FCAT874                                |
+--------------+----------------------------------------------------------------------------------+

`boards info <https://www.renesas.com/us/en/products/rzg-linux-platform/rzg-marcketplace/board-solutions.html#rzg2>`__

The current TF-A port has been tested on the HiHope RZ/G2M
SoC_id r8a774a1 revision ES1.3.


::

    ARM CA57 (ARMv8) 1.5 GHz dual core, with NEON/VFPv4, L1$ I/D 48K/32K, L2$ 1MB
    ARM CA53 (ARMv8) 1.2 GHz quad core, with NEON/VFPv4, L1$ I/D 32K/32K, L2$ 512K
    Memory controller for LPDDR4-3200 4GB in 2 channels(32-bit bus mode)
    Two- and three-dimensional graphics engines,
    Video processing units,
    Display Output,
    Video Input,
    SD card host interface,
    USB3.0 and USB2.0 interfaces,
    CAN interfaces,
    Ethernet AVB,
    Wi-Fi + BT,
    PCI Express Interfaces,
    Memories
        INTERNAL 384KB SYSTEM RAM
        DDR 4 GB LPDDR4
        QSPI FLASH 64MB
        EMMC 32 GB EMMC (HS400 240 MBYTES/S)
        MICROSD-CARD SLOT (SDR104 100 MBYTES/S)

Overview
--------
On RZ/G2 SoCs the BOOTROM starts the cpu at EL3; for this port BL2
will therefore be entered at this exception level (the Renesas' ATF
reference tree [1] resets into EL1 before entering BL2 - see its
bl2.ld.S)

BL2 initializes DDR before determining the boot reason (cold or warm).

Once BL2 boots, it determines the boot reason, writes it to shared
memory (BOOT_KIND_BASE) together with the BL31 parameters
(PARAMS_BASE) and jumps to BL31.

To all effects, BL31 is as if it is being entered in reset mode since
it still needs to initialize the rest of the cores; this is the reason
behind using direct shared memory access to  BOOT_KIND_BASE _and_
PARAMS_BASE instead of using registers to get to those locations (see
el3_common_macros.S and bl31_entrypoint.S for the RESET_TO_BL31 use
case).

[1] https://github.com/renesas-rz/meta-rzg2/tree/BSP-1.0.5/recipes-bsp/arm-trusted-firmware/files


How to build
------------

The TF-A build options depend on the target board so you will have to
refer to those specific instructions. What follows is customized to
the HiHope RZ/G2M development kit used in this port.

Build Tested:
~~~~~~~~~~~~~

.. code:: bash

       make bl2 bl31 rzg LOG_LEVEL=40 PLAT=rzg LSI=G2M RCAR_DRAM_SPLIT=2\
       RCAR_LOSSY_ENABLE=1 SPD="none" MBEDTLS_DIR=$mbedtls

System Tested:
~~~~~~~~~~~~~~
* mbed_tls:
  git@github.com:ARMmbed/mbedtls.git [devel]

|  commit 72ca39737f974db44723760623d1b29980c00a88
|  Merge: ef94c4fcf dd9ec1c57
|  Author: Janos Follath <janos.follath@arm.com>
|  Date:   Wed Oct 7 09:21:01 2020 +0100

* u-boot:
  The port has beent tested using mainline uboot with HiHope RZ/G2M board
  specific patches.

|  commit 46ce9e777c1314ccb78906992b94001194eaa87b
|  Author: Heiko Schocher <hs@denx.de>
|  Date:   Tue Nov 3 15:22:36 2020 +0100

* linux:
  The port has beent tested using mainline kernel.

|  commit f8394f232b1eab649ce2df5c5f15b0e528c92091
|  Author: Linus Torvalds <torvalds@linux-foundation.org>
|  Date:   Sun Nov 8 16:10:16 2020 -0800
|  Linux 5.10-rc3

TF-A Build Procedure
~~~~~~~~~~~~~~~~~~~~

-  Fetch all the above 3 repositories.

-  Prepare the AARCH64 toolchain.

-  Build u-boot using hihope_rzg2_defconfig.

   Result: u-boot-elf.srec

.. code:: bash

       make CROSS_COMPILE=aarch64-linux-gnu-
	  hihope_rzg2_defconfig

       make CROSS_COMPILE=aarch64-linux-gnu-

-  Build TF-A

   Result: bootparam_sa0.srec, cert_header_sa6.srec, bl2.srec, bl31.srec

.. code:: bash

       make bl2 bl31 rzg LOG_LEVEL=40 PLAT=rzg LSI=G2M RCAR_DRAM_SPLIT=2\
       RCAR_LOSSY_ENABLE=1 SPD="none" MBEDTLS_DIR=$mbedtls


Install Procedure
~~~~~~~~~~~~~~~~~

- Boot the board in Mini-monitor mode and enable access to the
  QSPI flash.


- Use the flash_writer utility[2] to flash all the SREC files.

[2] https://github.com/renesas-rz/rzg2_flash_writer


Boot trace
----------
::

   INFO:    ARM GICv2 driver initialized
   NOTICE:  BL2: RZ/G2 Initial Program Loader(CA57) Rev.2.0.6
   NOTICE:  BL2: PRR is RZ/G2M Ver.1.3
   NOTICE:  BL2: Board is HiHope RZ/G2M Rev.4.0
   NOTICE:  BL2: Boot device is QSPI Flash(40MHz)
   NOTICE:  BL2: LCM state is unknown
   NOTICE:  BL2: DDR3200(rev.0.40)
   NOTICE:  BL2: [COLD_BOOT]
   NOTICE:  BL2: DRAM Split is 2ch
   NOTICE:  BL2: QoS is default setting(rev.0.19)
   NOTICE:  BL2: DRAM refresh interval 1.95 usec
   NOTICE:  BL2: Periodic Write DQ Training
   NOTICE:  BL2: CH0: 400000000 - 47fffffff, 2 GiB
   NOTICE:  BL2: CH2: 600000000 - 67fffffff, 2 GiB
   NOTICE:  BL2: Lossy Decomp areas
   NOTICE:       Entry 0: DCMPAREACRAx:0x80000540 DCMPAREACRBx:0x570
   NOTICE:       Entry 1: DCMPAREACRAx:0x40000000 DCMPAREACRBx:0x0
   NOTICE:       Entry 2: DCMPAREACRAx:0x20000000 DCMPAREACRBx:0x0
   NOTICE:  BL2: FDT at 0xe631db30
   NOTICE:  BL2: v2.3(release):v2.4-rc0-2-g1433701e5
   NOTICE:  BL2: Built : 13:45:26, Nov  7 2020
   NOTICE:  BL2: Normal boot
   INFO:    BL2: Doing platform setup
   INFO:    BL2: Loading image id 3
   NOTICE:  BL2: dst=0xe631d200 src=0x8180000 len=512(0x200)
   NOTICE:  BL2: dst=0x43f00000 src=0x8180400 len=6144(0x1800)
   WARNING: r-car ignoring the BL31 size from certificate,using RCAR_TRUSTED_SRAM_SIZE instead
   INFO:    Loading image id=3 at address 0x44000000
   NOTICE:  rcar_file_len: len: 0x0003e000
   NOTICE:  BL2: dst=0x44000000 src=0x81c0000 len=253952(0x3e000)
   INFO:    Image id=3 loaded: 0x44000000 - 0x4403e000
   INFO:    BL2: Loading image id 5
   INFO:    Loading image id=5 at address 0x50000000
   NOTICE:  rcar_file_len: len: 0x00100000
   NOTICE:  BL2: dst=0x50000000 src=0x8300000 len=1048576(0x100000)
   INFO:    Image id=5 loaded: 0x50000000 - 0x50100000
   NOTICE:  BL2: Booting BL31
   INFO:    Entry point address = 0x44000000
   INFO:    SPSR = 0x3cd


   U-Boot 2021.01-rc1-00244-gac37e14fbd (Nov 04 2020 - 20:03:34 +0000)

   CPU: Renesas Electronics R8A774A1 rev 1.3
   Model: HopeRun HiHope RZ/G2M with sub board
   DRAM:  3.9 GiB
   MMC:   mmc@ee100000: 0, mmc@ee160000: 1
   Loading Environment from MMC... OK
   In:    serial@e6e88000
   Out:   serial@e6e88000
   Err:   serial@e6e88000
   Net:   eth0: ethernet@e6800000
   Hit any key to stop autoboot:  0
   =>
