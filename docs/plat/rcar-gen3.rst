Renesas R-Car
=============

"R-Car" is the nickname for Renesas' system-on-chip (SoC) family for
car information systems designed for the next-generation of automotive
computing for the age of autonomous vehicles.

The scalable R-Car hardware platform and flexible software platform
cover the full product range, from the premium class to the entry
level. Plug-ins are available for multiple open-source software tools.


Renesas R-Car Gen3 evaluation boards:
-------------------------------------

+------------+-----------------+-----------------------------+
|            |     Standard    |   Low Cost Boards (LCB)     |
+============+=================+=============================+
| R-Car H3   | - Salvator-X    | - R-Car Starter Kit Premier |
|            | - Salvator-XS   |                             |
+------------+-----------------+-----------------------------+
| R-Car M3-W | - Salvator-X    |                             |
|            | - Salvator-XS   | - R-Car Starter Kit Pro     |
+------------+-----------------+-----------------------------+
| R-Car M3-N | - Salvator-X    |                             |
|            | - Salvator-XS   |                             |
+------------+-----------------+-----------------------------+
| R-Car V3M  | - Eagle         | - Starter Kit               |
+------------+-----------------+-----------------------------+
| R-Car V3H  | - Condor        | - Starter Kit               |
+------------+-----------------+-----------------------------+
| R-Car D3   | - Draak         |                             |
+------------+-----------------+-----------------------------+

`boards info <https://elinux.org/R-Car>`__

The current TF-A port has been tested on the R-Car H3 Salvator-X
Soc_id r8a7795 revision ES1.1 (uses a Secure Payload Dispatcher)


::

    ARM CA57 (ARMv8) 1.5 GHz quad core, with NEON/VFPv4, L1$ I/D
    48K/32K, L2$ 2MB
    ARM CA53 (ARMv8) 1.2 GHz quad core, with NEON/VFPv4, L1$ I/D 32K/32K,
    L2$ 512K
    Memory controller for LPDDR4-3200 4GB in 2 channels, each 64-bit wide
    Two- and three-dimensional graphics engines,
    Video processing units,
    3 channels Display Output,
    6 channels Video Input,
    SD card host interface,
    USB3.0 and USB2.0 interfaces,
    CAN interfaces
    Ethernet AVB
    PCI Express Interfaces
    Memories
        INTERNAL 384KB SYSTEM RAM
        DDR 4 GB LPDDR4
        HYPERFLASH 64 MB HYPER FLASH (512 MBITS, 160 MHZ, 320 MBYTES/S)
        QSPI FLASH 16MB QSPI (128 MBITS,80 MHZ,80 MBYTES/S)1 HEADER QSPI
        MODULE
        EMMC 32 GB EMMC (HS400 240 MBYTES/S)
        MICROSD-CARD SLOT (SDR104 100 MBYTES/S)


Overview
--------
On the rcar-gen3 the BOOTROM starts the cpu at EL3; for this port BL2
will therefore be entered at this exception level (the Renesas' ATF
reference tree [1] resets into EL1 before entering BL2 - see its
bl2.ld.S)

BL2 initializes DDR (and on some platforms i2c to interface to the
PMIC) before determining the boot reason (cold or warm).

During suspend all CPUs are switched off and the DDR is put in backup
mode (some kind of self-refresh mode). This means that BL2 is always
entered in a cold boot scenario.

Once BL2 boots, it determines the boot reason, writes it to shared
memory (BOOT_KIND_BASE) together with the BL31 parameters
(PARAMS_BASE) and jumps to BL31.

To all effects, BL31 is as if it is being entered in reset mode since
it still needs to initialize the rest of the cores; this is the reason
behind using direct shared memory access to  BOOT_KIND_BASE _and_
PARAMS_BASE instead of using registers to get to those locations (see
el3_common_macros.S and bl31_entrypoint.S for the RESET_TO_BL31 use
case).

