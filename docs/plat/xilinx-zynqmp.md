ARM Trusted Firmware for Xilinx Zynq UltraScale+ MPSoC
================================

ARM Trusted Firmware implements the EL3 firmware layer for Xilinx Zynq
UltraScale + MPSoC.
The platform only uses the runtime part of ATF as ZynqMP already has a
BootROM (BL1) and FSBL (BL2).

BL31 is ATF.  
BL32 is an optional Secure Payload.  
BL33 is the non-secure world software (U-Boot, Linux etc).  

To build:
```bash
make ERROR_DEPRECATED=1 RESET_TO_BL31=1 CROSS_COMPILE=aarch64-none-elf- PLAT=zynqmp bl31
```

To build bl32 TSP you have to rebuild bl31 too:
```bash
make ERROR_DEPRECATED=1 RESET_TO_BL31=1 CROSS_COMPILE=aarch64-none-elf- PLAT=zynqmp SPD=tspd bl31 bl32
```

# ZynqMP platform specific build options
*   `ZYNQMP_ATF_MEM_BASE`: Specifies the base address of the bl31 binary.
*   `ZYNQMP_ATF_MEM_SIZE`: Specifies the size of the memory region of the bl31 binary.
*   `ZYNQMP_BL32_MEM_BASE`: Specifies the base address of the bl32 binary.
*   `ZYNQMP_BL32_MEM_SIZE`: Specifies the size of the memory region of the bl32 binary.

*   `ZYNQMP_CONSOLE`: Select the console driver. Options:
    -   `cadence`, `cadence0`: Cadence UART 0
    -   `cadence1`           : Cadence UART 1

# FSBL->ATF Parameter Passing
The FSBL populates a data structure with image information for the ATF. The ATF
uses that data to hand off to the loaded images. The address of the handoff data
structure is passed in the ```PMU_GLOBAL.GLOBAL_GEN_STORAGE6``` register. The
register is free to be used by other software once the ATF is bringing up
further firmware images.

# Power Domain Tree
The following power domain tree represents the power domain model used by the
ATF for ZynqMP:
```
                +-+
                |0|
                +-+
     +-------+---+---+-------+
     |       |       |       |
     |       |       |       |
     v       v       v       v
    +-+     +-+     +-+     +-+
    |0|     |1|     |2|     |3|
    +-+     +-+     +-+     +-+
```
The 4 leaf power domains represent the individual A53 cores, while resources
common to the cluster are grouped in the power domain on the top.
