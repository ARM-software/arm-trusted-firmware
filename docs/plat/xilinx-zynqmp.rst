Xilinx Zynq UltraScale+ MPSoC
=============================

Trusted Firmware-A (TF-A) implements the EL3 firmware layer for Xilinx Zynq
UltraScale + MPSoC.
The platform only uses the runtime part of TF-A as ZynqMP already has a
BootROM (BL1) and FSBL (BL2).

BL31 is TF-A.
BL32 is an optional Secure Payload.
BL33 is the non-secure world software (U-Boot, Linux etc).

To build:

.. code:: bash

    make CROSS_COMPILE=aarch64-none-elf- PLAT=zynqmp RESET_TO_BL31=1 bl31

To build bl32 TSP you have to rebuild bl31 too:

.. code:: bash

    make CROSS_COMPILE=aarch64-none-elf- PLAT=zynqmp SPD=tspd RESET_TO_BL31=1 bl31 bl32

To build TF-A for JTAG DCC console:

.. code:: bash

    make CROSS_COMPILE=aarch64-none-elf- PLAT=zynqmp RESET_TO_BL31=1 bl31 ZYNQMP_CONSOLE=dcc

ZynqMP platform specific build options
--------------------------------------

-  ``XILINX_OF_BOARD_DTB_ADDR`` : Specifies the base address of Device tree.
-  ``ZYNQMP_ATF_MEM_BASE``: Specifies the base address of the bl31 binary.
-  ``ZYNQMP_ATF_MEM_SIZE``: Specifies the size of the memory region of the bl31 binary.
-  ``ZYNQMP_BL32_MEM_BASE``: Specifies the base address of the bl32 binary.
-  ``ZYNQMP_BL32_MEM_SIZE``: Specifies the size of the memory region of the bl32 binary.

-  ``ZYNQMP_CONSOLE``: Select the console driver. Options:

   -  ``cadence``, ``cadence0``: Cadence UART 0
   -  ``cadence1`` : Cadence UART 1

ZynqMP Debug behavior
---------------------

With DEBUG=1, TF-A for ZynqMP uses DDR memory range instead of OCM memory range
due to size constraints.
For DEBUG=1 configuration for ZynqMP the BL31_BASE is set to the DDR location
of 0x1000 and BL31_LIMIT is set to DDR location of 0x7FFFF. By default the
above memory range will NOT be reserved in device tree.

To reserve the above memory range in device tree, the device tree base address
must be provided during build as,

make CROSS_COMPILE=aarch64-none-elf- PLAT=zynqmp RESET_TO_BL31=1 DEBUG=1 \
       XILINX_OF_BOARD_DTB_ADDR=<DTB address> bl31

The default DTB base address for ZynqMP platform is 0x100000. This default value
is not set in the code and to use this default address, user still needs to
provide it through the build command as above.

If the user wants to move the bl31 to a different DDR location, user can provide
the DDR address location using the build time parameters ZYNQMP_ATF_MEM_BASE and
ZYNQMP_ATF_MEM_SIZE.

The DDR address must be reserved in the DTB by the user, either by manually
adding the reserved memory node, in the device tree, with the required address
range OR let TF-A modify the device tree on the run.

To let TF-A access and modify the device tree, the DTB address must be provided
to the build command as follows,

make CROSS_COMPILE=aarch64-none-elf- PLAT=zynqmp RESET_TO_BL31=1 DEBUG=1 \
	ZYNQMP_ATF_MEM_BASE=<DDR address> ZYNQMP_ATF_MEM_SIZE=<size> \
	XILINX_OF_BOARD_DTB_ADDR=<DTB address> bl31

DDR Address Range Usage
-----------------------

When FSBL runs on RPU and TF-A is to be placed in DDR address range,
then the user needs to make sure that the DDR address is beyond 256KB.
In the RPU view, the first 256 KB is TCM memory.

For this use case, with the minimum base address in DDR for TF-A,
the build command example is;

make CROSS_COMPILE=aarch64-none-elf- PLAT=zynqmp RESET_TO_BL31=1 DEBUG=1 \
	ZYNQMP_ATF_MEM_BASE=0x40000 ZYNQMP_ATF_MEM_SIZE=<size>

Configurable Stack Size
-----------------------

The stack size in TF-A for ZynqMP platform is configurable.
The custom package can define the desired stack size as per the requirement in
the make file as follows,

PLATFORM_STACK_SIZE := <value>
$(eval $(call add_define,PLATFORM_STACK_SIZE))

FSBL->TF-A Parameter Passing
----------------------------

The FSBL populates a data structure with image information for TF-A. TF-A uses
that data to hand off to the loaded images. The address of the handoff data
structure is passed in the ``PMU_GLOBAL.GLOBAL_GEN_STORAGE6`` register. The
register is free to be used by other software once TF-A has brought up
further firmware images.

Power Domain Tree
-----------------

The following power domain tree represents the power domain model used by TF-A
for ZynqMP:

::

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

The 4 leaf power domains represent the individual A53 cores, while resources
common to the cluster are grouped in the power domain on the top.

CUSTOM SIP service support
--------------------------

- Dedicated SMC FID ZYNQMP_SIP_SVC_CUSTOM(0x82002000)(32-bit)/
  (0xC2002000)(64-bit) to be used by a custom package for
  providing CUSTOM SIP service.

- by default platform provides bare minimum definition for
  custom_smc_handler in this service.

- to use this service, custom package should implement their
  smc handler with the name custom_smc_handler. once custom package is
  included in TF-A build, their definition of custom_smc_handler is
  enabled.

Custom package makefile fragment inclusion in TF-A build
--------------------------------------------------------

- custom package is not directly part of TF-A source.

- <CUSTOM_PKG_PATH> is the location at which user clones a
  custom package locally.

- custom package needs to implement makefile fragment named
  custom_pkg.mk so as to get included in TF-A build.

- custom_pkg.mk specify all the rules to include custom package
  specific header files, dependent libs, source files that are
  supposed to be included in TF-A build.

- when <CUSTOM_PKG_PATH> is specified in TF-A build command,
  custom_pkg.mk is included from <CUSTOM_PKG_PATH> in TF-A build.

- TF-A build command:
  make CROSS_COMPILE=aarch64-none-elf- PLAT=zynqmp RESET_TO_BL31=1
  bl31 CUSTOM_PKG_PATH=<...>
