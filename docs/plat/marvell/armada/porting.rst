TF-A Porting Guide for Marvell Platforms
========================================

This section describes how to port TF-A to a customer board, assuming that the
SoC being used is already supported in TF-A.


Source Code Structure
---------------------

- The customer platform specific code shall reside under ``plat/marvell/armada/<soc family>/<soc>_cust``
  (e.g. 'plat/marvell/armada/a8k/a7040_cust').
- The platform name for build purposes is called ``<soc>_cust`` (e.g. ``a7040_cust``).
- The build system will reuse all files from within the soc directory, and take only the porting
  files from the customer platform directory.

Files that require porting are located at ``plat/marvell/armada/<soc family>/<soc>_cust`` directory.


Armada-70x0/Armada-80x0 Porting
-------------------------------

SoC Physical Address Map (marvell_plat_config.c)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

This file describes the SoC physical memory mapping to be used for the CCU,
IOWIN, AXI-MBUS and IOB address decode units (Refer to the functional spec for
more details).

In most cases, using the default address decode windows should work OK.

In cases where a special physical address map is needed (e.g. Special size for
PCIe MEM windows, large memory mapped SPI flash...), then porting of the SoC
memory map is required.

.. note::
   For a detailed information on how CCU, IOWIN, AXI-MBUS & IOB work, please
   refer to the SoC functional spec, and under
   ``docs/plat/marvell/armada/misc/mvebu-[ccu/iob/amb/io-win].rst`` files.

boot loader recovery (marvell_plat_config.c)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

- Background:

  Boot rom can skip the current image and choose to boot from next position if a
  specific value (``0xDEADB002``) is returned by the ble main function. This
  feature is used for boot loader recovery by booting from a valid flash-image
  saved in next position on flash (e.g. address 2M in SPI flash).

  Supported options to implement the skip request are:
    - GPIO
    - I2C
    - User defined

- Porting:

  Under marvell_plat_config.c, implement struct skip_image that includes
  specific board parameters.

  .. warning::
     To disable this feature make sure the struct skip_image is not implemented.

- Example:

In A7040-DB specific implementation
(``plat/marvell/armada/a8k/a70x0/board/marvell_plat_config.c``), the image skip is
implemented using GPIO: mpp 33 (SW5).

Before resetting the board make sure there is a valid image on the next flash
address:

 -tftp [valid address] flash-image.bin
 -sf update [valid address] 0x2000000 [size]

Press reset and keep pressing the button connected to the chosen GPIO pin. A
skip image request message is printed on the screen and boot rom boots from the
saved image at the next position.

DDR Porting (dram_port.c)
~~~~~~~~~~~~~~~~~~~~~~~~~

This file defines the dram topology and parameters of the target board.

The DDR code is part of the BLE component, which is an extension of ARM Trusted
Firmware (TF-A).

The DDR driver called mv_ddr is released separately apart from TF-A sources.

The BLE and consequently, the DDR init code is executed at the early stage of
the boot process.

Each supported platform of the TF-A has its own DDR porting file called
dram_port.c located at ``atf/plat/marvell/armada/a8k/<platform>/board`` directory.

Please refer to '<path_to_mv_ddr_sources>/doc/porting_guide.txt' for detailed
porting description.

The build target directory is "build/<platform>/release/ble".

Comphy Porting (phy-porting-layer.h or phy-default-porting-layer.h)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

- Background:
    Some of the comphy's parameters value depend on the HW connection between
    the SoC and the PHY. Every board type has specific HW characteristics like
    wire length. Due to those differences some comphy parameters vary between
    board types. Therefore each board type can have its own list of values for
    all relevant comphy parameters. The PHY porting layer specifies which
    parameters need to be suited and the board designer should provide relevant
    values.

    The PHY porting layer simplifies updating static values per board type,
    which are now grouped in one place.

    .. note::
        The parameters for the same type of comphy may vary even for the same
        board type, it is because the lanes from comphy-x to some PHY may have
        different HW characteristic than lanes from comphy-y to the same
        (multiplexed) or other PHY.

- Porting:
    The porting layer for PHY was introduced in TF-A. There is one file
    ``drivers/marvell/comphy/phy-default-porting-layer.h`` which contains the
    defaults. Those default parameters are used only if there is no appropriate
    phy-porting-layer.h file under: ``plat/marvell/armada/<soc
    family>/<platform>/board/phy-porting-layer.h``. If the phy-porting-layer.h
    exists, the phy-default-porting-layer.h is not going to be included.

    .. warning::
        Not all comphy types are already reworked to support the PHY porting
        layer, currently the porting layer is supported for XFI/SFI and SATA
        comphy types.

    The easiest way to prepare the PHY porting layer for custom board is to copy
    existing example to a new platform:

    - cp ``plat/marvell/armada/a8k/a80x0/board/phy-porting-layer.h`` "plat/marvell/armada/<soc family>/<platform>/board/phy-porting-layer.h"
    - adjust relevant parameters or
    - if different comphy index is used for specific feature, move it to proper table entry and then adjust.

    .. note::
        The final table size with comphy parameters can be different, depending
        on the CP module count for given SoC type.

- Example:
    Example porting layer for armada-8040-db is under:
    ``plat/marvell/armada/a8k/a80x0/board/phy-porting-layer.h``

    .. note::
        If there is no PHY porting layer for new platform (missing
        phy-porting-layer.h), the default values are used
        (drivers/marvell/comphy/phy-default-porting-layer.h) and the user is
        warned:

    .. warning::
        "Using default comphy parameters - it may be required to suit them for
        your board".
