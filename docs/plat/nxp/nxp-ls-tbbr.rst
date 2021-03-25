
--------------
NXP Platforms:
--------------
TRUSTED_BOARD_BOOT option can be enabled by specifying TRUSTED_BOARD_BOOT=1 on command line during make.



Bare-Minimum Preparation to run  TBBR on NXP Platforms:
=======================================================
- OTPMK(One Time Programable Key) needs to be burnt in fuses.
  -- It is the 256 bit key that stores a secret value used by the NXP SEC 4.0 IP in Trusted or Secure mode.

     Note: It is primarily for the purpose of decrypting additional secrets stored in system non-volatile memory.

  -- NXP CST tool gives an option to generate it.

   Use the below command from directory 'cst', with correct options.

   .. code:: shell

     ./gen_otpmk_drbg

- SRKH (Super Root Key Hash) needs to be burnt in fuses.
  -- It is the 256 bit hash of the list of the public keys of the SRK key pair.
  -- NXP CST tool gives an option to generate the RSA key pair and its hash.

   Use the below command from directory 'cst', with correct options.

   .. code:: shell

     ./gen_keys

Refer fuse frovisioning readme 'nxp-ls-fuse-prov.rst' for steps to blow these keys.



Two options are provided for TRUSTED_BOARD_BOOT:
================================================

-------------------------------------------------------------------------
Option 1: CoT using X 509 certificates
-------------------------------------------------------------------------

- This CoT is as provided by ARM.

- To use this option user needs to specify mbedtld dir path in MBEDTLS_DIR.

- To generate CSF header, path of CST repository needs to be specified as CST_DIR

- CSF header is embedded to each of the BL2 image.

- GENERATE_COT=1 adds the tool 'cert_create' to the build environment to generate:
  -- X509 Certificates as (.crt) files.
  -- X509 Pem key file as (.pem) files.

- SAVE_KEYS=1 saves the keys and certificates, if GENERATE_COT=1.
  -- For this to work, file name for cert and keys are provided as part of  compilation or build command.

     --- default file names will be used, incase not provided as part compilation or build command.
     --- default folder 'BUILD_PLAT' will be used to store them.

- ROTPK for x.509 certificates is generated and embedded in bl2.bin and
  verified as part of CoT by Boot ROM during secure boot.

- Compilation steps:

All Images
   .. code:: shell

       make PLAT=$PLAT TRUSTED_BOARD_BOOT=1 GENERATE_COT=1 MBEDTLS_DIR=$MBEDTLS_PATH CST_DIR=$CST_DIR_PATH \
       BOOT_MODE=<platform_supported_boot_mode> \
       RCW=$RCW_BIN \
       BL32=$TEE_BIN SPD=opteed\
       BL33=$UBOOT_SECURE_BIN \
       pbl \
       fip

Additional FIP_DDR Image (For NXP platforms like lx2160a)
   .. code:: shell

       make PLAT=$PLAT TRUSTED_BOARD_BOOT=1 GENERATE_COT=1 MBEDTLS_DIR=$MBEDTLS_PATH fip_ddr

      Note: make target 'fip_ddr' should never be combine with other make target 'fip', 'pbl' & 'bl2'.

-------------------------------------------------------------------------
Option 2: CoT using NXP CSF headers.
-------------------------------------------------------------------------

- This option is automatically selected when TRUSTED_BOARD_BOOT is set but MBEDTLS_DIR path is not specified.

- CSF header is embedded to each of the BL31, BL32 and  BL33 image.

- To generate CSF header, path of CST repository needs to be specified as CST_DIR

- Default input files for CSF header generation is added in this repo.

- Default input file requires user to generate RSA key pair named
  -- srk.pri, and
  -- srk.pub, and add them in ATF repo.
  -- These keys can be generated using gen_keys tool of CST.

- To change the input file , user can use the options BL33_INPUT_FILE, BL32_INPUT_FILE, BL31_INPUT_FILE

