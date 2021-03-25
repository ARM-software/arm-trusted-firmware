
Steps to blow fuses on NXP LS SoC:
==================================


- Enable POVDD
  -- Refer board GSG(Getting Started Guide) for the steps to enable POVDD.
  -- Once the POVDD is enabled, make sure to set variable POVDD_ENABLE := yes, in the platform.mk.

+---+-----------------+-----------+------------+-----------------+-----------------------------+
|   |   Platform      |  Jumper   |  Switch    | LED to Verify   |  Through GPIO Pin (=number) |
+===+=================+===========+============+=================+=============================+
| 1.| lx2160ardb      |  J9       |            |                 |             no              |
+---+-----------------+-----------+------------+-----------------+-----------------------------+
| 2.| lx2160aqds      |  J35      |            |                 |             no              |
+---+-----------------+-----------+------------+-----------------+-----------------------------+
| 3.| lx2162aqds      |  J35      | SW9[4] = 1 |    D15          |             no              |
+---+-----------------+-----------+------------+-----------------+-----------------------------+

- SFP registers to be written to:

+---+----------------------------------+----------------------+----------------------+
|   |   Platform                       |   OTPMKR0..OTPMKR7   |   SRKHR0..SRKHR7     |
+===+==================================+======================+======================+
| 1.| lx2160ardb/lx2160aqds/lx2162aqds | 0x1e80234..0x1e80250 | 0x1e80254..0x1e80270 |
+---+----------------------------------+----------------------+----------------------+

- At U-Boot prompt, verify that SNVS register - HPSR, whether OTPMK was written, already:

+---+----------------------------------+-------------------------------------------+---------------+
|   |   Platform                       |           OTPMK_ZERO_BIT(=value)          | SNVS_HPSR_REG |
+===+==================================+===========================================+===============+
| 1.| lx2160ardb/lx2160aqds/lx2162aqds | 27 (= 1 means not blown, =0 means blown)  | 0x01E90014    |
+---+----------------------------------+-------------------------------------------+---------------+

From u-boot prompt:

  --  Check for the OTPMK.
   .. code:: shell

        md $SNVS_HPSR_REG

      Command Output:
          01e90014: 88000900

          In case it is read as 00000000, then read this register using jtag (in development mode only through CW tap).
                       +0       +4       +8       +C
          [0x01E90014] 88000900

          Note: OTPMK_ZERO_BIT is 1, indicating that the OTPMK is not blown.

  --  Check for the SRK Hash.
   .. code:: shell

        md $SRKHR0 0x10

      Command Output:
          01e80254: 00000000 00000000 00000000 00000000    ................
          01e80264: 00000000 00000000 00000000 00000000    ................

          Note: Zero means that SRK hash is not blown.

- If not blown, then from the U-Boot prompt, using following commands:
  --  Provision the OTPMK.

   .. code:: shell

        mw.l $OTPMKR0  <OTMPKR_0_32Bit_val>
        mw.l $OTPMKR1  <OTMPKR_1_32Bit_val>
        mw.l $OTPMKR2  <OTMPKR_2_32Bit_val>
        mw.l $OTPMKR3  <OTMPKR_3_32Bit_val>
        mw.l $OTPMKR4  <OTMPKR_4_32Bit_val>
        mw.l $OTPMKR5  <OTMPKR_5_32Bit_val>
        mw.l $OTPMKR6  <OTMPKR_6_32Bit_val>
        mw.l $OTPMKR7  <OTMPKR_7_32Bit_val>

  --  Provision the SRK Hash.

   .. code:: shell

        mw.l $SRKHR0  <SRKHR_0_32Bit_val>
        mw.l $SRKHR1  <SRKHR_1_32Bit_val>
        mw.l $SRKHR2  <SRKHR_2_32Bit_val>
        mw.l $SRKHR3  <SRKHR_3_32Bit_val>
        mw.l $SRKHR4  <SRKHR_4_32Bit_val>
        mw.l $SRKHR5  <SRKHR_5_32Bit_val>
        mw.l $SRKHR6  <SRKHR_6_32Bit_val>
        mw.l $SRKHR7  <SRKHR_7_32Bit_val>

      Note: SRK Hash should be carefully written keeping in mind the SFP Block Endianness.