Depending on the boot reason BL31 initializes the rest of the cores:
in case of suspend, it uses a MBOX memory region to recover the
program counters.

[1] https://github.com/renesas-rcar/arm-trusted-firmware


How to build
------------

The TF-A build options depend on the target board so you will have to
refer to those specific instructions. What follows is customized to
the H3 SiP Salvator-X development system used in this port.

Build Tested:
~~~~~~~~~~~~~
RCAR_OPT="LSI=H3 RCAR_DRAM_SPLIT=1 RCAR_LOSSY_ENABLE=1"
MBEDTLS_DIR=$mbedtls_src

$ MBEDTLS_DIR=$mbedtls_src_tree make clean bl2 bl31 rcar_layout_tool \
PLAT=rcar ${RCAR_OPT} SPD=opteed

System Tested:
~~~~~~~~~~~~~~
* mbed_tls:
  git@github.com:ARMmbed/mbedtls.git [devel]

  commit 552754a6ee82bab25d1bdf28c8261a4518e65e4d
  Merge: 68dbc94 f34a4c1
  Author: Simon Butcher <simon.butcher@arm.com>
  Date:   Thu Aug 30 00:57:28 2018 +0100

* optee_os:
  https://github.com/BayLibre/optee_os

  Until it gets merged into OP-TEE, the port requires Renesas'
  Trusted   Environment with a modification to support power
  management.
  commit 80105192cba9e704ebe8df7ab84095edc2922f84

  Author: Jorge Ramirez-Ortiz <jramirez@baylibre.com>
  Date:   Thu Aug 30 16:49:49 2018 +0200
  plat-rcar: cpu-suspend: handle the power level
  Signed-off-by: Jorge Ramirez-Ortiz <jramirez@baylibre.com>

* u-boot:
  The port has beent tested using mainline uboot.

  commit 4cdeda511f8037015b568396e6dcc3d8fb41e8c0
  Author: Fabio Estevam <festevam@gmail.com>
  Date:   Tue Sep 4 10:23:12 2018 -0300

* linux:
  The port has beent tested using mainline kernel.

  commit 7876320f88802b22d4e2daf7eb027dd14175a0f8
  Author: Linus Torvalds <torvalds@linux-foundation.org>
  Date:   Sun Sep 16 11:52:37 2018 -0700
  Linux 4.19-rc4

TF-A Build Procedure
~~~~~~~~~~~~~~~~~~~~

-  Fetch all the above 4 repositories.

-  Prepare the AARCH64 toolchain.

-  Build u-boot using r8a7795_salvator-x_defconfig.
   Result: u-boot-elf.srec

.. code:: bash

       make CROSS_COMPILE=aarch64-linux-gnu-
	  r8a7795_salvator-x_defconfig

       make CROSS_COMPILE=aarch64-linux-gnu-

-  Build atf
   Result: bootparam_sa0.srec, cert_header_sa6.srec, bl2.srec, bl31.srec

.. code:: bash

       RCAR_OPT="LSI=H3 RCAR_DRAM_SPLIT=1 RCAR_LOSSY_ENABLE=1"

       MBEDTLS_DIR=$mbedtls_src_tree make clean bl2 bl31 rcar \
       PLAT=rcar ${RCAR_OPT} SPD=opteed

-  Build optee-os
   Result: tee.srec

.. code:: bash

       make -j8 PLATFORM="rcar" CFG_ARM64_core=y

Install Procedure
~~~~~~~~~~~~~~~~~

- Boot the board in Mini-monitor mode and enable access to the
  Hyperflash.


- Use the XSL2 Mini-monitor utility to accept all the SREC ascii
  transfers over serial.


Boot trace
----------

Notice that BL31 traces are not accessible via the console and that in
order to verbose the BL2 output you will have to compile TF-A with
LOG_LEVEL=50 and DEBUG=1

