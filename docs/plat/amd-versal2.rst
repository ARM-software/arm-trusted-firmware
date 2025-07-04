AMD Versal Gen 2
================

Trusted Firmware-A implements the EL3 firmware layer for AMD Versal Gen 2.
The platform only uses the runtime part of TF-A as AMD Versal Gen 2 already
has a BootROM (BL1) and PMC FW (BL2).

BL31 is TF-A.
BL32 is an optional Secure Payload.
BL33 is the non-secure world software (U-Boot, Linux etc).

To build:
```bash
make RESET_TO_BL31=1 CROSS_COMPILE=aarch64-none-elf- PLAT=versal2 bl31
```

To build TF-A for JTAG DCC console:
```bash
make RESET_TO_BL31=1 CROSS_COMPILE=aarch64-none-elf- PLAT=versal2 CONSOLE=dcc bl31
```

To build TF-A with Errata management interface
```bash
make RESET_TO_BL31=1 CROSS_COMPILE=aarch64-none-elf- PLAT=versal2 bl31 ERRATA_ABI_SUPPORT=1
```

To build TF-A with IPI CRC check:
```bash
make RESET_TO_BL31=1 CROSS_COMPILE=aarch64-none-elf- PLAT=versal2 bl31 IPI_CRC_CHECK=1
```

AMD Versal Gen 2 platform specific build options
-------------------------------------------------

*   `MEM_BASE`: Specifies the base address of the bl31 binary.
*   `MEM_SIZE`: Specifies the size of the memory region of the bl31 binary.
*   `BL32_MEM_BASE`: Specifies the base address of the bl32 binary.
*   `BL32_MEM_SIZE`: Specifies the size of the memory region of the bl32 binary.

*   `CONSOLE`: Select the console driver. Options:
    -   `pl011`, `pl011_0`: ARM pl011 UART 0 (default)
    -   `pl011_1`         : ARM pl011 UART 1
    -   `dcc`             : JTAG Debug Communication Channel(DCC)

Configurable Stack Size
-----------------------

The stack size in TF-A for the Versal platform is configurable.
The custom package can define the desired stack size as per the requirement in
the makefile as follows:

.. code-block:: shell

    PLATFORM_STACK_SIZE := <value>

    $(eval $(call add_define,PLATFORM_STACK_SIZE))

CUSTOM SIP Service Support
--------------------------

- Dedicated SMC FID ``SOC_SIP_SVC_CUSTOM(0x82002000)`` (32-bit) /
  ``(0xC2002000)`` (64-bit) is used by a custom package for providing
  CUSTOM SIP service.

- By default, the platform provides a bare minimum definition for
  ``custom_smc_handler`` in this service.

- To use this service, the custom package should implement its own SMC handler
  named ``custom_smc_handler``. Once the custom package is included in the
  TF-A build, its definition of ``custom_smc_handler`` is enabled.

Custom Package Makefile Fragment Inclusion in TF-A Build
--------------------------------------------------------

- Custom package is not directly part of the TF-A source.

- ``<CUSTOM_PKG_PATH>`` is the location where the user clones a
  custom package locally.

- The custom package must implement a makefile fragment named
  ``custom_pkg.mk`` so it can be included in the TF-A build.

- ``custom_pkg.mk`` should specify all the rules to include custom package
  specific header files, dependent libraries, and source files that are
  required to be part of the TF-A build.

- When ``<CUSTOM_PKG_PATH>`` is specified in the TF-A build command,
  ``custom_pkg.mk`` is included from ``<CUSTOM_PKG_PATH>``.

- Example TF-A build command:

.. code-block:: shell

    make CROSS_COMPILE=aarch64-none-elf- PLAT=versal2 RESET_TO_BL31=1 bl31 CUSTOM_PKG_PATH=<...>

Reference DEN0028E SMC calling convention
------------------------------------------

Allocated subranges of Function Identifier to SIP services
------------------------------------------------------------

+-----------------------+-------------------------------------------------------+
|    SMC Function       | Identifier Service type                               |
+-----------------------+-------------------------------------------------------+
| 0xC2000000-0xC200FFFF | Fast SMC64 SiP Service Calls as per SMCCC Section 6.1 |
+-----------------------+-------------------------------------------------------+

IPI SMC call ranges
-------------------

+---------------------------+-----------------------------------------------------------+
| SMC Function Identifier   | Service type                                              |
+---------------------------+-----------------------------------------------------------+
| 0xc2001000-0xc2001FFF     | Fast SMC64 SiP Service call range used for AMD-Xilinx IPI |
+---------------------------+-----------------------------------------------------------+

PM SMC call ranges
------------------

+---------------------------+---------------------------------------------------------------------------+
| SMC Function Identifier   |  Service type                                                             |
+---------------------------+---------------------------------------------------------------------------+
| 0xc2000000-0xc2000FFF     | Fast SMC64 SiP Service call range used for AMD-Xilinx Platform Management |
+---------------------------+---------------------------------------------------------------------------+

SMC function IDs for SiP Service queries
----------------------------------------

+--------------+--------------+--------------+
|  Service     | Call UID     | Revision     |
+--------------+--------------+--------------+
|  SiP Service | 0x8200_FF01  | 0x8200_FF03  |
+--------------+--------------+--------------+

Call UID Query – Returns a unique identifier of the service provider.

Revision Query – Returns revision details of the service implementor.
