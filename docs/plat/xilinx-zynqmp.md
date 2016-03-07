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
*   `ZYNQMP_ATF_LOCATION`: Specifies the location of the bl31 binary. Options:
    -   `tsram` : bl31 will be located in OCM (default)
    -   `tdram` : bl31 will be located in DRAM (address: 0x30000000)

*   `ZYNQMP_TSP_RAM_LOCATION`: Specifies the location of the bl32 binary and
    secure payload dispatcher. Options:
    -   `tsram` : bl32/spd will be located in OCM (default)
    -   `tdram` : bl32/spd will be located in DRAM (address: 0x30000000)

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