- There are 2 paths in secure boot flow :
  -- Development Mode (sb_en in RCW = 1, SFP->OSPR, ITS = 0)

     --- In this flow , even on ROTPK comparison failure, flow would continue.
     --- However SNVS is transitioned to non-secure state

  -- Production mode (SFP->OSPR, ITS = 1)

     --- Any failure is fatal failure

- Compilation steps:

All Images
   .. code:: shell

       make PLAT=$PLAT TRUSTED_BOARD_BOOT=1 CST_DIR=$CST_DIR_PATH \
       BOOT_MODE=<platform_supported_boot_mode> \
       RCW=$RCW_BIN \
       BL32=$TEE_BIN SPD=opteed\
       BL33=$UBOOT_SECURE_BIN \
       pbl \
       fip

Additional FIP_DDR Image (For NXP platforms like lx2160a)
   .. code:: shell

       make PLAT=$PLAT TRUSTED_BOARD_BOOT=1 CST_DIR=$CST_DIR_PATH fip_ddr

- Compilation Steps with build option for generic image processing filters to prepend CSF header:
  --  Generic image processing filters to prepend CSF header

      BL32_INPUT_FILE = < file name>
      BL33_INPUT_FILE = <file name>

   .. code:: shell

       make PLAT=$PLAT TRUSTED_BOARD_BOOT=1 CST_DIR=$CST_DIR_PATH \
       BOOT_MODE=<platform_supported_boot_mode> \
       RCW=$RCW_BIN \
       BL32=$TEE_BIN SPD=opteed\
       BL33=$UBOOT_SECURE_BIN \
       BL33_INPUT_FILE = <ip file> \
       BL32_INPUT_FILE = <ip_file> \
       BL31_INPUT_FILE = <ip file> \
       pbl \
       fip


Deploy ATF Images
=================
Same steps as mentioned in the readme "nxp-layerscape.rst".



Verification to check if Secure state is achieved:
==================================================

+---+----------------+-----------------+------------------------+----------------------------------+-------------------------------+
|   |   Platform     |  SNVS_HPSR_REG  | SYS_SECURE_BIT(=value) | SYSTEM_SECURE_CONFIG_BIT(=value) | SSM_STATE                     |
+===+================+=================+========================+==================================+===============================+
| 1.| lx2160ardb  or |    0x01E90014   | 15                     | 14-12                            | 11-8                          |
|   | lx2160aqds  or |                 | ( = 1, BootROM Booted) | ( = 010 means Intent to Secure,  | (=1111 means secure boot)     |
|   | lx2162aqds     |                 |                        | ( = 000 Unsecure)                | (=1011 means Non-secure Boot) |
+---+----------------+-----------------+------------------------+----------------------------------+-------------------------------+

- Production mode (SFP->OSPR, ITS = 1)
  -- Linux prompt will successfully come. if the TBBR is successful.

     --- Else, Linux boot will be successful.

  -- For secure-boot status, read SNVS Register $SNVS_HPSR_REG from u-boot prompt:

   .. code:: shell

        md $SNVS_HPSR_REG

      Command Output:
          1e90014: 8000AF00

          In case it is read as 00000000, then read this register using jtag (in development mode only through CW tap).
                       +0       +4       +8       +C
          [0x01E90014] 8000AF00


- Development Mode (sb_en in RCW = 1, SFP->OSPR, ITS = 0)
  -- Refer the SoC specific table to read the register to interpret whether the secure boot is achieved or not.
  -- Using JTAG (in development environment only, using CW tap):

     --- For secure-boot status, read SNVS Register $SNVS_HPSR_REG

   .. code:: shell

        ccs::display_regs 86 0x01E90014 4 0 1

      Command Output:
          Using the SAP chain position number 86, following is the output.

                       +0       +4       +8       +C
          [0x01E90014] 8000AF00

          Note: Chain position number will vary from one SoC to other SoC.

- Interpretation of the value:

  -- 0xA indicates BootROM booted, with intent to secure.
  -- 0xF = secure boot, as SSM_STATE.
