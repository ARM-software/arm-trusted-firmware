Intel Agilex SoCFPGA
========================

Agilex SoCFPGA is a FPGA with integrated quad-core 64-bit Arm Cortex A53 processor.

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

-  Build UEFI using Agilex platform as configuration
   This will be updated to use an updated UEFI using the latest EDK2 source

.. code:: bash

       make CROSS_COMPILE=aarch64-linux-gnu- device=agx

-  Build atf providing the previously generated UEFI as the BL33 image

.. code:: bash

       make CROSS_COMPILE=aarch64-linux-gnu- bl2 fip PLAT=agilex
       BL33=PEI.ROM

Install Procedure
~~~~~~~~~~~~~~~~~

- dd fip.bin to a A2 partition on the MMC drive to be booted in Agilex
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
        NOTICE:  BL2: v2.1(debug)
        NOTICE:  BL2: Built
        INFO:    BL2: Doing platform setup
        NOTICE:  BL2: Booting BL31
        INFO:    Entry point address = 0xffe1c000
        INFO:    SPSR = 0x3cd
        NOTICE:  BL31: v2.1(debug)
        NOTICE:  BL31: Built
        INFO:    ARM GICv2 driver initialized
        INFO:    BL31: Initializing runtime services
        WARNING: BL31: cortex_a53
        INFO:    BL31: Preparing for EL3 exit to normal world
        INFO:    Entry point address = 0x50000
        INFO:    SPSR = 0x3c9