::

   Initial Program Loader(CA57) Rev.1.0.22
   NOTICE:  BL2: PRR is R-Car H3 Ver.1.1
   NOTICE:  BL2: Board is Salvator-X Rev.1.0
   NOTICE:  BL2: Boot device is HyperFlash(80MHz)
   NOTICE:  BL2: LCM state is CM
   NOTICE:  AVS setting succeeded. DVFS_SetVID=0x53
   NOTICE:  BL2: DDR1600(rev.0.33)NOTICE:  [COLD_BOOT]NOTICE:  ..0
   NOTICE:  BL2: DRAM Split is 4ch
   NOTICE:  BL2: QoS is default setting(rev.0.37)
   NOTICE:  BL2: Lossy Decomp areas
   NOTICE:       Entry 0: DCMPAREACRAx:0x80000540 DCMPAREACRBx:0x570
   NOTICE:       Entry 1: DCMPAREACRAx:0x40000000 DCMPAREACRBx:0x0
   NOTICE:       Entry 2: DCMPAREACRAx:0x20000000 DCMPAREACRBx:0x0
   NOTICE:  BL2: v2.0(release):v2.0-rc0-32-gbcda69a
   NOTICE:  BL2: Built : 16:41:23, Oct  2 2018
   NOTICE:  BL2: Normal boot
   INFO:    BL2: Doing platform setup
   INFO:    BL2: Loading image id 3
   NOTICE:  BL2: dst=0xe6322000 src=0x8180000 len=512(0x200)
   NOTICE:  BL2: dst=0x43f00000 src=0x8180400 len=6144(0x1800)
   WARNING: r-car ignoring the BL31 size from certificate,using
   RCAR_TRUSTED_SRAM_SIZE instead
   INFO:    Loading image id=3 at address 0x44000000
   NOTICE:  rcar_file_len: len: 0x0003e000
   NOTICE:  BL2: dst=0x44000000 src=0x81c0000 len=253952(0x3e000)
   INFO:    Image id=3 loaded: 0x44000000 - 0x4403e000
   INFO:    BL2: Loading image id 4
   INFO:    Loading image id=4 at address 0x44100000
   NOTICE:  rcar_file_len: len: 0x00100000
   NOTICE:  BL2: dst=0x44100000 src=0x8200000 len=1048576(0x100000)
   INFO:    Image id=4 loaded: 0x44100000 - 0x44200000
   INFO:    BL2: Loading image id 5
   INFO:    Loading image id=5 at address 0x50000000
   NOTICE:  rcar_file_len: len: 0x00100000
   NOTICE:  BL2: dst=0x50000000 src=0x8640000 len=1048576(0x100000)
   INFO:    Image id=5 loaded: 0x50000000 - 0x50100000
   NOTICE:  BL2: Booting BL31
   INFO:    Entry point address = 0x44000000
   INFO:    SPSR = 0x3cd
   VERBOSE: Argument #0 = 0xe6325578
   VERBOSE: Argument #1 = 0x0
   VERBOSE: Argument #2 = 0x0
   VERBOSE: Argument #3 = 0x0
   VERBOSE: Argument #4 = 0x0
   VERBOSE: Argument #5 = 0x0
   VERBOSE: Argument #6 = 0x0
   VERBOSE: Argument #7 = 0x0


   U-Boot 2018.09-rc3-00028-g3711616 (Sep 27 2018 - 18:50:24 +0200)

   CPU: Renesas Electronics R8A7795 rev 1.1
   Model: Renesas Salvator-X board based on r8a7795 ES2.0+
   DRAM:  3.5 GiB
   Flash: 64 MiB
   MMC:   sd@ee100000: 0, sd@ee140000: 1, sd@ee160000: 2
   Loading Environment from MMC... OK
   In:    serial@e6e88000
   Out:   serial@e6e88000
   Err:   serial@e6e88000
   Net:   eth0: ethernet@e6800000
   Hit any key to stop autoboot:  0
   =>
