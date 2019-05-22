Intel Stratix 10 SoCFPGA
========================

Stratix 10 SoCFPGA is a FPGA with integrated quad-core 64-bit Arm Cortex A53 processor.

Upon boot, Boot ROM loads bl2 into OCRAM. Bl2 subsequently initializes
the hardware, then loads bl31 and bl33 (UEFI) into DDR and boots to bl33.

::

    Boot ROM --> Trusted Firmware-A --> UEFI

How to build
------------

Code Locations
~~~~~~~~~~~~~~

-  Trusted Firmware-A:
   `link <https://github.com/ARM-software/arm-trusted-firmware>`__

-  UEFI (to be updated with new upstreamed UEFI):
   `link <https://github.com/altera-opensource/uefi-socfpga>`__

Build Procedure
~~~~~~~~~~~~~~~

-  Fetch all the above 2 repositories into local host.
   Make all the repositories in the same ${BUILD\_PATH}.

-  Prepare the AARCH64 toolchain.

-  Build UEFI using Stratix 10 platform as configuration
   This will be updated to use an updated UEFI using the latest EDK2 source

.. code:: bash

       make CROSS_COMPILE=aarch64-linux-gnu- device=s10

-  Build atf providing the previously generated UEFI as the BL33 image

.. code:: bash

       make CROSS_COMPILE=aarch64-linux-gnu- bl2 fip PLAT=stratix10
       BL33=PEI.ROM

Install Procedure
~~~~~~~~~~~~~~~~~

- dd fip.bin to a A2 partition on the MMC drive to be booted in Stratix 10
  board.

- Generate a SOF containing bl2

.. code:: bash

        aarch64-linux-gnu-objcopy -I binary -O ihex --change-addresses 0xffe00000 bl2.bin bl2.hex
        quartus_cpf --bootloader bl2.hex <quartus_generated_sof> <output_sof_with_bl2>

- Configure SOF to board

.. code:: bash

        nios2-configure-sof <output_sof_with_bl2>

Boot trace
----------

::
         INFO:    DDR: DRAM calibration success.
         INFO:    ECC is disabled.
         INFO:    Init HPS NOC's DDR Scheduler.
         NOTICE:  BL2: v2.0(debug):v2.0-809-g7f8474a-dirty
         NOTICE:  BL2: Built : 17:38:19, Feb 18 2019
         INFO:    BL2: Doing platform setup
         INFO:    BL2: Loading image id 3
         INFO:    Loading image id=3 at address 0xffe1c000
         INFO:    Image id=3 loaded: 0xffe1c000 - 0xffe24034
         INFO:    BL2: Loading image id 5
         INFO:    Loading image id=5 at address 0x50000
         INFO:    Image id=5 loaded: 0x50000 - 0x550000
         NOTICE:  BL2: Booting BL31
         INFO:    Entry point address = 0xffe1c000
         INFO:    SPSR = 0x3cd
         NOTICE:  BL31: v2.0(debug):v2.0-810-g788c436-dirty
         NOTICE:  BL31: Built : 15:17:16, Feb 20 2019
         INFO:    ARM GICv2 driver initialized
         INFO:    BL31: Initializing runtime services
         WARNING: BL31: cortex_a53: CPU workaround for 855873 was missing!
         INFO:    BL31: Preparing for EL3 exit to normal world
         INFO:    Entry point address = 0x50000
         INFO:    SPSR = 0x3c9
         UEFI firmware (version 1.0 built at 11:26:18 on Nov  7 2018)
