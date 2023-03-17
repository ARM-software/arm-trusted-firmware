STM32 MPUs
==========

STM32 MPUs are microprocessors designed by STMicroelectronics
based on Arm Cortex-A. This page presents the common configuration of STM32
MPUs, more details and dedicated configuration can be found in each STM32 MPU
page (:ref:`STM32MP1` or :ref:`STM32MP2`)

Design
------
The STM32 MPU resets in the ROM code of the Cortex-A.
The primary boot core (core 0) executes the boot sequence while
secondary boot core (core 1) is kept in a holding pen loop.
The ROM code boot sequence loads the TF-A binary image from boot device
to embedded SRAM.

The TF-A image must be properly formatted with a STM32 header structure
for ROM code is able to load this image.
Tool stm32image can be used to prepend this header to the generated TF-A binary.

Boot
~~~~
Only BL2 (with STM32 header) is loaded by ROM code. The other binaries are
inside the FIP binary: BL31 (for Aarch64 platforms), BL32 (OP-TEE), U-Boot
and their respective device tree blobs.

Boot sequence
~~~~~~~~~~~~~

ROM code -> BL2 (compiled with RESET_TO_BL2) -> OP-TEE -> BL33 (U-Boot)

Build Instructions
------------------
Boot media(s) supported by BL2 must be specified in the build command.
Available storage medias are:

- ``STM32MP_SDMMC``
- ``STM32MP_EMMC``
- ``STM32MP_RAW_NAND``
- ``STM32MP_SPI_NAND``
- ``STM32MP_SPI_NOR``

Serial boot devices:

- ``STM32MP_UART_PROGRAMMER``
- ``STM32MP_USB_PROGRAMMER``


Other configuration flags:

- | ``DTB_FILE_NAME``: to precise board device-tree blob to be used.
  | Default: stm32mp157c-ev1.dtb
- | ``DWL_BUFFER_BASE``: the 'serial boot' load address of FIP,
  | default location (end of the first 128MB) is used when absent
- | ``STM32MP_EARLY_CONSOLE``: to enable early traces before clock driver is setup.
  | Default: 0 (disabled)
- | ``STM32MP_RECONFIGURE_CONSOLE``: to re-configure crash console (especially after BL2).
  | Default: 0 (disabled)
- | ``STM32MP_UART_BAUDRATE``: to select UART baud rate.
  | Default: 115200


Populate SD-card
----------------

Boot with FIP
~~~~~~~~~~~~~
The SD-card has to be formatted with GPT.
It should contain at least those partitions:

- fsbl: to copy the tf-a-stm32mp157c-ev1.stm32 binary (BL2)
- fip (GUID 19d5df83-11b0-457b-be2c-7559c13142a5): which contains the FIP binary

Usually, two copies of fsbl are used (fsbl1 and fsbl2) instead of one partition fsbl.

--------------

*Copyright (c) 2023, STMicroelectronics - All Rights Reserved*
