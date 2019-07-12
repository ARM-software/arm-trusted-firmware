Marvell IOB address decoding bindings
=====================================

IO bridge configration driver (3rd stage address translation) for Marvell Armada 8K and 8K+ SoCs.

The IOB includes a description of the address decoding configuration.

IOB supports up to n (in CP110 n=24) windows for external memory transaction.
When a transaction passes through the IOB, its address is compared to each of
the enabled windows. If there is a hit and it passes the security checks, it is
advanced to the target port.

Mandatory functions
-------------------

- marvell_get_iob_memory_map
     Returns the IOB windows configuration and the number of windows

Mandatory structures
--------------------

- iob_memory_map
     Array that includes the configuration of the windows. Every window/entry is
     a struct which has 3 parameters:

       - Base address of the window
       - Size of the window
       - Target-ID of the window

Target ID options
-----------------

- **0x0** = Internal configuration space
- **0x1** = MCI0
- **0x2** = PEX1_X1
- **0x3** = PEX2_X1
- **0x4** = PEX0_X4
- **0x5** = NAND flash
- **0x6** = RUNIT (NOR/SPI/BootRoom)
- **0x7** = MCI1

Example
-------

.. code:: c

	struct addr_map_win iob_memory_map[] = {
		{0x00000000f7000000,	0x0000000001000000,	PEX1_TID}, /* PEX1_X1 window */
		{0x00000000f8000000,	0x0000000001000000,	PEX2_TID}, /* PEX2_X1 window */
		{0x00000000f6000000,	0x0000000001000000,	PEX0_TID}, /* PEX0_X4 window */
		{0x00000000f9000000,	0x0000000001000000,	NAND_TID}  /* NAND window */
	};
