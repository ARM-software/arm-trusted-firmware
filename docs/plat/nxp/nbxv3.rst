Nodebox v3 (nbxv3) CPU Module
=============================

The Nodebox v3 (``nbxv3``) is a COM-HPC Server CPU module built on the
NXP LX2160A (16x Cortex-A72) for baremetal 100GE packet processing with
Class C clock management for telecom applications.

The platform port lives entirely under ``plat/nxp/soc-lx2160a/nbxv3/``
and reuses the shared LX2160A SoC layer. Board-specific behaviour is
kept out of the common ``soc.c`` through ``plat_soc_early_init_hook()``
and the weak board hooks.

Boot source
-----------

Production boot is FlexSPI NOR only (GigaDevice GD55LB02GF, 256 MiB
xSPI NOR, JEDEC ``C8 60 1C``). The console is UART1 at ``0x021C0000``,
115200 8N1, no flow control. SD / eMMC are not wired.

Two BL2 variants
----------------

A single source tree produces two BL2 images, selected at build time by
``NBXV3_BL2_BOOTSTRAP``:

- **Production XIP BL2** (default, ``NBXV3_BL2_BOOTSTRAP=0``):
  ``.text`` / ``.rodata`` execute in place from the FlexSPI NOR AHB
  window, embedded in the PBL at NOR offset ``0x9000``. ``.data`` /
  ``.bss`` / stack live in OCRAM. XIP avoids the LX2160A
  multi-bit-ECC trap on uninitialised OCRAM during the Service
  Processor's Block-Copy.

- **Bootstrap BL2** (``NBXV3_BL2_BOOTSTRAP=1``): OCRAM-resident,
  non-XIP. Loaded into OCRAM by a JTAG probe (OpenOCD) while the SoC
  runs the ROM-fallback RCW; used only for first-time / recovery NOR
  provisioning.

Unified runtime flash bootstrap
-------------------------------

Both factory-fresh and provisioned boards run the same BL2. The mode is
decided **at runtime** by reading ``DCFG_RCWSR0.MEM_PLL_RAT``: a value
below the DDR4-1600 DLL-lock minimum (16) means the Service Processor
fell back to a ROM hard-coded RCW, so the NOR is blank or corrupt and
provisioning is required. Otherwise the production RCW is live and BL2
proceeds normally.

::

   power-on / reset
          |
          v
   +-------------------------+
   | Service Processor loads |
   | RCW from NOR (or ROM    |
   | fallback if NOR blank)  |
   +-------------------------+
          |
          v
   BL2: plat_soc_early_init_hook()
   read DCFG_RCWSR0.MEM_PLL_RAT
          |
          +-----------------------------< MEM_PLL_RAT >= 16 >----------------+
          |                                (production RCW live)             |
          | MEM_PLL_RAT < 16                                                 |
          | (ROM fallback: NOR blank/corrupt)                                |
          v                                                                  v
   ====== BOOTSTRAP MODE (JTAG semihosting) ======            ====== PRODUCTION MODE ======
   open flash_manifest.json over ARM semihosting               bootstrap returns silently
          |                                                            |
          v                                                            v
   for each manifest region:                                   DDR init (SPD over I2C)
     - erase NOR slot                                                  |
     - stream file from JTAG host -> NOR                               v
     - SHA-256 verify via NOR AHB window                        load FIP (BL31 + BL33)
          |                                                            |
          v                                                            v
   SW_RST_REQ  ->  true POR                                     hand off to BL31 in DRAM
          |
          +--> board reboots; SP now loads the freshly
               written production RCW -> PRODUCTION MODE

The provisioning set is data-driven by ``flash_manifest.json`` (resolved
over the JTAG probe's semihosting stdio), not by C code. The manifest can
stage just the boot ROM's RCW (PBL), or a full set: PBL + FIP + DDR-FIP +
U-Boot + kernel + ramdisk. Smaller manifests provision faster over the
JTAG semihosting link. The schema is documented in
``plat/nxp/soc-lx2160a/nbxv3/MANIFEST.md``.

Board policy deviations from the LX2160A reference boards
---------------------------------------------------------

- **SMMU bypass is always on.** There is no untrusted DMA path on this
  product; the bypass is a performance decision for the DPAA2 / WRIOP
  packet path. Reference boards only bypass under secure boot.
- **POVDD strapped to ground.** ``TA_PROG_SFP`` is not
  software-controllable, so in-system fuse provisioning is not
  supported; fuses are burned by an external jig.
- **Verbose EA / SError handler** dumps ESR / FAR / ELR / SPSR_EL3 for
  kernel bring-up forensics.
- Optional features enabled in the board makefile:
  ``NXP_DDR_DUAL_DIMM_TOLERANT`` (tolerant SPD compare for two-lot DIMM
  pairing) and ``NXP_XSPI_DIAG`` (FlexSPI JEDEC / SFDP / SR1 probe
  helpers used by the bootstrap and bring-up diagnostics).

Building
--------

Production XIP BL2 + FIP (embedded in the PBL at NOR offset 0x9000):

.. code:: shell

    make PLAT=nbxv3 BOOT_MODE=flexspi_nor RCW=$RCW_BIN BL33=$UBOOT_BIN \
         pbl fip fip-ddr

Bootstrap BL2 (build, loaded by OpenOCD into OCRAM, resolves staged
images over ARM semihosting):

.. code:: shell

    make PLAT=nbxv3 NBXV3_BL2_BOOTSTRAP=1 BOOT_MODE=semihosting bl2

``SUPPORTED_BOOT_MODE`` is ``flexspi_nor semihosting``.

--------------

*Copyright (c) 2026, Free Mobile. All rights reserved.*
