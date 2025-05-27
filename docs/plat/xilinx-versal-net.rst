Xilinx Versal NET
=================

Trusted Firmware-A implements the EL3 firmware layer for Xilinx Versal NET.
The platform only uses the runtime part of TF-A as Xilinx Versal NET already
has a BootROM (BL1) and PMC FW (BL2).

BL31 is TF-A.
BL32 is an optional Secure Payload.
BL33 is the non-secure world software (U-Boot, Linux etc).

To build:
```bash
make RESET_TO_BL31=1 CROSS_COMPILE=aarch64-none-elf- PLAT=versal_net bl31
```

To build bl32 TSP you have to rebuild bl31 too
```bash
make CROSS_COMPILE=aarch64-none-elf- PLAT=versal_net SPD=tspd RESET_TO_BL31=1 bl31 bl32
```

To build TF-A for JTAG DCC console:
```bash
make RESET_TO_BL31=1 CROSS_COMPILE=aarch64-none-elf- PLAT=versal_net VERSAL_NET_CONSOLE=dcc bl31
```

To build TF-A with SDEI_SUPPORT:
```bash
make RESET_TO_BL31=1 CROSS_COMPILE=aarch64-none-elf- PLAT=versal_net SDEI_SUPPORT=1 bl31
```

Xilinx Versal NET platform specific build options
-------------------------------------------------

*   `VERSAL_NET_ATF_MEM_BASE`: Specifies the base address of the bl31 binary.
*   `VERSAL_NET_ATF_MEM_SIZE`: Specifies the size of the memory region of the bl31 binary.
*   `VERSAL_NET_BL32_MEM_BASE`: Specifies the base address of the bl32 binary.
*   `VERSAL_NET_BL32_MEM_SIZE`: Specifies the size of the memory region of the bl32 binary.

*   `VERSAL_NET_CONSOLE`: Select the console driver. Options:
    -   `pl011`, `pl011_0`: ARM pl011 UART 0 (default)
    -   `pl011_1`         : ARM pl011 UART 1
    -   `dcc`             : JTAG Debug Communication Channel(DCC)

*   `TFA_NO_PM` : Platform Management support.
    -    0 : Enable Platform Management (Default)
    -    1 : Disable Platform Management

*   `CPU_PWRDWN_SGI`: Select the SGI for triggering CPU power down request to
                      secondary cores on receiving power down callback from
                      firmware. Options:

    -   `0`   : SGI 0
    -   `1`   : SGI 1
    -   `2`   : SGI 2
    -   `3`   : SGI 3
    -   `4`   : SGI 4
    -   `5`   : SGI 5
    -   `6`   : SGI 6 (Default)
    -   `7`   : SGI 7

Reference DEN0028E SMC calling convention
------------------------------------------

Allocated subranges of Function Identifier to SIP services
-----------------------------------------------------------

+-----------------------+-------------------------------------------------------+
|    SMC Function       | Identifier Service type                               |
+-----------------------+------------------------------+------------------------+
| 0xC2000000-0xC200FFFF | Fast SMC64 SiP Service Calls as per SMCCC Section 6.1 |
+-----------------------+-------------------------------------------------------+

IPI SMC call ranges
-------------------------------------------------------------

+---------------------------+-----------------------------------------------------------+
| SMC Function Identifier   | Service type                                              |
+---------------------------+-----------------------------------------------------------+
| 0xc2001000-0xc2001FFF     | Fast SMC64 SiP Service call range used for AMD-Xilinx IPI |
+---------------------------+-----------------------------------------------------------+

PM SMC call ranges for SiP SVC version 0.1
--------------------------------------------------------

+---------------------------+---------------------------------------------------------------------------+
|   SMC Function Identifier |  Service type                                                             |
+---------------------------+---------------------------------------------------------------------------+
| 0xc2000000-0xc2000FFF     | Fast SMC64 SiP Service call range used for AMD-Xilinx Platform Management |
+---------------------------+---------------------------------------------------------------------------+

PM SMC call ranges for SiP SVC version 0.2
--------------------------------------------------------

+---------------------------+---------------------------------------------------------------------------+
|   SMC Function Identifier |  Service type                                                             |
+---------------------------+---------------------------------------------------------------------------+
| 0xc2000FFF                | Fast SMC64 SiP Service call used for pass-through of AMD-Xilinx Platform  |
|                           | Management APIs to firmware                                               |
+---------------------------+---------------------------------------------------------------------------+
| 0xc2000A00-0xc2000AFF     | Fast SMC64 SiP Service call range used for AMD-Xilinx Platform Management |
|                           | specific TF-A APIs                                                        |
+---------------------------+---------------------------------------------------------------------------+

SMC function IDs for SiP Service queries
----------------------------------------------

+--------------+--------------+--------------+
|  Service     | Call UID     | Revision     |
+--------------+--------------+--------------+
|  SiP Service | 0x8200_FF01  | 0x8200_FF03  |
+--------------+--------------+--------------+

Call UID Query – Returns a unique identifier of the service provider.

Revision Query – Returns revision details of the service implementor.
