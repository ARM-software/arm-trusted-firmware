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
