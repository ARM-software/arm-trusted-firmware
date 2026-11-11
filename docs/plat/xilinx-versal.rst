Xilinx Versal
=============

Trusted Firmware-A implements the EL3 firmware layer for Xilinx Versal.
The platform only uses the runtime part of TF-A as Xilinx Versal already has a
BootROM (BL1) and PMC FW (BL2).

BL31 is TF-A.
BL32 is an optional Secure Payload.
BL33 is the non-secure world software (U-Boot, Linux etc).

To build:

.. code:: bash

    make RESET_TO_BL31=1 CROSS_COMPILE=aarch64-none-elf- PLAT=versal bl31

To build bl32 TSP you have to rebuild bl31 too:

.. code:: bash

    make CROSS_COMPILE=aarch64-none-elf- PLAT=versal SPD=tspd RESET_TO_BL31=1 bl31 bl32

To build TF-A for JTAG DCC console:

.. code:: bash

    make RESET_TO_BL31=1 CROSS_COMPILE=aarch64-none-elf- PLAT=versal bl31 VERSAL_CONSOLE=dcc

To build TF-A with Errata management interface:

.. code:: bash

    make RESET_TO_BL31=1 CROSS_COMPILE=aarch64-none-elf- PLAT=versal bl31 ERRATA_ABI_SUPPORT=1

To build TF-A with Straight-Line Speculation (SLS):

.. code:: bash

    make RESET_TO_BL31=1 CROSS_COMPILE=aarch64-none-elf- PLAT=versal bl31 HARDEN_SLS_ALL=1

Xilinx Versal platform specific build options
---------------------------------------------

*   ``VERSAL_ATF_MEM_BASE``: Specifies the base address of the bl31 binary.
*   ``VERSAL_ATF_MEM_SIZE``: Specifies the size of the memory region of the bl31 binary.
*   ``VERSAL_BL32_MEM_BASE``: Specifies the base address of the bl32 binary.
*   ``VERSAL_BL32_MEM_SIZE``: Specifies the size of the memory region of the bl32 binary.
*   ``XILINX_OF_BOARD_DTB_ADDR``: Specifies the address of the device tree
    passed to OP-TEE when ``SPD=opteed``. For ``PLAT=versal``, BL31 runs from
    OCM (``0xfffe0000``) and the DTB address passed to OP-TEE is ``0x1000``.

*   ``VERSAL_CONSOLE``: Select the console driver. Options:

    -   ``pl011``, ``pl011_0``: ARM pl011 UART 0
    -   ``pl011_1``           : ARM pl011 UART 1

*   ``CPU_PWRDWN_SGI``: Select the SGI for triggering CPU power down request to
    secondary cores on receiving power down callback from firmware. Options:

    -   ``0``   : SGI 0
    -   ``1``   : SGI 1
    -   ``2``   : SGI 2
    -   ``3``   : SGI 3
    -   ``4``   : SGI 4
    -   ``5``   : SGI 5
    -   ``6``   : SGI 6 (Default)
    -   ``7``   : SGI 7

Versal Premium Gen 2 variant
----------------------------

``PLAT_VARIANT=PREMIUM_GEN2`` selects the Versal Premium Gen 2 build variant
of the Versal Gen 1 (``PLAT=versal``) port. All Versal Premium Gen 2 specific
overrides are isolated based on platform variant so ``PLAT=versal`` build
is unaffected when ``PLAT_VARIANT`` is left unset.

To build:

.. code-block:: shell

   make CROSS_COMPILE=aarch64-none-elf- PLAT=versal PLAT_VARIANT=PREMIUM_GEN2 bl31

Run from DDR only
~~~~~~~~~~~~~~~~~

- Versal Premium Gen 2 always executes BL31 out of the DDR "core runtime
  memory".

- This is enforced at compile time: if a custom ``VERSAL_ATF_MEM_BASE`` is
  supplied that would place BL31 in OCM, the build fails with an error
  stating that ``PLAT_VARIANT=PREMIUM_GEN2`` requires BL31 to execute from
  DDR and not OCM.

Transfer List (TL) support
~~~~~~~~~~~~~~~~~~~~~~~~~~

- Versal Premium Gen 2 replaces the device-tree handoff with the Transfer List
  implementation. TL support is disabled by default but can be enabled by
  setting ``TRANSFER_LIST=1`` through build arguments.

- When ``TRANSFER_LIST=1`` is enabled, dynamic translation tables
  (``PLAT_XLAT_TABLES_DYNAMIC=1``) are forced on so the extra transfer-list
  handoff windows in DDR can be mapped on demand.

- BL31 initialises the TL handoff from the firmware handoff window in DDR
  (``FW_HANDOFF_BASE`` / ``FW_HANDOFF_SIZE``), before the MMU is
  enabled, and populates the BL32/BL33 entry point info from the TL. If the TL
  carries no usable handoff entries, BL31 falls back to the build-time
  default configuration.

- A non-secure TL is created, and its BL33 entry point info is updated,
  before ``bl31_prepare_next_image_entry()`` runs. The overlay entries
  copied from the secure TL are populated afterwards, during
  ``bl31_plat_runtime_setup()``. Its base address defaults to 10 MB below
  ``PLAT_ARM_NS_IMAGE_BASE`` and can be overridden at build time with
  ``NS_FW_HANDOFF_BASE``.

Configurable Stack Size
-----------------------

The stack size in TF-A for the Versal platform is configurable.
The custom package can define the desired stack size as per the requirement in
the makefile as follows:

.. code:: bash

    PLATFORM_STACK_SIZE := <value>

    $(eval $(call add_define,PLATFORM_STACK_SIZE))

Configurable APU IPI ID
-----------------------

The APU IPI ID in TF-A for the Versal platform is configurable as per the design.

To build PLAT_IPI_ID_APU:

.. code:: bash

    make CROSS_COMPILE=aarch64-none-elf- PLAT=versal RESET_TO_BL31=1 bl31 PLAT_IPI_ID_APU=<value>

    $(eval $(call add_define,PLAT_IPI_ID_APU))

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

.. code:: bash

    make CROSS_COMPILE=aarch64-none-elf- PLAT=versal RESET_TO_BL31=1 bl31 CUSTOM_PKG_PATH=<...>

PLM->TF-A Parameter Passing
---------------------------

The PLM populates a data structure with image information for the TF-A. The TF-A
uses that data to hand off to the loaded images. The address of the handoff
data structure is passed in the ``PMC_GLOBAL_GLOB_GEN_STORAGE4`` register.
The register is free to be used by other software once the TF-A is bringing up
further firmware images.

Reference DEN0028E SMC calling convention
------------------------------------------

Allocated subranges of Function Identifier to SIP services
----------------------------------------------------------

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
----------------------------------------

+--------------+--------------+--------------+
|  Service     | Call UID     | Revision     |
+--------------+--------------+--------------+
|  SiP Service | 0x8200_FF01  | 0x8200_FF03  |
+--------------+--------------+--------------+

Call UID Query – Returns a unique identifier of the service provider.

Revision Query – Returns revision details of the service implementor.
