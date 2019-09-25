AMB - AXI MBUS address decoding
===============================

AXI to M-bridge decoding unit driver for Marvell Armada 8K and 8K+ SoCs.

The Runit offers a second level of address windows lookup. It is used to map
transaction towards the CD BootROM, SPI0, SPI1 and Device bus (NOR).

The Runit contains eight configurable windows. Each window defines a contiguous,
address space and the properties associated with that address space.

::

  Unit        Bank         ATTR
  Device-Bus  DEV_BOOT_CS  0x2F
              DEV_CS0      0x3E
              DEV_CS1      0x3D
              DEV_CS2      0x3B
              DEV_CS3      0x37
  SPI-0       SPI_A_CS0    0x1E
              SPI_A_CS1    0x5E
              SPI_A_CS2    0x9E
              SPI_A_CS3    0xDE
              SPI_A_CS4    0x1F
              SPI_A_CS5    0x5F
              SPI_A_CS6    0x9F
              SPI_A_CS7    0xDF
  SPI         SPI_B_CS0    0x1A
              SPI_B_CS1    0x5A
              SPI_B_CS2    0x9A
              SPI_B_CS3    0xDA
  BOOT_ROM    BOOT_ROM     0x1D
  UART        UART         0x01

Mandatory functions
-------------------

- marvell_get_amb_memory_map
    Returns the AMB windows configuration and the number of windows

Mandatory structures
--------------------

- amb_memory_map
    Array that include the configuration of the windows. Every window/entry is a
    struct which has 2 parameters:

      - Base address of the window
      - Attribute of the window

Examples
--------

.. code:: c

    struct addr_map_win amb_memory_map[] = {
        {0xf900,	AMB_DEV_CS0_ID},
    };
