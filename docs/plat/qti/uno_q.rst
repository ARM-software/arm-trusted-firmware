Arduino UNO Q
=============

Trusted Firmware-A (TF-A) platform port for the Arduino UNO Q board, based on
the Qualcomm QRB2210 (Agatti) SoC. The SoC support lives in
``plat/qti/bruin/agatti`` and the board in ``plat/qti/bruin/agatti/uno_q``.

Agatti specifics:

- CPUs: four Kryo 2xx Silver cores (Cortex-A53) in one cluster, powered up
  through the APCS per-CPU power controllers as on MSM8916.
- Interrupt controller: GIC-500 (GICv3).
- Debug UART: QUPv3 wrap 0, serial engine 4 at ``0x4a90000``.
- XBL enters the TZ image in the 100 KiB IMEM TZ window, so BL2 runs from
  ``0x0c100000``. BL31 runs from the pIMEM aperture at ``0x10100000``.
- BL32 (OP-TEE) runs from ``0x46200000``, a 37 MiB carve-out (4 MiB TEE RAM +
  33 MiB TA RAM) in the free DDR window between the ``smem`` and modem PIL
  reserved-memory regions. A smaller TA RAM pool makes any test that opens a
  nested TA session fail with ``TEEC_ERROR_OUT_OF_MEMORY``; this must stay in
  sync with OP-TEE's ``CFG_TZDRAM_START`` and the ``optee_mem``
  reserved-memory node in the Linux DT.
- The PMIC (PM4125) is set up for a shutdown or a warm reset before PS_HOLD
  is dropped.
- Storage is eMMC with 512-byte blocks.

Boot flow
---------

As on :ref:`Dragonwing RB3 Gen 2 development platform`: XBL loads BL2 from the
``tz`` partition and the FIP ELF from the ``uefi`` partition. BL2 loads BL31,
BL32 (OP-TEE) and BL33 (U-Boot) from the FIP.

BL2 never sees the FIP's ELF wrapper or its hash segment. ``generate_fip_elf.sh``
wraps ``fip.bin`` as a single ``PT_LOAD`` segment at ``0x5f800000`` and
``qtestsign`` adds the MBN hash segment and OEM test signature on top. XBL
loads the ``uefi`` partition content ("APPSBL" in its own boot trace) through
its normal ELF loader: it runs a segments-hash check against that hash
segment, then copies the segment to ``0x5f800000``. Only after that does BL2
run, and it reads the FIP's own table of contents directly from
``PLAT_QTI_FIP_IOBASE`` via ``io_fip.c``/``io_memmap.c`` — a fixed physical
offset, with no ELF or MBN parsing involved on the BL2 side.

In other words: the MBN/hash-segment shape of ``fip.elf`` exists for XBL's
generic image loader, not for BL2. XBL loads every early image (RPM, QSEE,
QHEE, STI, APPSBL, ...) through the same hash-checked path regardless of that
image's own internal format, so it needs a hash table in the expected place to
run its standard integrity check against, and does not proceed without one.
With secure boot off, the OEM test signature and certificate chain qtestsign
inserts are not cryptographically verified, but the segment hashes still are.

How to build
------------

Steps to build TF-A BL2 and the FIP payload::

	$ make CROSS_COMPILE=aarch64-none-elf- PLAT=uno_q SPD=opteed \
	    BL32=<path-to-optee-bin> BL33=<path-to-u-boot-bin> fip all

	$ ./tools/qti/generate_fip_elf.sh build/uno_q/release/fip.bin \
	    0x5f800000

XBL authenticates the TZ image with the QTI authenticator even when secure
boot is disabled, so ``bl2.elf`` must be signed as a TZ image with QTI
signing. An OEM test signature from `qtestsign
<https://github.com/msm8916-mainline/qtestsign>`__ is not accepted for BL2.
The ``fip.elf`` is signed with qtestsign.

How to flash
------------

Put the board in EDL mode and write the ``a`` slot with `qdl
<https://github.com/linux-msm/qdl>`__ and the eMMC firehose programmer shipped
with the board software::

	$ qdl --storage emmc prog_firehose_ddr.elf \
	    write tz_a bl2.mbn write uefi_a fip.elf

References
----------

- `Qualcomm QRB2210 <https://www.qualcomm.com/internet-of-things/products/q2-series/qrb2210>`__
- `Arduino UNO Q <https://www.arduino.cc/product-uno-q/>`__

--------------

*Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.*