- At U-Boot prompt, verify that SNVS registers for OTPMK are correctly written:

  --  Check for the OTPMK.
   .. code:: shell

        md $SNVS_HPSR_REG

      Command Output:
          01e90014: 80000900

          OTPMK_ZERO_BIT is zero, indicating that the OTPMK is blown.

          Note: In case it is read as 00000000, then read this register using jtag (in development mode only through CW tap).

   .. code:: shell

        md $OTPMKR0 0x10

      Command Output:
          01e80234: ffffffff ffffffff ffffffff ffffffff    ................
          01e80244: ffffffff ffffffff ffffffff ffffffff    ................

          Note: OTPMK will never be visible in plain.

  --  Check for the SRK Hash. For example, if following SRK hash is written:

       SFP SRKHR0 = fdc2fed4
       SFP SRKHR1 = 317f569e
       SFP SRKHR2 = 1828425c
       SFP SRKHR3 = e87b5cfd
       SFP SRKHR4 = 34beab8f
       SFP SRKHR5 = df792a70
       SFP SRKHR6 = 2dff85e1
       SFP SRKHR7 = 32a29687,

       then following would be the value on dumping SRK hash.

   .. code:: shell

        md $SRKHR0 0x10

      Command Output:
          01e80254: d4fec2fd 9e567f31 5c422818 fd5c7be8    ....1.V..(B\.{\.
          01e80264: 8fabbe34 702a79df e185ff2d 8796a232    4....y*p-...2...

          Note: SRK Hash is visible in plain based on the SFP Block Endianness.

- Caution: Donot proceed to the next step, until you are sure that OTPMK and SRKH are correctly blown from above steps.
  -- After the next step, there is no turning back.
  -- Fuses will be burnt, which cannot be undo.

- Write SFP_INGR[INST] with the PROGFB(0x2) instruction to blow the fuses.
  -- User need to save the SRK key pair and OTPMK Key forever, to continue using this board.

+---+----------------------------------+-------------------------------------------+-----------+
|   |   Platform                       | SFP_INGR_REG | SFP_WRITE_DATE_FRM_MIRROR_REG_TO_FUSE  |
+===+==================================+=======================================================+
| 1.| lx2160ardb/lx2160aqds/lx2162aqds | 0x01E80020   |    0x2                                 |
+---+----------------------------------+--------------+----------------------------------------+

   .. code:: shell

        md $SFP_INGR_REG  $SFP_WRITE_DATE_FRM_MIRROR_REG_TO_FUSE

- On reset, if the SFP register were read from u-boot, it will show the following:
  --  Check for the OTPMK.

   .. code:: shell

        md $SNVS_HPSR_REG

      Command Output:
          01e90014: 80000900

          In case it is read as 00000000, then read this register using jtag (in development mode only through CW tap).
                       +0       +4       +8       +C
          [0x01E90014] 80000900

          Note: OTPMK_ZERO_BIT is zero, indicating that the OTPMK is blown.

   .. code:: shell

        md $OTPMKR0 0x10

      Command Output:
          01e80234: ffffffff ffffffff ffffffff ffffffff    ................
          01e80244: ffffffff ffffffff ffffffff ffffffff    ................

          Note: OTPMK will never be visible in plain.

  -- SRK Hash

   .. code:: shell

        md $SRKHR0 0x10

      Command Output:
          01e80254: d4fec2fd 9e567f31 5c422818 fd5c7be8    ....1.V..(B\.{\.
          01e80264: 8fabbe34 702a79df e185ff2d 8796a232    4....y*p-...2...

          Note: SRK Hash is visible in plain based on the SFP Block Endianness.

Second method to do the fuse provsioning:
=========================================

This method is used for quick way to provision fuses.
Typically used by those who needs to provision number of boards.

- Enable POVDD:
  -- Refer the table above to enable POVDD.

     Note: If GPIO Pin supports enabling POVDD, it can be done through the below input_fuse_file.

  -- Once the POVDD is enabled, make sure to set variable POVDD_ENABLE := yes, in the platform.mk.

- User need to populate the "input_fuse_file", corresponding to the platform for:

  -- OTPMK
  -- SRKH

  Table of fuse provisioning input file for every supported platform:

+---+----------------------------------+-----------------------------------------------------------------+
|   |   Platform                       |                        FUSE_PROV_FILE                           |
+===+==================================+=================================================================+
| 1.| lx2160ardb/lx2160aqds/lx2162aqds | ${CST_DIR}/input_files/gen_fusescr/ls2088_1088/input_fuse_file  |
+---+----------------------------------+--------------+--------------------------------------------------+

- Create the TF-A binary with FUSE_PROG=1.

   .. code:: shell

        make PLAT=$PLAT FUSE_PROG=1\
          BOOT_MODE=<platform_supported_boot_mode> \
          RCW=$RCW_BIN \
          BL32=$TEE_BIN SPD=opteed\
          BL33=$UBOOT_SECURE_BIN \
          pbl \
          fip \
          fip_fuse \
          FUSE_PROV_FILE=../../apps/security/cst/input_files/gen_fusescr/ls2088_1088/input_fuse_file

- Deployment:
  -- Refer the nxp-layerscape.rst for deploying TF-A images.
  -- Deploying fip_fuse.bin:

       For Flexspi-Nor:

   .. code:: shell

        tftp 82000000  $path/fuse_fip.bin;
        i2c mw 66 50 20;sf probe 0:0; sf erase 0x880000 +$filesize; sf write 0x82000000 0x880000 $filesize;

      For SD or eMMC [file_size_in_block_sizeof_512 = (Size_of_bytes_tftp / 512)]:

   .. code:: shell

        tftp 82000000  $path/fuse_fip.bin;
        mmc write 82000000 0x4408 <file_size_in_block_sizeof_512>;

- Valiation:

+---+----------------------------------+---------------------------------------------------+
|   |   Platform                       |    Error_Register        | Error_Register_Address |
+===+==================================+===================================================+
| 1.| lx2160ardb/lx2160aqds/lx2162aqds | DCFG scratch 4 register  |     0x01EE020C         |
+---+----------------------------------+---------------------------------------------------+

   At the U-Boot prompt, check DCFG scratch 4 register for any error.

   .. code:: shell

        md $Error_Register_Address 1

      Command Ouput:
          01ee020c: 00000000

      Note:
       - 0x00000000 shows no error, then fuse provisioning is successful.
       - For non-zero value, refer the code header file ".../drivers/nxp/sfp/sfp_error_codes.h"
