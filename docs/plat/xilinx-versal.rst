Xilinx Versal
=============

Trusted Firmware-A implements the EL3 firmware layer for Xilinx Versal.
The platform only uses the runtime part of TF-A as Xilinx Versal already has a
BootROM (BL1) and PMC FW (BL2).

BL31 is TF-A.
BL32 is an optional Secure Payload.
BL33 is the non-secure world software (U-Boot, Linux etc).

To build:
```bash
make RESET_TO_BL31=1 CROSS_COMPILE=aarch64-none-elf- PLAT=versal bl31
```

To build ATF for different platform (supported are "silicon"(default) and "versal_virt")
```bash
make RESET_TO_BL31=1 CROSS_COMPILE=aarch64-none-elf- PLAT=versal VERSAL_PLATFORM=versal_virt bl31
```

To build TF-A for JTAG DCC console
```bash
make RESET_TO_BL31=1 CROSS_COMPILE=aarch64-none-elf- PLAT=versal bl31 VERSAL_CONSOLE=dcc
```

To build TF-A with Straight-Line Speculation(SLS)
```bash
make RESET_TO_BL31=1 CROSS_COMPILE=aarch64-none-elf- PLAT=versal bl31 HARDEN_SLS_ALL=1
```

Xilinx Versal platform specific build options
---------------------------------------------

*   `VERSAL_ATF_MEM_BASE`: Specifies the base address of the bl31 binary.
*   `VERSAL_ATF_MEM_SIZE`: Specifies the size of the memory region of the bl31 binary.
*   `VERSAL_BL32_MEM_BASE`: Specifies the base address of the bl32 binary.
*   `VERSAL_BL32_MEM_SIZE`: Specifies the size of the memory region of the bl32 binary.

*   `VERSAL_CONSOLE`: Select the console driver. Options:
    -   `pl011`, `pl011_0`: ARM pl011 UART 0
    -   `pl011_1`         : ARM pl011 UART 1

*   `VERSAL_PLATFORM`: Select the platform. Options:
    -   `versal_virt`	: Versal Virtual platform
    -   `spp_itr6`	: SPP ITR6
    -   `emu_itr6`	: EMU ITR6

# PLM->TF-A Parameter Passing
------------------------------
The PLM populates a data structure with image information for the TF-A. The TF-A
uses that data to hand off to the loaded images. The address of the handoff
data structure is passed in the ```PMC_GLOBAL_GLOB_GEN_STORAGE4``` register.
The register is free to be used by other software once the TF-A is bringing up
further firmware images.
