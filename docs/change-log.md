# Change Log & Release Notes

This document contains a summary of the new features, changes, fixes and known
issues in each release of Trusted Firmware-A.

## [2.7.0](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/refs/tags/v2.6.0..refs/tags/v2.7.0) (2022-05-20)

### New Features

- **Architecture**

  - **Statistical profiling Extension (FEAT_SPE)**

    - add support for FEAT_SPEv1p2 ([f20eb89](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/f20eb893a072bb9b404eedb886e8c65fe76ffb45))

  - **Branch Record Buffer Extension (FEAT_BRBE)**

    - add BRBE support for NS world ([744ad97](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/744ad97445ce7aa65adaef376d0b5bafc12a90d3))

  - **Extended Cache Index (FEAT_CCIDX)**

    - update the do_dcsw_op function to support FEAT_CCIDX ([d0ec1cc](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/d0ec1cc437c59e64ecba44710dbce82a04ff892d))

- **Platforms**

  - add SZ_* macros ([1af59c4](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/1af59c457010e6e3e6536752736eb02115bca543))

  - **Allwinner**

    - add SMCCC SOCID support ([436cd75](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/436cd754f2b0f9c0ce3094961bd1e179eeff2fc1))
    - allow to skip PMIC regulator setup ([67412e4](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/67412e4d7ae3defaac78ef5e351c63e06cfd907a))
    - apx803: add aldo1 regulator ([a29f6e7](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/a29f6e76cbf76d509c00f84f068b59864d210dfd))
    - choose PSCI states to avoid translation ([159c36f](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/159c36fd2fc5afbe979e5028b9e845ed4b7a40f1))
    - provide CPU idle states to the rich OS ([e2b1877](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/e2b18771fc2a0528dda18dbdaac08dd8530df25a))
    - simplify CPU_SUSPEND power state encoding ([52466ec](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/52466ec38ef312da62ad062720a03a183329f831))

  - **Arm**

    - **FVP**

      - measure critical data ([cf21064](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/cf21064ec8a1889f64de48e30e38285227d27745))
      - update HW_CONFIG DT loading mechanism ([39f0b86](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/39f0b86a76534d0b7c71dd0c8b34f1a74480386b))
      - enable RSS backend based measured boot ([c44e50b](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/c44e50b72567205650c6455f3a258f36af0c84dd))

    - **Morello**

      - add changes to enable TBBR boot ([4af5397](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/4af53977533bee7b5763d3efad1448545c2ebef7))
      - add DTS for Morello SoC platform ([572c8ce](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/572c8ce255397f7cff9640676e510817a8e4c6a3))
      - add support for nt_fw_config ([6ad6465](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/6ad6465e5ce452688cac079f16d26f64e9f4ce3c))
      - add TARGET_PLATFORM flag ([8840711](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/8840711f33131969ec6b62ca3da079cf0573ac8b))
      - configure DMC-Bing mode ([9b8c431](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/9b8c431e2b2d656da7f8c4158e3d32e104446fec))
      - expose scmi protocols in fdts ([87639aa](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/87639aab0b6a30d4f49d069c0ea06900b11072a6))
      - split platform_info sds struct ([4a7a9da](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/4a7a9dafbc953089957a0cc1a7183731a5b003e1))
      - zero out the DDR memory space ([2d39b39](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/2d39b39704c1e4f2a189543ac4ff05ae58e5f5c8))

    - **N1SDP**

      - add support for nt_fw_config ([cf85030](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/cf85030efe73439e06295f8185b0a6bebf7b5eae))
      - enable trusted board boot on n1sdp ([fe2b37f](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/fe2b37f6858168a56c3d393bc72f560468d02165))

    - **RD**

      - **RD-N2**

        - add board support for rdn2cfg2 variant ([efeb438](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/efeb43808d2e3ed23e1d51d5e86460db92971e96))
        - add support for rdedmunds variant ([ef515f0](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/ef515f0d3466a8beded4fd662718abbd97391b13))

    - **SGI**

      - add page table translation entry for secure uart ([33d10ac](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/33d10ac8bf134519f303fd7ce5fb5d583be2f515))
      - deviate from arm css common uart related definitions ([f2cccca](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/f2ccccaa81ec14a80fedb48c37226e5d852ada7a))
      - enable fpregs context save and restore ([18fa43f](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/18fa43f753b79cfc3cc5426a3ef50b04efbf6206))
      - route TF-A logs via secure uart ([987e2b7](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/987e2b7c20eb4ab4215ff5289b715300f5cec054))

    - **TC**

      - add reserved memory region for Gralloc ([ad60a42](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/ad60a42cd79713984065dca8540c091c49755f32))
      - enable CI-700 PMU for profiling ([fbfc598](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/fbfc59840f9cd0ea53921c7f6fb9f4850a3b42ee))
      - enable GPU ([82117bb](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/82117bb48180175c25936b0ff9e33563e25e18f4))
      - enable SMMU for DPU ([4a6ebee](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/4a6ebeeca37ece34a58982c8b6ebdc8cfd70814b))
      - enable tracing ([59da207](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/59da207e2f2f028c9051c89bc5a05e95d996c18c))

    - **Corstone-1000**

      - identify bank to load fip ([cf89fd5](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/cf89fd57ed3286d7842eef41cd72a3977eb6d317))
      - implement platform specific psci reset ([a599c80](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/a599c80d063975cbeedbc86cfb619fca8545c487))
      - made changes to accommodate 3MB for optee ([854d1c1](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/854d1c103a9b73bbde7ef1b89b06b29e3cc053bb))

  - **Intel**

    - add macro to switch between different UART PORT ([447e699](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/447e699f70f1a1d1b85a8136b445eba689166c5d))
    - add RSU 'Max Retry' SiP SMC services ([4c26957](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/4c26957be253a7ab3acb316f42bf3ee10c409ed2))
    - add SiP service for DCMF status ([984e236](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/984e236e0dee46708534a23c637271a931ceb67e))
    - add SMC for enquiring firmware version ([c34b2a7](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/c34b2a7a1a38dba88b6b668a81bd07c757525830))
    - add SMC support for Get USERCODE ([93a5b97](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/93a5b97ec9e97207769db18ae34886e6b8bf2ea4))
    - add SMC support for HWMON voltage and temp sensor ([52cf9c2](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/52cf9c2cd4882534d02e8996e4ff1143ee59290e))
    - add SMC support for ROM Patch SHA384 mailbox ([77902fc](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/77902fca8fe7449473b09198e1fe197f7b4765d7))
    - add SMC/PSCI services for DCMF version support ([44eb782](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/44eb782e15c9af532f2455b37bd53ca93830f6e2))
    - add SMPLSEL and DRVSEL setup for Stratix 10 MMC ([bb0fcc7](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/bb0fcc7e011ec4319a79734ba44353015860e39f))
    - add support for F2S and S2F bridge SMC with mask to enable, disable and reset bridge ([11f4f03](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/11f4f03043ef05762f4d6337804c39dc8f9af54f))
    - allow to access all register addresses if DEBUG=1 ([7e954df](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/7e954dfc2ba83262f7596dd0f17de75163e49e5e))
    - create source file for firewall configuration ([afa0b1a](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/afa0b1a82a404c616da2da8f52cdcd587938955f))
    - enable firewall for OCRAM in BL31 ([ae19fef](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/ae19fef33707700a91b0b672aa784e084a6ca500))
    - enable SMC SoC FPGA bridges enable/disable ([b7f3044](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/b7f3044e8725d9af997999547630892cf9e2f0ad))
    - extend attestation service to Agilex family ([581182c](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/581182c1916df03860744d8e32941c72b2cc3fda))
    - implement timer init divider via cpu frequency. ([#1](https://review.trustedfirmware.org:29418/TF-A/trusted-firmware-a/issues/1)) ([f65bdf3](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/f65bdf3a54eed8f7651761c25bf6cc7437f4474b))
    - initial commit for attestation service ([d174083](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/d17408316db10db611e23716e8a5b9b9f53ad509))
    - single certificate feature enablement ([7facace](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/7facacec6328e505b243a4974d045d45fe068afd))
    - support AES Crypt Service ([6726390](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/6726390eb02e9659cfaf2d3598be9bf12fbc5901))
    - support crypto service key operation ([342a061](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/342a0618c7ff89327ac5b34dc0713509ffae609b))
    - support crypto service session ([6dc00c2](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/6dc00c24ab0100a2aae0f416c72470f8ed17e149))
    - support ECDH request ([4944686](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/49446866a515c2db855d456f39df3d586b2084b7))
    - support ECDSA Get Public Key ([d2fee94](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/d2fee94afa6ba7e76508e6bead7eb2936c5eafb8))
    - support ECDSA HASH Signing ([6925410](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/692541051b8cb0f435ae46c5d7351231ee292319))
    - support ECDSA HASH Verification ([7e25eb8](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/7e25eb87016ba8355cf0a3a5f71fb8b8785de044))
    - support ECDSA SHA-2 Data Signature Verification ([5830506](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/583050607e43cef8b544a5700386a019e54c422f))
    - support ECDSA SHA-2 Data Signing ([07912da](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/07912da1b7663451493fb5e40e4c33deeb18a639))
    - support extended random number generation ([24f9dc8](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/24f9dc8a43fea350416ca9312a78ab4e786da8ad))
    - support HMAC SHA-2 MAC verify request ([c05ea29](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/c05ea2969070be90a7dbb2d0344c66d89401edf6))
    - support session based SDOS encrypt and decrypt ([537ff05](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/537ff052579862a4865d36d06940feaa796d16da))
    - support SHA-2 hash digest generation on a blob ([7e8249a](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/7e8249a2dbacfa751990c47644f0403311c6e260))
    - support SiP SVC version ([f0c40b8](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/f0c40b897f8a25bc50c53239dcf750dd395ebabf))
    - support version 2 SiP SVC SMC function ID for mailbox commands ([c436707](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/c436707bc6eed31ab61408ef40db6063d05f0912))
    - support version 2 SiP SVC SMC function ID for non-mailbox commands ([ad47f14](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/ad47f1422f3f9aa4a622e08b71fc8f5caab98a98))
    - update to support maximum response data size ([b703fac](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/b703facaaae1e3fe5afa4742b436bb07e065b5e9))

  - **Marvell**

    - **Armada**

      - **A3K**

        - add north and south bridge reset registers ([a4d35ff](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/a4d35ff381c625d61bcc22f9f9a1a45d8663b19d))

  - **MediaTek**

    - introduce mtk makefile ([500d40d](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/500d40d877617653d347fb6308144973d4297ab9))

    - **MT8195**

      - apply erratas of CA78 for MT8195 ([c21a736](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/c21a736d6f3fa9fb0647bff404b0174ebf1acd91))
      - add EMI MPU surppot for SCP and DSP ([690cb12](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/690cb1265ea84851bd6405a0a6a57d2f1c9f03a3))
      - dump EMI MPU configurations ([20ef588](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/20ef588e86ad8f3cf13382c164463046db261feb))
      - improve SPM wakeup log ([ab45305](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/ab45305062f50f81e5c3f800ef4c6cef5097cb04))

    - **MT8186**

      - add DFD control in SiP service ([e46e9df](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/e46e9df0d0e05f2aaee613fc4f697fcc8d79c0b3))
      - add SPM suspend driver ([7ac6a76](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/7ac6a76c47d429778723aa804b64c48220a10f11))
      - add Vcore DVFS driver ([635e6b1](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/635e6b108e773daf37c00f46e6fbb1cae4e78f96))
      - disable 26MHz clock while suspending ([9457cec](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/9457cec8c02f78ba56fd9298dd795766c89281a2))
      - initialize platform for MediaTek MT8186 ([27132f1](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/27132f13ca871dc3cf1aa6938995284cf5016e00))
      - add power-off function for PSCI ([a68346a](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/a68346a772859ee6971ec14c6473d2a853e9c66f))
      - add CPU hotplug ([1da57e5](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/1da57e54b2270b3b49710afa6fd947b01d61b261))
      - add DCM driver ([95ea87f](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/95ea87ffc2445c77f070e6a2f78ffa424810faed))
      - add EMI MPU basic driver ([1b17e34](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/1b17e34c5d7740a357b2027d88aef7760b346616))
      - add MCDI drivers ([06cb65e](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/06cb65ef079941d0525dca75dd0e110e9330906d))
      - add pinctrl support ([af5a0c4](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/af5a0c40aff21c4b8771365f19dcb01d6086b30d))
      - add pwrap and pmic driver ([5bc88ec](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/5bc88ec61c75ed42b41d84817aa4d6ee68a2efc8))
      - add reboot function for PSCI ([24dd5a7](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/24dd5a7b71544c503446e58cb23c0cfd09245a3c))
      - add RTC drivers ([6e5d76b](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/6e5d76bac8786120d037953f5a6fd67aaff035c1))
      - add SiP service ([5aab27d](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/5aab27dc4294110a6c0b69bf5ec5343e7df883a7))
      - add sys_cirq support ([109b91e](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/109b91e38c8d4f73941c8574759560a1f1636d05))
      - apply erratas for MT8186 ([572f8ad](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/572f8adbb062c36835fbb82944dd2ed772134bfd))
      - initialize delay_timer ([d73e15e](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/d73e15e66a33398c8fc51c83f975a3f35494faf5))
      - initialize GIC ([206f125](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/206f125cc177bc110eb87d40ffc7fa18b28c01ce))
      - initialize systimer ([a6a0af5](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/a6a0af57c3369dfc6fc2f25877d812a24e9be311))

  - **NXP**

    - add SoC erratum a008850 ([3d14a30](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/3d14a30b88762e901e134acc89c6ac4fa9e3f321))
    - add ifc nor and nand as io devices ([b759727](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/b759727f5936a687314168dd8912d30897a8c6be))
    - add RCPM2 registers definition ([d374060](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/d374060abe9b63296f63f1e3c811aeeddb7a093c))
    - add CORTEX A53 helper functions ([3ccc8ac](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/3ccc8ac3e5da48819a2fc90ec48a175515de38cb))

    - **i.MX**

      - **i.MX 8M**

        - add a simple csu driver for imx8m family ([71c40d3](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/71c40d3bb7c90a6c36d5c49d0830ca95aba65a2f))
        - add imx csu/rdc enum type defines for imx8m ([0c6dfc4](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/0c6dfc47847608b6ade0c00716e93afc6725362c))
        - enable conditional build for SDEI ([d2a339d](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/d2a339dfa1665edf87a30a4318af954e764c205c))
        - enable the coram_s tz by default on imx8mn/mp ([d5ede92](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/d5ede92d78c829d8a3adad0759219b79e0dc0707))
        - enable the csu init on imx8m ([0a76495](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/0a76495bc2cb0c5291027020a3cd2d3adf31c8ed))
        - do not release JR0 to NS if HAB is using it ([77850c9](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/77850c96f23bcdc76ecb0ecd27a982c00fde5d9d))
        - switch to xlat_tables_v2 ([4f8d5b0](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/4f8d5b018efc42d1ffa76fca8efb0d16a57f5edd))

        - **i.MX 8M Mini**

          - enable optee fdt overlay support ([9d0eed1](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/9d0eed111cb1294605b6d82291fef16a51d35e46))
          - enable Trusty OS on imx8mm ([ff3acfe](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/ff3acfe3cc1658917376152913a9d1b5b9b8de34))
          - add support for measured boot ([cb2c4f9](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/cb2c4f93c18b948fbfde9d50ab7d30362be0e00a))

        - **i.MX 8M Plus**

          - add trusty for imx8mp ([8b9c21b](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/8b9c21b480dd5c3265be1105a9462b3f5657a6b1))
          - enable BL32 fdt overlay support on imx8mp ([aeff146](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/aeff14640a91f6d33bfdbc0dc7b0e920f6d14b91))

        - **i.MX 8M Nano**

          - enable optee fdt overlay support ([2612891](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/26128912884b26fab67bce9d87ba0e1c85a0be1e))
          - enable Trusty OS for imx8mn ([99349c8](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/99349c8ecba910dabbaa72b9be91f3ed762036f5))

        - **i.MX 8M Q**

          - enable optee fdt overlay support ([023750c](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/023750c6a898e77c185839f5e56f8e23538f718a))
          - enable trusty for imx8mq ([a18e393](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/a18e393339e1d481f4fdf0d621fe4f39ce93a4fe))

    - **Layerscape**

      - add CHASSIS 3 support for tbbr ([9550ce9](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/9550ce9ddd7729a961f51ed61ea4b2030e284dcb))
      - add new soc errata a009660 support ([785ee93](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/785ee93cc3bd9b43d88fee5acefbd131bf6f2756))
      - add new soc errata a010539 support ([85bd092](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/85bd0929433875e0b84fdc2046d9ec2cf0164903))
      - add soc helper macro definition for chassis 3 ([602cf53](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/602cf53b6f507cea88f4af5c07bed9325bc7a9b8))
      - define more chassis 3 hardware address ([0d396d6](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/0d396d6455a659c4e679f02fae1f9043713474b0))
      - print DDR errata information ([3412716](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/3412716b30260958b30d1fa2e1c6d8cce195cd7d))

      - **LS1043A**

        - add ls1043a soc support ([3b0de91](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/3b0de9182501fae9de372efd1faaf35a7bf74f68))

        - **LS1043ARDB**

          - add ls1043ardb board support ([e4bd65f](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/e4bd65fed8a12d06181c1343cf786ac91badb6b0)

      - **LX2**

        - enable DDR erratas for lx2 platforms ([cd960f5](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/cd960f5009ee062bba9c479505caee6bbe644649))

      - **LS1046A**

        - add new SoC platform ls1046a ([cc70859](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/cc708597fa72094c5a01df60e6538e4a7429c2a0))

        - **LS1046ARDB**

          - add ls1046ardb board support ([bb52f75](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/bb52f7560b62043ed08a753f399dc80e8c1582d3))

        - **LS1046AFRWY**

          - add ls1046afrwy board support ([b51dc56](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/b51dc56ab9ea79e4709f0d0ce965525d0d3da918))

        - **LS1046AQDS**

          - add board ls1046aqds support ([16662dc](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/16662dc40dd2578d3000528ece090ed39ed18b9c))

      - **LS1088A**

        - add new SoC platform ls1088a ([9df5ba0](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/9df5ba05b4fe4cd44157363a897b73553ba6e2f1))

        - **LS1088ARDB**

          - add ls1088ardb board support ([2771dd0](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/2771dd0293b6cda6811e8bed95f2354a3ee0124e))

        - **LS1088AQDS**

          - add ls1088aqds board support ([0b0e676](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/0b0e67669814139c6818e61e03d0d0e3314fdc99))

  - **QEMU**

    - add SPMD support with SPMC at S-EL1 ([f58237c](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/f58237ccd9fd2350730d60ab7de59b5c376bfb35))
    - add support for measured boot ([5e69026](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/5e690269d579d9461be3c5f5e3f59d4c666863a0))

  - **QTI**

    - **MSM8916**

      - allow booting secondary CPU cores ([a758c0b](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/a758c0b65c6730fb07846899d6436ba257484d34))
      - initial platform port ([dddba19](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/dddba19a6a3cb7a1039beaffc3169c4eb3291afd))
      - setup hardware for non-secure world ([af64473](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/af6447315c8534331513ca6b6556af661e0ba88b))

  - **Renesas**

    - **R-Car**

      - **R-Car 3**

        - modify sequence for update value for WUPMSKCA57/53 ([d9912cf](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/d9912cf3d1022fc6d38a6059290040985de56e63))
        - modify type for Internal function argument ([ffb725b](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/ffb725be98ffd010c851629a6da75bf57f770c7f))
        - update IPL and Secure Monitor Rev.3.0.3 ([14d9727](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/14d9727e334300b3f5f57e76a9f6e21431e6c6b5))

  - **ST**

    - add a function to configure console ([53612f7](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/53612f72938f37244a5f10ae7c57abe7358c221f))
    - add STM32CubeProgrammer support on UART ([fb3e798](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/fb3e7985c9b657c535c02b722ecc413f643e671e))
    - add STM32MP_UART_PROGRAMMER target ([9083fa1](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/9083fa11ead67272b94329e8f84257de6658620d))
    - add early console in BL2 ([c768b2b](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/c768b2b22f4fb16cf8be8b4815a1984b29918c20))
    - disable authentication based on part_number ([49abdfd](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/49abdfd8cececb91a4bc7e7b29a30c09dce461c7))
    - get pin_count from the gpio-ranges property ([d0f2cf3](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/d0f2cf3b148df75d5cbbd42dfa18012043e5d1f4))
    - map 2MB for ROM code ([1697ad8](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/1697ad8cc81307972d31cec3b27d58f589eeeb3f))
    - protect UART during platform init ([acf28c2](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/acf28c267b3679a0770b2010f2ec3fb3c2d19975))
    - update stm32image tool for header v2 ([2d8886a](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/2d8886aceed613b9be25f20900914cacc8bb0fb9))
    - update the security based on new compatible ([812daf9](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/812daf916c9c977a4f6d7d745d22b90c8492fc71))
    - use newly introduced clock framework ([33667d2](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/33667d299bd5398ca549f542345e0f321b483d17))

    - **ST32MP1**

      - adaptations for STM32MP13 image header ([a530874](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/a5308745ee3ab3b77ca942052e60968bcc01340d))
      - add "Boot mode" management for STM32MP13 ([296ac80](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/296ac8012b77ea84079b38cc60ee786a5f91857f))
      - add a second fixed regulator ([225ce48](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/225ce4822ccf2e7c7c1fca6cf3918d4399158613))
      - add GUID values for updatable images ([8d6b476](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/8d6b4764f3e54431c3d01342d39d1efa70c3dbf9))
      - add GUID's for identifying firmware images to be booted ([41bd8b9](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/41bd8b9e2ad3b755505684601f07d4f7f8ec04c4))
      - add helper to enable high speed mode in low voltage ([dea02f4](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/dea02f4eaed855c2f05d8a1d7eefca313e98e5b4))
      - add logic to pass the boot index to the Update Agent ([ba02add](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/ba02add9ea8fb9a8b0a533c1065a77c7dda4f2a6))
      - add logic to select the images to be booted ([8dd7553](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/8dd755314fdfa077465bd6cd5e248be392d90378))
      - add NVMEM layout compatibility definition ([dfbdbd0](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/dfbdbd0625990267c6742268118ea748e77c6123))
      - add part numbers for STM32MP13 ([30eea11](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/30eea116cdd66b3fa1e1208e185eb7285a83d898))
      - add regulator framework compilation ([bba9fde](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/bba9fdee589fb9a7aca5963f53b7ce67c30520b3))
      - add sdmmc compatible in platform define ([3331d36](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/3331d3637c295993a78f22afe7463cf1c334d329))
      - add sign-compare warning ([c10f3a4](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/c10f3a4559ebf7a654a9719fec619e81e6ee1d69))
      - add stm32_get_boot_interface function ([a6bfa75](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/a6bfa75cf25241a486ab371ae105ea7ebf2d34d8))
      - add support for building the FWU feature ([ad216c1](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/ad216c106682f1d2565b2a08e11a601b418dc8a4))
      - add support for reading the metadata partition ([0ca180f](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/0ca180f6416160a523ff442f1ad0b768a9a3a948))
      - add timeout in IO compensation ([de02e9b](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/de02e9b0ec29548b8ce5ef6ee9adcd9c5edb0518))
      - allow configuration of DDR AXI ports number ([88f4fb8](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/88f4fb8fa759b1761954067346ee674b454bdfde))
      - call pmic_voltages_init() in platform init ([ffd1b88](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/ffd1b889225a8aec124df9e330f41dc638fd7180))
      - chip rev. Z is 0x1001 on STM32MP13 ([ef0b8a6](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/ef0b8a6c1b1a0eab3626041f3168f82bdb410836))
      - enable BL2_IN_XIP_MEM to remove relocation sections ([d958d10](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/d958d10eb360024e15f3c921dc3863a0cee98830))
      - enable format-signedness warning ([cff26c1](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/cff26c19169dd94857e8180cc46b7aa4ccac574a))
      - get CPU info from SYSCFG on STM32MP13 ([6512c3a](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/6512c3a62a4a7baaf32597284b242bc7172b7e26))
      - introduce new flag for STM32MP13 ([bdec516](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/bdec516ee862bfadc25a4d0c02a3b8d859c1fa25))
      - manage HSLV on STM32MP13 ([fca10a8](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/fca10a8f1b47231ef92634a0adf1a26cbfc97c2a))
      - manage monotonic counter ([f5a3688](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/f5a3688b8608df0f269a0b6df18632ebb9e26a01))
      - new way to access platform OTP ([ae3ce8b](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/ae3ce8b28eac73e9a41fdb28424d9f0f4b5f200e))
      - preserve the PLL4 settings for USB boot ([bf1af15](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/bf1af154db2c89028a8a551c18885add35d38966))
      - register fixed regulator ([967a8e6](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/967a8e63c33822680e3a4631430dcd9a4a64becd))
      - remove unsupported features on STM32MP13 ([111a384](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/111a384c90afc629e644e7a8284abbd4311cc6b3))
      - retry 3 times FWU trial boot ([f87de90](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/f87de907c87e5b2091592c131c4d3d2f737bef01))
      - select platform compilation either by flag or DT ([99a5d8d](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/99a5d8d01d38474b056766651bd746a4fe93ab20))
      - skip TOS_FW_CONFIG if not in FIP ([b706608](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/b7066086424c2f6fd04880db852306d6f487962e))
      - stm32mp_is_single_core() for STM32MP13 ([7b48a9f](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/7b48a9f3286b8f174acf8821fec48fd2e4771514))
      - update BACKUP_BOOT_MODE for STM32MP13 ([4b031ab](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/4b031ab4c50d0b9f7127daa7f4eec634f39de970))
      - update boot API for header v2.0 ([5f52eb1](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/5f52eb15970e57d2777d114948fc1110e3dd3f6c))
      - update CFG0 OTP for STM32MP13 ([1c37d0c](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/1c37d0c1d378769249c797de5b13d73cf6f17a53))
      - update console management for SP_min ([aafff04](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/aafff0435448c8409935132be41758e0031f0822))
      - update IO compensation on STM32MP13 ([8e07ab5](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/8e07ab5f705b213af28831f7c3e9878154e07df0))
      - update IP addresses for STM32MP13 ([52ac998](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/52ac9983d67522b6b821391941c8b0d01fd68941))
      - update memory mapping for STM32MP13 ([48ede66](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/48ede6615168118c674288f2e4f8ee1b11d2fa02))
      - updates for STM32MP13 device tree compilation ([d38eaf9](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/d38eaf99d327bc1400f51c87b6d8a2f92cd828c6))
      - usb descriptor update for STM32MP13 ([d59b9d5](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/d59b9d53b9cfb2443575c62c6716eb5508374a7b))
      - use clk_enable/disable functions ([c7a66e7](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/c7a66e720ae1a1a5ef98eaf9ff327cd352549010))
      - use only one filter for TZC400 on STM32MP13 ([b7d0058](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/b7d0058a3a9153a3863cf76a6763ea751b3ab48d))
      - warn when debug enabled on secure chip ([ac4b8b0](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/ac4b8b06eb23134d2a9002834541d33f8d43661b))

  - **Texas Instruments**

    - add enter sleep method ([cf5868b](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/cf5868b8cd7239dee69bdf6ba3ab87bd06bf15f5))
    - add gic save and restore calls ([b40a467](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/b40a467783e5911f97d6e92ebdeb34ca2f005552))
    - add PSCI handlers for system suspend ([2393c27](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/2393c27680a1ec636e413051e87e986df5a866fe))
    - allow build config of low power mode support ([a9f46fa](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/a9f46fad82b807a9f0a967245e3ac10ee8dd0ef1))
    - increase SEC_SRAM_SIZE to 128k ([38164e6](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/38164e64bd853a8329475e9168c5fcb94ecc528b))

  - **Xilinx**

    - **Versal**

      - add SPP/EMU platform support for versal ([be73459](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/be73459a945d8fa781fcc864943ccd0a8d92421c))
      - add common interfaces to handle EEMI commands ([1397967](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/1397967490c9f0ebff0d20a566260d1475fe065e))
      - add SMCCC call TF_A_PM_REGISTER_SGI ([fcf6f46](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/fcf6f469318d693a024d42ae2d0f4afb26c1e85d))
      - add support to reset SGI ([bf70449](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/bf70449ba2d1ffd20b01741c491dc0f565009b3d))
      - add UART1 as console ([2c79149](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/2c791499c26b40c31ce7f68c3bf0dca777fc62de))
      - enhance PM_IOCTL EEMI API to support additional arg ([d34a5db](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/d34a5db8a76abdfc8fa68f43b24b971699086a06))
      - get version for ATF related EEMI APIs ([da6e654](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/da6e654bc8b03ee784d0e96a71c4e591e63930f2))
      - remove the time stamp configuration ([18e2a79](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/18e2a79f8a5eaa72a2a7e641c2481beb9f827dce))

    - **ZynqMP**

      - disable the -mbranch-protection flag ([67abd47](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/67abd4762bd563be94e734bb0fe4087e88d5d446))
      - fix section `coherent_ram' will not fit in region `RAM' ([9b4ed0a](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/9b4ed0af02a8ff1fd9a81af5213fde16d3eb8d92))
      - add feature check support ([223a628](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/223a6284b8a0a3ead884a7f0cf333a464d32e319))
      - add support to get info of xilfpga ([cc077c2](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/cc077c22273075db328bd30fa12c28abf9eef052))
      - add uart1 as console ([ea66e4a](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/ea66e4af0baf5d5b905e72f824a672f16a6e0f98))
      - increase the max xlat tables when debug build is enabled ([4c4b961](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/4c4b9615b1d9512a4a89aa08e722547cc491a07b))
      - pass ioctl calls to firmware ([76ff8c4](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/76ff8c459e9e6d105e614d68648bd6680806f93e))
      - pm_api_clock_get_num_clocks cleanup ([e682d38](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/e682d38b56854e1586b25d929dbc83543b4c66e4))

- **Bootloader Images**

  - add XLAT tables symbols in linker script ([bb5b942](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/bb5b942e6f133198daedcca0b74ec598af260a54))

  - **BL2**

    - add support to separate no-loadable sections ([96a8ed1](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/96a8ed14b74cca33a8caf567d0f0a2d3b2483a3b))

  - **BL31**

    - aarch64: RESET_TO_BL31_WITH_PARAMS ([25844ff](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/25844ff728e4a0e5430ba2032457aba7b780a701))

- **Services**

  - **RME**

    - add dummy platform token to RMMD ([0f9159b](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/0f9159b7ebb7e784a8ed998869ff21095fa105b1))
    - add dummy realm attestation key to RMMD ([a043510](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/a0435105f229a65c7861b5997793f905cf90b823))

  - **SPM**

    - update ff-a boot protocol documentation ([573ac37](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/573ac37373d3e8b2c31b3aaeed759e4656e060ec))

    - **EL3 SPMC**

      - allow BL32 specific defines to be used by SPMC_AT_EL3 ([2d65ea1](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/2d65ea1930d4ce26cc176a8c60e9401d0b4f862a))
      - add plat hook for memory transactions ([a8be4cd](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/a8be4cd057bce5f0b4ac6af396c0c870474d1ef4))
      - add EL3 SPMC #defines ([44639ab](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/44639ab73e43e0b79da834dff8c85266d68e5066))
      - introduce accessor function to obtain datastore ([6a0788b](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/6a0788bc0e704283e52c80990aa2bb6e047a0cc2))
      - add FF-A secure partition manager core ([5096aeb](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/5096aeb2ba646548a7a6ab59e975b996e6c9026a))
      - add FFA_FEATURES handler ([55a2963](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/55a296387b9720855df429a08c886f47a4a45057))
      - add FFA_PARTITION_INFO_GET handler ([f74e277](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/f74e27723bb54ad1318fa462fbcff70af555b2e6))
      - add FFA_RUN handler ([aad20c8](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/aad20c85cb6f4bc91318d3c6488cf72a20fdbe96))
      - add FFA_RX_RELEASE handler ([f0c25a0](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/f0c25a082fc8b891d4d21518028118561caa4735))
      - add function to determine the return path from the SPMC ([20fae0a](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/20fae0a7ce7fd407cd3efb7745017ee6ab605159))
      - add helper function to obtain endpoint mailbox ([f16b6ee](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/f16b6ee3deac93706efe465f399c9542e12d5eeb))
      - add helper function to obtain hyp structure ([a7c0050](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/a7c00505f85684326a223535a319c170d14826f6))
      - add helper to obtain a partitions FF-A version ([c2b1434](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/c2b1434369292081f907c548e496f59e197eb2f1))
      - add partition mailbox structs ([e1df600](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/e1df6008d9b4a00da25ec08fbdcbd3a5967fdb54))
      - add support for direct req/resp ([9741327](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/9741327df577c3f43db42b26bda607429e62af0b))
      - add support for FF-A power mgmt. messages in the EL3 SPMC ([59bd2ad](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/59bd2ad83c13ed3c84bb9b841032c95927358890))
      - add support for FFA_MSG_WAIT ([c4db76f](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/c4db76f066f236fe490ebc7a50833a04e08f5151))
      - add support for FFA_SPM_ID_GET ([46872e0](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/46872e01f5efb555fef8367595b59e5d2f75cec0))
      - add support for forwarding a secure interrupt to the SP ([729d779](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/729d7793f830781ff8ed44d144c3346c6e4251a3))
      - add support for handling FFA_ERROR ABI ([d663fe7](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/d663fe7a3002ff028c190eb732278b878e78b7b7))
      - add support for v1.1 FF-A boot protocol ([2e21921](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/2e21921502b1317031cf2a2f69c5d47ac88a505d))
      - add support for v1.1 FF-A memory data structures ([7e804f9](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/7e804f9695c48681c91e9e6fc6175eb6997df867))
      - enable building of the SPMC at EL3 ([1d63ae4](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/1d63ae4d0d8374a732113565be90d58861506e39))
      - enable checking of execution ctx count ([5b0219d](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/5b0219ddd5da42413f4c2be9302224b5b71295ff))
      - enable handling FF-A RX/TX Mapping ABIs ([1a75224](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/1a752245ecae6487844c57667e24b704e6df8079))
      - enable handling FFA_VERSION ABI ([0c7707f](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/0c7707fdf21fc2a8658f5a4bdfd2f8883d02ada5))
      - enable handling of the NS bit ([0560b53](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/0560b53e71ab6daefa8e75665a718605478746a4))
      - enable parsing of messaging methods from manifest ([3de378f](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/3de378ff8c9430c964cbe9b0c58fa5afc4d237ce))
      - enable parsing of UUID from SP Manifest ([857f579](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/857f5790da3770a9ca52416274eec4e545c9be53))
      - enable the SPMC to pass the linear core ID in a register ([f014300](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/f0143004e548582760aacd6f15f5499b18081a69))
      - prevent read only xlat tables with the EL3 SPMC ([70d986d](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/70d986ddbbf56a20c7550c079dd4dc9462332594))
      - support FFA_ID_GET ABI ([d5fe923](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/d5fe92350cb018ae7083ed26a6a16508ccd82a86))
      - allow forwarding of FFA_FRAG_RX/TX calls ([642db98](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/642db9840712044b9c496e04a7acd60580e54117))
      - enable handling of FF-A SMCs with the SPMC at EL3 ([bb01a67](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/bb01a67306f47271adde051e541c760028c1a0f1))
      - update SPMC init flow to use EL3 implementation ([6da7607](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/6da76075bf4b953d621aa15c379e62a5f785de3f))
      - add logical partition framework ([7affa25](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/7affa25cad400101c016082be2d102be0f4fce80))
      - add FF-A memory management code ([e0b1a6d](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/e0b1a6d59e57c0dbe87f5b8f8166f1123664f058))
      - prevent duplicated sharing of memory regions ([fef85e1](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/fef85e1e53fcf44e8d9ed50c89d8a764bf1b7738))
      - support multiple endpoints in memory transactions ([f0244e5](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/f0244e5dd1b8cbab75ef00c1b9b56eed5b3cad4b))

    - **SPMD**

      - forward FFA_VERSION from SPMD to SPMC ([9944f55](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/9944f55761c4d5cc1feefaf5e33bf7fb83d8f5f3))
      - enable SPMD to forward FFA_VERSION to EL3 SPMC ([9576fa9](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/9576fa93a2effc23a533b80dce41d7104a8d200b))
      - add FFA_MSG_SEND2 forwarding in SPMD ([c2eba07](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/c2eba07c47f8d831629104eeffcec11ed7d3b0a5))
      - add FFA_RX_ACQUIRE forwarding in SPMD ([d555233](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/d555233fe5a04dfd99fd6ac30bacc5284285c131))

    - **SPM MM**

      - add support to save and restore fp regs ([15dd6f1](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/15dd6f19da8ee4b20ba525e0a742d0df9e46e071))

- **Libraries**

  - **CPU Support**

    - add library support for Poseidon CPU ([1471475](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/1471475516cbf1b4a411d5ef853bd92d0edd542e))
    - add support for Cortex-X1 ([6e8eca7](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/6e8eca78e5db966e10e2fa2737e9be4d5af51fa9))
    - add L1PCTL macro definiton for CPUACTLR_EL1 ([8bbb1d8](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/8bbb1d80a58dbdf96fcabbdebbfbd21d2d5344a4))

  - **EL3 Runtime**

    - add arch-features detection mechanism ([6a0da73](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/6a0da73647546aea1d10b4b2347bac9d532bcb43))
    - replace ARM_ARCH_AT_LEAST macro with FEAT flags ([0ce220a](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/0ce220afb24f0511332b251952019d7011ccc282))

  - **FCONF**

    - add a helper to get image index ([9e3f409](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/9e3f409398af447b1d03001dd981007a9bb1617e))
    - add NS load address in configuration DTB nodes ([ed4bf52](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/ed4bf52c33b6860d58a2ffc946bd293ec76bbdaa))

  - **Standard C Library**

    - add support for length specifiers ([701e94b](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/701e94b08f382691b0deabd4df882abd87e17ab5))

  - **PSA**

    - add initial attestation API ([0848565](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/084856513d6730a50a3d65ac9c3bdae465117c40))
    - add measured boot API ([758c647](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/758c64715b691be92de623f81032494e38a43cc8))
    - mock PSA APIs ([0ce2072](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/0ce2072d9b9f419bb19595454395a33a5857ca2f))

- **Drivers**

  - **Generic Clock**

    - add a minimal clock framework ([847c6bc](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/847c6bc8e6d55b1c0f31a52407aa61515cd6c612))

  - **FWU**

    - add a function to pass metadata structure to platforms ([9adce87](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/9adce87efc8acc947b8b49d700c9773a7f071e02))
    - add basic definitions for GUID handling ([19d63df](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/19d63df1af72b312109b827cca793625ba6fcd16))
    - add platform hook for getting the boot index ([40c175e](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/40c175e75bc442674a5dc793c601b09681158ab9))
    - pass a const metadata structure to platform routines ([6aaf257](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/6aaf257de4a4070ebc233f35a09bce4c39ea038c))
    - simplify the assert to check for fwu init ([40b085b](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/40b085bddf60cf8c533b379ccb41e6668c5080dd))

  - **Measured Boot**

    - add RSS backend ([0442ebd](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/0442ebd2e9bcf5fa4344d8fa8ef4b69a3b249e33))

  - **GUID Partition Tables Support**

    - add a function to identify a partition by GUID ([3cb1065](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/3cb1065581f6d9a8507af8dbca3779d139aa0ca7))
    - cleanup partition and gpt headers ([2029f93](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/2029f930097b0c3b1b1faa660032d16ed01a5c86))
    - copy the partition GUID into the partition structure ([7585ec4](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/7585ec4d36ebb7e286cfec959b2de084eded8201))
    - make provision to store partition GUID value ([938e8a5](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/938e8a500a25a949cfd25f0cb79f6c1359c9b40c))
    - verify crc while loading gpt header ([a283d19](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/a283d19f82ddb635d9d9fa061e7fd956167ebe60))

  - **Arm**

    - **GIC**

      - allow overriding GICD_PIDR2_GICV2 address ([a7521bd](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/a7521bd5d887bfd69d99a55a81416e38ba9ebc97))

      - **GIC-600AE**

        - disable SMID for unavailable blocks ([3f0094c](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/3f0094c15d433cd3de413a4633a4ac2b8e1d1f2e))
        - enable all GICD, PPI, ITS SMs ([6a1c17c](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/6a1c17c770139c00395783e7568220d61264c247))
        - introduce support for RAS error handling ([308dce4](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/308dce40679f63db504cd3d746a0c37a2a05f473))

    - **SMMU**

      - add SMMU abort transaction function ([6c5c532](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/6c5c5320511ab8202fb9eccce9e66b4e4e0d9a33))
      - configure SMMU Root interface ([52a314a](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/52a314af254966a604e192fcc3326737354f217a))

    - **MHU**

      - add MHU driver ([af26d7d](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/af26d7d6f01068809f17cc2d49a9b3d573c640a9))

    - **RSS**

      - add RSS communication driver ([ce0c40e](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/ce0c40edc93aa403cdd2eb6c630ad23e28b01c3e))

    - **TZC**

      - **TZC-380**

        - add sub-region register definition ([fdafe2b](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/fdafe2b5ead66a1b5175db77bcc7cedafa14a059))

  - **Marvell**

    - **Armada**

      - **A3K**

        - **A3720**

          - preserve x1/x2 regs in console_a3700_core_init() ([7c85a75](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/7c85a7572960efbaabe20c9db037bcec66be3e98))

  - **MediaTek**

    - **APU**

      - add mt8195 APU clock and pll SiP call ([296b590](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/296b590206aa6db51e5c82b1a97a4f9707b49c4d))
      - add mt8195 APU iommap regions ([339e492](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/339e4924a7a3fd11bc176e0bf3e01d76133d364c))
      - add mt8195 APU mcu boot and stop SiP call ([88906b4](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/88906b443734399be5c07a5bd690b63d3d82cefa))

  - **NXP**

    - **DCFG**

      - add Chassis 3 support ([df02aee](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/df02aeeec640d2358301e903d9c8c473d455be9e))
      - add gic address align register definition ([3a8c9d7](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/3a8c9d78d4c65544d789bd64bd005ac10b5b352d))
      - add some macro definition ([1b29fe5](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/1b29fe534b8732193850fced2da1dc449450bd3b))

    - **NXP Crypto**

      - add chassis 3 support ([d60364d](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/d60364d48e31b33b57049d848b7462eb0e0de612))

    - **DDR**

      - add rawcard 1F support ([f2de48c](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/f2de48cb143c20ccd7a9c141df3d34cae74049de))
      - add workaround for errata A050958 ([291adf5](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/291adf521a54a365e54964bff4dae53d51c65936))

    - **GIC**

      - add some macros definition for gicv3 ([9755fd2](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/9755fd2ec2894323136715848910b13053cfe0ce))

    - **CSU**

      - add bypass bit mask definition ([ec5fc50](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/ec5fc501f15922967bf5d8260072ba1f9aec9640))

    - **IFC NAND**

      - add IFC NAND flash driver ([28279cf](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/28279cf2c141caf5e4e7156f874cde6f5a0d271b))

    - **IFC NOR**

      - add IFC nor flash driver ([e2fdc77](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/e2fdc77ba4eee91f0d1490e34f0fff552fc55dc9))

    - **TZC-380**

      - add tzc380 platform driver support ([de9e57f](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/de9e57ff1f3769e770eac44b94127eb7239a63f2))

  - **ST**

    - introduce fixed regulator driver ([5d6a264](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/5d6a2646f7759a5a2b3daed0d8aef4588c552ba4))

    - **Clock**

      - add clock driver for STM32MP13 ([9be88e7](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/9be88e75c198b08c508d8e470964720a781294b3))
      - assign clocks to the correct BL ([7418cf3](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/7418cf397916c97cb4ecf159b1f497a84299b695))
      - check HSE configuration in serial boot ([31e9750](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/31e9750bc17bd472d4f2a3db297461efc301be51))
      - define secure and non-secure gate clocks ([aaa09b7](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/aaa09b713c6f539fb5b2ee7e2dfd75f2d46875f5))
      - do not refcount on non-secure clocks in bl32 ([3d69149](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/3d69149a7e9e9a899d57f48bee26f98614f88935))
      - manage disabled oscillator ([bcccdac](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/bcccdacc7e7b7b985df942b3fae26cb9038a2574))

    - **DDR**

      - add read valid training support ([5def13e](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/5def13eb01ebac5656031bdc388a215d012fdaf8))

    - **GPIO**

      - allow to set a gpio in output mode ([53584e1](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/53584e1d5b2b843ea3bb9e01e3f01ea7c364ee6a))
      - do not apply secure config in BL2 ([fc0aa10](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/fc0aa10a2cd3cab887a8baa602891d1f45db2537))
      - add a function to reset a pin ([737ad29](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/737ad29bf992a7a79d538d1e0b47c7f38d9a4b9d))

    - **SDMMC2**

      - allow compatible to be defined in platform code ([6481a8f](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/6481a8f1e045ac80f0325b8bfe7089ba23deaf7b))
      - manage cards power cycle ([258bef9](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/258bef913aa76ead1b10c257d1695d9c0ef1c79d))

    - **ST PMIC**

      - add pmic_voltages_init() function ([5278ec3](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/5278ec3faf2010fd6aea1d8cd4294dd229c5c21d))
      - register the PMIC to regulator framework ([85fb175](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/85fb175b5ef854bc4607db98a4cfb5f35d822cee))

    - **STPMIC1**

      - add new services ([ea552bf](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/ea552bf5a57b573a6b09e396e3466b3c4af727f0))
      - add USB OTG regulators ([13fbfe0](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/13fbfe046e71393961d2c70a4f748a15f9c15f77))

    - **Regulator**

      - add support for regulator-always-on ([9b4ca70](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/9b4ca70d97d9a2556752b511ff9fe52012faff02))
      - add a regulator framework ([d5b4a2c](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/d5b4a2c4e7fd0bcb9f08584b242e69a2e591fb71))

    - **UART**

      - manage oversampling by 8 ([1f60d1b](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/1f60d1bd33d434b0c82a74e276699ee5a2f63833))
      - add uart driver for STM32MP1 ([165ad55](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/165ad5561ef598ea6261ba082610eeff3f208df7))

- **Miscellaneous**

  - **Debug**

    - update print_memory_map.py ([d16bfe0](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/d16bfe0feffe6a20399fb91d86fd8f7282b941dd))

  - **DT Bindings**

    - add bindings for STM32MP13 ([1b8898e](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/1b8898eb32c3872a34fc59f4216736f23af0c6ea))
    - add TZC400 bindings for STM32MP13 ([24d3da7](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/24d3da76d221390bb47d501c2ed77a1a7d2b42e7))

  - **FDT Wrappers**

    - add function to find or add a sudnode ([dea8ee0](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/dea8ee0d3f13f8d1638745b76e86bd7617bf92e7))

  - **FDTs**

    - add the ability to supply idle state information ([2b2b565](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/2b2b565717cc0299e75e8806004d1a3548e9fbf7))

    - **STM32MP1**

      - add DDR support for STM32MP13 ([e6fddbc](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/e6fddbc995947d4e5a5dc6607c76cd46fdd840e2))
      - add DT files for STM32MP13 ([3b99ab6](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/3b99ab6e370a01caec14bc5422a86001eaf291b8))
      - add nvmem_layout node and OTP definitions ([ff8767c](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/ff8767cbfc2bb851a2f6cc32fbe3693ddbfb7d12))
      - add st-io_policies node for STM32MP13 ([2bea351](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/2bea35122d102492f18c427535ce6c9b7016e356))
      - add support for STM32MP13 DK board ([2b7f7b7](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/2b7f7b751f4b0f7a8a0f4a35407af22cc269e529))
      - update NVMEM nodes ([375b79b](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/375b79bb4a773fe6a5dd971272c72bf12155050e))

- **Documentation**

  - context management refactor proposal ([3274226](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/327422633bef112a10579d4daeca0f596cd02911))

  - **Threat Model**

    - Threat Model for TF-A v8-R64 Support ([dc66922](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/dc669220d5666c2c808bc11ba81c86a9b071271a))

- **Tools**

  - **Secure Partition Tool**

    - add python SpSetupActions framework ([b1e6a41](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/b1e6a41572240839e62099aa00298174b18c696a))
    - delete c version of the sptool ([f4ec476](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/f4ec47613fef8db8037195147dc2ac6fb6f154ff))
    - python version of the sptool ([2e82874](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/2e82874cc9b7922e000dd4d7718e3153e347b1d7)
    - use python version of sptool ([822c727](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/822c72791f791d26e233df0c15a655c3dbd8b117))

### Resolved Issues

- **Architecture**

  - **Activity Monitors Extension (FEAT_AMU)**

    - add default value for ENABLE_FEAT_FGT and ENABLE_FEAT_ECV flags ([820371b](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/820371b13028a6f620a62cf73a951883d051666b))
    - fault handling on EL2 context switch ([f74cb0b](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/f74cb0be8ac80eb3072555cb04eb09375d4cb31f))
    - limit virtual offset register access to NS world ([a4c3945](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/a4c394561af31ae0417ed9ff3b3152adb7cd5355))

  - **Scalable Vector Extension (FEAT_SVE)**

    - disable ENABLE_SVE_FOR_NS for AARCH32 ([24ab2c0](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/24ab2c0af74be174acf755a36b3ebba867184e60))

- **Platforms**

  - **Allwinner**

    - improve DTB patching error handling ([79808f1](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/79808f10c32d441572666551b1545846079af15b))

  - **Arm**

    - fix fvp and juno build with USE_ROMLIB option ([861250c](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/861250c3b26d64f859f5f37686e453d5074fa976))
    - increase ARM_BL_REGIONS count ([dcb1959](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/dcb1959161935aa58d2bb852f3cef0b96458a4e1))
    - remove reclamation of functions starting with "init" ([6c87abd](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/6c87abdda400354ebf4f5351086c32a4620475c9))
    - use PLAT instead of TARGET_PLATFORM ([c5f3de8](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/c5f3de8dabc9b955b6051a6c6116d40b10a84f5d))
    - fix SP count limit without dual root CoT ([9ce15fe](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/9ce15fe8910580efca46b9f102e117402ce769db))

    - **FVP**

      - FCONF Trace Not Shown ([0c55c10](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/0c55c10305df6217fd978d58ce203dbad3edd4d5))
      - disable reclaiming init code by default ([fdb9166](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/fdb9166b9494402eb2da7e0b004c121b322725e0))
      - extend memory map to include all DRAM memory regions ([e803542](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/e80354212f591c8813dec27353e8241e03155b4c))
      - fix NULL pointer dereference issue ([a42b426](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/a42b426b8548e3304e995f1a49d2470d71072949))
      - op-tee sp manifest doesn't map gicd ([69cde5c](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/69cde5cd9563f0c665862f1e405ae8e8d2818c6e))

    - **Morello**

      - change the AP runtime UART address ([07302a2](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/07302a23ec1af856b3d4de0439161a8c23414f84))
      - fix SoC reference clock frequency ([e8b7a80](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/e8b7a80436c2bc81c61fc4703d6580f2fe9226a9))
      - include errata workaround for 1868343 ([f94c84b](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/f94c84baa2a2bad75397b0ec6a0922fe8a475847))

    - **SGI**

      - disable SVE for NS to support SPM_MM builds ([78d7e81](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/78d7e819798ace643b6e22025dc76aedb199bbd5))

    - **TC**

      - remove the bootargs node ([68fe3ce](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/68fe3cec25bc9ea4e1bafdb1d9f5315e245d650b))

    - **Corstone-1000**

      - change base address of FIP in the flash ([1559450](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/1559450132c5e712f4d6896e53e4f1cb521fa465))

  - **Broadcom**

    - allow build to specify mbedTLS absolute path ([903d574](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/903d5742953d9d4b224e71d8b1e62635e83f44a9))
    - fix the build failure with mbedTLS config ([95b5c01](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/95b5c0126b802b894ea0177d973978e06b6a254d))

  - **Intel**

    - add flash dcache after return response for INTEL_SIP_SMC_MBOX_SEND_CMD ([ac097fd](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/ac097fdf07ad63b567ca751dc518f8445a0baef6))
    - allow non-secure access to FPGA Crypto Services (FCS) ([4837a64](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/4837a640934630f8034ceec1bb84cc40673d8a6b))
    - always set doorbell to SDM after sending command ([e93551b](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/e93551bb3bd8ac43779fa70c7363ee2568da45ca))
    - assert if bl_mem_params is NULL pointer ([35fe7f4](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/35fe7f400a7f1d65ff2fee5531d20f6c2f3e6f39))
    - bit-wise configuration flag handling ([276a436](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/276a43663e8e315fa1bf0aa4824051d88705858b))
    - change SMC return arguments for INTEL_SIP_SMC_MBOX_SEND_CMD ([108514f](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/108514ff7160a86efb791449a4635ffe0f9fdf2c))
    - configuration status based on start request ([e40910e](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/e40910e2dc3fa59bcce83ec1cf9a33b3e85012c4))
    - define macros to handle buffer entries ([7db1895](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/7db1895f0be2f8c6710bf51d8441d5e53e3ef0fe))
    - enable HPS QSPI access by default ([000267b](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/000267be22d3c0077c0fd0a8377ceeed5aada4c3))
    - extend SDM command to return the SDM firmware version ([c026dfe](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/c026dfe38cfae379457a6ef53130bd5ebc9d7808))
    - extending to support large file size for AES encryption and decryption ([dcb144f](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/dcb144f1fbcef73ddcc448d5ed6134aa279069b6))
    - extending to support large file size for SHA-2 ECDSA data signing and signature verifying ([1d97dd7](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/1d97dd74cd128edd7ad45b725603444333c7b262))
    - extending to support large file size for SHA2/HMAC get digest and verifying ([70a7e6a](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/70a7e6af958f3541476a8de6baac8e376fcc67f9))
    - fix bit masking issue in intel_secure_reg_update ([c9c0709](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/c9c070994caedf123212aad23b6942122c5dd793))
    - fix configuration status based on start request ([673afd6](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/673afd6f8e7266900b00a7cbeb275fe1a3d69cce))
    - fix ddr address range checker ([12d71ac](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/12d71ac6627bb6822a0314e737794a8503df79dd))
    - fix ECC Double Bit Error handling ([c703d75](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/c703d752cce4fd101599378e72db66ccf53644fa))
    - fix fpga config write return mechanism ([ef51b09](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/ef51b097bfa906bf1cee8ee641a1b7bcc8c5f3c0))
    - flush dcache before sending certificate to mailbox ([49d44ec](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/49d44ec5f357b1bcf8eae9e91fbd72aef09e00dd))
    - get config status OK status ([07915a4](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/07915a4fd5848fbac69dcbf28f00353eed10a942))
    - introduce a generic response error code ([651841f](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/651841f20110ce6fac650e3ac47b0a9cce18e6f3))
    - make FPGA memory configurations platform specific ([f571183](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/f571183b066b1a91b7fb178c3aad9d6360d1918c))
    - modify how configuration type is handled ([ec4f28e](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/ec4f28ecec8887a685d6119c096ad346da1ea53e))
    - null pointer handling for resp_len ([a250c04](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/a250c04b0cc807f626df92a7091ff13b3a3aa9ed))
    - refactor NOC header ([bc1a573](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/bc1a573d5519f121cb872fce1d88fe2e0db07b2c))
    - reject non 4-byte align request size for FPGA Crypto Service (FCS) ([52ed157](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/52ed157fd66812debb13a792c21f763de01aef70))
    - remove redundant NOC header declarations ([58690cd](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/58690cd629b4ccdefe5313f805219598074a3501))
    - remove unused printout ([0d19eda](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/0d19eda0dd2ffae27d0551b1f0a06a2b8f96c853))
    - update certificate mask for FPGA Attestation ([fe5637f](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/fe5637f27aebfdab42915c2ced2c34d8685ee2bb))
    - update encryption and decryption command logic ([02d3ef3](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/02d3ef333d4a0a07a3e40defb12a8cde3a7cba03))
    - use macro as return value ([e0fc2d1](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/e0fc2d1907b1c8a062c44a435be77a12ffeed84b))

  - **Marvell**

    - **Armada**

      - **A3K**

        - change fatal error to warning when CM3 reset is not implemented ([30cdbe7](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/30cdbe7043832f7bd96b40294ac062a8fc9c540f))
        - fix comment about BootROM address range ([5a60efa](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/5a60efa12a57cde98240f861e45609cb9b94d58d))

  - **Mediatek**

    - **MT8186**

      - remove unused files in drivers/mcdi ([bc714ba](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/bc714bafe7ae8ca29075ba9bf3985c0e15ae0f64))
      - extend MMU region size ([0fe7ae9](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/0fe7ae9c64aa6f6d5b06a80de9c88081057d5dbe))

  - **NVIDIA**

    - **Tegra**

      - **Tegra 194**

        - remove incorrect erxctlr assert ([e272c61](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/e272c61ce8185deb397dcf168ec72bdaa5926a33))

  - **NXP**

    - fix total dram size checking ([0259a3e](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/0259a3e8282ed17c1d378a27f820f44b3bebab07))
    - increase soc name maximum length ([3ccd7e4](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/3ccd7e45a2c3ff9fa7794f0284c9d0298e7cb982))

    - **i.MX**

      - **i.MX 8M**

        - check the validation of domain id ([eb7fb93](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/eb7fb938c3ce34ccfb143ae8ba695df899098436))

        - **i.MX 8M Plus**

          - change the BL31 physical load address ([32d5042](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/32d5042204e8b41caa4c0c1ed5b48bad9f1cb1b5))

    - **Layerscape**

      - fix build issue of mmap_add_ddr_region_dynamically ([e2818d0](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/e2818d0afc20a60d302f85f4c915e4ae4cc3cb9c))
      - fix coverity issue ([5161cfd](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/5161cfde9bfaa3a715d160fcd4870f276adad332))
      - update WA for Errata A-050426 ([72feaad](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/72feaad980cdc472868bc95914202bf57ed51b2d))

      - **LX2**

        - drop erratum A-009810 ([e36b0e4](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/e36b0e4910aea56f90a6ab9b8cf3dc4008220031))

  - **Renesas**

    - **R-Car**

      - **R-Car 3**

        - change stack size of BL31 ([d544dfc](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/d544dfcc4959d203b06dbfb85fb0ad895178b379))
        - fix SYSTEM_OFF processing for R-Car D3 ([1b49ba0](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/1b49ba0fde5eb9e47fe50152c192579101feb718))
        - fix to bit operation for WUPMSKCA57/53 ([82bb6c2](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/82bb6c2e88314a5b3f2326c95095c3b20a389947))

  - **Socionext**

    - **Synquacer**

      - initialise CNTFRQ in Non Secure CNTBaseN ([4d4911d](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/4d4911d77d4d59c7dd18d7fc3724ddb1fa3582b7))

  - **ST**

    - add missing header include ([b1391b2](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/b1391b294ca7803f46bc47048b4a02a15dda9a16))
    - don't try to read boot partition on SD cards ([9492b39](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/9492b391a35c66e1e7630e95347259191b28314d))
    - fix NULL pointer dereference issues ([2deff90](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/2deff904a953c6a87331ab6830ab80e3889d9e23))
    - manage UART clock and reset only in BL2 ([9e52d45](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/9e52d45fdf619561e0a7a833b77aaacc947a4dfd))
    - remove extra chars from dtc version ([03d2077](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/03d20776efc20a04a5191a4f39965079a4d60b3c))

    - **ST32MP1**

      - add missing debug.h ([356ed96](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/356ed961184847dcd088cfcda44b71eeb0ef2377))
      - correct dtc version check ([429f10e](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/429f10e3367748abd33b4f6f9ee362c0ba74dd95))
      - correct include order ([ff7675e](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/ff7675ebf94999618dbde14bb59741cefb2b2edd))
      - correct types in messages ([43bbdca](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/43bbdca04f5a20bb4e648e18fc63061b6a6e4ecf))
      - deconfigure UART RX pins ([d7176f0](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/d7176f0319cd399aae9a906e5d78e67b32e183f5))
      - do not reopen debug features ([21cfa45](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/21cfa4531a76a7c3cad00e874400b97e2f68723c))
      - fix enum prints ([ceab2fc](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/ceab2fc3442dbda1c4beaff3c4fe708a04c02303))
      - include assert.h to fix build failure ([570c71b](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/570c71b20a195ade510f5d584c69325d2634c50b))
      - remove interrupt_provider warning for dtc ([ca88c76](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/ca88c761d34854ed3e0b16b9c5f39b0790d320ab))
      - restrict DEVICE2 mapping in BL2 ([db3e0ec](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/db3e0ece7157181a3529d14172368003eb63dc30))
      - rework switch/case for MISRA ([f7130e8](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/f7130e81cf9c3682232bb9319b1798184b44920f))
      - set reset pulse duration to 31ms ([9a73a56](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/9a73a56c353d32742e03b828647562bdbe2ddbb2))

  - **Xilinx**

    - fix coding style violations ([bb1768c](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/bb1768c67ea06ac466e2cdc7e5338c3d23dac79d))
    - fix mismatching function prototype ([81333ea](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/81333eac716b25a9fd112cc4f5990e069f3bdb40))

    - **Versal**

      - resolve misra R10.1 in pm services ([775bf1b](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/775bf1bbd32c2df47f4ff597eb8a452d2983e590))
      - resolve misra R10.3 ([b2bb3ef](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/b2bb3efb8f590f31b1205c51d56be1dd6f473fbb))
      - resolve misra R10.3 in pm services ([5d1c211](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/5d1c211e225d40d2926bf34483c90f907a6c5dc3))
      - resolve misra R10.6 ([93d4625](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/93d462562727f4f428e6f975a972226dafbfd305))
      - resolve misra R10.6 in pm services ([fa98d7f](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/fa98d7f2f8752e37f740b43f533547288552a393))
      - resolve misra R14.4 ([a62c40d](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/a62c40d42703d5f60a8d80938d2cff721ee131bd))
      - resolve misra R15.6 ([b9fa2d9](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/b9fa2d9fc154feffe78e677ace54b0e34f011439))
      - resolve misra R15.6 in pm services ([4156719](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/4156719550ceddf5b1b4a47464fb32f7506e0dca))
      - resolve misra R15.7 ([bc2637e](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/bc2637e3799dbc9642447ddb719e0262347b1309))
      - resolve misra R16.3 in pm services ([27ae531](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/27ae5310883b0db7d4e2dd4fbc1fd58e675f75b5))
      - resolve misra R17.7 ([526a1fd](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/526a1fd1472874561988777f8ecd8b87734a0671))
      - resolve misra R20.7 in pm services ([5dada62](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/5dada6227b949ef702bfab7986bc083689afdaf7))
      - resolve misra R7.2 ([0623dce](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/0623dcea0f6e7a5c9d65413445df8a96a2b40d42))
      - fix coverity scan warnings ([0b15187](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/0b15187225a9134e3acbc7693646b21d43617b3b))
      - fix the incorrect log message ([ea04b3f](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/ea04b3fe183b6661f656b4cc38cb93a73d9bc202))

    - **ZynqMP**

      - define and enable ARM_XLAT_TABLES_LIB_V1 ([c884c9a](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/c884c9a55b167383ff3d96d2d0a30ac6842bcc86))
      - query node status to power up APU ([b35b556](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/b35b556718b60b78cb5d96b0c137e2fe82eb0086))
      - resolve misra 7.2 warnings ([5bcbd2d](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/5bcbd2de127292f3ad076217e08468388c6844b0))
      - resolve misra 8.3 warnings ([944e7ea](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/944e7ea94f2594e2b128c671cf7415265302596b))
      - resolve misra R10.3 ([2b57da6](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/2b57da6c91ebe14588e63e5a24f31ef32711eca2))
      - resolve misra R14.4 warnings ([dd1fe71](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/dd1fe7178b578916b1e133b7c65c183e1f994371))
      - resolve misra R15.6 warnings ([eb0d2b1](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/eb0d2b17722c01a22bf3ec1123f7bed2bf891b09))
      - resolve misra R15.7 warnings ([16de22d](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/16de22d037644359ef2a04058134f9c326b36633))
      - resolve misra R16.3 warnings ([e7e5d30](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/e7e5d30308ccfb931f7b6d0afa6c5c23971e95c0))
      - resolve misra R8.4 warnings ([610eeac](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/610eeac89438d603435bde694eb4ddab07f46e45))
      - update the log message to verbose ([1277af9](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/1277af9bacca36b46d7aa341187bb3abef84332f))
      - use common interface for eemi apis ([a469c1e](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/a469c1e1f4c1cd69f98ce45d6e0709de091b8cb3))

- **Bootloader Images**

  - **BL1**

    - invalidate SP in data cache during secure SMC ([f1cbbd6](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/f1cbbd6332bb85672dc72cbcc4ac7023323c6936))

  - **BL2**

    - correct messages with image_id ([e4c77db](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/e4c77db9c80d87009611a3079454877e6ce45a04))
    - define RAM_NOLOAD for XIP ([cc562e7](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/cc562e74101d800b0b0ee3422fb7f4f8321ae2b7))

- **Services**

  - **RME**

    - enable/disable SVE/FPU for Realms ([a4cc85c](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/a4cc85c129d031d9c887cf59b1baeaef18a43010))
    - align RMI and GTSI FIDs with SMCCC ([b9fd2d3](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/b9fd2d3ce3d4e543a2e04dc237cd4e7ff7765c7a))
    - preserve x4-x7 as per SMCCCv1.1 ([1157830](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/11578303fd04a8da36fddb5e6de44f026bf4d24c))

    - **TRP**

      - Distinguish between cold and warm boot ([00e8113](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/00e8113145aa12d89db72068bdd3157f08575d14))

  - **SPM**

    - **EL3 SPMC**

      - fix incorrect FF-A version usage ([25eb2d4](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/25eb2d41a6d2ede1e945bbc67ae3f740b92a40bb))
      - fix FF-A memory transaction validation ([3954bc3](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/3954bc3c03439dbdc7029cf2418c79a037918ce4))

- **Libraries**

  - **CPU Support**

    - workaround for  Cortex-A710 2282622 ([ef934cd](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/ef934cd17c30dcc39cd9022a1c4e9523ec8ba617))
    - workaround for  Cortex-A710 erratum 2267065 ([cfe1a8f](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/cfe1a8f7123f0dc8376b2075cc6e8e32b13739b2))
    - workaround for Cortex A78 AE erratum 2376748 ([92e8708](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/92e870843e9bd654fd1041d66f284c19ca9c0d4f))
    - workaround for Cortex A78 AE erratum 2395408 ([3f4d81d](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/3f4d81dfd26649fbcbbbe993a9f0236f5bb07c8a))
    - workaround for Cortex X2 erratum 2002765 ([34ee76d](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/34ee76dbdfeee85f123cb903ea95dbee5e9a44a5))
    - workaround for Cortex X2 erratum 2058056 ([e16045d](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/e16045de50e8b430e6601ba0e1e47097d8310f3d))
    - workaround for Cortex X2 erratum 2083908 ([1db6cd6](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/1db6cd60279e2d082876692a65cf9c532f506a69))
    - workaround for Cortex-A510 erratum 1922240 ([8343563](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/83435637bfafbf1ce642a5fabb52e8d7b2819e36))
    - workaround for Cortex-A510 erratum 2041909 ([e72bbe4](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/e72bbe47ba7f2a0087654fd99ae24b5b7b444943))
    - workaround for Cortex-A510 erratum 2042739 ([d48088a](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/d48088acbe400133037ae74acf1b722b059119bb))
    - workaround for Cortex-A510 erratum 2172148 ([c0959d2](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/c0959d2c460cbf7c14e7ba2a57d69ecddae80fd8))
    - workaround for Cortex-A510 erratum 2218950 ([cc79018](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/cc79018b71e45acb524fc5d429d394497ad53646))
    - workaround for Cortex-A510 erratum 2250311 ([7f304b0](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/7f304b02a802b7293d7a8b4f4030c5ff00158404))
    - workaround for Cortex-A510 erratum 2288014 ([d5e2512](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/d5e2512c6b86409686f5d1282922ebdf72459fc2))
    - workaround for Cortex-A710 erratum 2008768 ([af220eb](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/af220ebbe467aa580e6b9ba554676f78ffec930f))
    - workaround for Cortex-A710 erratum 2136059 ([8a855bd](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/8a855bd24329e081cf13a257c7d2dc3ab4e5dcca))
    - workaround for Cortex-A78 erratum 2376745 ([5d796b3](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/5d796b3a25150faff68013880f5a9350cbc53889))
    - workaround for Cortex-A78 erratum 2395406 ([3b577ed](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/3b577ed53d104cfb324390b7519da5e7744d1001))
    - workaround for Cortex-X2 errata 2017096 ([e7ca443](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/e7ca4433fa591233e7e2912b689ab56e531f9775))
    - workaround for Cortex-X2 errata 2081180 ([c060b53](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/c060b5337a43cd42f55b99d83096bb44b51b5335))
    - workaround for Cortex-X2 erratum 2147715 ([63446c2](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/63446c27d11453faacfddecffa44d3880615d412))
    - workaround for Cortex-X2 erratum 2216384 ([4dff759](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/4dff7594f94f1e788aef709cc5b3d079693b6242))
    - workaround for DSU-110 erratum 2313941 ([7e3273e](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/7e3273e8e4dca44e7cb88a827b94e662fa8f83e9))
    - workaround for Rainier erratum 1868343 ([a72144f](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/a72144fb7a30c2782a583a3b0064e741d1fe2c9f))
    - workarounds for cortex-x1 errata ([7b76c20](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/7b76c20d8eb4271b381371ce0d510fbe6ad825bf))
    - use CPU_NO_EXTRA3_FUNC for all variants ([b2ed998](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/b2ed99894d326993961680fb8e786c267a712400))

  - **EL3 Runtime**

    - set unset pstate bits to default ([7d33ffe](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/7d33ffe4c116506ed63e820d5b6edad81680cd11))

    - **Context Management**

      - add barrier before el3 ns exit ([0482503](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/04825031b2384a08504821f39e98e23bb6f93f11))
      - remove registers accessible only from secure state from EL2 context ([7f41bcc](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/7f41bcc76d8857b4678c90796ebd85794ff3ee5f))
      - refactor the cm_setup_context function ([2bbad1d](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/2bbad1d126248435e26f9d0d9f5920d8806148d7))
      - remove initialization of EL2 registers when EL2 is used ([fd5da7a](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/fd5da7a84731e9687f56c263ff3aa8ebed75075a))
      - add cm_prepare_el3_exit_ns function ([8b95e84](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/8b95e8487006ff77a7d84fba5bd20ba7e68d8330))
      - refactor initialization of EL1 context registers ([b515f54](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/b515f5414b00a8b7ca9b21363886ea976bd19914))

  - **FCONF**

    - correct image_id type in messages ([cec2fb2](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/cec2fb2b1a8359bf1f349a5b8c8a91a1845f4ca1))

  - **PSCI**

    - correct parent_node type in messages ([b9338ee](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/b9338eee7fbcac7f4b55f27b064572e847810422))

  - **GPT**

    - rework delegating/undelegating sequence ([6a00e9b](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/6a00e9b0c8c37fc446f83ef63e95a75353e31e8b))

  - **Translation Tables**

    - fix bug on VERBOSE trace ([956d76f](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/956d76f69d0c96829784c5a6d16aa79e4e0ecab1))

  - **Standard C Library**

    - correct some messages ([a211fde](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/a211fde940d4dbd8e95e4f352af2a066a4f89f30))
    - fix snprintf corner cases ([c1f5a09](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/c1f5a0925ddf84981d9e176d146bfddb48eb45d1))
    - limit snprintf radix value ([b30dd40](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/b30dd4030dcef950eac05393013ee019c3cb3205))
    - snprintf: include stdint.h ([410c925](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/410c925ab31693dc74d654ff9167c8eed3ec5a62))

  - **Locks**

    - add __unused for clang ([5a030ce](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/5a030ce4aed271344087bca723903e10fef59ac9))

- **Drivers**

  - **FWU**

    - rename is_fwu_initialized ([aae7c96](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/aae7c96de63914c954f0fc64cd795844832483fc))

  - **I/O**

    - **MTD**

      - correct types in messages ([6e86b46](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/6e86b462490429fee6db877338a649b0e199b0ec))

  - **Measured Boot**

    - add RMM entry to event_log_metadata ([f4e3e1e](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/f4e3e1e85f64d8930e89c1396bc9785512f656bd))

  - **MTD**

    - correct types in messages ([6e86b46](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/6e86b462490429fee6db877338a649b0e199b0ec))

  - **SCMI**

    - add missing \n in ERROR message ([0dc9f52](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/0dc9f52a2a9f0b9686c65dd60c84e0bcca552144))
    - make msg_header variable volatile ([99477f0](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/99477f051ef857a1e0600cb98858fc74c007e1ff))
    - use same type for message_id ([2355ebf](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/2355ebff6f6312086868f44b8ad7f821f6385208))

  - **UFS**

    - delete call to inv_dcache_range for utrd ([c5ee858](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/c5ee8588bf9a36075723e5aacceefa93fd2de8c9))
    - disables controller if enabled ([b3f03b2](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/b3f03b20135fc5fcd5e6ec7e5ca49f1e59b5602e))
    - don't zero out buf before ufs read ([2ef6b8d](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/2ef6b8d378e7f7c1b1eb7abe176989c3f996f2dc))
    - don't zero out the write buffer ([cd3ea90](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/cd3ea90b200534b8c9d81619731c9ce198478a3c))
    - fix cache maintenance issues ([38a5ecb](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/38a5ecb756e217a80ed951747797ab150449ee9b))
    - move nutrs assignment to ufs_init ([0956319](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/0956319b580726029ddc4e00cde6c5a348b99052))
    - read and write attribute based on spec ([a475518](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/a475518337e15935469543b1cce353e5b337ef52))

  - **Arm**

    - **GIC**

      - **GICv3**

        - fix iroute value wrong issue ([65bc2d2](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/65bc2d224b836c230888796c4eda455997dccd8b))

    - **TZC**

      - **TZC-400**

        - correct message with filter ([bdc88d2](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/bdc88d2154448957f452cb472ff95ccec5808ca1))

  - **Marvell**

    - **COMPHY**

      - change reg_set() / reg_set16() to update semantics ([95c26d6](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/95c26d6489bd8b2fc8b8e14bc2da5d2918055acc))

      - **Armada 3700**

        - drop MODE_REFDIV constant ([9fdecc7](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/9fdecc72f0fce17ca2cd8e4c3b26c01262166d10))
        - fix comment about COMPHY status register ([4bcfd8c](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/4bcfd8c02e3e3aa27b55dedeed11fb16bac991a9))
        - fix comments about selector register values ([71183ef](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/71183ef6654c2a485458307a84ce7c473524689a))
        - fix Generation Setting registers names ([e5a2aac](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/e5a2aac5bbc6dedb20edcc8e7850be2813cb668b))
        - fix PIN_PU_IVREF register name ([c9f138e](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/c9f138ebfef90d5b7b5651f06efd81bcbc55366b))
        - fix reference clock selection value names ([6ba97f8](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/6ba97f83dbb314b076588b97415a4078924e1903))
        - fix SerDes frequency register value name ([bdcf44f](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/bdcf44f1af496e06b693b781fe16bbc2a05fa365))
        - use reg_set() according to update semantics ([4d01bfe](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/4d01bfe66522b13f0d9042206e986551c94fc01e))

    - **Armada**

      - **A3K**

        - **A3720**

          - configure UART after TX FIFO reset ([15546db](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/15546dbf40e5ea81a982a1e6d1e5ba729b06ae51))
          - do external reset during initialization ([0ee80f3](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/0ee80f35a28d651d243a6d56678800f9697d14c0))

  - **NXP**

    - ddr: corrects mapping of HNFs nodes ([e3a2349](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/e3a234971abb2402cbf376eca6fcb657a7709fae))

    - **QSPI**

      - fix include path for QSPI driver ([ae95b17](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/ae95b1782b7a3ab9bbe46ae9ab31f48fb6ebe137))

    - **NXP Crypto**

      - refine code to avoid hang issue for some of toolchain ([fa7fdfa](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/fa7fdfabf07d91439b0869ffd8e805f0166294bf))

    - **DDR**

      - fix coverity issue ([f713e59](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/f713e5954e0906443cd20ae97e229ddbb9ab7005))

  - **ST**

    - **Clock**

      - check _clk_stm32_get_parent return ([b8eab51](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/b8eab512bf9d253f96b0333ee0f1bffa1afc3170))
      - correct stm32_clk_parse_fdt_by_name ([7417cda](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/7417cda6aeaf6abf48dfbe22dc965b626f61c613))
      - correct types in error messages ([44fb470](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/44fb470b7f298645ac31ada4491553824d77d934))
      - initialize pllcfg table ([175758b](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/175758b2777eb6df3c4aefd79448e97e76a15272))
      - print enums as unsigned ([9fa9a0c](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/9fa9a0c55cc830e609415d2cedd2d34fcbec1008))

    - **DDR**

      - add missing debug.h ([15ca2c5](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/15ca2c5e14abe415e70d08fb595973dd3e3b0af9))
      - correct DDR warnings ([a078134](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/a078134e2305ca5695731bc275a5ca892cc38880))

     - **FMC**

      - fix type in message ([afcdc9d](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/afcdc9d8d71e2b60071d3d34704f0e598e67a514))

    - **SDMMC2**

      - check regulator enable/disable return ([d50e7a7](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/d50e7a71cb5f8ecfbe2eb69c163d532bab82cbf0))
      - correct cmd_idx type in messages ([bc1c98a](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/bc1c98a8c79b6f72395123ea8ed857a488746d4b))

    - **ST PMIC**

      - add static const to pmic_ops ([57e6018](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/57e6018305a97f4e3627d16d8b1886419f274b4a))
      - correct verbose message ([47065ff](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/47065ffe44c701b231322ec7160c8624d50a9deb))

    - **SPI**

      - always check SR_TCF flags in stm32_qspi_wait_cmd() ([55de583](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/55de58323e458b38b455439a8846cb663deb5508))
      - remove SR_BUSY bit check before sending command ([5993b91](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/5993b9157fd049d06194083032771ffcf73da086))

    - **UART**

      - correctly fill BRR register ([af7775a](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/af7775ab535138ff49643f749110dca143d4122c))

  - **USB**

    - correct type in message ([bd9cd63](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/bd9cd63ba096cb16161efa4df40f957421660df1))

- **Miscellaneous**

  - **AArch64**

    - fix encodings for MPAMVPM* registers ([e926558](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/e92655849d0a9e5893eb2d7e5f42cf8b931d4db6))

  - **FDTs**

    - **STM32MP1**

      - correct memory mapping for STM32MP13 ([99605fb](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/99605fb1166794db1dedf1b7280cb184945c229c))
      - remove mmc1 alias if not needed ([a0e9724](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/a0e972438b99012da422411c8e504a19bdad44a2))

  - **PIE**

    - align fixup_gdt_reloc() for aarch64 ([5ecde2a](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/5ecde2a271ac0f3762c16f5a277a70e55e172f0b))
    - do not skip __RW_END__ address during relocation ([4f1a658](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/4f1a658f899a169e702b1c7146b59f7c04b0338b))

  - **Security**

    - apply SMCCC_ARCH_WORKAROUND_3 to A73/A75/A72/A57 ([9b2510b](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/9b2510b69de26cc7f571731b415f6dec82669b6c))
    - loop workaround for CVE-2022-23960 for Cortex-A76 ([a10a5cb](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/a10a5cb609045de216c01111ec3fcf09a092da0b))
    - report CVE 2022 23960 missing for aarch32 A57 and A72 ([2e5d7a4](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/2e5d7a4b6b26d9d8b6c8e580c33d877e591b1fb3))
    - update Cortex-A15 CPU lib files for CVE-2022-23960 ([187a617](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/187a61761ef5d59bed0c94cca725bd6f116f64d0))
    - workaround for CVE-2022-23960 ([c2a1521](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/c2a15217c3053117f4d39233002cb1830fa96670))
    - workaround for CVE-2022-23960 ([1fe4a9d](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/1fe4a9d181ead0dcb2bc494e90552d3e7f0aaf4c))
    - workaround for CVE-2022-23960 for A76AE, A78AE, A78C ([5f802c8](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/5f802c8832f3c5824ca6de17593205ebbf8bf585))
    - workaround for CVE-2022-23960 for Cortex-A57, Cortex-A72 ([be9121f](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/be9121fd311ff48c94f3d90fe7efcf84586119e4))
    - workaround for CVE-2022-23960 for Cortex-X1 ([e81e999](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/e81e999b9da33ab5d2d3e5185b1ad7c46046329c))

- **Tools**

  - **NXP Tools**

    - fix create_pbl print log ([31af441](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/31af441a0445d4a5e88ddcc371c51b3701c25839))
    - fix tool location path for byte_swape ([a89412a](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/a89412a649020367a3ed0f87658ee131cd3dcd18))

  - **Firmware Image Package Tool**

    - avoid packing the zero size images in the FIP ([ab556c9](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/ab556c9c646f1b5f1b500449a5813a4eecdc0302))
    - respect OPENSSL_DIR ([0a956f8](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/0a956f81805b46b1530f30dd79d16950dc491a7b)

  - **Secure Partition Tool**

    - add leading zeroes in UUID conversion ([b06344a](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/b06344a3f2c5a0fede3646627f37d1fce3d3d585))
    - update Optee FF-A manifest ([ca0fdbd](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/ca0fdbd8e0d625ece0f87ca16eacabf13db70921))

  - **Certificate Creation Tool**

    - let distclean Makefile target remove the cert_create tool ([e15591a](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/e15591aaf47ab45941f0d7a03abf3e4a830ac1d9))

- **Dependencies**

  - **commitlint**

    - change scope-case to lower-case ([804e52e](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/804e52e9a770de72913f27b5bc9e7dd965e114c5))

## [2.6.0](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/refs/tags/v2.5..refs/tags/v2.6) (2021-11-22)

### ⚠ BREAKING CHANGES

- **Architecture**

  - **Activity Monitors Extension (FEAT_AMU)**

    - The public AMU API has been reduced to enablement only
      to facilitate refactoring work. These APIs were not previously used.

      **See:** privatize unused AMU APIs ([b4b726e](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/b4b726ea868359cf683c07337b69fe91a2a6929a))

    - The `PLAT_AMU_GROUP1_COUNTERS_MASK` platform definition
      has been removed. Platforms should specify per-core AMU counter masks
      via FCONF or a platform-specific mechanism going forward.

      **See:** remove `PLAT_AMU_GROUP1_COUNTERS_MASK` ([6c8dda1](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/6c8dda19e5f484f8544365fd71d965f0afc39244))

- **Libraries**

  - **FCONF**

    - FCONF is no longer added to BL1 and BL2 automatically
      when the FCONF Makefile (`fconf.mk`) is included. When including this
      Makefile, consider whether you need to add `${FCONF_SOURCES}` and
      `${FCONF_DYN_SOURCES}` to `BL1_SOURCES` and `BL2_SOURCES`.

      **See:** clean up source collection ([e04da4c](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/e04da4c8e132f43218f18ad3b41479ca54bb9263))

- **Drivers**

  - **Arm**

    - **Ethos-N**

      - multi-device support

        **See:** multi-device support ([1c65989](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/1c65989e70c9734defc666e824628620b2060b92))

### New Features

- **Architecture**

  - **Activity Monitors Extension (FEAT_AMU)**

    - enable per-core AMU auxiliary counters ([742ca23](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/742ca2307f4e9f82cb2c21518819425e5bcc0f90))

  - **Support for the `HCRX_EL2` register (FEAT_HCX)**

    - add build option to enable FEAT_HCX ([cb4ec47](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/cb4ec47b5c73e04472984acf821e6be41b98064f))

  - **Scalable Matrix Extension (FEAT_SME)**

    - enable SME functionality ([dc78e62](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/dc78e62d80e64bf4fe5d5bf4844a7bd1696b7c92))

  - **Scalable Vector Extension (FEAT_SVE)**

    - enable SVE for the secure world ([0c5e7d1](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/0c5e7d1ce376cabcebebc43dbf238fe4482ab2dc))

  - **System Register Trace Extensions (FEAT_ETMv4, FEAT_ETE and FEAT_ETEv1.1)**

    - enable trace system registers access from lower NS ELs ([d4582d3](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/d4582d30885673987240cf01fd4f5d2e6780e84c))
    - initialize trap settings of trace system registers access ([2031d61](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/2031d6166a58623ae59034bc2353fcd2fabe9c30))

  - **Trace Buffer Extension (FEAT_TRBE)**

    - enable access to trace buffer control registers from lower NS EL ([813524e](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/813524ea9d2e4138246b8f77a772299e52fb33bc))
    - initialize trap settings of trace buffer control registers access ([40ff907](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/40ff90747098ed9d2a09894d1a886c10ca76cee6))

  - **Self-hosted Trace Extension (FEAT_TRF)**

    - enable trace filter control register access from lower NS EL ([8fcd3d9](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/8fcd3d9600bb2cb6809c6fc68f945ce3ad89633d))
    - initialize trap settings of trace filter control registers access ([5de20ec](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/5de20ece38f782c8459f546a08c6a97b9e0f5bc5))

  - **RME**

    - add context management changes for FEAT_RME ([c5ea4f8](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/c5ea4f8a6679131010636eb524d2a15b709d0196))
    - add ENABLE_RME build option and support for RMM image ([5b18de0](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/5b18de09e80f87963df9a2e451c47e2321b8643a))
    - add GPT Library ([1839012](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/1839012d5b5d431f7ec307230eae9890a5fe7477))
    - add Realm security state definition ([4693ff7](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/4693ff7225faadc5ad1bcd1c2fb3fbbb8fe1aed0))
    - add register definitions and helper functions for FEAT_RME ([81c272b](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/81c272b3b71af38bc5cfb10bbe5722e328a1578e))
    - add RMM dispatcher (RMMD) ([77c2775](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/77c2775323a5ff8b77230f05c0cc57f830e9f153))
    - add Test Realm Payload (TRP) ([50a3056](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/50a3056a3cd33d395e8712e1d1e67a8840bf3db1))
    - add xlat table library changes for FEAT_RME ([3621823](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/362182386bafbda9e6671be921fa30cc20610d30))
    - disable Watchdog for Arm platforms if FEAT_RME enabled ([07e96d1](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/07e96d1d2958b6f121476fd391ac67bf8c2c4735))
    - run BL2 in root world when FEAT_RME is enabled ([6c09af9](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/6c09af9f8b36cdfa1dc4d5052f7e4792f63fa88a))

- **Platforms**

  - **Allwinner**

    - add R329 support ([13bacd3](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/13bacd3bc3e6b76009adf9183e5396b6457eb12c))

  - **Arm**

    - add FWU support in Arm platforms ([2f1177b](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/2f1177b2b9ebec3b2fe92607cd771bda1dc9cbfc))
    - add GPT initialization code for Arm platforms ([deb4b3a](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/deb4b3a63e3a52f2e9823865a1932f6289ccb7ac))
    - add GPT parser support ([ef1daa4](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/ef1daa420f7b2920b2ee35379de2aefed6ab2605))
    - enable PIE when RESET_TO_SP_MIN=1 ([7285fd5](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/7285fd5f9aa6d9cc0e0f1dc9c71785b46a88d999))

    - **FPGA**

      - add ITS autodetection ([d7e39c4](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/d7e39c43f2f58aabb085ed7b8f461f9ece6002d0))
      - add kernel trampoline ([de9fdb9](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/de9fdb9b5925ae08137d4212a85e9a1d319509c9))
      - determine GICR base by probing ([93b785f](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/93b785f5ae66a6418581c304c83a346e8baa5aa3))
      - query PL011 to learn system frequency ([d850169](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/d850169c9c233c4bc413d8319196557b54683688))
      - support GICv4 images ([c69f815](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/c69f815b09ab85d3ace8fd2979ffafb1184ec76c))
      - write UART baud base clock frequency into DTB ([422b44f](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/422b44fb56db7ca8b1a2f9f706733d7d4c2fdeb1))

    - **FVP**

      - enable external SP images in BL2 config ([33993a3](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/33993a3737737a03ee5a9d386d0a027bdc947c9c))
      - add memory map for FVP platform for FEAT_RME ([c872072](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/c8720729726faffc39ec64f3a02440a48c8c305a))
      - add RMM image support for FVP platform ([9d870b7](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/9d870b79c16ef09b0c4a9db18e071c2fa235d1ad))
      - enable trace extension features by default ([cd3f0ae](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/cd3f0ae6f855b2998bc09e5c3a458528c92acb90))
      - pass Event Log addr and size from BL1 to BL2 ([0500f44](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/0500f4479eb1d0d5ab9e83dac42b633a5ff677dd))

    - **FVP-R**

      - support for TB-R has been added
      - configure system registers to boot rich OS ([28bbbf3](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/28bbbf3bf583e0c85004727e694455dfcabd50a4))

    - **RD**

      - **RD-N2**

        - add support for variant 1 of rd-n2 platform ([fe5d5bb](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/fe5d5bbfe6bd0f386f92bdc419a7e04d885d5b43))
        - add tzc master source ids for soc dma ([3139270](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/3139270693ab0fc6d66fed4fe11e183829b47e2e))

    - **SGI**

      - add CPU specific handler for Neoverse N2 ([d932a58](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/d932a5831e26620d61d171d0fd8bc2f14938e6f1))
      - add CPU specific handler for Neoverse V1 ([cbee43e](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/cbee43ebd69377bce1c4fa8d40c6fd67f2be2ee4))
      - increase max BL2 size ([7186a29](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/7186a29bbfe3044d5e8001ddfe1d9238578e0944))
      - enable AMU for RD-V1-MC ([e8b119e](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/e8b119e03ad9de5fc440e5929287c94c22fc3946))
      - enable use of PSCI extended state ID format ([7bd64c7](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/7bd64c70e91f73a236b84fb51d5045e308479b5a))
      - introduce platform variant build option ([cfe1506](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/cfe1506ee8303d9e0714b3a5b2cd165f76ad5d11))

    - **TC**

      - enable MPMM ([c19a82b](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/c19a82bef08df58350f1b6668e0604ff8a5bd46d))
      - Enable SVE for both secure and non-secure world ([10198ea](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/10198eab3aa7b0eeba10d9667197816b052ba3e4))
      - populate HW_CONFIG in BL31 ([34a87d7](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/34a87d74d9fbbe8037431ea5101110a9f1cf30e1))
      - introduce TC1 platform ([6ec0c65](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/6ec0c65b09745fd0f4cee44ee3aa99870303f448))
      - add DRAM2 to TZC non-secure region ([76b4a6b](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/76b4a6bb208c22b1c5971964a209ff7d54982348))

      - add bootargs node ([4a840f2](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/4a840f27cd7a05d8e3687aa325adcd019c0d22ee))
      - add cpu capacity to provide scheduling information ([309f593](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/309f5938e610c73cb51b3ba175fed971f49d0888))
      - add Ivy partition ([a19bd32](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/a19bd32ed14c33571f3715198d47bac9d0f2808e))
      - add support for trusted services ([ca93248](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/ca9324819ee308f9b3a4bb004f02a512c8f301f6))
      - update Matterhorn ELP DVFS clock index ([a2f6294](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/a2f6294c98935895d4592ef7e30058ca6e995f4b))
      - update mhuv2 dts node to align with upstream driver ([63067ce](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/63067ce87e4afa193b2c7f6a4917d1e54b61b000))

    - **Diphda**

      - adding the diphda platform ([bf3ce99](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/bf3ce9937182e5d8d91e058baabb8213acedacdb))
      - disabling non volatile counters in diphda ([7f70cd2](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/7f70cd29235cc5e96ff6b5f509c7e4260bec5610))
      - enabling stack protector for diphda ([c7e4f1c](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/c7e4f1cfb84136a7521f26e403a6635ffdce4a2b))

  - **Marvell**

    - introduce t9130_cex7_eval ([d01139f](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/d01139f3b59a1bc6542e74f52ff3fb26eea23c69))

    - **Armada**

      - **A8K**

        - allow overriding default paths ([0b702af](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/0b702afc3aabc349a513a5b00397b58a62fea634))

  - **MediaTek**

    - enable software reset for CIRQ ([b3b162f](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/b3b162f3b48e087f6656513862a6f9e1fa0757b1))

    - **MT8192**

      - add DFD control in SiP service ([5183e63](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/5183e637a0496ad8dfbd8c892bc874ac6a1531bf))

    - **MT8195**

      - add DFD control in SiP service ([3b994a7](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/3b994a75306cc487144dd8e2e15433799e62e6f2))
      - add display port control in SiP service ([7eb4223](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/7eb42237575eb3f241c9b22efc5fe91368470aa6))
      - remove adsp event from wakeup source ([c260b32](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/c260b3246b6be27c7463d36ce7f76368c94a8540))
      - add DCM driver ([49d3bd8](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/49d3bd8c4c80ecd19ecfd74812ff1eaa01478cdd))
      - add EMI MPU basic drivers ([75edd34](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/75edd34ade8efaa8a76c5fd59103454023632989))
      - add SPM suspend driver ([859e346](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/859e346b89461f31df17b76ef25ce9e8d2a7279d))
      - add support for PTP3 ([0481896](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/048189637ead887787bd5bc47b1dfab98f321705))
      - add vcore-dvfs support ([d562130](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/d562130ea9637b885135a5efe41cb98f2365754f))
      - support MCUSYS off when system suspend ([d336e09](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/d336e093dd9ec917ce69484eae8914d98efa328d))

  - **NXP**

    - add build macro for BOOT_MODE validation checking ([cd1280e](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/cd1280ea2e5c8be6f28485a2d5054d06e54e74c1))
    - add CCI and EPU address definition ([6cad59c](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/6cad59c429b4382ad62aee3a67fa1b3fd4ad38b7))
    - add EESR register definition ([8bfb168](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/8bfb16813aff9b3dcbeaa2f77027d44b97f04b6d))
    - add SecMon register definition for ch_3_2 ([66f7884](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/66f7884b5229b1d2977d73d105af1c34cb55f95d))
    - define common macro for ARM registers ([35efe7a](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/35efe7a4cea4b3c55b661aac49ef1a85ca8feaa9))
    - define default PSCI features if not defined ([a204785](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/a2047853224083328ef67cacbc17a2001ba14701))
    - define default SD buffer ([4225ce8](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/4225ce8b87635287ecf5cd3baaf31ea703a2640b))

    - **i.MX**

      - **i.MX 8M**

        - add sdei support for i.MX8MN ([ce2be32](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/ce2be321e8a5865871810b36c580181ea95a1a64))
        - add sdei support for i.MX8MP ([6b63125](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/6b63125c415491417e1c389e4015be5ebdee2841))
        - add SiP call for secondary boot ([9ce232f](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/9ce232fe985a0bb308af459ede8a22629255d4e7))
        - add system_reset2 implementation ([60a0dde](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/60a0dde91bd03f4011c1d52d4d3aea8166e939a0))

        - **i.MX 8M Mini**

          - enlarge BL33 (U-boot) size in FIP ([d53c9db](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/d53c9dbf9ff9c435552b62f47fb95bfe86d025e3))

        - **i.MX 8M Plus**

          - add imx8mp_private.h to the build ([91566d6](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/91566d663b26434813fa674412bb695be1965557))
          - add in BL2 with FIP ([75fbf55](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/75fbf5546b7beca93e4782bc35906f9536392e04))
          - add initial definition to facilitate FIP layout ([f696843](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/f696843eab5cf0547b6c6307eaccea25678654c4))
          - enable Trusted Boot ([a16ecd2](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/a16ecd2cff36b3a8a76d223f4e272e165c941b31))

    - **Layerscape**

      - add ls1028a soc and board support ([52a1e9f](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/52a1e9ff37251987b71b743951038cd8d1fa0ba4))

      - **LX2**

        - add SUPPORTED_BOOT_MODE definition ([28b3221](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/28b3221aebdd48577e2288a75cd2f7547da514e9))

        - **LS1028A**

          - add ls1028a soc support ([9d250f0](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/9d250f03d7a38cac86655495879b2151b877db0d))

          - **LS1028ARDB**

            - add ls1028ardb board support ([34e2112](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/34e2112d1a3a8e4ea33a24bdc6505518266333a9))

  - **QTI**

    - **SC7280**

      - add support for pmk7325 ([b8a0511](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/b8a05116ed2a87a9689c4f9be6218a4bce88034a))
      - support for qti sc7280 plat ([46ee50e](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/46ee50e0b34e19d383a28bc3b3dadbfb4c07b270))

  - **Renesas**

    - **R-Car**

      - change process for Suspend To RAM ([731aa26](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/731aa26f38d76645b6d50077c28dffb9b02dd08a))

      - **R-Car 3**

        - add a DRAM size setting for M3N ([f95d551](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/f95d551217a287bd909aa3c82f4ade4986ad7244))
        - add new board revision for Salvator-XS/H3ULCB ([4379a3e](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/4379a3e9744cf3b0844446335aca40357a889b9a))
        - add optional support for gzip-compressed BL33 ([ddf2ca0](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/ddf2ca03979ea9fad305b1bc59beb6e27f0e1c02))
        - add process of SSCG setting for R-Car D3 ([14f0a08](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/14f0a0817297905c03ddf2c4c6040482ef71d744))
        - add process to back up X6 and X7 register's value ([7d58aed](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/7d58aed3b05fa8c677a7c823c1ca5017a462a3d3))
        - add SYSCEXTMASK bit set/clear in scu_power_up ([63a7a34](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/63a7a34706eedba4d13ce6fc661a634801cf8909))
        - apply ERRATA_A53_1530924 and ERRATA_A57_1319537 ([2892fed](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/2892fedaf27d8bbc68780a4a2c506c768e81b9f1))
        - change the memory map for OP-TEE ([a4d821a](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/a4d821a5a625d941f95ec39fb51ac4fc07c46c5c))
        - emit RPC status to DT fragment if RPC unlocked ([12c75c8](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/12c75c8886a0ee69d7e279a48cbeb8d1602826b3))
        - keep RWDT enabled ([8991086](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/899108601a0c3b08ead5e686d92ea0794700ff35))
        - modify LifeC register setting for R-Car D3 ([5460f82](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/5460f82806752e419fdd6862e8ca9c5fefbee3f2))
        - modify operation register from SYSCISR to SYSCISCR ([d10f876](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/d10f87674ecee54cffe1ab554cc05733fd16c7f0))
        - modify SWDT counter setting for R-Car D3 ([053c134](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/053c134683cf74fbf4efad311815b806821f1436))
        - remove access to RMSTPCRn registers in R-Car D3 ([71f2239](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/71f2239f53cd3137ad6abdaf0334dc53f2f21cb1))
        - update DDR setting for R-Car D3 ([042d710](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/042d710d1d917357c5142b340c79978264d3afb1))
        - update IPL and Secure Monitor Rev.3.0.0 ([c5f5bb1](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/c5f5bb17abfcf6c0eeb3e6c3d70499de0bd6abc0))
        - use PRR cut to determine DRAM size on M3 ([42ffd27](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/42ffd279dd1a686b19e2f1b69d2e35413d5efeba))

  - **ST**

    - add a new DDR firewall management ([4584e01](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/4584e01dc643665038004f6c8a4f8bd64e14dacb))
    - add a USB DFU stack ([efbd65f](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/efbd65fa7b5cf70f20d6b18152741ccdf8a65bb6))
    - add helper to save boot interface ([7e87ba2](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/7e87ba2598a07facdeb73237dcb350a261ac17b6))
    - add STM32CubeProgrammer support on USB ([afad521](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/afad5214a79259f56bc2003b00859abfe8a18d4d))
    - add STM32MP_EMMC_BOOT option ([214c8a8](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/214c8a8d08b2b3c24f12cbc69f497f44851ca524))
    - create new helper for DT access ([ea97bbf](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/ea97bbf6a001b270fd0a25b4b0d0c382e277f3f8))
    - implement platform functions for SMCCC_ARCH_SOC_ID ([3d20178](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/3d201787e8246022b1f193283c12e7cb4bfc83ff))
    - improve FIP image loading from MMC ([18b415b](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/18b415be9d631b3e0c3a3caacc5f02edb9413f6b))
    - manage io_policies with FCONF ([d5a84ee](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/d5a84eeaac2c8ce14d3f2662dc9523b4abf41516))
    - use FCONF to configure platform ([29332bc](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/29332bcd680ce7e5f864813d9a900360f5e35d41))
    - use FIP to load images ([1d204ee](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/1d204ee4ab12893fceb12097bd4f0a074be253b2))

    - **ST32MP1**

      - add STM32MP_USB_PROGRAMMER target ([fa92fef](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/fa92fef0a024cdb537fe56c84a0156cc48c1ac2d))
      - add USB DFU support for STM32MP1 ([942f6be](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/942f6be211d4816ad2568d30d807b8fd53d7f981))

  - **Xilinx**

    - **Versal**

      - add support for SLS mitigation ([302b4df](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/302b4dfb8fb0041959b8593a098ccae6c61e3238))

    - **ZynqMP**

      - add support for runtime feature config ([578f468](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/578f468ac058bbb60b08f78e2aa2c20cdc601620))
      - sync IOCTL IDs ([38c0b25](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/38c0b2521a0ea0951f4e1ee678ccdbce5fc07a98))
      - add SDEI support ([4143268](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/4143268a5ca8f91f1014e0d83edf766946ffff76))
      - add support for XCK26 silicon ([7a30e08](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/7a30e08b70e7fbb745554d500182bb6e258c5ab8))
      - extend DT description by TF-A ([0a8143d](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/0a8143dd636d4234dd2e79d32cb49dc80675c68f))

- **Bootloader Images**

  - import BL_NOBITS_{BASE,END} when defined ([9aedca0](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/9aedca021d917c7435aa2a0405972aa9d44493a2))

- **Services**

  - **FF-A**

    - adding notifications SMC IDs ([fc3f480](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/fc3f480023e3a52460add25f18dd550dde44d9ff))
    - change manifest messaging method ([bb320db](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/bb320dbc4751f7ea0c37ffba07d14628e58081d0))
    - feature retrieval through FFA_FEATURES call ([96b71eb](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/96b71eb9597efbf4857216cac1caeefc9e8bbf3e))
    - update FF-A version to v1.1 ([e1c732d](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/e1c732d46fa91231b39209621ead1e5a5fb2c497))
    - add Ivy partition to tb fw config ([1bc02c2](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/1bc02c2e0f63b6a7863e10cf6189292d42e693db))
    - add support for FFA_SPM_ID_GET ([70c121a](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/70c121a258e43dc2462ed528b44d92594ffb27b3))
    - route secure interrupts to SPMC ([8cb99c3](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/8cb99c3fc3539bb9926e73a1c33fd72f424fc453))

- **Libraries**

  - **CPU Support**

    - add support for Hayes CPU ([7bd8dfb](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/7bd8dfb85a8bf5c22d6a39f4538b89cc748090d1))
    - add support for Hunter CPU ([fb9e5f7](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/fb9e5f7bb76e9764b3ecd7973668c851015fa1b4))
    - add support for Demeter CPU ([f4616ef](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/f4616efafbc1004f1330f515b898e7617e338875))
    - workaround for Cortex A78 AE erratum 1941500 ([47d6f5f](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/47d6f5ff16d1f2ad009d630a381054b10fa0a06f))
    - workaround for Cortex A78 AE erratum 1951502 ([8913047](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/8913047a52e646877812617a2d98cff99494487b))

  - **MPMM**

    - add support for MPMM ([6812078](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/68120783d6d6f99c605e9f746ee0e91e2908feb1))

  - **OP-TEE**

    - introduce optee_header_is_valid() ([b84a850](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/b84a850864c05fef587fcbb301f955428966de64))

  - **PSCI**

    - require validate_power_state to expose CPU_SUSPEND ([a1d5ac6](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/a1d5ac6a5aa5d9d18a481de20d272f64a71391f7))

  - **SMCCC**

    - add bit definition for SMCCC_ARCH_SOC_ID ([96b0596](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/96b0596ea25e1f03b862a5bfaa92add6c3e51a33))

- **Drivers**

  - **FWU**

    - add FWU metadata header and build options ([5357f83](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/5357f83d4ee89fb831d7e4f6149ae2f652e1b9af))
    - add FWU driver ([0ec3ac6](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/0ec3ac60d86b75d132e7a63fc09ea47e67f90bbd))
    - avoid booting with an alternate boot source ([4b48f7b](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/4b48f7b56577a78cdc9a2b47280cb62cbae0f7c3))
    - avoid NV counter upgrade in trial run state ([c0bfc88](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/c0bfc88f8e8e03974834cbcacbbfbd5f202a2857))
    - initialize FWU driver in BL2 ([396b339](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/396b339dc20b97ddd75146e03467a255e28f31b9))
    - introduce FWU platform-specific functions declarations ([efb2ced](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/efb2ced256dacbab71ca11cbc87f70f413ca6729))

  - **I/O**

    - **MTD**

      - offset management for FIP usage ([9a9ea82](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/9a9ea82948fd2f1459b6351cb0641f3f77b4e6de))

  - **Measured Boot**

    - add documentation to build and run PoC ([a125c55](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/a125c556230501ee0f5ec9f8b0b721625d484a41))
    - move init and teardown functions to platform layer ([47bf3ac](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/47bf3ac31ec84d4b221fdef760c04b5f4416cba4))
    - image hash measurement and recording in BL1 ([48ba034](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/48ba0345f7b42880ec4442d7e90e3e1af95feadd))
    - update tb_fw_config with event log properties ([e742bcd](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/e742bcdae0d28dc14a2aa0b4ca30f50420bb5ebe))

  - **MMC**

    - boot partition read support ([5014b52](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/5014b52dec0c2527ca85c0fbe9c9281a24cc7b10))

  - **MTD**

    - **NAND**

      - count bad blocks before a given offset ([bc3eebb](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/bc3eebb25d5ee340e56047d0e46b81d5af85ff17))

  - **SCMI**

    - add power domain protocol ([7e4833c](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/7e4833cdde8235d228f1f1c40f52b989ad5aa98a))

  - **Arm**

    - **Ethos-N**

      - multi-device support ([1c65989](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/1c65989e70c9734defc666e824628620b2060b92))

    - **GIC**

      - **GICv3**

        - detect GICv4 feature at runtime ([858f40e](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/858f40e379684fefc8b52c7b9e60576bc3794a69))
        - introduce GIC component identification ([73a643e](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/73a643eed9d88910a09ca666bc7ab7f5e532324e))
        - multichip: detect GIC-700 at runtime ([feb7081](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/feb7081863f454b9e465efc074ca669f7a4c783d))

        - **GIC-600AE**

          - introduce support for Fault Management Unit ([2c248ad](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/2c248ade2e958eed33127b4ea767fbb7499f31a7))

    - **TZC**

      - **TZC-400**

        - update filters by region ([ce7ef9d](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/ce7ef9d146ce5ca6b9be5ef049377b3817d53d10))

  - **MediaTek**

    - **APU**

      - add mt8192 APU device apc driver ([f46e1f1](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/f46e1f18539d6d992c82ae605c2cd2a1d0757fa4))
      - add mt8192 APU iommap regions ([2671f31](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/2671f3187249d641c55929c812d6691aeeff502a))
      - add mt8192 APU SiP call support ([ca4c0c2](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/ca4c0c2e78eb19d442de4608d9096a755b540a37))
      - setup mt8192 APU_S_S_4 and APU_S_S_5 permission ([77b6801](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/77b6801966d203e09ca118fad42543e934d73e6f))

    - **EMI MPU**

      - add MPU support for DSP ([6c4973b](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/6c4973b0a9a75aa83233b696c97d573426eebd98))

  - **NXP**

    - **DCFG**

      - define RSTCR_RESET_REQ ([6c5d140](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/6c5d140ed99cfec47b239acc242c0f3db1e3bf7c))

    - **FLEXSPI**

      - add MT35XU02G flash info ([a4f5015](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/a4f5015a0080134251e9272719f5dad1ce2aa842))

  - **Renesas**

    - **R-Car3**

      - add extra offset if booting B-side ([993d809](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/993d809cc115ce23dd2df1df19dc8bb548cc19cd))
      - add function to judge a DDR rank ([726050b](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/726050b8e2d2ee2234e103e2df55f9c7f262c851))

  - **ST**

    - manage boot part in io_mmc ([f3d2750](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/f3d2750aa2293c0279bc447a85771827ca8b74c1))

    - **USB**

      - add device driver for STM32MP1 ([9a138eb](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/9a138eb5f29f6747e181a1b3b4199ad57721a3e0))

  - **USB**

    - add a USB device stack ([859bfd8](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/859bfd8d42341c6dea2b193db79dc4828e074ad7))

- **Miscellaneous**

  - **Debug**

    - add new macro ERROR_NL() to print just a newline ([fd1360a](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/fd1360a339e84ccd49f8a2d8a42e4c131a681b3c))

  - **CRC32**

    - **Hardware CRC32**

      - add support for HW computed CRC ([a1cedad](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/a1cedadf73863ff103fecd64fa188334e1541337))

    - **Software CRC32**

      - add software CRC32 support ([f216937](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/f21693704a7bac275e12b44ae30fd210bc317175))

  - **DT Bindings**

    - add STM32MP1 TZC400 bindings ([43de546](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/43de546b909947ab44f104aaee02b98fba70f44c))

  - **FDT Wrappers**

    - add CPU enumeration utility function ([2d9ea36](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/2d9ea360350303e37a8dd39f3599ac88aaef0ff9))

  - **FDTs**

    - add for_each_compatible_node macro ([ff76614](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/ff766148b52bfecf09728a83fc3becc7941d943c))
    - introduce wrapper function to read DT UUIDs ([d13dbb6](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/d13dbb6f1d5e28737a3319af035a6cb991bc6f8f))
    - add firewall regions into STM32MP1 DT ([86b43c5](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/86b43c58a4105c8cef13d860dd73fa9bd560526a))
    - add IO policies for STM32MP1 ([21e002f](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/21e002fb777fad9d02a94dc961f077fb444517fa))
    - add STM32MP1 fw-config DT files ([d9e0586](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/d9e0586b619b331eb2db75911ca82f927e20bd1c))

    - **STM32MP1**

      - align DT with latest kernel ([e8a953a](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/e8a953a9b85806f7324c8c7245435d5b9226c279))
      - delete nodes for non-used boot devices ([4357db5](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/4357db5b17ce6ba7357dd99276f34ab497ce60ef))

  - **NXP**

    - **OCRAM**

      - add driver for OCRAM initialization ([10b1e13](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/10b1e13bd200849ff134dd8d2fde341a8526f563))

    - **PSCI**

      - define CPUECTLR_TIMER_2TICKS ([3a2cc2e](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/3a2cc2e262890cffee1fc46835e85be6055189e8))

- **Dependencies**

  - **libfdt**

    - also allow changing base address ([4d585fe](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/4d585fe52feb231d5e73ec50a505122d5e9bf450))

### Resolved Issues

- **Architecture**

- **Platforms**

  - print newline before fatal abort error message ([a5fea81](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/a5fea8105887d0dd15edf94aebd591b1b6b5ef05))

  - **Allwinner**

    - delay after enabling CPU power ([86a7429](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/86a7429e477786dad6fab002538aef825f4ca35a))

  - **Arm**

    - correct UUID strings in FVP DT ([748bdd1](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/748bdd19aa27c15438d829bdba42fe4062a265a1))
    - fix a VERBOSE trace ([5869ebd](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/5869ebd0e87f1de987e51994103440fa8c77b26f))
    - remove unused memory node ([be42c4b](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/be42c4b4bf3c44f2970b7a1658c46b8d5863cad1))

    - **FPGA**

      - allow build after MAKE_* changes ([9d38a3e](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/9d38a3e698331e3c8192cc3e0cc8584e6ed987d9))
      - avoid re-linking from executable ELF file ([a67ac76](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/a67ac7648cd814ed8f8d4ece1b265c6d48c6dc81))
      - Change PL011 UART IRQ ([195381a](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/195381a91313bc0bce2cfa087f3c55136a9e8496))
      - limit BL31 memory usage ([d457230](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/d4572303ed45faceffed859955b0e71724fddfd2))
      - reserve BL31 memory ([13e16fe](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/13e16fee86451e2f871c2aac757b32299fe5ead6))
      - streamline generated axf file ([9177e4f](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/9177e4fd9356b0f249be8b6fe14f222e10f1e6cd))
      - enable AMU extension ([d810e30](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/d810e30dd6b47e0725dccbcb42ca0a0c5215ee34))
      - increase initrd size ([c3ce73b](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/c3ce73be0bfe31fa28805fe92b3e727232ffd37a))

    - **FVP**

      - fix fvp_cpu_standby() function ([3202ce8](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/3202ce8bbb4af8580736d2a1634ad45c3f89d931))
      - spmc optee manifest remove SMC allowlist ([183725b](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/183725b39d75e362a32b3c5d0be110c255c56bdd))
      - allow changing the kernel DTB load address ([672d669](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/672d669d6c72f92c6b81464d1d421e392bc1aa3e))
      - bump BL2 stack size ([d22f1d3](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/d22f1d358731f0f55f2f392fa587f0fa8d315aa5))
      - provide boot files via semihosting ([749d0fa](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/749d0fa80d1c7ca30b4092a381a06deeeaf1747f))
      - OP-TEE SP manifest per latest SPMC changes ([b7bc51a](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/b7bc51a7a747bf40d219b2041e5b3ce56737a71b))

    - **FVP-R**

      - fix compilation error in release mode ([7d96e79](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/7d96e79a1a2efdf85f1ed46cdd5c577b58054f53))

    - **Morello**

      - initialise CNTFRQ in Non Secure CNTBaseN ([7f2d23d](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/7f2d23d9d790df90021de6c5165ef10fe5cc5590))

    - **TC**

      - enable AMU extension ([b5863ca](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/b5863cab9adb3fed0c1e4dfb92cf906794e7bdb4))
      - change UUID to string format ([1c19536](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/1c1953653c20b4a8c61a7deb3fc493d496d8c478))
      - remove "arm,psci" from psci node ([814646b](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/814646b4cb792ab14df04e28360fefd168399b3c))
      - remove ffa and optee device tree node ([f1b44a9](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/f1b44a9050fbc12e8c260107bfff2930476df062))
      - set cactus-tertiary vcpu count to 1 ([05f667f](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/05f667f0c670ba9682050714561309f00210c282))

    - **SGI**

      - avoid redefinition of 'efi_guid' structure ([f34322c](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/f34322c1cea1e355aeb4133df6aa601d719be5a3))

  - **Marvell**

    - Check the required libraries before building doimage ([dd47809](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/dd47809e9ea75188060bf8b294efa8578d255c63))

    - **Armada**

      - select correct pcie reference clock source ([371648e](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/371648e1c76b5230bf8e153629064c02086365c9))
      - fix MSS loader for A8K family ([dceac43](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/dceac436f620e60cd0149194377871b225216079))

      - **A3K**

        - disable HANDLE_EA_EL3_FIRST by default ([3017e93](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/3017e932768c7357a1a41493c58323419e9a1ec9))
        - enable workaround for erratum 1530924 ([975563d](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/975563dbfc012b6e8a7765dd8e48220e1bc53dec))
        - Fix building uart-images.tgz.bin archive ([d3f8db0](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/d3f8db07b618e79c05805a1598e5e834e42fea98))
        - Fix check for external dependences ([2baf503](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/2baf50385ba2b460afef4a7919b13b3a350fd03a))
        - fix printing info messages on output ([9f6d154](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/9f6d15408340af07ed3c2500202b147189eaa7ef))
        - update information about PCIe abort hack ([068fe91](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/068fe919613197bf221c00fb84a1d94c66a7a8ca))
        - Remove encryption password ([076374c](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/076374c9b97d47b10ba5c6034817866c08d66ed4))

      - **A8K**

        - Add missing build dependency for BLE target ([04738e6](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/04738e69917f8e8790bf4cf83ceb05f85e1f45bb))
        - Correctly set include directories for individual targets ([559ab2d](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/559ab2df4a35cd82b2a67a0bebeb3028544a6766))
        - Require that MV_DDR_PATH is correctly set ([528dafc](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/528dafc367c4f49d4904c4335422502dacf469bf))
        - fix number of CPU power switches. ([5cf6faf](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/5cf6fafe223da89c60e2323c242ea188b17e98c3))

  - **MediaTek**

    - **MT8183**

      - fix out-of-bound access ([420c26b](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/420c26b33a29c8328a1806ccb2f5a5885041fdfc))

    - **MT8195**

      - use correct print format for uint64_t ([964ee4e](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/964ee4e6be70ef638d6c875a761ab5ca359d84fe))
      - fix error setting for SPM ([1f81ccc](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/1f81cccedd40cb397813b0fa826ea1d793b02089))
      - extend MMU region size ([9ff8b8c](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/9ff8b8ca9393e31e790eb2c8e7ea5c5f41f45198))
      - fix coverity fail ([85e4d14](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/85e4d14df157b5641421ea2b844c146ddc230152))

  - **NXP**

    - **i.MX**

      - do not keep mmc_device_info in stack ([99d37c8](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/99d37c8cb8196a7296311fb4f97f80f086021c74))

      - **i.MX 8M**

        - **i.MX 8M Mini**

          - fix FTBFS on SPD=opteed ([10bfc77](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/10bfc77e7b3afce17185114ac66361a0914f7784))

    - **Layerscape**

      - **LX2**

        - **LS1028A**

          - define endianness of scfg and gpio ([2475f63](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/2475f63bdec6c24c13f7d6ec7f70275b1bde5c15))
          - fix compile error when enable fuse provision ([a0da9c4](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/a0da9c4bd296ec1a47683a1ee05f5d1ed71828c7))

  - **QEMU**

    - (NS_DRAM0_BASE + NS_DRAM0_SIZE) ADDR overflow 32bit ([325716c](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/325716c97b7835b8d249f12c1461556bab8c53a0))
    - reboot/shutdown with low to high gpio ([bd2ad12](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/bd2ad12ef10f558a5b15f5768b66e7b2606c6498))

  - **QTI**

    - **SC1780**

      - qti smc addition ([cc35a37](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/cc35a3771d28a96906f8d0f393ff664924a2d4dc))

  - **Raspberry Pi**

    - **Raspberry Pi 4**

      - drop /memreserve/ region ([5d2793a](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/5d2793a61aded9602af86e90a571f64ff07f93b3))

  - **Renesas**

    - **R-Car**

      - change process that copy code to system ram ([49593cc](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/49593cc1ce0d0471aeef7ca24a5415da2dd55bea))
      - fix cache maintenance process of reading cert header ([c77ab18](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/c77ab18ec7c8e0f3d953177b835e004a9b53515f))
      - fix to load image when option BL2_DCACHE_ENABLE is enabled ([d2ece8d](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/d2ece8dba2f31091b1fa6c302d4255495bb15705))

      - **R-Car 3**

        - fix disabling MFIS write protection for R-Car D3 ([a8c0c3e](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/a8c0c3e9d0df2215ed3b9ef66f4596787d957566))
        - fix eMMC boot support for R-Car D3 ([77ab366](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/77ab3661e55c39694c7ee81de2d1615775711b64))
        - fix source file to make about GICv2 ([fb3406b](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/fb3406b6b573cb0b35138ca3c89c5641d3d7b790))
        - fix version judgment for R-Car D3 ([c3d192b](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/c3d192b8e52823dcbc32e21e47c30693d38bb49f))
        - generate two memory nodes for larger than 2 GiB channel 0 ([21924f2](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/21924f2466b9b5e1243c142932e6f498da5633e9))

  - **Rockchip**

    - **RK3399**

      - correct LPDDR4 resume sequence ([2c4b0c0](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/2c4b0c05c6546e24eb7209ffb3bb465d4feed164))
      - fix dram section placement ([f943b7c](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/f943b7c8e292e3aad2fcbdd0a37505f62b3b4c87))

  - **Socionext**

    - **Synquacer**

      - update scmi power domain off handling ([f7f5d2c](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/f7f5d2c4cd209c2d21244da4fa442050eb4531ab))

  - **ST**

    - add STM32IMAGE_SRC ([f223505](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/f22350583c2e26ea291eae3dc54db867fdf0d9af))
    - add UART reset in crash console init ([b38e2ed](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/b38e2ed29ef791dad0cb61fed81b74d612f58b01))
    - apply security at the end of BL2 ([99080bd](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/99080bd1273331007f0b2d6f64fed51ac6861bcd))
    - correct BSEC error code management ([72c7884](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/72c7884092684af4cc3c49e08f913b3ffed783ba))
    - correct IO compensation disabling ([c2d18ca](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/c2d18ca80f4bd32f58ba07f53d9bb2586df18fc0))
    - correct signedness comparison issue ([5657dec](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/5657decc7ffa1376c0a97b6d14ea1428877f5af4))
    - improve DDR get size function ([91ffc1d](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/91ffc1deffa2c1c64efe4dfaf27b78f2621a8b0b))
    - only check header major when booting ([8ce8918](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/8ce89187459ec77dd9ffdffba3a2b77838d51b6d))
    - panic if boot interface is wrong ([71693a6](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/71693a66341e7d9d683ef32981243cb4c4439351))
    - remove double space ([306dcd6](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/306dcd6b0d1981b75e103c560a4034bdaa6862d5))

    - **ST32MP1**

      - add bl prefix for internal linker script ([7684ddd](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/7684dddcfb14c45bad33b091410a0bf14a3a9830))

  - **Xilinx**

    - **Versal**

      - correct IPI buffer offset ([e1e5b13](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/e1e5b1339b9f73f7f1893d8a6d4dfe4b19ba0ad1))
      - use sync method for blocking calls ([fa58171](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/fa58171534976f94b93a44184afd050d8225e404))

    - **ZynqMP**

      - use sync method for blocking calls ([c063c5a](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/c063c5a4f92d5787536e595ca4906b458b0f26cb))

- **Services**

  - drop warning on unimplemented calls ([67fad51](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/67fad514ee974dcf0252fa0e9219eb3c580eb714))

  - **RME**

    - fixes a shift by 64 bits bug in the RME GPT library ([322b344](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/322b344e30cb87b9293060d5946b3c17fe3b9133))

  - **SPM**

    - do not compile if SVE/SME is enabled ([4333f95](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/4333f95bedb5f2b53dcb62e0e9c563794ec33c07))
    - error macro to use correct print format ([0c23e6f](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/0c23e6f44d41593b6e7f97594c12b5791bd75189))
    - revert workaround hafnium as hypervisor ([3221fce](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/3221fce842c0b5aea984bb8dbc1393082bd88a58))
    - fixing coverity issue for SPM Core. ([f7fb0bf](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/f7fb0bf77f3434bfb67411cad65e704fdef27f76))

- **Libraries**

  - **LIBC**

    - use long for 64-bit types on aarch64 ([4ce3e99](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/4ce3e99a336b74611349595ea7fd5ed0277c3eeb))

  - **CPU Support**

    - correct Demeter CPU name ([4cb576a](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/4cb576a0c5bd2e7669606996a9f79602596df07c))
    - workaround for Cortex A78 erratum 2242635 ([1ea9190](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/1ea9190c6a4d2299c6dc19adc0bbe93d4f051eff))
    - workaround for Cortex-A710 erratum 2058056 ([744bdbf](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/744bdbf732ffd2abf84b2431624051e93bc29f7b))
    - workaround for Neoverse V1 erratum 2216392 ([4c8fe6b](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/4c8fe6b17fa994a630b2a30f8666df103f2e370d))
    - workaround for Neoverse-N2 erratum 2138953 ([ef8f0c5](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/ef8f0c52ddf83e815a029319971682d7a26b6a6f))
    - workaround for Neoverse-N2 erratum 2138958 ([c948185](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/c948185c973c13df36c62c4bcb50e22b14d6e06a))
    - workaround for Neoverse-N2 erratum 2242400 ([603806d](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/603806d1376c4b18211fb1d4cc338153de026c32))
    - workaround for Neoverse-N2 erratum 2242415 ([5819e23](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/5819e23bc47c860872141caf42bddddb1b8679a5))
    - workaround for Neoverse-N2 erratum 2280757 ([0d2d999](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/0d2d99924e1be548e75c46cfd536f7503cf863e0))
    - rename Matterhorn, Matterhorn ELP, and Klein CPUs ([c6ac4df](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/c6ac4df622befb5bb42ac136745094e1498c91d8))

  - **EL3 Runtime**

    - correct CASSERT for pauth ([b4f8d44](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/b4f8d44597faf641177134ee08db7c3fcef5aa14))
    - fix SVE and AMU extension enablement flags ([68ac5ed](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/68ac5ed0493b24e6a0a178171a47db75a31cc423))
    - random typos in tf-a code base ([2e61d68](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/2e61d6871cc310e9404fe5cfa10b9828f1c869a7))
    - Remove save/restore of EL2 timer registers ([a7cf274](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/a7cf2743f3eb487912302aafc748c81bbd1fc603))

  - **OP-TEE**

    - correct signedness comparison ([21d2be8](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/21d2be83a2eabb328071e857e538ced3c8351874))

  - **GPT**

    - add necessary barriers and remove cache clean ([77612b9](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/77612b90acaffc82cea712f4a431c727bbb968ec))
    - use correct print format for uint64_t ([2461bd3](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/2461bd3a89f7f2cdf4a7302536746733970cfe53))

  - **Translation Tables**

    - remove always true check in assert ([74d720a](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/74d720a026735263d2f290fd05370dad0d4c7219))

- **Drivers**

  - **Authentication**

    - avoid NV counter upgrade without certificate validation ([a2a5a94](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/a2a5a9456969266dc68d5845f31e05be0c3ff2e3))

    - **CryptoCell-713**

      - fix a build failure with CC-713 library ([e5fbee5](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/e5fbee5085c682ac3438e6f66c8bdaffb6076fa2))

  - **MTD**

    - fix MISRA issues and logic improvement ([5130ad1](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/5130ad14d52a0196422fed8a7d08e25659890b15))
    - macronix quad enable bit issue ([c332740](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/c3327408eb4b5852c0ed9d8933c35aaa6de34c21))

    - **NAND**

      - **SPI NAND**

        - check correct manufacturer id ([4490b79](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/4490b7963303fbe59b07a66c8498a803eb5c239c))
        - check that parameters have been set ([bc453ab](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/bc453ab1b2fd4267d34f2b9587f73b8940ee1538))

  - **SCMI**

    - entry: add weak functions ([b3c8fd5](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/b3c8fd5d778144340d289ad4825123106aac4a96))
    - smt: fix build for aarch64 ([0e223c6](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/0e223c6a9e5a2d92cae00fdd16a02a3f8971b114))
    - mention "SCMI" in driver initialisation message ([e0baae7](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/e0baae7316bfdf3e49e5e158f79eb80cd51fc700))
    - relax requirement for exact protocol version ([125868c](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/125868c94150f52ff85cdb59aee623ab1f9f259d))

  - **UFS**

    - add reset before DME_LINKSTARTUP ([905635d](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/905635d5e74e3c7b7b2412a673009c8aaabb73e1))

  - **Arm**

    - **GIC**

      - **GICv3**

        - add dsb in both disable and enable function of gicv3_cpuif ([5a5e0aa](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/5a5e0aac398989536dc4be790820af89da3d093a))

      - **GIC-600AE**

	- fix timeout calculation ([7f322f2](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/7f322f228e76caa5480f827af0aa6751f00fc1c4))

    - **TZC**

      - **TZC-400**

        - never disable filter 0 ([ef378d3](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/ef378d3ec1ef9d7c28baef32ed409688e962542b))

  - **Marvell**

    - **COMPHY**

      - fix name of 3.125G SerDes mode ([a669983](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/a669983c78828e3f4a4f14b9e5a6ee79dcfde20f))

      - **Armada 3700**

        - configure phy selector also for PCIe ([0f3a122](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/0f3a1221093256999af5f2a80e9b3d7231b9f5fb))
        - fix address overflow ([c074f70](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/c074f70ce5d85e1735b589b323fac99d7eb988b5))
        - handle failures in power functions ([49b664e](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/49b664e75f43fda08dddef4f0510d346bdd25565))

      - **CP110**

        - fix error code in pcie power on ([c0a909c](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/c0a909cdcce2d9a2ceefe672ad2fc1cae7e39ec4))

    - **Armada**

      - **A3K**

        - **A3720**

          - fix configuring UART clock ([b9185c7](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/b9185c75f7ec2b600ebe0d49281e216a2456b764))
          - fix UART clock rate value and divisor calculation ([66a7752](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/66a7752834382595d26214783ae4698fd1f00bd6))
          - fix UART parent clock rate determination ([5a91c43](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/5a91c439cbeb1f64b8b9830de91efad5113d3c89))

  - **MediaTek**

    - **PMIC Wrapper**

      - update idle flow ([9ed4e6f](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/9ed4e6fb669b8fcafc4e8acfa6a36db305d27ac8))

    - **MT8192**

      - **SPM**

        - add missing bit define for debug purpose ([310c3a2](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/310c3a26e17d99aafc73b3504d0b6dfbdb97fd4c))

  - **NXP**

    - **FLEXSPI**

      - fix warm boot wait time for MT35XU512A ([1ff7e46](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/1ff7e46b092b74891bc2dc7263e4dfae947b2223))

    - **SCFG**

      - fix endianness checking ([fb90cfd](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/fb90cfd4eee504f1d16aa143728af427dc6e0ed8))

    - **SFP**

      - fix compile warning ([3239a17](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/3239a17561c124df7095391c0d64e86910660cdc))

  - **Renesas**

    - **R-Car3**

      - console: fix a return value of console_rcar_init ([bb273e3](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/bb273e3be1c4f1cddeac9ceaac95fb56e41e6b98))
      - ddr: update DDR setting for H3, M3, M3N ([ec767c1](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/ec767c1b99675fbb50ef1b2fdb2d38e881e4789d))
      - emmc: remove CPG_CPGWPR redefinition ([36d5645](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/36d5645aec947ab00b925b21141e59e58e1efd8c))
      - fix CPG registers redefinition ([0dae56b](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/0dae56bb2f0aa1f89ec98ebe3931fb19751a5c72))
      - i2c_dvfs: fix I2C operation ([b757d3a](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/b757d3a1d901bee9b7ad430702575adba04889ba))

  - **ST**

    - **Clock**

      - use correct return value ([8f97c4f](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/8f97c4fab1769b3f7f37a2a7a01ade36e5c94eaa))
      - correctly manage RTC clock source ([1550909](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/15509093f0ba9a10f97c6f92bc3bb9fcf79a48ce))
      - fix MCU/AXI parent clock ([b8fe48b](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/b8fe48b6f2b07fce49363cb3c0f8dac9e286439b))
      - fix MPU clock rate ([602ae2f](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/602ae2f23c2bc9d79a9ab2b7c5dde1932fffc984))
      - fix RTC clock rating ([cbd2e8a](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/cbd2e8a6afdd05c4b404d7998134a3f60cc15518))
      - keep RTC clock always on ([5b111c7](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/5b111c74795ea5e9c8a12d0e6b18d77e431311ed))
      - keep RTCAPB clock always on ([373f06b](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/373f06be4ee1114369b96763481b58885623aea4))
      - set other clocks as always on ([bf39318](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/bf39318d93c270ff72bda4b46e4771aba7aea313))

    - **I/O**

      - **STM32 Image**

        - invalidate cache on local buf ([a5bcf82](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/a5bcf82402ff415326b4dba42aae95c499821e94))
        - uninitialized variable warning ([c1d732d](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/c1d732d0db2463998036c678619007da79a25b3f))

    - **ST PMIC**

      - initialize i2c_state ([4282284](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/42822844bfed2e9ffaeae850cc60f5c3d4d9d654))
      - missing error check ([a4bcfe9](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/a4bcfe94e73db89ce2ebbb23c8e33e51eea5026a))

    - **STPMIC1**

      - fix power switches activation ([0161991](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/0161991184e5feacacc679bdb9c92681b85235eb))
      - update error cases return ([ed6a852](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/ed6a85234653c5ee2520389b769ff47e321df8a4))

    - **UART**

      - **STM32 Console**

        - do not skip init for crash console ([49c7f0c](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/49c7f0cef4cc864185828750f1f61f3f33f284f7))

  - **USB**

    - add a optional ops get_other_speed_config_desc ([216c122](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/216c1223c2c65bd1c119a28b9406f70a9ee7b063))
    - fix Null pointer dereferences in usb_core_set_config ([0cb9870](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/0cb9870ddfa1b2fec50debe6d6333cbcb3df1e7e))
    - remove deadcode when USBD_EP_NB = 1 ([7ca4928](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/7ca49284be083b03ae11aa348b40358876ee5d4b))
    - remove unnecessary cast ([025f5ef](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/025f5ef201a39ba7285f368139e690bbd7a44653))

- **Miscellaneous**

  - use correct printf format for uint64_t ([4ef449c](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/4ef449c15a4055d92632cb7e72267f525a7e2fca))

  - **DT Bindings**

    - fix static checks ([0861fcd](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/0861fcdd3e3f2625e133de3dae9c548de7c1ee48))

  - **FDTs**

    - avoid output on missing DT property ([49e789e](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/49e789e353efaf97f84eca016c6a1b8a2b3e3d98))
    - fix OOB write in uuid parsing function ([d0d6424](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/d0d642450f1f3a0f43e0e156ef57a0c460dd48cf))

    - **Morello**

      - fix scmi clock specifier to cluster mappings ([387a906](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/387a9065a271ecde0e47dc5a9f9d037637502beb))

    - **STM32MP1**

      - correct copyright dates ([8d26029](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/8d26029168fe70a86de524ed68c56e8666823714))
      - set ETH clock on PLL4P on ST boards ([3e881a8](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/3e881a8834a955f1e552300bdbf1dafd02ea8f1c))
      - update PLL nodes for ED1/EV1 boards ([cdbbb9f](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/cdbbb9f7ecd4687fa52e1c655b631377c24862b9))
      - use 'kHz' as kilohertz abbreviation ([4955d08](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/4955d08de7aa664387d2e5f690e78b85ac23a402))

  - **PIE**

    - invalidate data cache in the entire image range if PIE is enabled ([596d20d](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/596d20d9e4d50c02b5a0cce8cad2a1c205cd687a))

  - **Security**

    - Set MDCR_EL3.MCCD bit ([12f6c06](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/12f6c0649732a35a7ed45ba350a963f09a5710ca))

  - **SDEI**

    - fix assert while kdump issue ([d39db26](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/d39db2695ba626b9c0ee38652fe160b4e84b15d9))
    - print event number in hex format ([6b94356](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/6b94356b577744d425476a029c47bd35eb13c148))
    - set SPSR for SDEI based on TakeException ([37596fc](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/37596fcb43e34ed4bcf1bd3e86d8dec1011edab8))

- **Documentation**

  - fix TF-A v2.6 release date in the release information page ([c90fa47](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/c90fa47202b762fe8f54e9c0561e94d37907b6ad))
  - fix `FF-A` substitution ([a61940c](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/a61940ca739eb89be7c1bb2408a9178c2da5cb70))
  - fix typos in v2.5 release documentation ([481c7b6](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/481c7b6b9107a3f71ee750f89cacdd8f9c729838))
  - remove "experimental" tag for stable features ([700e768](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/700e7685dd4682a929645a79de39f503c9140b2d))

  - **Contribution Guidelines**

    - fix formatting for code snippet ([d0bbe81](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/d0bbe8150eb35fe2bac1567751bf84a8f073dd39))

- **Build System**

  - use space in WARNINGS list ([34b508b](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/34b508be9f021831423a8a14f56dff547e24c743))

  - **Git Hooks**

    - downgrade `package-lock.json` version ([7434b65](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/7434b65208175bdf3f44e0e62aaaeabc9c494ee3))

- **Tools**

  - **STM32 Image**

    - improve the tool ([8d0036d](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/8d0036d3d8c8ac1524539ea90382acafb1e524c0))

  - **SPTOOL**

    - SP UUID little to big endian in TF-A build ([dcdbcdd](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/dcdbcddebdee8d4d2c6c8316f615b428758b22ac))

  - **DOIMAGE**

    - Fix doimage syntax breaking secure mode build ([6d55ef1](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/6d55ef1a24dc92a3b737aaa02141f550caaace06))

- **Dependencies**

  - **checkpatch**

    - do not check merge commits ([77a0a7f](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/77a0a7f1d96b188849d1d8d8884b3c93857d3f69))

## [2.5.0](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/refs/tags/v2.4..refs/tags/v2.5) (2021-05-17)

### New Features

- Architecture support

  - Added support for speculation barrier(`FEAT_SB`) for non-Armv8.5 platforms
    starting from Armv8.0
  - Added support for Activity Monitors Extension version 1.1(`FEAT_AMUv1p1`)
  - Added helper functions for Random number generator(`FEAT_RNG`) registers
  - Added support for Armv8.6 Multi-threaded PMU extensions (`FEAT_MTPMU`)
  - Added support for MTE Asymmetric Fault Handling extensions(`FEAT_MTE3`)
  - Added support for Privileged Access Never extensions(`FEAT_PANx`)

- Bootloader images

  - Added PIE support for AArch32 builds
  - Enable Trusted Random Number Generator service for BL32(sp_min)

- Build System

  - Added build option for Arm Feature Modifiers

- Drivers

  - Added support for interrupts in TZC-400 driver
  - Broadcom
    - Added support for I2C, MDIO and USB drivers
  - Marvell
    - Added support for secure read/write of dfc register-set
    - Added support for thermal sensor driver
    - Implement a3700_core_getc API in console driver
    - Added rx training on 10G port
  - Marvell Mochi
    - Added support for cn913x in PCIe mode
  - Marvell Armada A8K
    - Added support for TRNG-IP-76 driver and accessing RNG register
  - Mediatek MT8192
    - Added support for following drivers
      - MPU configuration for SCP/PCIe
      - SPM suspend
      - Vcore DVFS
      - LPM
      - PTP3
      - UART save and restore
      - Power-off
      - PMIC
      - CPU hotplug and MCDI support
      - SPMC
      - MPU
  - Mediatek MT8195
    - Added support for following drivers
      - GPIO, NCDI, SPMC drivers
      - Power-off
      - CPU hotplug, reboot and MCDI
      - Delay timer and sys timer
      - GIC
  - NXP
    - Added support for
      - non-volatile storage API
      - chain of trust and trusted board boot using two modes: MBEDTLS and CSF
      - fip-handler necessary for DDR initialization
      - SMMU and console drivers
      - crypto hardware accelerator driver
      - following drivers: SD, EMMC, QSPI, FLEXSPI, GPIO, GIC, CSU, PMU, DDR
      - NXP Security Monitor and SFP driver
      - interconnect config APIs using ARM CCN-CCI driver
      - TZC APIs to configure DDR region
      - generic timer driver
      - Device configuration driver
  - IMX
    - Added support for image loading and io-storage driver for TBBR fip booting
  - Renesas
    - Added support for PFC and EMMC driver
    - RZ Family:
      - G2N, G2E and G2H SoCs
        - Added support for watchdog, QoS, PFC and DRAM initialization
    - RZG Family:
      - G2M
        - Added support for QoS and DRAM initialization
  - Xilinx
    - Added JTAG DCC support for Versal and ZynqMP SoC family.

- Libraries

  - C standard library
    - Added support to print `%` in `snprintf()` and `printf()` APIs
    - Added support for strtoull, strtoll, strtoul, strtol APIs from FreeBSD
      project
  - CPU support
    - Added support for
      - Cortex_A78C CPU
      - Makalu ELP CPU
      - Makalu CPU
      - Matterhorn ELP CPU
      - Neoverse-N2 CPU
  - CPU Errata
    - Arm Cortex-A76: Added workaround for erratum 1946160
    - Arm Cortex-A77: Added workaround for erratum 1946167
    - Arm Cortex-A78: Added workaround for erratum 1941498 and 1951500
    - Arm Neoverse-N1: Added workaround for erratum 1946160
  - Flattened device tree(libfdt)
    - Added support for wrapper function to read UUIDs in string format from dtb

- Platforms

  - Added support for MediaTek MT8195
  - Added support for Arm RD-N2 board
  - Allwinner
    - Added support for H616 SoC
  - Arm
    - Added support for GPT parser
    - Protect GICR frames for fused/unused cores
  - Arm Morello
    - Added VirtIO network device to Morello FVP fdts
  - Arm RD-N2
    - Added support for variant 1 of RD-N2 platform
    - Enable AMU support
  - Arm RD-V1
    - Enable AMU support
  - Arm SGI
    - Added support for platform variant build option
  - Arm TC0
    - Added Matterhorn ELP CPU support
    - Added support for opteed
  - Arm Juno
    - Added support to use hw_config in BL31
    - Use TRNG entropy source for SMCCC TRNG interface
    - Condition Juno entropy source with CRC instructions
  - Marvell Mochi
    - Added support for detection of secure mode
  - Marvell ARMADA
    - Added support for new compile option A3720_DB_PM_WAKEUP_SRC
    - Added support doing system reset via CM3 secure coprocessor
    - Made several makefile enhancements required to build WTMI_MULTI_IMG and
      TIMDDRTOOL
    - Added support for building DOIMAGETOOL tool
    - Added new target mrvl_bootimage
  - Mediatek MT8192
    - Added support for rtc power off sequence
  - Mediatek MT8195
    - Added support for SiP service
  - STM32MP1
    - Added support for
      - Seeed ODYSSEY SoM and board
      - SDMMC2 and I2C2 pins in pinctrl
      - I2C2 peripheral in DTS
      - PIE for BL32
      - TZC-400 interrupt managament
      - Linux Automation MC-1 board
  - Renesas RZG
    - Added support for identifying EK874 RZ/G2E board
    - Added support for identifying HopeRun HiHope RZ/G2H and RZ/G2H boards
  - Rockchip
    - Added support for stack protector
  - QEMU
    - Added support for `max` CPU
    - Added Cortex-A72 support to `virt` platform
    - Enabled trigger reboot from secure pl061
  - QEMU SBSA
    - Added support for sbsa-ref Embedded Controller
  - NXP
    - Added support for warm reset to retain ddr content
    - Added support for image loader necessary for loading fip image
    - lx2160a SoC Family
      - Added support for
        - new platform lx2160a-aqds
        - new platform lx2160a-rdb
        - new platform lx2162a-aqds
        - errata handling
  - IMX imx8mm
    - Added support for trusted board boot
  - TI K3
    - Added support for lite device board
    - Enabled Cortex-A72 erratum 1319367
    - Enabled Cortex-A53 erratum 1530924
  - Xilinx ZynqMP
    - Added support for PS and system reset on WDT restart
    - Added support for error management
    - Enable support for log messages necessary for debug
    - Added support for PM API SMC call for efuse and register access

- Processes

  - Introduced process for platform deprecation
  - Added documentation for TF-A threat model
  - Provided a copy of the MIT license to comply with the license requirements
    of the arm-gic.h source file (originating from the Linux kernel project and
    re-distributed in TF-A).

- Services

  - Added support for TRNG firmware interface service
  - Arm
    - Added SiP service to configure Ethos-N NPU
  - SPMC
    - Added documentation for SPM(Hafnium) SMMUv3 driver
  - SPMD
    - Added support for
      - FFA_INTERRUPT forwading ABI
      - FFA_SECONDARY_EP_REGISTER ABI
      - FF-A v1.0 boot time power management, SPMC secondary core boot and early
        run-time power management

- Tools

  - FIPTool
    - Added mechanism to allow platform specific image UUID
  - git hooks
    - Added support for conventional commits through commitlint hook, commitizen
      hook and husky configuration files.
  - NXP tool
    - Added support for a tool that creates pbl file from BL2
  - Renesas RZ/G2
    - Added tool support for creating bootparam and cert_header images
  - CertCreate
    - Added support for platform-defined certificates, keys, and extensions
      using the platform's makefile
  - shared tools
    - Added EFI_GUID representation to uuid helper data structure

### Changed

- Common components

  - Print newline after hex address in aarch64 el3_panic function
  - Use proper `#address-cells` and `#size-cells` for reserved-memory in dtbs

- Drivers

  - Move SCMI driver from ST platform directory and make it common to all
    platforms
  - Arm GICv3
    - Shift eSPI register offset in GICD_OFFSET_64()
    - Use mpidr to probe GICR for current CPU
  - Arm TZC-400
    - Adjust filter tag if it set to FILTER_BIT_ALL
  - Cadence
    - Enhance UART driver APIs to put characters to fifo
  - Mediatek MT8192
    - Move timer driver to common folder
    - Enhanced sys_cirq driver to add more IC services
  - Renesas
    - Move ddr and delay driver to common directory
  - Renesas rcar
    - Treat log as device memory in console driver
  - Renesas RZ Family:
    - G2N and G2H SoCs
      - Select MMC_CH1 for eMMC channel
  - Marvell
    - Added support for checking if TRNG unit is present
  - Marvell A3K
    - Set TXDCLK_2X_SEL bit during PCIe initialization
    - Set mask parameter for every reg_set call
  - Marvell Mochi
    - Added missing stream IDs configurations
  - MbedTLS
    - Migrated to Mbed TLS v2.26.0
  - IMX imx8mp
    - Change the bl31 physical load address
  - QEMU SBSA
    - Enable secure variable storage
  - SCMI
    - Update power domain protocol version to 2.0
  - STM32
    - Remove dead code from nand FMC driver

- Libraries

  - C Standard Library
    - Use macros to reduce duplicated code between snprintf and printf
  - CPU support
    - Sanity check pointers before use in AArch32 builds
    - Arm Cortex-A78
      - Remove rainier cpu workaround for errata 1542319
    - Arm Makalu ELP
      - Added "\_arm" suffix to Makalu ELP CPU lib

- Miscellaneous

  - Editorconfig
    - set max line length to 100

- Platforms

  - Allwinner
    - Added reserved-memory node to DT
    - Express memmap more dynamically
    - Move SEPARATE_NOBITS_REGION to platforms
    - Limit FDT checks to reduce code size
    - Use CPUIDLE hardware when available
    - Allow conditional compilation of SCPI and native PSCI ops
    - Always use a 3MHz RSB bus clock
    - Enable workaround for Cortex-A53 erratum 1530924
    - Fixed non-default PRELOADED_BL33_BASE
    - Leave CPU power alone during BL31 setup
    - Added several psci hooks enhancements to improve system shutdown/reset
      sequence
    - Return the PMIC to I2C mode after use
    - Separate code to power off self and other CPUs
    - Split native and SCPI-based PSCI implementations
  - Allwinner H6
    - Added R_PRCM security setup for H6 board
    - Added SPC security setup for H6 board
    - Use RSB for the PMIC connection on H6
  - Arm
    - Store UUID as a string, rather than ints
    - Replace FIP base and size macro with a generic name
    - Move compile time switch from source to dt file
    - Don't provide NT_FW_CONFIG when booting hafnium
    - Do not setup 'disabled' regulator
    - Increase SP max size
    - Remove false dependency of ARM_LINUX_KERNEL_AS_BL33 on RESET_TO_BL31 and
      allow it to be enabled independently
  - Arm FVP
    - Do not map GIC region in BL1 and BL2
  - Arm Juno
    - Refactor juno_getentropy() to return 64 bits on each call
  - Arm Morello
    - Remove "virtio-rng" from Morello FVP
    - Enable virtIO P9 device for Morello fvp
  - Arm RDV1
    - Allow all PSCI callbacks on RD-V1
    - Rename rddaniel to rdv1
  - Arm RDV1MC
    - Rename rddanielxlr to rdv1mc
    - Initialize TZC-400 controllers
  - Arm TC0
    - Updated GICR base address
    - Use scmi_dvfs clock index 1 for cores 4-7 through fdt
    - Added reserved-memory node for OP-TEE fdts
    - Enabled Theodul DSU in TC platform
    - OP-TEE as S-EL1 SP with SPMC at S-EL2
    - Update Matterhorm ELP DVFS clock index
  - Arm SGI
    - Allow access to TZC controller on all chips
    - Define memory regions for multi-chip platforms
    - Allow access to nor2 flash and system registers from S-EL0
    - Define default list of memory regions for DMC-620 TZC
    - Improve macros defining cper buffer memory region
    - Refactor DMC-620 error handling SMC function id
    - Refactor SDEI specific macros
    - Added platform id value for RDN2 platform
    - Refactored header file inclusions and inclusion of memory mapping
  - Arm RDN2
    - Allow usage of secure partitions on RDN2 platform
    - Update GIC redistributor and TZC base address
  - Arm SGM775
    - Deprecate Arm sgm775 FVP platform
  - Marvell
    - Increase TX FIFO EMPTY timeout from 2ms to 3ms
    - Update delay code to be compatible with 1200 MHz CPU
  - Marvell ARMADA
    - Postpone MSS CPU startup to BL31 stage
    - Allow builds without MSS support
    - Use MSS SRAM in secure mode
    - Added missing FORCE, .PHONY and clean targets
    - Cleanup MSS SRAM if used for copy
    - Move definition of mrvl_flash target to common marvell_common.mk file
    - Show informative build messages and blank lines
  - Marvell ARMADA A3K
    - Added a new target mrvl_uart which builds UART image
    - Added checks that WTP, MV_DDR_PATH and CRYPTOPP_PATH are correctly defined
    - Allow use of the system Crypto++ library
    - Build \$(WTMI_ENC_IMG) in \$(BUILD_PLAT) directory
    - Build intermediate files in \$(BUILD_PLAT) directory
    - Build UART image files directly in \$(BUILD_UART) subdirectory
    - Correctly set DDR_TOPOLOGY and CLOCKSPRESET for WTMI
    - Do not use 'echo -e' in Makefile
    - Improve 4GB DRAM usage from 3.375 GB to 3.75 GB
    - Remove unused variable WTMI_SYSINIT_IMG from Makefile
    - Simplify check if WTP variable is defined
    - Split building \$(WTMI_MULTI_IMG) and \$(TIMDDRTOOL)
  - Marvell ARMADA A8K
    - Allow CP1/CP2 mapping at BLE stage
  - Mediatek MT8183
    - Added timer V20 compensation
  - Nvidia Tegra
    - Rename SMC API
  - TI K3
    - Make plat_get_syscnt_freq2 helper check CNT_FID0 register
    - Fill non-message data fields in sec_proxy with 0x0
    - Update ti_sci_msg_req_reboot ABI to include domain
    - Enable USE_COHERENT_MEM only for the generic board
    - Explicitly map SEC_SRAM_BASE to 0x0
    - Use BL31_SIZE instead of computing
    - Define the correct number of max table entries and increase SRAM size to
      account for additional table
  - Raspberry Pi4
    - Switch to gicv2.mk and GICV2_SOURCES
  - Renesas
    - Move headers and assembly files to common folder
  - Renesas rzg
    - Added device tree memory node enhancements
  - Rockchip
    - Switch to using common gicv3.mk
  - STM32MP1
    - Set BL sizes regardless of flags
  - QEMU
    - Include gicv2.mk for compiling GICv2 source files
    - Change DEVICE2 definition for MMU
    - Added helper to calculate the position shift from MPIDR
  - QEMU SBSA
    - Include libraries for Cortex-A72
    - Increase SHARED_RAM_SIZE
    - Addes support in spm_mm for upto 512 cores
    - Added support for topology handling
  - QTI
    - Mandate SMC implementation
  - Xilinx
    - Rename the IPI CRC checksum macro
    - Use fno-jump-tables flag in CPPFLAGS
  - Xilinx versal
    - Added the IPI CRC checksum macro support
    - Mark IPI calls secure/non-secure
    - Enable sgi to communicate with linux using IPI
    - Remove Cortex-A53 compilation
  - Xilinx ZynqMP
    - Configure counter frequency during initialization
    - Filter errors related to clock gate permissions
    - Implement pinctrl request/release EEMI API
    - Reimplement pinctrl get/set config parameter EEMI API calls
    - Reimplement pinctrl set/get function EEMI API
    - Update error codes to match Linux and PMU Firmware
    - Update PM version and support PM version check
    - Update return type in query functions
    - Added missing ids for 43/46/47dr devices
    - Checked for DLL status before doing reset
    - Disable ITAPDLYENA bit for zero ITAP delay
    - Include GICv2 makefile
    - Remove the custom crash implementation

- Services

  - SPMD
    - Lock the g_spmd_pm structure
    - Declare third cactus instance as UP SP
    - Provide number of vCPUs and VM size for first SP
    - Remove `chosen` node from SPMC manifests
    - Move OP-TEE SP manifest DTS to FVP platform
    - Update OP-TEE SP manifest with device-regions node
    - Remove device-memory node from SPMC manifests
  - SPM_MM
    - Use sp_boot_info to set SP context
  - SDEI
    - Updata the affinity of shared event

- Tools

  - FIPtool
    - Do not print duplicate verbose lines about building fiptool
  - CertCreate
    - Updated tool for platform defined certs, keys & extensions
    - Create only requested certificates
    - Avoid duplicates in extension stack

### Resolved Issues

- Several fixes for typos and mis-spellings in documentation

- Build system

  - Fixed \$\{FIP_NAME} to be rebuilt only when needed in Makefile
  - Do not mark file targets as .PHONY target in Makefile

- Drivers

  - Authorization
    - Avoid NV counter upgrade without certificate validation
  - Arm GICv3
    - Fixed logical issue for num_eints
    - Limit SPI ID to avoid misjudgement in GICD_OFFSET()
    - Fixed potential GICD context override with ESPI enabled
  - Marvell A3700
    - Fixed configuring polarity invert bits
  - Arm TZC-400
    - Correct FAIL_CONTROL Privileged bit
    - Fixed logical error in FILTER_BIT definitions
  - Renesas rcar
    - Fixed several coding style violations reported by checkpatch

- Libraries

  - Arch helpers
    - Fixed assertions in processing dynamic relocations for AArch64 builds
  - C standard library
    - Fixed MISRA issues in memset() ABI
  - RAS
    - Fixed bug of binary search in RAS interrupt handler

- Platforms

  - Arm
    - Fixed missing copyrights in arm-gic.h file
    - Fixed the order of header files in several dts files
    - Fixed error message printing in board makefile
    - Fixed bug of overriding the last node in image load helper API
    - Fixed stdout-path in fdts files of TC0 and N1SDP platforms
    - Turn ON/OFF redistributor in sync with GIC CPU interface ON/OFF for css
      platforms
  - Arm FVP
    - Fixed Generic Timer interrupt types in platform dts files
  - Arm Juno
    - Fixed parallel build issue for romlib config
  - Arm SGI
    - Fixed bug in SDEI receive event of RAS handler
  - Intel Agilex
    - Fixed PLAT_MAX_PWR_LVL value
  - Marvell
    - Fixed SPD handling in dram port
  - Marvell ARMADA
    - Fixed TRNG return SMC handling
    - Fixed the logic used for LD selector mask
    - Fixed MSS firmware loader for A8K family
  - ST
    - Fixed few violations reported by coverity static checks
  - STM32MP1
    - Fixed SELFREF_TO_X32 mask in ddr driver
    - Do not keep mmc_device_info in stack
    - Correct plat_crash_console_flush()
  - QEMU SBSA
    - Fixed memory type of secure NOR flash
  - QTI
    - Fixed NUM_APID and REG_APID_MAP() argument in SPMI driver
  - Intel
    - Do not keep mmc_device_info in stack
  - Hisilicon
    - Do not keep mmc_device_info in stack

- Services

  - EL3 runtime
    - Fixed the EL2 context save/restore routine by removing EL2 generic timer
      system registers
    - Added fix for exception handler in BL31 by synchronizing pending EA using
      DSB barrier
  - SPMD
    - Fixed error codes to use int32_t type
  - TSPD
    - Added bug fix in tspd interrupt handling when TSP_NS_INTR_ASYNC_PREEMPT is
      enabled
  - TRNG
    - Fixed compilation errors with -O0 compile option
  - DebugFS
    - Checked channel index before calling clone function
  - PSCI
    - Fixed limit of 256 CPUs caused by cast to unsigned char
  - TSP
    - Fixed compilation erros when built with GCC 11.0.0 toolchain

- Tools

  - FIPtool
    - Do not call `make clean` for `all` target
  - CertCreate
    - Fixed bug to avoid cleaning when building the binary
    - Used preallocated parts of the HASH struct to avoid leaking HASH struct
      fields
    - Free arguments copied with strdup
    - Free keys after use
    - Free X509_EXTENSION structures on stack to avoid leaking them
    - Optimized the code to avoid unnecessary attempts to create non-requested
      certificates

## [2.4.0](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/refs/tags/v2.3..refs/tags/v2.4) (2020-11-17)

### New Features

- Architecture support
  - Armv8.6-A
    - Added support for Armv8.6 Enhanced Counter Virtualization (ECV)
    - Added support for Armv8.6 Fine Grained Traps (FGT)
    - Added support for Armv8.6 WFE trap delays
- Bootloader images
  - Added support for Measured Boot
- Build System
  - Added build option `COT_DESC_IN_DTB` to create Chain of Trust at runtime
  - Added build option `OPENSSL_DIR` to direct tools to OpenSSL libraries
  - Added build option `RAS_TRAP_LOWER_EL_ERR_ACCESS` to enable trapping RAS
    register accesses from EL1/EL2 to EL3
  - Extended build option `BRANCH_PROTECTION` to support branch target
    identification
- Common components
  - Added support for exporting CPU nodes to the device tree
  - Added support for single and dual-root Chains of Trust in secure partitions
- Drivers
  - Added Broadcom RNG driver
  - Added Marvell `mg_conf_cm3` driver
  - Added System Control and Management Interface (SCMI) driver
  - Added STMicroelectronics ETZPC driver
  - Arm GICv3
    - Added support for detecting topology at runtime
  - Dual Root
    - Added support for platform certificates
  - Marvell Cache LLC
    - Added support for mapping the entire LLC into SRAM
  - Marvell CCU
    - Added workaround for erratum 3033912
  - Marvell CP110 COMPHY
    - Added support for SATA COMPHY polarity inversion
    - Added support for USB COMPHY polarity inversion
    - Added workaround for erratum IPCE_COMPHY-1353
  - STM32MP1 Clocks
    - Added `RTC` as a gateable clock
    - Added support for shifted clock selector bit masks
    - Added support for using additional clocks as parents
- Libraries
  - C standard library
    - Added support for hexadecimal and pointer format specifiers in `snprint()`
    - Added assembly alternatives for various library functions
  - CPU support
    - Arm Cortex-A53
      - Added workaround for erratum 1530924
    - Arm Cortex-A55
      - Added workaround for erratum 1530923
    - Arm Cortex-A57
      - Added workaround for erratum 1319537
    - Arm Cortex-A76
      - Added workaround for erratum 1165522
      - Added workaround for erratum 1791580
      - Added workaround for erratum 1868343
    - Arm Cortex-A72
      - Added workaround for erratum 1319367
    - Arm Cortex-A77
      - Added workaround for erratum 1508412
      - Added workaround for erratum 1800714
      - Added workaround for erratum 1925769
    - Arm Neoverse-N1
      - Added workaround for erratum 1868343
  - EL3 Runtime
    - Added support for saving/restoring registers related to nested
      virtualization in EL2 context switches if the architecture supports it
  - FCONF
    - Added support for Measured Boot
    - Added support for populating Chain of Trust properties
    - Added support for loading the `fw_config` image
  - Measured Boot
    - Added support for event logging
- Platforms
  - Added support for Arm Morello
  - Added support for Arm TC0
  - Added support for iEi PUZZLE-M801
  - Added support for Marvell OCTEON TX2 T9130
  - Added support for MediaTek MT8192
  - Added support for NXP i.MX 8M Nano
  - Added support for NXP i.MX 8M Plus
  - Added support for QTI CHIP SC7180
  - Added support for STM32MP151F
  - Added support for STM32MP153F
  - Added support for STM32MP157F
  - Added support for STM32MP151D
  - Added support for STM32MP153D
  - Added support for STM32MP157D
  - Arm
    - Added support for platform-owned SPs
    - Added support for resetting to BL31
  - Arm FPGA
    - Added support for Klein
    - Added support for Matterhorn
    - Added support for additional CPU clusters
  - Arm FVP
    - Added support for performing SDEI platform setup at runtime
    - Added support for SMCCC's `SMCCC_ARCH_SOC_ID` command
    - Added an `id` field under the NV-counter node in the device tree to
      differentiate between trusted and non-trusted NV-counters
    - Added support for extracting the clock frequency from the timer node in
      the device tree
  - Arm Juno
    - Added support for SMCCC's `SMCCC_ARCH_SOC_ID` command
  - Arm N1SDP
    - Added support for cross-chip PCI-e
  - Marvell
    - Added support for AVS reduction
  - Marvell ARMADA
    - Added support for twin-die combined memory device
  - Marvell ARMADA A8K
    - Added support for DDR with 32-bit bus width (both ECC and non-ECC)
  - Marvell AP806
    - Added workaround for erratum FE-4265711
  - Marvell AP807
    - Added workaround for erratum 3033912
  - Nvidia Tegra
    - Added debug printouts indicating SC7 entry sequence completion
    - Added support for SDEI
    - Added support for stack protection
    - Added support for GICv3
    - Added support for SMCCC's `SMCCC_ARCH_SOC_ID` command
  - Nvidia Tegra194
    - Added support for RAS exception handling
    - Added support for SPM
  - NXP i.MX
    - Added support for SDEI
  - QEMU SBSA
    - Added support for the Secure Partition Manager
  - QTI
    - Added RNG driver
    - Added SPMI PMIC arbitrator driver
    - Added support for SMCCC's `SMCCC_ARCH_SOC_ID` command
  - STM32MP1
    - Added support for exposing peripheral interfaces to the non-secure world
      at runtime
    - Added support for SCMI clock and reset services
    - Added support for STM32MP15x CPU revision Z
    - Added support for SMCCC services in `SP_MIN`
- Services
  - Secure Payload Dispatcher
    - Added a provision to allow clients to retrieve the service UUID
  - SPMC
    - Added secondary core endpoint information to the SPMC context structure
  - SPMD
    - Added support for booting OP-TEE as a guest S-EL1 Secure Partition on top
      of Hafnium in S-EL2
    - Added a provision for handling SPMC messages to register secondary core
      entry points
    - Added support for power management operations
- Tools
  - CertCreate
    - Added support for secure partitions
  - CertTool
    - Added support for the `fw_config` image
  - FIPTool
    - Added support for the `fw_config` image

### Changed

- Architecture support
- Bootloader images
- Build System
  - The top-level Makefile now supports building FipTool on Windows
  - The default value of `KEY_SIZE` has been changed to to 2048 when RSA is in
    use
  - The previously-deprecated macro `__ASSEMBLY__` has now been removed
- Common components
  - Certain functions that flush the console will no longer return error
    information
- Drivers
  - Arm GIC
    - Usage of `drivers/arm/gic/common/gic_common.c` has now been deprecated in
      favour of `drivers/arm/gic/vX/gicvX.mk`
    - Added support for detecting the presence of a GIC600-AE
    - Added support for detecting the presence of a GIC-Clayton
  - Marvell MCI
    - Now performs link tuning for all MCI interfaces to improve performance
  - Marvell MoChi
    - PIDI masters are no longer forced into a non-secure access level when
      `LLC_SRAM` is enabled
    - The SD/MMC controllers are now accessible from guest virtual machines
  - Mbed TLS
    - Migrated to Mbed TLS v2.24.0
  - STM32 FMC2 NAND
    - Adjusted FMC node bindings to include an EBI controller node
  - STM32 Reset
    - Added an optional timeout argument to assertion functions
  - STM32MP1 Clocks
    - Enabled several additional system clocks during initialization
- Libraries
  - C Standard Library
    - Improved `memset` performance by avoiding single-byte writes
    - Added optimized assembly variants of `memset`
  - CPU support
    - Renamed Cortex-Hercules to Cortex-A78
    - Renamed Cortex-Hercules AE to Cortex-A78 AE
    - Renamed Neoverse Zeus to Neoverse V1
  - Coreboot
    - Updated ‘coreboot_get_memory_type’ API to take an extra argument as a
      ’memory size’ that used to return a valid memory type.
  - libfdt
    - Updated to latest upstream version
- Platforms
  - Allwinner
    - Disabled non-secure access to PRCM power control registers
  - Arm
    - `BL32_BASE` is now platform-dependent when `SPD_spmd` is enabled
    - Added support for loading the Chain of Trust from the device tree
    - The firmware update check is now executed only once
    - NV-counter base addresses are now loaded from the device tree when
      `COT_DESC_IN_DTB` is enabled
    - Now loads and populates `fw_config` and `tb_fw_config`
    - FCONF population now occurs after caches have been enabled in order to
      reduce boot times
  - Arm Corstone-700
    - Platform support has been split into both an FVP and an FPGA variant
  - Arm FPGA
    - DTB and BL33 load addresses have been given sensible default values
    - Now reads generic timer counter frequency, GICD and GICR base addresses,
      and UART address from DT
    - Now treats the primary PL011 UART as an SBSA Generic UART
  - Arm FVP
    - Secure interrupt descriptions, UART parameters, clock frequencies and
      GICv3 parameters are now queried through FCONF
    - UART parameters are now queried through the device tree
    - Added an owner field to Cactus secure partitions
    - Increased the maximum size of BL2 when the Chain of Trust is loaded from
      the device tree
    - Reduces the maximum size of BL31
    - The `FVP_USE_SP804_TIMER` and `FVP_VE_USE_SP804_TIMER` build options have
      been removed in favour of a common `USE_SP804_TIMER` option
    - Added a third Cactus partition to manifests
    - Device tree nodes now store UUIDs in big-endian
  - Arm Juno
    - Increased the maximum size of BL2 when optimizations have not been applied
    - Reduced the maximum size of BL31 and BL32
  - Marvell AP807
    - Enabled snoop filters
  - Marvell ARMADA A3K
    - UART recovery images are now suffixed with `.bin`
  - Marvell ARMADA A8K
    - Option `BL31_CACHE_DISABLE` is now disabled (`0`) by default
  - Nvidia Tegra
    - Added VPR resize supported check when processing video memory resize
      requests
    - Added SMMU verification to prevent potential issues caused by undetected
      corruption of the SMMU configuration during boot
    - The GIC CPU interface is now properly disabled after CPU off
    - The GICv2 sources list and the `BL31_SIZE` definition have been made
      platform-specific
    - The SPE driver will no longer flush the console when writing individual
      characters
  - Nvidia Tegra194
    - TZDRAM setup has been moved to platform-specific early boot handlers
    - Increased verbosity of debug prints for RAS SErrors
    - Support for powering down CPUs during CPU suspend has been removed
    - Now verifies firewall settings before using resources
  - TI K3
    - The UART number has been made configurable through `K3_USART`
  - Rockchip RK3368
    - The maximum number of memory map regions has been increased to 20
  - Socionext Uniphier
    - The maximum size of BL33 has been increased to support larger bootloaders
  - STM32
    - Removed platform-specific DT functions in favour of using existing generic
      alternatives
  - STM32MP1
    - Increased verbosity of exception reports in debug builds
    - Device trees have been updated to align with the Linux kernel
    - Now uses the ETZPC driver to configure secure-aware interfaces for
      assignment to the non-secure world
    - Finished good variants have been added to the board identifier
      enumerations
    - Non-secure access to clocks and reset domains now depends on their state
      of registration
    - NEON is now disabled in `SP_MIN`
    - The last page of `SYSRAM` is now used as SCMI shared memory
    - Checks to verify platform compatibility have been added to verify that an
      image is compatible with the chip ID of the running platform
  - QEMU SBSA
    - Removed support for Arm's Cortex-A53
- Services
  - Renamed SPCI to FF-A
  - SPMD
    - No longer forwards requests to the non-secure world when retrieving
      partition information
    - SPMC manifest size is now retrieved directly from SPMD instead of the
      device tree
    - The FF-A version handler now returns SPMD's version when the origin of the
      call is secure, and SPMC's version when the origin of the call is
      non-secure
  - SPMC
    - Updated the manifest to declare CPU nodes in descending order as per the
      SPM (Hafnium) multicore requirement
    - Updated the device tree to mark 2GB as device memory for the first
      partition excluding trusted DRAM region (which is reserved for SPMC)
    - Increased the number of EC contexts to the maximum number of PEs as per
      the FF-A specification
- Tools
  - FIPTool
    - Now returns `0` on `help` and `help <command>`
  - Marvell DoImage
    - Updated Mbed TLS support to v2.8
  - SPTool
    - Now appends CertTool arguments

### Resolved Issues

- Bootloader images
  - Fixed compilation errors for dual-root Chains of Trust caused by symbol
    collision
  - BL31
    - Fixed compilation errors on platforms with fewer than 4 cores caused by
      initialization code exceeding the end of the stacks
    - Fixed compilation errors when building a position-independent image
- Build System
  - Fixed invalid empty version strings
  - Fixed compilation errors on Windows caused by a non-portable architecture
    revision comparison
- Drivers
  - Arm GIC
    - Fixed spurious interrupts caused by a missing barrier
  - STM32 Flexible Memory Controller 2 (FMC2) NAND driver
    - Fixed runtime instability caused by incorrect error detection logic
  - STM32MP1 Clock driver
    - Fixed incorrectly-formatted log messages
    - Fixed runtime instability caused by improper clock gating procedures
  - STMicroelectronics Raw NAND driver
    - Fixed runtime instability caused by incorrect unit conversion when waiting
      for NAND readiness
- Libraries
  - AMU
    - Fixed timeout errors caused by excess error logging
  - EL3 Runtime
    - Fixed runtime instability caused by improper register save/restore routine
      in EL2
  - FCONF
    - Fixed failure to initialize GICv3 caused by overly-strict device tree
      requirements
  - Measured Boot
    - Fixed driver errors caused by a missing default value for the `HASH_ALG`
      build option
  - SPE
    - Fixed feature detection check that prevented CPUs supporting SVE from
      detecting support for SPE in the non-secure world
  - Translation Tables
    - Fixed various MISRA-C 2012 static analysis violations
- Platforms
  - Allwinner A64
    - Fixed USB issues on certain battery-powered device caused by improperly
      activated USB power rail
  - Arm
    - Fixed compilation errors caused by increase in BL2 size
    - Fixed compilation errors caused by missing Makefile dependencies to
      generated files when building the FIP
    - Fixed MISRA-C 2012 static analysis violations caused by unused structures
      in include directives intended to be feature-gated
  - Arm FPGA
    - Fixed initialization issues caused by incorrect MPIDR topology mapping
      logic
  - Arm RD-N1-edge
    - Fixed compilation errors caused by mismatched parentheses in Makefile
  - Arm SGI
    - Fixed crashes due to the flash memory used for cold reboot attack
      protection not being mapped
  - Intel Agilex
    - Fixed initialization issues caused by several compounding bugs
  - Marvell
    - Fixed compilation warnings caused by multiple Makefile inclusions
  - Marvell ARMADA A3K
    - Fixed boot issue in debug builds caused by checks on the BL33 load address
      that are not appropriate for this platform
  - Nvidia Tegra
    - Fixed incorrect delay timer reads
    - Fixed spurious interrupts in the non-secure world during cold boot caused
      by the arbitration bit in the memory controller not being cleared
    - Fixed faulty video memory resize sequence
  - Nvidia Tegra194
    - Fixed incorrect alignment of TZDRAM base address
  - NXP iMX8M
    - Fixed CPU hot-plug issues caused by race condition
  - STM32MP1
    - Fixed compilation errors in highly-parallel builds caused by incorrect
      Makefile dependencies
  - STM32MP157C-ED1
    - Fixed initialization issues caused by missing device tree hash node
  - Raspberry Pi 3
    - Fixed compilation errors caused by incorrect dependency ordering in
      Makefile
  - Rockchip
    - Fixed initialization issues caused by non-critical errors when parsing FDT
      being treated as critical
  - Rockchip RK3368
    - Fixed runtime instability caused by incorrect CPUID shift value
  - QEMU
    - Fixed compilation errors caused by incorrect dependency ordering in
      Makefile
  - QEMU SBSA
    - Fixed initialization issues caused by FDT exceeding reserved memory size
  - QTI
    - Fixed compilation errors caused by inclusion of a non-existent file
- Services
  - FF-A (previously SPCI)
    - Fixed SPMD aborts caused by incorrect behaviour when the manifest is
      page-aligned
- Tools
  - Fixed compilation issues when compiling tools from within their respective
    directories
  - FIPTool
    - Fixed command line parsing issues on Windows when using arguments whose
      names also happen to be a subset of another's
  - Marvell DoImage
    - Fixed PKCS signature verification errors at boot on some platforms caused
      by generation of misaligned images

### Known Issues

- Platforms
  - NVIDIA Tegra
    - Signed comparison compiler warnings occurring in libfdt are currently
      being worked around by disabling the warning for the platform until the
      underlying issue is resolved in libfdt

## [2.3.0](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/refs/tags/v2.2..refs/tags/v2.3) (2020-04-20)

### New Features

- Arm Architecture
  - Add support for Armv8.4-SecEL2 extension through the SPCI defined SPMD/SPMC
    components.
  - Build option to support EL2 context save and restore in the secure world
    (CTX_INCLUDE_EL2_REGS).
  - Add support for SMCCC v1.2 (introducing the new SMCCC_ARCH_SOC_ID SMC). Note
    that the support is compliant, but the SVE registers save/restore will be
    done as part of future S-EL2/SPM development.
- BL-specific
  - Enhanced BL2 bootloader flow to load secure partitions based on firmware
    configuration data (fconf).
  - Changes necessary to support SEPARATE_NOBITS_REGION feature
  - TSP and BL2_AT_EL3: Add Position Independent Execution `PIE` support
- Build System
  - Add support for documentation build as a target in Makefile
  - Add `COT` build option to select the Chain of Trust to use when the Trusted
    Boot feature is enabled (default: `tbbr`).
  - Added creation and injection of secure partition packages into the FIP.
  - Build option to support SPMC component loading and run at S-EL1 or S-EL2
    (SPMD_SPM_AT_SEL2).
  - Enable MTE support
  - Enable Link Time Optimization in GCC
  - Enable -Wredundant-decls warning check
  - Makefile: Add support to optionally encrypt BL31 and BL32
  - Add support to pass the nt_fw_config DTB to OP-TEE.
  - Introduce per-BL `CPPFLAGS`, `ASFLAGS`, and `LDFLAGS`
  - build_macros: Add CREATE_SEQ function to generate sequence of numbers
- CPU Support
  - cortex-a57: Enable higher performance non-cacheable load forwarding
  - Hercules: Workaround for Errata 1688305
  - Klein: Support added for Klein CPU
  - Matterhorn: Support added for Matterhorn CPU
- Drivers
  - auth: Add `calc_hash` function for hash calculation. Used for authentication
    of images when measured boot is enabled.
  - cryptocell: Add authenticated decryption framework, and support for
    CryptoCell-713 and CryptoCell-712 RSA 3K
  - gic600: Add support for multichip configuration and Clayton
  - gicv3: Introduce makefile, Add extended PPI and SPI range, Add support for
    probing multiple GIC Redistributor frames
  - gicv4: Add GICv4 extension for GIC driver
  - io: Add an IO abstraction layer to load encrypted firmwares
  - mhu: Derive doorbell base address
  - mtd: Add SPI-NOR, SPI-NAND, SPI-MEM, and raw NAND framework
  - scmi: Allow use of multiple SCMI channels
  - scu: Add a driver for snoop control unit
- Libraries
  - coreboot: Add memory range parsing and use generic base address
  - compiler_rt: Import popcountdi2.c and popcountsi2.c files, aeabi_ldivmode.S
    file and dependencies
  - debugFS: Add DebugFS functionality
  - el3_runtime: Add support for enabling S-EL2
  - fconf: Add Firmware Configuration Framework (fconf) (experimental).
  - libc: Add memrchr function
  - locks: bakery: Use is_dcache_enabled() helper and add a DMB to the
    'read_cache_op' macro
  - psci: Add support to enable different personality of the same soc.
  - xlat_tables_v2: Add support to pass shareability attribute for normal memory
    region, use get_current_el_maybe_constant() in is_dcache_enabled(),
    read-only xlat tables for BL31 memory, and add enable_mmu()
- New Platforms Support
  - arm/arm_fpga: New platform support added for FPGA
  - arm/rddaniel: New platform support added for rd-daniel platform
  - brcm/stingray: New platform support added for Broadcom stingray platform
  - nvidia/tegra194: New platform support for Nvidia Tegra194 platform
- Platforms
  - allwinner: Implement PSCI system suspend using SCPI, add a msgbox driver for
    use with SCPI, and reserve and map space for the SCP firmware
  - allwinner: axp: Add AXP805 support
  - allwinner: power: Add DLDO4 power rail
  - amlogic: axg: Add a build flag when using ATOS as BL32 and support for the
    A113D (AXG) platform
  - arm/a5ds: Add ethernet node and L2 cache node in devicetree
  - arm/common: Add support for the new `dualroot` chain of trust
  - arm/common: Add support for SEPARATE_NOBITS_REGION
  - arm/common: Re-enable PIE when RESET_TO_BL31=1
  - arm/common: Allow boards to specify second DRAM Base address and to define
    PLAT_ARM_TZC_FILTERS
  - arm/corstone700: Add support for mhuv2 and stack protector
  - arm/fvp: Add support for fconf in BL31 and SP_MIN. Populate power domain
    descriptor dynamically by leveraging fconf APIs.
  - arm/fvp: Add Cactus/Ivy Secure Partition information and use two instances
    of Cactus at S-EL1
  - arm/fvp: Add support to run BL32 in TDRAM and BL31 in secure DRAM
  - arm/fvp: Add support for GICv4 extension and BL2 hash calculation in BL1
  - arm/n1sdp: Setup multichip gic routing table, update platform macros for
    dual-chip setup, introduce platform information SDS region, add support to
    update presence of External LLC, and enable the NEOVERSE_N1_EXTERNAL_LLC
    flag
  - arm/rdn1edge: Add support for dual-chip configuration and use CREATE_SEQ
    helper macro to compare chip count
  - arm/sgm: Always use SCMI for SGM platforms
  - arm/sgm775: Add support for dynamic config using fconf
  - arm/sgi: Add multi-chip mode parameter in HW_CONFIG dts, macros for remote
    chip device region, chip_id and multi_chip_mode to platform variant info,
    and introduce number of chips macro
  - brcm: Add BL2 and BL31 support common across Broadcom platforms
  - brcm: Add iproc SPI Nor flash support, spi driver, emmc driver, and support
    to retrieve plat_toc_flags
  - hisilicon: hikey960: Enable system power off callback
  - intel: Enable bridge access, SiP SMC secure register access, and uboot
    entrypoint support
  - intel: Implement platform specific system reset 2
  - intel: Introduce mailbox response length handling
  - imx: console: Use CONSOLE_T_BASE for UART base address and generic console_t
    data structure
  - imx8mm: Provide uart base as build option and add the support for opteed spd
    on imx8mq/imx8mm
  - imx8qx: Provide debug uart num as build
  - imx8qm: Apply clk/pinmux configuration for DEBUG_CONSOLE and provide debug
    uart num as build param
  - marvell: a8k: Implement platform specific power off and add support for
    loading MG CM3 images
  - mediatek: mt8183: Add Vmodem/Vcore DVS init level
  - qemu: Support optional encryption of BL31 and BL32 images and
    ARM_LINUX_KERNEL_AS_BL33 to pass FDT address
  - qemu: Define ARMV7_SUPPORTS_VFP
  - qemu: Implement PSCI_CPU_OFF and qemu_system_off via semihosting
  - renesas: rcar_gen3: Add new board revision for M3ULCB
  - rockchip: Enable workaround for erratum 855873, claim a macro to enable hdcp
    feature for DP, enable power domains of rk3399 before reset, add support for
    UART3 as serial output, and initialize reset and poweroff GPIOs with known
    invalid value
  - rpi: Implement PSCI CPU_OFF, use MMIO accessor, autodetect Mini-UART vs.
    PL011 configuration, and allow using PL011 UART for RPi3/RPi4
  - rpi3: Include GPIO driver in all BL stages and use same "clock-less" setup
    scheme as RPi4
  - rpi3/4: Add support for offlining CPUs
  - st: stm32mp1: platform.mk: Support generating multiple images in one build,
    migrate to implicit rules, derive map file name from target name, generate
    linker script with fixed name, and use PHONY for the appropriate targets
  - st: stm32mp1: Add support for SPI-NOR, raw NAND, and SPI-NAND boot device,
    QSPI, FMC2 driver
  - st: stm32mp1: Use stm32mp_get_ddr_ns_size() function, set XN attribute for
    some areas in BL2, dynamically map DDR later and non-cacheable during its
    test, add a function to get non-secure DDR size, add DT helper for reg by
    name, and add compilation flags for boot devices
  - socionext: uniphier: Turn on ENABLE_PIE
  - ti: k3: Add PIE support
  - xilinx: versal: Add set wakeup source, client wakeup, query data, request
    wakeup, PM_INIT_FINALIZE, PM_GET_TRUSTZONE_VERSION, PM IOCTL, support for
    suspend related, and Get_ChipID APIs
  - xilinx: versal: Implement power down/restart related EEMI, SMC handler for
    EEMI, PLL related PM, clock related PM, pin control related PM, reset
    related PM, device related PM , APIs
  - xilinx: versal: Enable ipi mailbox service
  - xilinx: versal: Add get_api_version support and support to send PM API to
    PMC using IPI
  - xilinx: zynqmp: Add checksum support for IPI data, GET_CALLBACK_DATA
    function, support to query max divisor, CLK_SET_RATE_PARENT in gem clock
    node, support for custom type flags, LPD WDT clock to the pm_clock
    structure, idcodes for new RFSoC silicons ZU48DR and ZU49DR, and id for new
    RFSoC device ZU39DR
- Security
  - Use Speculation Barrier instruction for v8.5+ cores
  - Add support for optional firmware encryption feature (experimental).
  - Introduce a new `dualroot` chain of trust.
  - aarch64: Prevent speculative execution past ERET
  - aarch32: Stop speculative execution past exception returns.
- SPCI
  - Introduced the Secure Partition Manager Dispatcher (SPMD) component as a new
    standard service.
- Tools
  - cert_create: Introduce CoT build option and TBBR CoT makefile, and define
    the dualroot CoT
  - encrypt_fw: Add firmware authenticated encryption tool
  - memory: Add show_memory script that prints a representation of the memory
    layout for the latest build

### Changed

- Arm Architecture
  - PIE: Make call to GDT relocation fixup generalized
- BL-Specific
  - Increase maximum size of BL2 image
  - BL31: Discard .dynsym .dynstr .hash sections to make ENABLE_PIE work
  - BL31: Split into two separate memory regions
  - Unify BL linker scripts and reduce code duplication.
- Build System
  - Changes to drive cert_create for dualroot CoT
  - Enable -Wlogical-op always
  - Enable -Wshadow always
  - Refactor the warning flags
  - PIE: Pass PIE options only to BL31
  - Reduce space lost to object alignment
  - Set lld as the default linker for Clang builds
  - Remove -Wunused-const-variable and -Wpadded warning
  - Remove -Wmissing-declarations warning from WARNING1 level
- Drivers
  - authentication: Necessary fix in drivers to upgrade to mbedtls-2.18.0
  - console: Integrate UART base address in generic console_t
  - gicv3: Change API for GICR_IPRIORITYR accessors and separate GICD and GICR
    accessor functions
  - io: Change seek offset to signed long long and panic in case of io setup
    failure
  - smmu: SMMUv3: Changed retry loop to delay timer
  - tbbr: Reduce size of hash and ECDSA key buffers when possible
- Library Code
  - libc: Consolidate the size_t, unified, and NULL definitions, and unify
    intmax_t and uintmax_t on AArch32/64
  - ROMLIB: Optimize memory layout when ROMLIB is used
  - xlat_tables_v2: Use ARRAY_SIZE in REGISTER_XLAT_CONTEXT_FULL_SPEC, merge
    REGISTER_XLAT_CONTEXT\_{FULL_SPEC,RO_BASE_TABLE}, and simplify end address
    checks in mmap_add_region_check()
- Platforms
  - allwinner: Adjust SRAM A2 base to include the ARISC vectors, clean up MMU
    setup, reenable USE_COHERENT_MEM, remove unused include path, move the
    NOBITS region to SRAM A1, convert AXP803 regulator setup code into a driver,
    enable clock before resetting I2C/RSB
  - allwinner: h6: power: Switch to using the AXP driver
  - allwinner: a64: power: Use fdt_for_each_subnode, remove obsolete register
    check, remove duplicate DT check, and make sunxi_turn_off_soc static
  - allwinner: Build PMIC bus drivers only in BL31, clean up PMIC-related error
    handling, and synchronize PMIC enumerations
  - arm/a5ds: Change boot address to point to DDR address
  - arm/common: Check for out-of-bound accesses in the platform io policies
  - arm/corstone700: Updating the kernel arguments to support initramfs, use
    fdts DDR memory and XIP rootfs, and set UART clocks to 32MHz
  - arm/fvp: Modify multithreaded dts file of DynamIQ FVPs, slightly bump the
    stack size for bl1 and bl2, remove re-definition of topology related build
    options, stop reclaiming init code with Clang builds, and map only the
    needed DRAM region statically in BL31/SP_MIN
  - arm/juno: Maximize space allocated to SCP_BL2
  - arm/sgi: Bump bl1 RW limit, mark remote chip shared ram as non-cacheable,
    move GIC related constants to board files, include AFF3 affinity in core
    position calculation, move bl31_platform_setup to board file, and move
    topology information to board folder
  - common: Refactor load_auth_image_internal().
  - hisilicon: Remove uefi-tools in hikey and hikey960 documentation
  - intel: Modify non secure access function, BL31 address mapping, mailbox's
    get_config_status, and stratix10 BL31 parameter handling
  - intel: Remove un-needed checks for qspi driver r/w and s10 unused source
    code
  - intel: Change all global sip function to static
  - intel: Refactor common platform code
  - intel: Create SiP service header file
  - marvell: armada: scp_bl2: Allow loading up to 8 images
  - marvell: comphy-a3700: Support SGMII COMPHY power off and fix USB3 powering
    on when on lane 2
  - marvell: Consolidate console register calls
  - mediatek: mt8183: Protect 4GB~8GB dram memory, refine GIC driver for low
    power scenarios, and switch PLL/CLKSQ/ck_off/axi_26m control to SPM
  - qemu: Update flash address map to keep FIP in secure FLASH0
  - renesas: rcar_gen3: Update IPL and Secure Monitor Rev.2.0.6, update DDR
    setting for H3, M3, M3N, change fixed destination address of BL31 and BL32,
    add missing #{address,size}-cells into generated DT, pass DT to OpTee OS,
    and move DDR drivers out of staging
  - rockchip: Make miniloader ddr_parameter handling optional, cleanup securing
    of ddr regions, move secure init to separate file, use base+size for secure
    ddr regions, bring TZRAM_SIZE values in lined, and prevent macro expansion
    in paths
  - rpi: Move plat_helpers.S to common
  - rpi3: gpio: Simplify GPIO setup
  - rpi4: Skip UART initialisation
  - st: stm32m1: Use generic console_t data structure, remove second QSPI flash
    instance, update for FMC2 pin muxing, and reduce MAX_XLAT_TABLES to 4
  - socionext: uniphier: Make on-chip SRAM and I/O register regions configurable
  - socionext: uniphier: Make PSCI related, counter control, UART, pinmon, NAND
    controller, and eMMC controller base addresses configurable
  - socionext: uniphier: Change block_addressing flag and the return value type
    of .is_usb_boot() to bool
  - socionext: uniphier: Run BL33 at EL2, call uniphier_scp_is_running() only
    when on-chip STM is supported, define PLAT_XLAT_TABLES_DYNAMIC only for BL2,
    support read-only xlat tables, use enable_mmu() in common function, shrink
    UNIPHIER_ROM_REGION_SIZE, prepare uniphier_soc_info() for next SoC, extend
    boot device detection for future SoCs, make all BL images completely
    position-independent, make uniphier_mmap_setup() work with PIE, pass SCP
    base address as a function parameter, set buffer offset and length for
    io_block dynamically, and use more mmap_add_dynamic_region() for loading
    images
  - spd/trusty: Disable error messages seen during boot, allow gic base to be
    specified with GICD_BASE, and allow getting trusty memsize from
    BL32_MEM_SIZE instead of TSP_SEC_MEM_SIZE
  - ti: k3: common: Enable ARM cluster power down and rename device IDs to be
    more consistent
  - ti: k3: drivers: ti_sci: Put sequence number in coherent memory and remove
    indirect structure of const data
  - xilinx: Move ipi mailbox svc to xilinx common
  - xilinx: zynqmp: Use GIC framework for warm restart
  - xilinx: zynqmp: pm: Move custom clock flags to typeflags, remove
    CLK_TOPSW_LSBUS from invalid clock list and rename FPD WDT clock ID
  - xilinx: versal: Increase OCM memory size for DEBUG builds and adjust cpu
    clock, Move versal_def.h and versal_private to include directory
- Tools
  - sptool: Updated sptool to accommodate building secure partition packages.

### Resolved Issues

- Arm Architecture
  - Fix crash dump for lower EL
- BL-Specific
  - Bug fix: Protect TSP prints with lock
  - Fix boot failures on some builds linked with ld.lld.
- Build System
  - Fix clang build if CC is not in the path.
  - Fix 'BL stage' comment for build macros
- Code Quality
  - coverity: Fix various MISRA violations including null pointer violations, C
    issues in BL1/BL2/BL31 and FDT helper functions, using boolean essential,
    type, and removing unnecessary header file and comparisons to LONG_MAX in
    debugfs devfip
  - Based on coding guidelines, replace all `unsigned long` depending on if
    fixed based on AArch32 or AArch64.
  - Unify type of "cpu_idx" and Platform specific defines across PSCI module.
- Drivers
  - auth: Necessary fix in drivers to upgrade to mbedtls-2.18.0
  - delay_timer: Fix non-standard frequency issue in udelay
  - gicv3: Fix compiler dependent behavior
  - gic600: Fix include ordering according to the coding style and power up
    sequence
- Library Code
  - el3_runtime: Fix stack pointer maintenance on EA handling path, fixup
    'cm_setup_context' prototype, and adds TPIDR_EL2 register to the context
    save restore routines
  - libc: Fix SIZE_MAX on AArch32
  - locks: T589: Fix insufficient ordering guarantees in bakery lock
  - pmf: Fix 'tautological-constant-compare' error, Make the runtime
    instrumentation work on AArch32, and Simplify PMF helper macro definitions
    across header files
  - xlat_tables_v2: Fix assembler warning of PLAT_RO_XLAT_TABLES
- Platforms
  - allwinner: Fix H6 GPIO and CCU memory map addresses and incorrect ARISC code
    patch offset check
  - arm/a5ds: Correct system freq and Cache Writeback Granule, and cleanup
    enable-method in devicetree
  - arm/fvp: Fix incorrect GIC mapping, BL31 load address and image size for
    RESET_TO_BL31=1, topology description of cpus for DynamIQ based FVP, and
    multithreaded FVP power domain tree
  - arm/fvp: spm-mm: Correcting instructions to build SPM for FVP
  - arm/common: Fix ROTPK hash generation for ECDSA encryption, BL2 bug in
    dynamic configuration initialisation, and current RECLAIM_INIT_CODE behavior
  - arm/rde1edge: Fix incorrect topology tree description
  - arm/sgi: Fix the incorrect check for SCMI channel ID
  - common: Flush dcache when storing timestamp
  - intel: Fix UEFI decompression issue, memory calibration, SMC SIP service,
    mailbox config return status, mailbox driver logic, FPGA manager on
    reconfiguration, and mailbox send_cmd issue
  - imx: Fix shift-overflow errors, the rdc memory region slot's offset,
    multiple definition of ipc_handle, missing inclusion of cdefs.h, and correct
    the SGIs that used for secure interrupt
  - mediatek: mt8183: Fix AARCH64 init fail on CPU0
  - rockchip: Fix definition of struct param_ddr_usage
  - rpi4: Fix documentation of armstub config entry
  - st: Correct io possible NULL pointer dereference and device_size type, nand
    xor_ecc.val assigned value, static analysis tool issues, and fix incorrect
    return value and correctly check pwr-regulators node
  - xilinx: zynqmp: Correct syscnt freq for QEMU and fix clock models and IDs of
    GEM-related clocks

### Known Issues

- Build System
  - dtb: DTB creation not supported when building on a Windows host.

    This step in the build process is skipped when running on a Windows host. A
    known issue from the 1.6 release.

  - Intermittent assertion firing `ASSERT: services/spd/tspd/tspd_main.c:105`
- Coverity
  - Intermittent Race condition in Coverity Jenkins Build Job
- Platforms
  - arm/juno: System suspend from Linux does not function as documented in the
    user guide

    Following the instructions provided in the user guide document does not
    result in the platform entering system suspend state as expected. A message
    relating to the hdlcd driver failing to suspend will be emitted on the Linux
    terminal.

  - mediatek/mt6795: This platform does not build in this release

## [2.2.0](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/refs/tags/v2.1..refs/tags/v2.2) (2019-10-22)

### New Features

- Architecture
  - Enable Pointer Authentication (PAuth) support for Secure World

    - Adds support for ARMv8.3-PAuth in BL1 SMC calls and BL2U image for
      firmware updates.

  - Enable Memory Tagging Extension (MTE) support in both secure and non-secure
    worlds

    - Adds support for the new Memory Tagging Extension arriving in ARMv8.5. MTE
      support is now enabled by default on systems that support it at EL0.
    - To enable it at ELx for both the non-secure and the secure world, the
      compiler flag `CTX_INCLUDE_MTE_REGS` includes register saving and
      restoring when necessary in order to prevent information leakage between
      the worlds.

  - Add support for Branch Target Identification (BTI)
- Build System
  - Modify FVP makefile for CPUs that support both AArch64/32
  - AArch32: Allow compiling with soft-float toolchain
  - Makefile: Add default warning flags
  - Add Makefile check for PAuth and AArch64
  - Add compile-time errors for HW_ASSISTED_COHERENCY flag
  - Apply compile-time check for AArch64-only CPUs
  - build_macros: Add mechanism to prevent bin generation.
  - Add support for default stack-protector flag
  - spd: opteed: Enable NS_TIMER_SWITCH
  - plat/arm: Skip BL2U if RESET_TO_SP_MIN flag is set
  - Add new build option to let each platform select which implementation of
    spinlocks it wants to use
- CPU Support
  - DSU: Workaround for erratum 798953 and 936184
  - Neoverse N1: Force cacheable atomic to near atomic
  - Neoverse N1: Workaround for erratum 1073348, 1130799, 1165347, 1207823,
    1220197, 1257314, 1262606, 1262888, 1275112, 1315703, 1542419
  - Neoverse Zeus: Apply the MSR SSBS instruction
  - cortex-Hercules/HerculesAE: Support added for Cortex-Hercules and
    Cortex-HerculesAE CPUs
  - cortex-Hercules/HerculesAE: Enable AMU for Cortex-Hercules and
    Cortex-HerculesAE
  - cortex-a76AE: Support added for Cortex-A76AE CPU
  - cortex-a76: Workaround for erratum 1257314, 1262606, 1262888, 1275112,
    1286807
  - cortex-a65/a65AE: Support added for Cortex-A65 and Cortex-A65AE CPUs
  - cortex-a65: Enable AMU for Cortex-A65
  - cortex-a55: Workaround for erratum 1221012
  - cortex-a35: Workaround for erratum 855472
  - cortex-a9: Workaround for erratum 794073
- Drivers
  - console: Allow the console to register multiple times

  - delay: Timeout detection support

  - gicv3: Enabled multi-socket GIC redistributor frame discovery and migrated
    ARM platforms to the new API

    - Adds `gicv3_rdistif_probe` function that delegates the responsibility of
      discovering the corresponding redistributor base frame to each CPU itself.

  - sbsa: Add SBSA watchdog driver

  - st/stm32_hash: Add HASH driver

  - ti/uart: Add an AArch32 variant
- Library at ROM (romlib)
  - Introduce BTI support in Library at ROM (romlib)
- New Platforms Support
  - amlogic: g12a: New platform support added for the S905X2 (G12A) platform
  - amlogic: meson/gxl: New platform support added for Amlogic Meson S905x (GXL)
  - arm/a5ds: New platform support added for A5 DesignStart
  - arm/corstone: New platform support added for Corstone-700
  - intel: New platform support added for Agilex
  - mediatek: New platform support added for MediaTek mt8183
  - qemu/qemu_sbsa: New platform support added for QEMU SBSA platform
  - renesas/rcar_gen3: plat: New platform support added for D3
  - rockchip: New platform support added for px30
  - rockchip: New platform support added for rk3288
  - rpi: New platform support added for Raspberry Pi 4
- Platforms
  - arm/common: Introduce wrapper functions to setup secure watchdog
  - arm/fvp: Add Delay Timer driver to BL1 and BL31 and option for defining
    platform DRAM2 base
  - arm/fvp: Add Linux DTS files for 32 bit threaded FVPs
  - arm/n1sdp: Add code for DDR ECC enablement and BL33 copy to DDR, Initialise
    CNTFRQ in Non Secure CNTBaseN
  - arm/juno: Use shared mbedtls heap between BL1 and BL2 and add basic support
    for dynamic config
  - imx: Basic support for PicoPi iMX7D, rdc module init, caam module init,
    aipstz init, IMX_SIP_GET_SOC_INFO, IMX_SIP_BUILDINFO added
  - intel: Add ncore ccu driver
  - mediatek/mt81\*: Use new bl31_params_parse() helper
  - nvidia: tegra: Add support for multi console interface
  - qemu/qemu_sbsa: Adding memory mapping for both FLASH0/FLASH1
  - qemu: Added gicv3 support, new console interface in AArch32, and
    sub-platforms
  - renesas/rcar_gen3: plat: Add R-Car V3M support, new board revision for
    H3ULCB, DBSC4 setting before self-refresh mode
  - socionext/uniphier: Support console based on multi-console
  - st: stm32mp1: Add OP-TEE, Avenger96, watchdog, LpDDR3, authentication
    support and general SYSCFG management
  - ti/k3: common: Add support for J721E, Use coherent memory for shared data,
    Trap all asynchronous bus errors to EL3
  - xilinx/zynqmp: Add support for multi console interface, Initialize IPI table
    from zynqmp_config_setup()
- PSCI
  - Adding new optional PSCI hook `pwr_domain_on_finish_late`
    - This PSCI hook `pwr_domain_on_finish_late` is similar to
      `pwr_domain_on_finish` but is guaranteed to be invoked when the respective
      core and cluster are participating in coherency.
- Security
  - Speculative Store Bypass Safe (SSBS): Further enhance protection against
    Spectre variant 4 by disabling speculative loads/stores (SPSR.SSBS bit) by
    default.
  - UBSAN support and handlers
    - Adds support for the Undefined Behaviour sanitizer. There are two types of
      support offered - minimalistic trapping support which essentially
      immediately crashes on undefined behaviour and full support with full
      debug messages.
- Tools
  - cert_create: Add support for bigger RSA key sizes (3KB and 4KB), previously
    the maximum size was 2KB.
  - fiptool: Add support to build fiptool on Windows.

### Changed

- Architecture
  - Refactor ARMv8.3 Pointer Authentication support code
  - backtrace: Strip PAC field when PAUTH is enabled
  - Prettify crash reporting output on AArch64.
  - Rework smc_unknown return code path in smc_handler
    - Leverage the existing `el3_exit()` return routine for smc_unknown return
      path rather than a custom set of instructions.
- BL-Specific
  - Invalidate dcache build option for BL2 entry at EL3
  - Add missing support for BL2_AT_EL3 in XIP memory
- Boot Flow
  - Add helper to parse BL31 parameters (both versions)
  - Factor out cross-BL API into export headers suitable for 3rd party code
  - Introduce lightweight BL platform parameter library
- Drivers
  - auth: Memory optimization for Chain of Trust (CoT) description
  - bsec: Move bsec_mode_is_closed_device() service to platform
  - cryptocell: Move Cryptocell specific API into driver
  - gicv3: Prevent pending G1S interrupt from becoming G0 interrupt
  - mbedtls: Remove weak heap implementation
  - mmc: Increase delay between ACMD41 retries
  - mmc: stm32_sdmmc2: Correctly manage block size
  - mmc: stm32_sdmmc2: Manage max-frequency property from DT
  - synopsys/emmc: Do not change FIFO TH as this breaks some platforms
  - synopsys: Update synopsys drivers to not rely on undefined overflow
    behaviour
  - ufs: Extend the delay after reset to wait for some slower chips
- Platforms
  - amlogic/meson/gxl: Remove BL2 dependency from BL31
  - arm/common: Shorten the Firmware Update (FWU) process
  - arm/fvp: Remove GIC initialisation from secondary core cold boot
  - arm/sgm: Temporarily disable shared Mbed TLS heap for SGM
  - hisilicon: Update hisilicon drivers to not rely on undefined overflow
    behaviour
  - imx: imx8: Replace PLAT_IMX8\* with PLAT_imx8\*, remove duplicated linker
    symbols and deprecated code include, keep only IRQ 32 unmasked, enable all
    power domain by default
  - marvell: Prevent SError accessing PCIe link, Switch to xlat_tables_v2, do
    not rely on argument passed via smc, make sure that comphy init will use
    correct address
  - mediatek: mt8173: Refactor RTC and PMIC drivers
  - mediatek: mt8173: Apply MULTI_CONSOLE framework
  - nvidia: Tegra: memctrl_v2: fix "overflow before widen" coverity issue
  - qemu: Simplify the image size calculation, Move and generalise FDT PSCI
    fixup, move gicv2 codes to separate file
  - renesas/rcar_gen3: Convert to multi-console API, update QoS setting, Update
    IPL and Secure Monitor Rev2.0.4, Change to restore timer counter value at
    resume, Update DDR setting rev.0.35, qos: change subslot cycle, Change
    periodic write DQ training option.
  - rockchip: Allow SOCs with undefined wfe check bits, Streamline and complete
    UARTn_BASE macros, drop rockchip-specific imported linker symbols for bl31,
    Disable binary generation for all SOCs, Allow console device to be set by
    DTB, Use new bl31_params_parse functions
  - rpi/rpi3: Move shared rpi3 files into common directory
  - socionext/uniphier: Set CONSOLE_FLAG_TRANSLATE_CRLF and clean up console
    driver
  - socionext/uniphier: Replace DIV_ROUND_UP() with div_round_up() from
    utils_def.h
  - st/stm32mp: Split stm32mp_io_setup function, move
    stm32_get_gpio_bank_clock() to private file, correctly handle Clock
    Spreading Generator, move oscillator functions to generic file, realign
    device tree files with internal devs, enable RTCAPB clock for dual-core
    chips, use a common function to check spinlock is available, move
    check_header() to common code
  - ti/k3: Enable SEPARATE_CODE_AND_RODATA by default, Remove shared RAM space,
    Drop \_ADDRESS from K3_USART_BASE to match other defines, Remove MSMC port
    definitions, Allow USE_COHERENT_MEM for K3, Set L2 latency on A72 cores
- PSCI
  - PSCI: Lookup list of parent nodes to lock only once
- Secure Partition Manager (SPM): SPCI Prototype
  - Fix service UUID lookup
  - Adjust size of virtual address space per partition
  - Refactor xlat context creation
  - Move shim layer to TTBR1_EL1
  - Ignore empty regions in resource description
- Security
  - Refactor SPSR initialisation code
  - SMMUv3: Abort DMA transactions
    - For security DMA should be blocked at the SMMU by default unless
      explicitly enabled for a device. SMMU is disabled after reset with all
      streams bypassing the SMMU, and abortion of all incoming transactions
      implements a default deny policy on reset.
    - Moves `bl1_platform_setup()` function from arm_bl1_setup.c to FVP
      platforms' fvp_bl1_setup.c and fvp_ve_bl1_setup.c files.
- Tools
  - cert_create: Remove RSA PKCS#1 v1.5 support

### Resolved Issues

- Architecture
  - Fix the CAS spinlock implementation by adding a missing DSB in
    `spin_unlock()`
  - AArch64: Fix SCTLR bit definitions
    - Removes incorrect `SCTLR_V_BIT` definition and adds definitions for
      ARMv8.3-Pauth `EnIB`, `EnDA` and `EnDB` bits.
  - Fix restoration of PAuth context
    - Replace call to `pauth_context_save()` with `pauth_context_restore()` in
      case of unknown SMC call.
- BL-Specific Issues
  - Fix BL31 crash reporting on AArch64 only platforms
- Build System
  - Remove several warnings reported with W=2 and W=1
- Code Quality Issues
  - SCTLR and ACTLR are 32-bit for AArch32 and 64-bit for AArch64
  - Unify type of "cpu_idx" across PSCI module.
  - Assert if power level value greater then PSCI_INVALID_PWR_LVL
  - Unsigned long should not be used as per coding guidelines
  - Reduce the number of memory leaks in cert_create
  - Fix type of cot_desc_ptr
  - Use explicit-width data types in AAPCS parameter structs
  - Add python configuration for editorconfig
  - BL1: Fix type consistency
  - Enable -Wshift-overflow=2 to check for undefined shift behavior
  - Updated upstream platforms to not rely on undefined overflow behaviour
- Coverity Quality Issues
  - Remove GGC ignore -Warray-bounds
  - Fix Coverity #261967, Infinite loop
  - Fix Coverity #343017, Missing unlock
  - Fix Coverity #343008, Side affect in assertion
  - Fix Coverity #342970, Uninitialized scalar variable
- CPU Support
  - cortex-a12: Fix MIDR mask
- Drivers
  - console: Remove Arm console unregister on suspend
  - gicv3: Fix support for full SPI range
  - scmi: Fix wrong payload length
- Library Code
  - libc: Fix sparse warning for \_\_assert()
  - libc: Fix memchr implementation
- Platforms
  - rpi: rpi3: Fix compilation error when stack protector is enabled
  - socionext/uniphier: Fix compilation fail for SPM support build config
  - st/stm32mp1: Fix TZC400 configuration against non-secure DDR
  - ti/k3: common: Fix RO data area size calculation
- Security
  - AArch32: Disable Secure Cycle Counter
    - Changes the implementation for disabling Secure Cycle Counter. For ARMv8.5
      the counter gets disabled by setting `SDCR.SCCD` bit on CPU cold/warm
      boot. For the earlier architectures PMCR register is saved/restored on
      secure world entry/exit from/to Non-secure state, and cycle counting gets
      disabled by setting PMCR.DP bit.
  - AArch64: Disable Secure Cycle Counter
    - For ARMv8.5 the counter gets disabled by setting `MDCR_El3.SCCD` bit on
      CPU cold/warm boot. For the earlier architectures PMCR_EL0 register is
      saved/restored on secure world entry/exit from/to Non-secure state, and
      cycle counting gets disabled by setting PMCR_EL0.DP bit.

### Deprecations

- Common Code
  - Remove MULTI_CONSOLE_API flag and references to it
  - Remove deprecated `plat_crash_console_*`
  - Remove deprecated interfaces `get_afflvl_shift`, `mpidr_mask_lower_afflvls`,
    `eret`
  - AARCH32/AARCH64 macros are now deprecated in favor of `__aarch64__`
  - `__ASSEMBLY__` macro is now deprecated in favor of `__ASSEMBLER__`
- Drivers
  - console: Removed legacy console API
  - console: Remove deprecated finish_console_register
  - tzc: Remove deprecated types `tzc_action_t` and `tzc_region_attributes_t`
- Secure Partition Manager (SPM):
  - Prototype SPCI-based SPM (services/std_svc/spm) will be replaced with
    alternative methods of secure partitioning support.

### Known Issues

- Build System Issues
  - dtb: DTB creation not supported when building on a Windows host.

    This step in the build process is skipped when running on a Windows host. A
    known issue from the 1.6 release.
- Platform Issues
  - arm/juno: System suspend from Linux does not function as documented in the
    user guide

    Following the instructions provided in the user guide document does not
    result in the platform entering system suspend state as expected. A message
    relating to the hdlcd driver failing to suspend will be emitted on the Linux
    terminal.

  - mediatek/mt6795: This platform does not build in this release

## [2.1.0](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/refs/tags/v2.0..refs/tags/v2.1) (2019-03-29)

### New Features

- Architecture

  - Support for ARMv8.3 pointer authentication in the normal and secure worlds

    The use of pointer authentication in the normal world is enabled whenever
    architectural support is available, without the need for additional build
    flags.

    Use of pointer authentication in the secure world remains an experimental
    configuration at this time. Using both the `ENABLE_PAUTH` and
    `CTX_INCLUDE_PAUTH_REGS` build flags, pointer authentication can be enabled
    in EL3 and S-EL1/0.

    See the {ref}`Firmware Design` document for additional details on the use of
    pointer authentication.

  - Enable Data Independent Timing (DIT) in EL3, where supported

- Build System

  - Support for BL-specific build flags

  - Support setting compiler target architecture based on `ARM_ARCH_MINOR` build
    option.

  - New `RECLAIM_INIT_CODE` build flag:

    A significant amount of the code used for the initialization of BL31 is not
    needed again after boot time. In order to reduce the runtime memory
    footprint, the memory used for this code can be reclaimed after
    initialization.

    Certain boot-time functions were marked with the `__init` attribute to
    enable this reclamation.

- CPU Support

  - cortex-a76: Workaround for erratum 1073348
  - cortex-a76: Workaround for erratum 1220197
  - cortex-a76: Workaround for erratum 1130799
  - cortex-a75: Workaround for erratum 790748
  - cortex-a75: Workaround for erratum 764081
  - cortex-a73: Workaround for erratum 852427
  - cortex-a73: Workaround for erratum 855423
  - cortex-a57: Workaround for erratum 817169
  - cortex-a57: Workaround for erratum 814670
  - cortex-a55: Workaround for erratum 903758
  - cortex-a55: Workaround for erratum 846532
  - cortex-a55: Workaround for erratum 798797
  - cortex-a55: Workaround for erratum 778703
  - cortex-a55: Workaround for erratum 768277
  - cortex-a53: Workaround for erratum 819472
  - cortex-a53: Workaround for erratum 824069
  - cortex-a53: Workaround for erratum 827319
  - cortex-a17: Workaround for erratum 852423
  - cortex-a17: Workaround for erratum 852421
  - cortex-a15: Workaround for erratum 816470
  - cortex-a15: Workaround for erratum 827671

- Documentation

  - Exception Handling Framework documentation
  - Library at ROM (romlib) documentation
  - RAS framework documentation
  - Coding Guidelines document

- Drivers

  - ccn: Add API for setting and reading node registers

    - Adds `ccn_read_node_reg` function
    - Adds `ccn_write_node_reg` function

  - partition: Support MBR partition entries

  - scmi: Add `plat_css_get_scmi_info` function

    Adds a new API `plat_css_get_scmi_info` which lets the platform register a
    platform-specific instance of `scmi_channel_plat_info_t` and remove the
    default values

  - tzc380: Add TZC-380 TrustZone Controller driver

  - tzc-dmc620: Add driver to manage the TrustZone Controller within the DMC-620
    Dynamic Memory Controller

- Library at ROM (romlib)

  - Add platform-specific jump table list

  - Allow patching of romlib functions

    This change allows patching of functions in the romlib. This can be done by
    adding "patch" at the end of the jump table entry for the function that
    needs to be patched in the file jmptbl.i.

- Library Code

  - Support non-LPAE-enabled MMU tables in AArch32
  - mmio: Add `mmio_clrsetbits_16` function
    - 16-bit variant of `mmio_clrsetbits`
  - object_pool: Add Object Pool Allocator
    - Manages object allocation using a fixed-size static array
    - Adds `pool_alloc` and `pool_alloc_n` functions
    - Does not provide any functions to free allocated objects (by design)
  - libc: Added `strlcpy` function
  - libc: Import `strrchr` function from FreeBSD
  - xlat_tables: Add support for ARMv8.4-TTST
  - xlat_tables: Support mapping regions without an explicitly specified VA

- Math

  - Added softudiv macro to support software division

- Memory Partitioning And Monitoring (MPAM)

  - Enabled MPAM EL2 traps (`MPAMHCR_EL2` and `MPAM_EL2`)

- Platforms

  - amlogic: Add support for Meson S905 (GXBB)

  - arm/fvp_ve: Add support for FVP Versatile Express platform

  - arm/n1sdp: Add support for Neoverse N1 System Development platform

  - arm/rde1edge: Add support for Neoverse E1 platform

  - arm/rdn1edge: Add support for Neoverse N1 platform

  - arm: Add support for booting directly to Linux without an intermediate
    loader (AArch32)

  - arm/juno: Enable new CPU errata workarounds for A53 and A57

  - arm/juno: Add romlib support

    Building a combined BL1 and ROMLIB binary file with the correct page
    alignment is now supported on the Juno platform. When `USE_ROMLIB` is set
    for Juno, it generates the combined file `bl1_romlib.bin` which needs to be
    used instead of bl1.bin.

  - intel/stratix: Add support for Intel Stratix 10 SoC FPGA platform

  - marvell: Add support for Armada-37xx SoC platform

  - nxp: Add support for i.MX8M and i.MX7 Warp7 platforms

  - renesas: Add support for R-Car Gen3 platform

  - xilinx: Add support for Versal ACAP platforms

- Position-Independent Executable (PIE)

  PIE support has initially been added to BL31. The `ENABLE_PIE` build flag is
  used to enable or disable this functionality as required.

- Secure Partition Manager

  - New SPM implementation based on SPCI Alpha 1 draft specification

    A new version of SPM has been implemented, based on the SPCI (Secure
    Partition Client Interface) and SPRT (Secure Partition Runtime) draft
    specifications.

    The new implementation is a prototype that is expected to undergo intensive
    rework as the specifications change. It has basic support for multiple
    Secure Partitions and Resource Descriptions.

    The older version of SPM, based on MM (ARM Management Mode Interface
    Specification), is still present in the codebase. A new build flag, `SPM_MM`
    has been added to allow selection of the desired implementation. This flag
    defaults to 1, selecting the MM-based implementation.

- Security

  - Spectre Variant-1 mitigations (`CVE-2017-5753`)

  - Use Speculation Store Bypass Safe (SSBS) functionality where available

    Provides mitigation against `CVE-2018-19440` (Not saving x0 to x3 registers
    can leak information from one Normal World SMC client to another)

### Changed

- Build System

  - Warning levels are now selectable with `W=<1,2,3>`
  - Removed unneeded include paths in PLAT_INCLUDES
  - "Warnings as errors" (Werror) can be disabled using `E=0`
  - Support totally quiet output with `-s` flag
  - Support passing options to checkpatch using `CHECKPATCH_OPTS=<opts>`
  - Invoke host compiler with `HOSTCC / HOSTCCFLAGS` instead of `CC / CFLAGS`
  - Make device tree pre-processing similar to U-boot/Linux by:
    - Creating separate `CPPFLAGS` for DT preprocessing so that compiler options
      specific to it can be accommodated.
    - Replacing `CPP` with `PP` for DT pre-processing

- CPU Support

  - Errata report function definition is now mandatory for CPU support files

    CPU operation files must now define a `<name>_errata_report` function to
    print errata status. This is no longer a weak reference.

- Documentation

  - Migrated some content from GitHub wiki to `docs/` directory
  - Security advisories now have CVE links
  - Updated copyright guidelines

- Drivers

  - console: The `MULTI_CONSOLE_API` framework has been rewritten in C

  - console: Ported multi-console driver to AArch32

  - gic: Remove 'lowest priority' constants

    Removed `GIC_LOWEST_SEC_PRIORITY` and `GIC_LOWEST_NS_PRIORITY`. Platforms
    should define these if required, or instead determine the correct priority
    values at runtime.

  - delay_timer: Check that the Generic Timer extension is present

  - mmc: Increase command reply timeout to 10 milliseconds

  - mmc: Poll eMMC device status to ensure `EXT_CSD` command completion

  - mmc: Correctly check return code from `mmc_fill_device_info`

- External Libraries

  - libfdt: Upgraded from 1.4.2 to 1.4.6-9

  >

  - mbed TLS: Upgraded from 2.12 to 2.16

  >

  This change incorporates fixes for security issues that should be reviewed to
  determine if they are relevant for software implementations using Trusted
  Firmware-A. See the [mbed TLS releases] page for details on changes from the
  2.12 to the 2.16 release.

- Library Code

  - compiler-rt: Updated `lshrdi3.c` and `int_lib.h` with changes from LLVM
    master branch (r345645)
  - cpu: Updated macro that checks need for `CVE-2017-5715` mitigation
  - libc: Made setjmp and longjmp C standard compliant
  - libc: Allowed overriding the default libc (use `OVERRIDE_LIBC`)
  - libc: Moved setjmp and longjmp to the `libc/` directory

- Platforms

  - Removed Mbed TLS dependency from plat_bl_common.c

  - arm: Removed unused `ARM_MAP_BL_ROMLIB` macro

  - arm: Removed `ARM_BOARD_OPTIMISE_MEM` feature and build flag

  - arm: Moved several components into `drivers/` directory

    This affects the SDS, SCP, SCPI, MHU and SCMI components

  - arm/juno: Increased maximum BL2 image size to `0xF000`

    This change was required to accommodate a larger `libfdt` library

- SCMI

  - Optimized bakery locks when hardware-assisted coherency is enabled using the
    `HW_ASSISTED_COHERENCY` build flag

- SDEI

  - Added support for unconditionally resuming secure world execution after {{
    SDEI }} event processing completes

    {{ SDEI }} interrupts, although targeting EL3, occur on behalf of the
    non-secure world, and may have higher priority than secure world interrupts.
    Therefore they might preempt secure execution and yield execution to the
    non-secure {{ SDEI }} handler. Upon completion of {{ SDEI }} event handling,
    resume secure execution if it was preempted.

- Translation Tables (XLAT)

  - Dynamically detect need for `Common not Private (TTBRn_ELx.CnP)` bit

    Properly handle the case where `ARMv8.2-TTCNP` is implemented in a CPU that
    does not implement all mandatory v8.2 features (and so must claim to
    implement a lower architecture version).

### Resolved Issues

- Architecture
  - Incorrect check for SSBS feature detection
  - Unintentional register clobber in AArch32 reset_handler function
- Build System
  - Dependency issue during DTB image build
  - Incorrect variable expansion in Arm platform makefiles
  - Building on Windows with verbose mode (`V=1`) enabled is broken
  - AArch32 compilation flags is missing `$(march32-directive)`
- BL-Specific Issues
  - bl2: `uintptr_t is not defined` error when `BL2_IN_XIP_MEM` is defined
  - bl2: Missing prototype warning in `bl2_arch_setup`
  - bl31: Omission of Global Offset Table (GOT) section
- Code Quality Issues
  - Multiple MISRA compliance issues
  - Potential NULL pointer dereference (Coverity-detected)
- Drivers
  - mmc: Local declaration of `scr` variable causes a cache issue when
    invalidating after the read DMA transfer completes
  - mmc: `ACMD41` does not send voltage information during initialization,
    resulting in the command being treated as a query. This prevents the command
    from initializing the controller.
  - mmc: When checking device state using `mmc_device_state()` there are no
    retries attempted in the event of an error
  - ccn: Incorrect Region ID calculation for RN-I nodes
  - console: `Fix MULTI_CONSOLE_API` when used as a crash console
  - partition: Improper NULL checking in gpt.c
  - partition: Compilation failure in `VERBOSE` mode (`V=1`)
- Library Code
  - common: Incorrect check for Address Authentication support

  - xlat: Fix XLAT_V1 / XLAT_V2 incompatibility

    The file `arm_xlat_tables.h` has been renamed to `xlat_tables_compat.h` and
    has been moved to a common folder. This header can be used to guarantee
    compatibility, as it includes the correct header based on
    `XLAT_TABLES_LIB_V2`.

  - xlat: armclang unused-function warning on `xlat_clean_dcache_range`

  - xlat: Invalid `mm_cursor` checks in `mmap_add` and `mmap_add_ctx`

  - sdei: Missing `context.h` header
- Platforms
  - common: Missing prototype warning for `plat_log_get_prefix`

  - arm: Insufficient maximum BL33 image size

  - arm: Potential memory corruption during BL2-BL31 transition

    On Arm platforms, the BL2 memory can be overlaid by BL31/BL32. The memory
    descriptors describing the list of executable images are created in BL2 R/W
    memory, which could be possibly corrupted later on by BL31/BL32 due to
    overlay. This patch creates a reserved location in SRAM for these
    descriptors and are copied over by BL2 before handing over to next BL image.

  - juno: Invalid behaviour when `CSS_USE_SCMI_SDS_DRIVER` is not set

    In `juno_pm.c` the `css_scmi_override_pm_ops` function was used regardless
    of whether the build flag was set. The original behaviour has been restored
    in the case where the build flag is not set.
- Tools
  - fiptool: Incorrect UUID parsing of blob parameters
  - doimage: Incorrect object rules in Makefile

### Deprecations

- Common Code
  - `plat_crash_console_init` function
  - `plat_crash_console_putc` function
  - `plat_crash_console_flush` function
  - `finish_console_register` macro
- AArch64-specific Code
  - helpers: `get_afflvl_shift`
  - helpers: `mpidr_mask_lower_afflvls`
  - helpers: `eret`
- Secure Partition Manager (SPM)
  - Boot-info structure

### Known Issues

- Build System Issues
  - dtb: DTB creation not supported when building on a Windows host.

    This step in the build process is skipped when running on a Windows host. A
    known issue from the 1.6 release.
- Platform Issues
  - arm/juno: System suspend from Linux does not function as documented in the
    user guide

    Following the instructions provided in the user guide document does not
    result in the platform entering system suspend state as expected. A message
    relating to the hdlcd driver failing to suspend will be emitted on the Linux
    terminal.

  - arm/juno: The firmware update use-cases do not work with motherboard
    firmware version \< v1.5.0 (the reset reason is not preserved). The Linaro
    18.04 release has MB v1.4.9. The MB v1.5.0 is available in Linaro 18.10
    release.

  - mediatek/mt6795: This platform does not build in this release

## [2.0.0](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/refs/tags/v1.6..refs/tags/v2.0) (2018-10-02)

### New Features

- Removal of a number of deprecated APIs

  - A new Platform Compatibility Policy document has been created which
    references a wiki page that maintains a listing of deprecated interfaces and
    the release after which they will be removed.
  - All deprecated interfaces except the MULTI_CONSOLE_API have been removed
    from the code base.
  - Various Arm and partner platforms have been updated to remove the use of
    removed APIs in this release.
  - This release is otherwise unchanged from 1.6 release

### Issues resolved since last release

- No issues known at 1.6 release resolved in 2.0 release

### Known Issues

- DTB creation not supported when building on a Windows host. This step in the
  build process is skipped when running on a Windows host. Known issue from 1.6
  version.
- As a result of removal of deprecated interfaces the Nvidia Tegra, Marvell
  Armada 8K and MediaTek MT6795 platforms do not build in this release. Also
  MediaTek MT8173, NXP QorIQ LS1043A, NXP i.MX8QX, NXP i.MX8QMa, Rockchip
  RK3328, Rockchip RK3368 and Rockchip RK3399 platforms have not been confirmed
  to be working after the removal of the deprecated interfaces although they do
  build.

## [1.6.0](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/refs/tags/v1.5..refs/tags/v1.6) (2018-09-21)

### New Features

- Addressing Speculation Security Vulnerabilities

  - Implement static workaround for CVE-2018-3639 for AArch32 and AArch64
  - Add support for dynamic mitigation for CVE-2018-3639
  - Implement dynamic mitigation for CVE-2018-3639 on Cortex-A76
  - Ensure {{ SDEI }} handler executes with CVE-2018-3639 mitigation enabled

- Introduce RAS handling on AArch64

  - Some RAS extensions are mandatory for Armv8.2 CPUs, with others mandatory
    for Armv8.4 CPUs however, all extensions are also optional extensions to the
    base Armv8.0 architecture.
  - The Armv8 RAS Extensions introduced Standard Error Records which are a set
    of standard registers to configure RAS node policy and allow RAS Nodes to
    record and expose error information for error handling agents.
  - Capabilities are provided to support RAS Node enumeration and iteration
    along with individual interrupt registrations and fault injections support.
  - Introduce handlers for Uncontainable errors, Double Faults and EL3 External
    Aborts

- Enable Memory Partitioning And Monitoring (MPAM) for lower EL's

  - Memory Partitioning And Monitoring is an Armv8.4 feature that enables
    various memory system components and resources to define partitions.
    Software running at various ELs can then assign themselves to the desired
    partition to control their performance aspects.
  - When ENABLE_MPAM_FOR_LOWER_ELS is set to 1, EL3 allows lower ELs to access
    their own MPAM registers without trapping to EL3. This patch however,
    doesn't make use of partitioning in EL3; platform initialisation code should
    configure and use partitions in EL3 if required.

- Introduce ROM Lib Feature

  - Support combining several libraries into a self-called "romlib" image, that
    may be shared across images to reduce memory footprint. The romlib image is
    stored in ROM but is accessed through a jump-table that may be stored in
    read-write memory, allowing for the library code to be patched.

- Introduce Backtrace Feature

  - This function displays the backtrace, the current EL and security state to
    allow a post-processing tool to choose the right binary to interpret the
    dump.
  - Print backtrace in assert() and panic() to the console.

- Code hygiene changes and alignment with MISRA C-2012 guideline with fixes
  addressing issues complying to the following rules:

  - MISRA rules 4.9, 5.1, 5.3, 5.7, 8.2-8.5, 8.8, 8.13, 9.3, 10.1, 10.3-10.4,
    10.8, 11.3, 11.6, 12.1, 14.4, 15.7, 16.1-16.7, 17.7-17.8, 20.7, 20.10,
    20.12, 21.1, 21.15, 22.7
  - Clean up the usage of void pointers to access symbols
  - Increase usage of static qualifier to locally used functions and data
  - Migrated to use of u_register_t for register read/write to better match
    AArch32 and AArch64 type sizes
  - Use int-ll64 for both AArch32 and AArch64 to assist in consistent format
    strings between architectures
  - Clean up TF-A libc by removing non arm copyrighted implementations and
    replacing them with modified FreeBSD and SCC implementations

- Various changes to support Clang linker and assembler

  - The clang assembler/preprocessor is used when Clang is selected. However,
    the clang linker is not used because it is unable to link TF-A objects due
    to immaturity of clang linker functionality at this time.

- Refactor support APIs into Libraries

  - Evolve libfdt, mbed TLS library and standard C library sources as proper
    libraries that TF-A may be linked against.

- CPU Enhancements

  - Add CPU support for Cortex-Ares and Cortex-A76
  - Add AMU support for Cortex-Ares
  - Add initial CPU support for Cortex-Deimos
  - Add initial CPU support for Cortex-Helios
  - Implement dynamic mitigation for CVE-2018-3639 on Cortex-A76
  - Implement Cortex-Ares erratum 1043202 workaround
  - Implement DSU erratum 936184 workaround
  - Check presence of fix for errata 843419 in Cortex-A53
  - Check presence of fix for errata 835769 in Cortex-A53

- Translation Tables Enhancements

  - The xlat v2 library has been refactored in order to be reused by different
    TF components at different EL's including the addition of EL2. Some
    refactoring to make the code more generic and less specific to TF, in order
    to reuse the library outside of this project.

- SPM Enhancements

  - General cleanups and refactoring to pave the way to multiple partitions
    support

- SDEI Enhancements

  - Allow platforms to define explicit events
  - Determine client EL from NS context's SCR_EL3
  - Make dispatches synchronous
  - Introduce jump primitives for BL31
  - Mask events after CPU wakeup in {{ SDEI }} dispatcher to conform to the
    specification

- Misc TF-A Core Common Code Enhancements

  - Add support for eXecute In Place (XIP) memory in BL2
  - Add support for the SMC Calling Convention 2.0
  - Introduce External Abort handling on AArch64 External Abort routed to EL3
    was reported as an unhandled exception and caused a panic. This change
    enables Trusted Firmware-A to handle External Aborts routed to EL3.
  - Save value of ACTLR_EL1 implementation-defined register in the CPU context
    structure rather than forcing it to 0.
  - Introduce ARM_LINUX_KERNEL_AS_BL33 build option, which allows BL31 to
    directly jump to a Linux kernel. This makes for a quicker and simpler boot
    flow, which might be useful in some test environments.
  - Add dynamic configurations for BL31, BL32 and BL33 enabling support for
    Chain of Trust (COT).
  - Make TF UUID RFC 4122 compliant

- New Platform Support

  - Arm SGI-575
  - Arm SGM-775
  - Allwinner sun50i_64
  - Allwinner sun50i_h6
  - NXP QorIQ LS1043A
  - NXP i.MX8QX
  - NXP i.MX8QM
  - NXP i.MX7Solo WaRP7
  - TI K3
  - Socionext Synquacer SC2A11
  - Marvell Armada 8K
  - STMicroelectronics STM32MP1

- Misc Generic Platform Common Code Enhancements

  - Add MMC framework that supports both eMMC and SD card devices

- Misc Arm Platform Common Code Enhancements

  - Demonstrate PSCI MEM_PROTECT from el3_runtime
  - Provide RAS support
  - Migrate AArch64 port to the multi console driver. The old API is deprecated
    and will eventually be removed.
  - Move BL31 below BL2 to enable BL2 overlay resulting in changes in the layout
    of BL images in memory to enable more efficient use of available space.
  - Add cpp build processing for dtb that allows processing device tree with
    external includes.
  - Extend FIP io driver to support multiple FIP devices
  - Add support for SCMI AP core configuration protocol v1.0
  - Use SCMI AP core protocol to set the warm boot entrypoint
  - Add support to Mbed TLS drivers for shared heap among different BL images to
    help optimise memory usage
  - Enable non-secure access to UART1 through a build option to support a serial
    debug port for debugger connection

- Enhancements for Arm Juno Platform

  - Add support for TrustZone Media Protection 1 (TZMP1)

- Enhancements for Arm FVP Platform

  - Dynamic_config: remove the FVP dtb files
  - Set DYNAMIC_WORKAROUND_CVE_2018_3639=1 on FVP by default
  - Set the ability to dynamically disable Trusted Boot Board authentication to
    be off by default with DYN_DISABLE_AUTH
  - Add librom enhancement support in FVP
  - Support shared Mbed TLS heap between BL1 and BL2 that allow a reduction in
    BL2 size for FVP

- Enhancements for Arm SGI/SGM Platform

  - Enable ARM_PLAT_MT flag for SGI-575
  - Add dts files to enable support for dynamic config
  - Add RAS support
  - Support shared Mbed TLS heap for SGI and SGM between BL1 and BL2

- Enhancements for Non Arm Platforms

  - Raspberry Pi Platform
  - Hikey Platforms
  - Xilinx Platforms
  - QEMU Platform
  - Rockchip rk3399 Platform
  - TI Platforms
  - Socionext Platforms
  - Allwinner Platforms
  - NXP Platforms
  - NVIDIA Tegra Platform
  - Marvell Platforms
  - STMicroelectronics STM32MP1 Platform

### Issues resolved since last release

- No issues known at 1.5 release resolved in 1.6 release

### Known Issues

- DTB creation not supported when building on a Windows host. This step in the
  build process is skipped when running on a Windows host. Known issue from 1.5
  version.

## [1.5.0](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/refs/tags/v1.4..refs/tags/v1.5) (2018-03-20)

### New features

- Added new firmware support to enable RAS (Reliability, Availability, and
  Serviceability) functionality.

  - Secure Partition Manager (SPM): A Secure Partition is a software execution
    environment instantiated in S-EL0 that can be used to implement simple
    management and security services. The SPM is the firmware component that is
    responsible for managing a Secure Partition.

  - SDEI dispatcher: Support for interrupt-based {{ SDEI }} events and all
    interfaces as defined by the {{ SDEI }} specification v1.0, see
    [SDEI Specification]

  - Exception Handling Framework (EHF): Framework that allows dispatching of EL3
    interrupts to their registered handlers which are registered based on their
    priorities. Facilitates firmware-first error handling policy where
    asynchronous exceptions may be routed to EL3.

    Integrated the TSPD with EHF.

- Updated PSCI support:

  - Implemented PSCI v1.1 optional features `MEM_PROTECT` and `SYSTEM_RESET2`.
    The supported PSCI version was updated to v1.1.

  - Improved PSCI STAT timestamp collection, including moving accounting for
    retention states to be inside the locks and fixing handling of wrap-around
    when calculating residency in AArch32 execution state.

  - Added optional handler for early suspend that executes when suspending to a
    power-down state and with data caches enabled.

    This may provide a performance improvement on platforms where it is safe to
    perform some or all of the platform actions from `pwr_domain_suspend` with
    the data caches enabled.

- Enabled build option, BL2_AT_EL3, for BL2 to allow execution at EL3 without
  any dependency on TF BL1.

  This allows platforms which already have a non-TF Boot ROM to directly load
  and execute BL2 and subsequent BL stages without need for BL1. This was not
  previously possible because BL2 executes at S-EL1 and cannot jump straight to
  EL3.

- Implemented support for SMCCC v1.1, including `SMCCC_VERSION` and
  `SMCCC_ARCH_FEATURES`.

  Additionally, added support for `SMCCC_VERSION` in PSCI features to enable
  discovery of the SMCCC version via PSCI feature call.

- Added Dynamic Configuration framework which enables each of the boot loader
  stages to be dynamically configured at runtime if required by the platform.
  The boot loader stage may optionally specify a firmware configuration file
  and/or hardware configuration file that can then be shared with the next boot
  loader stage.

  Introduced a new BL handover interface that essentially allows passing of 4
  arguments between the different BL stages.

  Updated cert_create and fip_tool to support the dynamic configuration files.
  The COT also updated to support these new files.

- Code hygiene changes and alignment with MISRA guideline:

  - Fix use of undefined macros.
  - Achieved compliance with Mandatory MISRA coding rules.
  - Achieved compliance for following Required MISRA rules for the default build
    configurations on FVP and Juno platforms : 7.3, 8.3, 8.4, 8.5 and 8.8.

- Added support for Armv8.2-A architectural features:

  - Updated translation table set-up to set the CnP (Common not Private) bit for
    secure page tables so that multiple PEs in the same Inner Shareable domain
    can use the same translation table entries for a given stage of translation
    in a particular translation regime.
  - Extended the supported values of ID_AA64MMFR0_EL1.PARange to include the
    52-bit Physical Address range.
  - Added support for the Scalable Vector Extension to allow Normal world
    software to access SVE functionality but disable access to SVE, SIMD and
    floating point functionality from the Secure world in order to prevent
    corruption of the Z-registers.

- Added support for Armv8.4-A architectural feature Activity Monitor Unit (AMU)

  extensions.

  In addition to the v8.4 architectural extension, AMU support on Cortex-A75 was
  implemented.

- Enhanced OP-TEE support to enable use of pageable OP-TEE image. The Arm
  standard platforms are updated to load up to 3 images for OP-TEE; header,
  pager image and paged image.

  The chain of trust is extended to support the additional images.

- Enhancements to the translation table library:

  - Introduced APIs to get and set the memory attributes of a region.
  - Added support to manage both privilege levels in translation regimes that
    describe translations for 2 Exception levels, specifically the EL1&0
    translation regime, and extended the memory map region attributes to include
    specifying Non-privileged access.
  - Added support to specify the granularity of the mappings of each region, for
    instance a 2MB region can be specified to be mapped with 4KB page tables
    instead of a 2MB block.
  - Disabled the higher VA range to avoid unpredictable behaviour if there is an
    attempt to access addresses in the higher VA range.
  - Added helpers for Device and Normal memory MAIR encodings that align with
    the Arm Architecture Reference Manual for Armv8-A (Arm DDI0487B.b).
  - Code hygiene including fixing type length and signedness of constants,
    refactoring of function to enable the MMU, removing all instances where the
    virtual address space is hardcoded and added comments that document
    alignment needed between memory attributes and attributes specified in
    TCR_ELx.

- Updated GIC support:

  - Introduce new APIs for GICv2 and GICv3 that provide the capability to
    specify interrupt properties rather than list of interrupt numbers alone.
    The Arm platforms and other upstream platforms are migrated to use interrupt
    properties.

  - Added helpers to save / restore the GICv3 context, specifically the
    Distributor and Redistributor contexts and architectural parts of the ITS
    power management. The Distributor and Redistributor helpers also support the
    implementation-defined part of GIC-500 and GIC-600.

    Updated the Arm FVP platform to save / restore the GICv3 context on system
    suspend / resume as an example of how to use the helpers.

    Introduced a new TZC secured DDR carve-out for use by Arm platforms for
    storing EL3 runtime data such as the GICv3 register context.

- Added support for Armv7-A architecture via build option ARM_ARCH_MAJOR=7. This
  includes following features:

  - Updates GICv2 driver to manage GICv1 with security extensions.
  - Software implementation for 32bit division.
  - Enabled use of generic timer for platforms that do not set
    ARM_CORTEX_Ax=yes.
  - Support for Armv7-A Virtualization extensions \[DDI0406C_C\].
  - Support for both Armv7-A platforms that only have 32-bit addressing and
    Armv7-A platforms that support large page addressing.
  - Included support for following Armv7 CPUs: Cortex-A12, Cortex-A17,
    Cortex-A7, Cortex-A5, Cortex-A9, Cortex-A15.
  - Added support in QEMU for Armv7-A/Cortex-A15.

- Enhancements to Firmware Update feature:

  - Updated the FWU documentation to describe the additional images needed for
    Firmware update, and how they are used for both the Juno platform and the
    Arm FVP platforms.

- Enhancements to Trusted Board Boot feature:

  - Added support to cert_create tool for RSA PKCS1# v1.5 and SHA384, SHA512 and
    SHA256.
  - For Arm platforms added support to use ECDSA keys.
  - Enhanced the mbed TLS wrapper layer to include support for both RSA and
    ECDSA to enable runtime selection between RSA and ECDSA keys.

- Added support for secure interrupt handling in AArch32 sp_min, hardcoded to
  only handle FIQs.

- Added support to allow a platform to load images from multiple boot sources,
  for example from a second flash drive.

- Added a logging framework that allows platforms to reduce the logging level at
  runtime and additionally the prefix string can be defined by the platform.

- Further improvements to register initialisation:

  - Control register PMCR_EL0 / PMCR is set to prohibit cycle counting in the
    secure world. This register is added to the list of registers that are saved
    and restored during world switch.
  - When EL3 is running in AArch32 execution state, the Non-secure version of
    SCTLR is explicitly initialised during the warmboot flow rather than relying
    on the hardware to set the correct reset values.

- Enhanced support for Arm platforms:

  - Introduced driver for Shared-Data-Structure (SDS) framework which is used
    for communication between SCP and the AP CPU, replacing Boot-Over_MHU (BOM)
    protocol.

    The Juno platform is migrated to use SDS with the SCMI support added in v1.3
    and is set as default.

    The driver can be found in the plat/arm/css/drivers folder.

  - Improved memory usage by only mapping TSP memory region when the TSPD has
    been included in the build. This reduces the memory footprint and avoids
    unnecessary memory being mapped.

  - Updated support for multi-threading CPUs for FVP platforms - always check
    the MT field in MPDIR and access the bit fields accordingly.

  - Support building for platforms that model DynamIQ configuration by
    implementing all CPUs in a single cluster.

  - Improved nor flash driver, for instance clearing status registers before
    sending commands. Driver can be found plat/arm/board/common folder.

- Enhancements to QEMU platform:

  - Added support for TBB.
  - Added support for using OP-TEE pageable image.
  - Added support for LOAD_IMAGE_V2.
  - Migrated to use translation table library v2 by default.
  - Added support for SEPARATE_CODE_AND_RODATA.

- Applied workarounds CVE-2017-5715 on Arm Cortex-A57, -A72, -A73 and -A75, and
  for Armv7-A CPUs Cortex-A9, -A15 and -A17.

- Applied errata workaround for Arm Cortex-A57: 859972.

- Applied errata workaround for Arm Cortex-A72: 859971.

- Added support for Poplar 96Board platform.

- Added support for Raspberry Pi 3 platform.

- Added Call Frame Information (CFI) assembler directives to the vector entries
  which enables debuggers to display the backtrace of functions that triggered a
  synchronous abort.

- Added ability to build dtb.

- Added support for pre-tool (cert_create and fiptool) image processing enabling
  compression of the image files before processing by cert_create and fiptool.

  This can reduce fip size and may also speed up loading of images. The image
  verification will also get faster because certificates are generated based on
  compressed images.

  Imported zlib 1.2.11 to implement gunzip() for data compression.

- Enhancements to fiptool:

  - Enabled the fiptool to be built using Visual Studio.
  - Added padding bytes at the end of the last image in the fip to be facilitate
    transfer by DMA.

### Issues resolved since last release

- TF-A can be built with optimisations disabled (-O0).
- Memory layout updated to enable Trusted Board Boot on Juno platform when
  running TF-A in AArch32 execution mode (resolving [tf-issue#501]).

### Known Issues

- DTB creation not supported when building on a Windows host. This step in the
  build process is skipped when running on a Windows host.

## [1.4.0](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/refs/tags/v1.3..refs/tags/v1.4) (2017-07-07)

### New features

- Enabled support for platforms with hardware assisted coherency.

  A new build option HW_ASSISTED_COHERENCY allows platforms to take advantage of
  the following optimisations:

  - Skip performing cache maintenance during power-up and power-down.
  - Use spin-locks instead of bakery locks.
  - Enable data caches early on warm-booted CPUs.

- Added support for Cortex-A75 and Cortex-A55 processors.

  Both Cortex-A75 and Cortex-A55 processors use the Arm DynamIQ Shared Unit
  (DSU). The power-down and power-up sequences are therefore mostly managed in
  hardware, reducing complexity of the software operations.

- Introduced Arm GIC-600 driver.

  Arm GIC-600 IP complies with Arm GICv3 architecture. For FVP platforms, the
  GIC-600 driver is chosen when FVP_USE_GIC_DRIVER is set to FVP_GIC600.

- Updated GICv3 support:

  - Introduced power management APIs for GICv3 Redistributor. These APIs allow
    platforms to power down the Redistributor during CPU power on/off. Requires
    the GICv3 implementations to have power management operations.

    Implemented the power management APIs for FVP.

  - GIC driver data is flushed by the primary CPU so that secondary CPU do not
    read stale GIC data.

- Added support for Arm System Control and Management Interface v1.0 (SCMI).

  The SCMI driver implements the power domain management and system power
  management protocol of the SCMI specification (Arm DEN 0056ASCMI) for
  communicating with any compliant power controller.

  Support is added for the Juno platform. The driver can be found in the
  plat/arm/css/drivers folder.

- Added support to enable pre-integration of TBB with the Arm TrustZone
  CryptoCell product, to take advantage of its hardware Root of Trust and crypto
  acceleration services.

- Enabled Statistical Profiling Extensions for lower ELs.

  The firmware support is limited to the use of SPE in the Non-secure state and
  accesses to the SPE specific registers from S-EL1 will trap to EL3.

  The SPE are architecturally specified for AArch64 only.

- Code hygiene changes aligned with MISRA guidelines:

  - Fixed signed / unsigned comparison warnings in the translation table
    library.
  - Added U(\_x) macro and together with the existing ULL(\_x) macro fixed some
    of the signed-ness defects flagged by the MISRA scanner.

- Enhancements to Firmware Update feature:

  - The FWU logic now checks for overlapping images to prevent execution of
    unauthenticated arbitrary code.
  - Introduced new FWU_SMC_IMAGE_RESET SMC that changes the image loading state
    machine to go from COPYING, COPIED or AUTHENTICATED states to RESET state.
    Previously, this was only possible when the authentication of an image
    failed or when the execution of the image finished.
  - Fixed integer overflow which addressed TFV-1: Malformed Firmware Update SMC
    can result in copy of unexpectedly large data into secure memory.

- Introduced support for Arm Compiler 6 and LLVM (clang).

  TF-A can now also be built with the Arm Compiler 6 or the clang compilers. The
  assembler and linker must be provided by the GNU toolchain.

  Tested with Arm CC 6.7 and clang 3.9.x and 4.0.x.

- Memory footprint improvements:

  - Introduced `tf_snprintf`, a reduced version of `snprintf` which has support
    for a limited set of formats.

    The mbedtls driver is updated to optionally use `tf_snprintf` instead of
    `snprintf`.

  - The `assert()` is updated to no longer print the function name, and
    additional logging options are supported via an optional platform define
    `PLAT_LOG_LEVEL_ASSERT`, which controls how verbose the assert output is.

- Enhancements to TF-A support when running in AArch32 execution state:

  - Support booting SP_MIN and BL33 in AArch32 execution mode on Juno. Due to
    hardware limitations, BL1 and BL2 boot in AArch64 state and there is
    additional trampoline code to warm reset into SP_MIN in AArch32 execution
    state.
  - Added support for Arm Cortex-A53/57/72 MPCore processors including the
    errata workarounds that are already implemented for AArch64 execution state.
  - For FVP platforms, added AArch32 Trusted Board Boot support, including the
    Firmware Update feature.

- Introduced Arm SiP service for use by Arm standard platforms.

  - Added new Arm SiP Service SMCs to enable the Non-secure world to read PMF
    timestamps.

    Added PMF instrumentation points in TF-A in order to quantify the overall
    time spent in the PSCI software implementation.

  - Added new Arm SiP service SMC to switch execution state.

    This allows the lower exception level to change its execution state from
    AArch64 to AArch32, or vice verse, via a request to EL3.

- Migrated to use SPDX\[0\] license identifiers to make software license
  auditing simpler.

  \:::\{note} Files that have been imported by FreeBSD have not been modified.
  \:::

  \[0\]: <https://spdx.org/>

- Enhancements to the translation table library:

  - Added version 2 of translation table library that allows different
    translation tables to be modified by using different 'contexts'. Version 1
    of the translation table library only allows the current EL's translation
    tables to be modified.

    Version 2 of the translation table also added support for dynamic regions;
    regions that can be added and removed dynamically whilst the MMU is enabled.
    Static regions can only be added or removed before the MMU is enabled.

    The dynamic mapping functionality is enabled or disabled when compiling by
    setting the build option PLAT_XLAT_TABLES_DYNAMIC to 1 or 0. This can be
    done per-image.

  - Added support for translation regimes with two virtual address spaces such
    as the one shared by EL1 and EL0.

    The library does not support initializing translation tables for EL0
    software.

  - Added support to mark the translation tables as non-cacheable using an
    additional build option `XLAT_TABLE_NC`.

- Added support for GCC stack protection. A new build option
  ENABLE_STACK_PROTECTOR was introduced that enables compilation of all BL
  images with one of the GCC -fstack-protector-\* options.

  A new platform function plat_get_stack_protector_canary() was introduced that
  returns a value used to initialize the canary for stack corruption detection.
  For increased effectiveness of protection platforms must provide an
  implementation that returns a random value.

- Enhanced support for Arm platforms:

  - Added support for multi-threading CPUs, indicated by `MT` field in MPDIR. A
    new build flag `ARM_PLAT_MT` is added, and when enabled, the functions
    accessing MPIDR assume that the `MT` bit is set for the platform and access
    the bit fields accordingly.

    Also, a new API `plat_arm_get_cpu_pe_count` is added when `ARM_PLAT_MT` is
    enabled, returning the Processing Element count within the physical CPU
    corresponding to `mpidr`.

  - The Arm platforms migrated to use version 2 of the translation tables.

  - Introduced a new Arm platform layer API `plat_arm_psci_override_pm_ops`
    which allows Arm platforms to modify `plat_arm_psci_pm_ops` and therefore
    dynamically define PSCI capability.

  - The Arm platforms migrated to use IMAGE_LOAD_V2 by default.

- Enhanced reporting of errata workaround status with the following policy:

  - If an errata workaround is enabled:

    - If it applies (i.e. the CPU is affected by the errata), an INFO message is
      printed, confirming that the errata workaround has been applied.
    - If it does not apply, a VERBOSE message is printed, confirming that the
      errata workaround has been skipped.

  - If an errata workaround is not enabled, but would have applied had it been,
    a WARN message is printed, alerting that errata workaround is missing.

- Added build options ARM_ARCH_MAJOR and ARM_ARM_MINOR to choose the
  architecture version to target TF-A.

- Updated the spin lock implementation to use the more efficient CAS (Compare
  And Swap) instruction when available. This instruction was introduced in
  Armv8.1-A.

- Applied errata workaround for Arm Cortex-A53: 855873.

- Applied errata workaround for Arm-Cortex-A57: 813419.

- Enabled all A53 and A57 errata workarounds for Juno, both in AArch64 and
  AArch32 execution states.

- Added support for Socionext UniPhier SoC platform.

- Added support for Hikey960 and Hikey platforms.

- Added support for Rockchip RK3328 platform.

- Added support for NVidia Tegra T186 platform.

- Added support for Designware emmc driver.

- Imported libfdt v1.4.2 that addresses buffer overflow in fdt_offset_ptr().

- Enhanced the CPU operations framework to allow power handlers to be registered
  on per-level basis. This enables support for future CPUs that have multiple
  threads which might need powering down individually.

- Updated register initialisation to prevent unexpected behaviour:

  - Debug registers MDCR-EL3/SDCR and MDCR_EL2/HDCR are initialised to avoid
    unexpected traps into the higher exception levels and disable secure
    self-hosted debug. Additionally, secure privileged external debug on Juno is
    disabled by programming the appropriate Juno SoC registers.
  - EL2 and EL3 configurable controls are initialised to avoid unexpected traps
    in the higher exception levels.
  - Essential control registers are fully initialised on EL3 start-up, when
    initialising the non-secure and secure context structures and when preparing
    to leave EL3 for a lower EL. This gives better alignment with the Arm ARM
    which states that software must initialise RES0 and RES1 fields with 0 / 1.

- Enhanced PSCI support:

  - Introduced new platform interfaces that decouple PSCI stat residency
    calculation from PMF, enabling platforms to use alternative methods of
    capturing timestamps.
  - PSCI stat accounting performed for retention/standby states when requested
    at multiple power levels.

- Simplified fiptool to have a single linked list of image descriptors.

- For the TSP, resolved corruption of pre-empted secure context by aborting any
  pre-empted SMC during PSCI power management requests.

### Issues resolved since last release

- TF-A can be built with the latest mbed TLS version (v2.4.2). The earlier
  version 2.3.0 cannot be used due to build warnings that the TF-A build system
  interprets as errors.
- TBBR, including the Firmware Update feature is now supported on FVP platforms
  when running TF-A in AArch32 state.
- The version of the AEMv8 Base FVP used in this release has resolved the issue
  of the model executing a reset instead of terminating in response to a
  shutdown request using the PSCI SYSTEM_OFF API.

### Known Issues

- Building TF-A with compiler optimisations disabled (-O0) fails.
- Trusted Board Boot currently does not work on Juno when running Trusted
  Firmware in AArch32 execution state due to error when loading the sp_min to
  memory because of lack of free space available. See [tf-issue#501] for more
  details.
- The errata workaround for A53 errata 843419 is only available from binutils
  2.26 and is not present in GCC4.9. If this errata is applicable to the
  platform, please use GCC compiler version of at least 5.0. See [PR#1002] for
  more details.

## [1.3.0](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/refs/tags/v1.2..refs/tags/v1.3) (2016-10-13)

### New features

- Added support for running TF-A in AArch32 execution state.

  The PSCI library has been refactored to allow integration with **EL3 Runtime
  Software**. This is software that is executing at the highest secure privilege
  which is EL3 in AArch64 or Secure SVC/Monitor mode in AArch32. See
  \{ref}`PSCI Library Integration guide for Armv8-A AArch32 systems`.

  Included is a minimal AArch32 Secure Payload, **SP-MIN**, that illustrates the
  usage and integration of the PSCI library with EL3 Runtime Software running in
  AArch32 state.

  Booting to the BL1/BL2 images as well as booting straight to the Secure
  Payload is supported.

- Improvements to the initialization framework for the PSCI service and Arm
  Standard Services in general.

  The PSCI service is now initialized as part of Arm Standard Service
  initialization. This consolidates the initializations of any Arm Standard
  Service that may be added in the future.

  A new function `get_arm_std_svc_args()` is introduced to get arguments
  corresponding to each standard service and must be implemented by the EL3
  Runtime Software.

  For PSCI, a new versioned structure `psci_lib_args_t` is introduced to
  initialize the PSCI Library. **Note** this is a compatibility break due to the
  change in the prototype of `psci_setup()`.

- To support AArch32 builds of BL1 and BL2, implemented a new, alternative
  firmware image loading mechanism that adds flexibility.

  The current mechanism has a hard-coded set of images and execution order
  (BL31, BL32, etc). The new mechanism is data-driven by a list of image
  descriptors provided by the platform code.

  Arm platforms have been updated to support the new loading mechanism.

  The new mechanism is enabled by a build flag (`LOAD_IMAGE_V2`) which is
  currently off by default for the AArch64 build.

  **Note** `TRUSTED_BOARD_BOOT` is currently not supported when `LOAD_IMAGE_V2`
  is enabled.

- Updated requirements for making contributions to TF-A.

  Commits now must have a 'Signed-off-by:' field to certify that the
  contribution has been made under the terms of the
  {download}`Developer Certificate of Origin <../dco.txt>`.

  A signed CLA is no longer required.

  The {ref}`Contributor's Guide` has been updated to reflect this change.

- Introduced Performance Measurement Framework (PMF) which provides support for
  capturing, storing, dumping and retrieving time-stamps to measure the
  execution time of critical paths in the firmware. This relies on defining
  fixed sample points at key places in the code.

- To support the QEMU platform port, imported libfdt v1.4.1 from
  <https://git.kernel.org/pub/scm/utils/dtc/dtc.git>

- Updated PSCI support:

  - Added support for PSCI NODE_HW_STATE API for Arm platforms.
  - New optional platform hook, `pwr_domain_pwr_down_wfi()`, in `plat_psci_ops`
    to enable platforms to perform platform-specific actions needed to enter
    powerdown, including the 'wfi' invocation.
  - PSCI STAT residency and count functions have been added on Arm platforms by
    using PMF.

- Enhancements to the translation table library:

  - Limited memory mapping support for region overlaps to only allow regions to
    overlap that are identity mapped or have the same virtual to physical
    address offset, and overlap completely but must not cover the same area.

    This limitation will enable future enhancements without having to support
    complex edge cases that may not be necessary.

  - The initial translation lookup level is now inferred from the virtual
    address space size. Previously, it was hard-coded.

  - Added support for mapping Normal, Inner Non-cacheable, Outer Non-cacheable
    memory in the translation table library.

    This can be useful to map a non-cacheable memory region, such as a DMA
    buffer.

  - Introduced the MT_EXECUTE/MT_EXECUTE_NEVER memory mapping attributes to
    specify the access permissions for instruction execution of a memory region.

- Enabled support to isolate code and read-only data on separate memory pages,
  allowing independent access control to be applied to each.

- Enabled SCR_EL3.SIF (Secure Instruction Fetch) bit in BL1 and BL31 common
  architectural setup code, preventing fetching instructions from non-secure
  memory when in secure state.

- Enhancements to FIP support:

  - Replaced `fip_create` with `fiptool` which provides a more consistent and
    intuitive interface as well as additional support to remove an image from a
    FIP file.
  - Enabled printing the SHA256 digest with info command, allowing quick
    verification of an image within a FIP without having to extract the image
    and running sha256sum on it.
  - Added support for unpacking the contents of an existing FIP file into the
    working directory.
  - Aligned command line options for specifying images to use same naming
    convention as specified by TBBR and already used in cert_create tool.

- Refactored the TZC-400 driver to also support memory controllers that
  integrate TZC functionality, for example Arm CoreLink DMC-500. Also added
  DMC-500 specific support.

- Implemented generic delay timer based on the system generic counter and
  migrated all platforms to use it.

- Enhanced support for Arm platforms:

  - Updated image loading support to make SCP images (SCP_BL2 and SCP_BL2U)
    optional.
  - Enhanced topology description support to allow multi-cluster topology
    definitions.
  - Added interconnect abstraction layer to help platform ports select the right
    interconnect driver, CCI or CCN, for the platform.
  - Added support to allow loading BL31 in the TZC-secured DRAM instead of the
    default secure SRAM.
  - Added support to use a System Security Control (SSC) Registers Unit enabling
    TF-A to be compiled to support multiple Arm platforms and then select one at
    runtime.
  - Restricted mapping of Trusted ROM in BL1 to what is actually needed by BL1
    rather than entire Trusted ROM region.
  - Flash is now mapped as execute-never by default. This increases security by
    restricting the executable region to what is strictly needed.

- Applied following erratum workarounds for Cortex-A57: 833471, 826977, 829520,
  828024 and 826974.

- Added support for Mediatek MT6795 platform.

- Added support for QEMU virtualization Armv8-A target.

- Added support for Rockchip RK3368 and RK3399 platforms.

- Added support for Xilinx Zynq UltraScale+ MPSoC platform.

- Added support for Arm Cortex-A73 MPCore Processor.

- Added support for Arm Cortex-A72 processor.

- Added support for Arm Cortex-A35 processor.

- Added support for Arm Cortex-A32 MPCore Processor.

- Enabled preloaded BL33 alternative boot flow, in which BL2 does not load BL33
  from non-volatile storage and BL31 hands execution over to a preloaded BL33.
  The User Guide has been updated with an example of how to use this option with
  a bootwrapped kernel.

- Added support to build TF-A on a Windows-based host machine.

- Updated Trusted Board Boot prototype implementation:

  - Enabled the ability for a production ROM with TBBR enabled to boot test
    software before a real ROTPK is deployed (e.g. manufacturing mode). Added
    support to use ROTPK in certificate without verifying against the platform
    value when `ROTPK_NOT_DEPLOYED` bit is set.
  - Added support for non-volatile counter authentication to the Authentication
    Module to protect against roll-back.

- Updated GICv3 support:

  - Enabled processor power-down and automatic power-on using GICv3.
  - Enabled G1S or G0 interrupts to be configured independently.
  - Changed FVP default interrupt driver to be the GICv3-only driver. **Note**
    the default build of TF-A will not be able to boot Linux kernel with GICv2
    FDT blob.
  - Enabled wake-up from CPU_SUSPEND to stand-by by temporarily re-routing
    interrupts and then restoring after resume.

### Issues resolved since last release

### Known issues

- The version of the AEMv8 Base FVP used in this release resets the model
  instead of terminating its execution in response to a shutdown request using
  the PSCI `SYSTEM_OFF` API. This issue will be fixed in a future version of the
  model.
- Building TF-A with compiler optimisations disabled (`-O0`) fails.
- TF-A cannot be built with mbed TLS version v2.3.0 due to build warnings that
  the TF-A build system interprets as errors.
- TBBR is not currently supported when running TF-A in AArch32 state.

## [1.2.0](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/refs/tags/v1.1..refs/tags/v1.2) (2015-12-22)

### New features

- The Trusted Board Boot implementation on Arm platforms now conforms to the
  mandatory requirements of the TBBR specification.

  In particular, the boot process is now guarded by a Trusted Watchdog, which
  will reset the system in case of an authentication or loading error. On Arm
  platforms, a secure instance of Arm SP805 is used as the Trusted Watchdog.

  Also, a firmware update process has been implemented. It enables authenticated
  firmware to update firmware images from external interfaces to SoC
  Non-Volatile memories. This feature functions even when the current firmware
  in the system is corrupt or missing; it therefore may be used as a recovery
  mode.

- Improvements have been made to the Certificate Generation Tool (`cert_create`)
  as follows.

  - Added support for the Firmware Update process by extending the Chain of
    Trust definition in the tool to include the Firmware Update certificate and
    the required extensions.
  - Introduced a new API that allows one to specify command line options in the
    Chain of Trust description. This makes the declaration of the tool's
    arguments more flexible and easier to extend.
  - The tool has been reworked to follow a data driven approach, which makes it
    easier to maintain and extend.

- Extended the FIP tool (`fip_create`) to support the new set of images involved
  in the Firmware Update process.

- Various memory footprint improvements. In particular:

  - The bakery lock structure for coherent memory has been optimised.
  - The mbed TLS SHA1 functions are not needed, as SHA256 is used to generate
    the certificate signature. Therefore, they have been compiled out, reducing
    the memory footprint of BL1 and BL2 by approximately 6 KB.
  - On Arm development platforms, each BL stage now individually defines the
    number of regions that it needs to map in the MMU.

- Added the following new design documents:

  - {ref}`Authentication Framework & Chain of Trust`
  - {ref}`Firmware Update (FWU)`
  - {ref}`CPU Reset`
  - {ref}`PSCI Power Domain Tree Structure`

- Applied the new image terminology to the code base and documentation, as
  described in the {ref}`Image Terminology` document.

- The build system has been reworked to improve readability and facilitate
  adding future extensions.

- On Arm standard platforms, BL31 uses the boot console during cold boot but
  switches to the runtime console for any later logs at runtime. The TSP uses
  the runtime console for all output.

- Implemented a basic NOR flash driver for Arm platforms. It programs the device
  using CFI (Common Flash Interface) standard commands.

- Implemented support for booting EL3 payloads on Arm platforms, which reduces
  the complexity of developing EL3 baremetal code by doing essential baremetal
  initialization.

- Provided separate drivers for GICv3 and GICv2. These expect the entire
  software stack to use either GICv2 or GICv3; hybrid GIC software systems are
  no longer supported and the legacy Arm GIC driver has been deprecated.

- Added support for Juno r1 and r2. A single set of Juno TF-A binaries can run
  on Juno r0, r1 and r2 boards. Note that this TF-A version depends on a Linaro
  release that does *not* contain Juno r2 support.

- Added support for MediaTek mt8173 platform.

- Implemented a generic driver for Arm CCN IP.

- Major rework of the PSCI implementation.

  - Added framework to handle composite power states.
  - Decoupled the notions of affinity instances (which describes the
    hierarchical arrangement of cores) and of power domain topology, instead of
    assuming a one-to-one mapping.
  - Better alignment with version 1.0 of the PSCI specification.

- Added support for the SYSTEM_SUSPEND PSCI API on Arm platforms. When invoked
  on the last running core on a supported platform, this puts the system into a
  low power mode with memory retention.

- Unified the reset handling code as much as possible across BL stages. Also
  introduced some build options to enable optimization of the reset path on
  platforms that support it.

- Added a simple delay timer API, as well as an SP804 timer driver, which is
  enabled on FVP.

- Added support for NVidia Tegra T210 and T132 SoCs.

- Reorganised Arm platforms ports to greatly improve code shareability and
  facilitate the reuse of some of this code by other platforms.

- Added support for Arm Cortex-A72 processor in the CPU specific framework.

- Provided better error handling. Platform ports can now define their own error
  handling, for example to perform platform specific bookkeeping or post-error
  actions.

- Implemented a unified driver for Arm Cache Coherent Interconnects used for
  both CCI-400 & CCI-500 IPs. Arm platforms ports have been migrated to this
  common driver. The standalone CCI-400 driver has been deprecated.

### Issues resolved since last release

- The Trusted Board Boot implementation has been redesigned to provide greater
  modularity and scalability. See the
  \{ref}`Authentication Framework & Chain of Trust` document. All missing
  mandatory features are now implemented.
- The FVP and Juno ports may now use the hash of the ROTPK stored in the Trusted
  Key Storage registers to verify the ROTPK. Alternatively, a development public
  key hash embedded in the BL1 and BL2 binaries might be used instead. The
  location of the ROTPK is chosen at build-time using the `ARM_ROTPK_LOCATION`
  build option.
- GICv3 is now fully supported and stable.

### Known issues

- The version of the AEMv8 Base FVP used in this release resets the model
  instead of terminating its execution in response to a shutdown request using
  the PSCI `SYSTEM_OFF` API. This issue will be fixed in a future version of the
  model.
- While this version has low on-chip RAM requirements, there are further RAM
  usage enhancements that could be made.
- The upstream documentation could be improved for structural consistency,
  clarity and completeness. In particular, the design documentation is
  incomplete for PSCI, the TSP(D) and the Juno platform.
- Building TF-A with compiler optimisations disabled (`-O0`) fails.

## [1.1.0](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/refs/tags/v1.0..refs/tags/v1.1) (2015-02-04)

### New features

- A prototype implementation of Trusted Board Boot has been added. Boot loader
  images are verified by BL1 and BL2 during the cold boot path. BL1 and BL2 use
  the PolarSSL SSL library to verify certificates and images. The OpenSSL
  library is used to create the X.509 certificates. Support has been added to
  `fip_create` tool to package the certificates in a FIP.

- Support for calling CPU and platform specific reset handlers upon entry into
  BL3-1 during the cold and warm boot paths has been added. This happens after
  another Boot ROM `reset_handler()` has already run. This enables a developer
  to perform additional actions or undo actions already performed during the
  first call of the reset handlers e.g. apply additional errata workarounds.

- Support has been added to demonstrate routing of IRQs to EL3 instead of S-EL1
  when execution is in secure world.

- The PSCI implementation now conforms to version 1.0 of the PSCI specification.
  All the mandatory APIs and selected optional APIs are supported. In
  particular, support for the `PSCI_FEATURES` API has been added. A capability
  variable is constructed during initialization by examining the `plat_pm_ops`
  and `spd_pm_ops` exported by the platform and the Secure Payload Dispatcher.
  This is used by the PSCI FEATURES function to determine which PSCI APIs are
  supported by the platform.

- Improvements have been made to the PSCI code as follows.

  - The code has been refactored to remove redundant parameters from internal
    functions.
  - Changes have been made to the code for PSCI `CPU_SUSPEND`, `CPU_ON` and
    `CPU_OFF` calls to facilitate an early return to the caller in case a
    failure condition is detected. For example, a PSCI `CPU_SUSPEND` call
    returns `SUCCESS` to the caller if a pending interrupt is detected early in
    the code path.
  - Optional platform APIs have been added to validate the `power_state` and
    `entrypoint` parameters early in PSCI `CPU_ON` and `CPU_SUSPEND` code paths.
  - PSCI migrate APIs have been reworked to invoke the SPD hook to determine the
    type of Trusted OS and the CPU it is resident on (if applicable). Also,
    during a PSCI `MIGRATE` call, the SPD hook to migrate the Trusted OS is
    invoked.

- It is now possible to build TF-A without marking at least an extra page of
  memory as coherent. The build flag `USE_COHERENT_MEM` can be used to choose
  between the two implementations. This has been made possible through these
  changes.

  - An implementation of Bakery locks, where the locks are not allocated in
    coherent memory has been added.
  - Memory which was previously marked as coherent is now kept coherent through
    the use of software cache maintenance operations.

  Approximately, 4K worth of memory is saved for each boot loader stage when
  `USE_COHERENT_MEM=0`. Enabling this option increases the latencies associated
  with acquire and release of locks. It also requires changes to the platform
  ports.

- It is now possible to specify the name of the FIP at build time by defining
  the `FIP_NAME` variable.

- Issues with dependencies on the 'fiptool' makefile target have been rectified.
  The `fip_create` tool is now rebuilt whenever its source files change.

- The BL3-1 runtime console is now also used as the crash console. The crash
  console is changed to SoC UART0 (UART2) from the previous FPGA UART0 (UART0)
  on Juno. In FVP, it is changed from UART0 to UART1.

- CPU errata workarounds are applied only when the revision and part number
  match. This behaviour has been made consistent across the debug and release
  builds. The debug build additionally prints a warning if a mismatch is
  detected.

- It is now possible to issue cache maintenance operations by set/way for a
  particular level of data cache. Levels 1-3 are currently supported.

- The following improvements have been made to the FVP port.

  - The build option `FVP_SHARED_DATA_LOCATION` which allowed relocation of
    shared data into the Trusted DRAM has been deprecated. Shared data is now
    always located at the base of Trusted SRAM.
  - BL2 Translation tables have been updated to map only the region of DRAM
    which is accessible to normal world. This is the region of the 2GB DDR-DRAM
    memory at 0x80000000 excluding the top 16MB. The top 16MB is accessible to
    only the secure world.
  - BL3-2 can now reside in the top 16MB of DRAM which is accessible only to the
    secure world. This can be done by setting the build flag
    `FVP_TSP_RAM_LOCATION` to the value `dram`.

- Separate translation tables are created for each boot loader image. The
  `IMAGE_BLx` build options are used to do this. This allows each stage to
  create mappings only for areas in the memory map that it needs.

- A Secure Payload Dispatcher (OPTEED) for the OP-TEE Trusted OS has been added.
  Details of using it with TF-A can be found in {ref}`OP-TEE Dispatcher`

### Issues resolved since last release

- The Juno port has been aligned with the FVP port as follows.

  - Support for reclaiming all BL1 RW memory and BL2 memory by overlaying the
    BL3-1/BL3-2 NOBITS sections on top of them has been added to the Juno port.
  - The top 16MB of the 2GB DDR-DRAM memory at 0x80000000 is configured using
    the TZC-400 controller to be accessible only to the secure world.
  - The Arm GIC driver is used to configure the GIC-400 instead of using a GIC
    driver private to the Juno port.
  - PSCI `CPU_SUSPEND` calls that target a standby state are now supported.
  - The TZC-400 driver is used to configure the controller instead of direct
    accesses to the registers.

- The Linux kernel version referred to in the user guide has DVFS and HMP
  support enabled.

- DS-5 v5.19 did not detect Version 5.8 of the Cortex-A57-A53 Base FVPs in CADI
  server mode. This issue is not seen with DS-5 v5.20 and Version 6.2 of the
  Cortex-A57-A53 Base FVPs.

### Known issues

- The Trusted Board Boot implementation is a prototype. There are issues with
  the modularity and scalability of the design. Support for a Trusted Watchdog,
  firmware update mechanism, recovery images and Trusted debug is absent. These
  issues will be addressed in future releases.
- The FVP and Juno ports do not use the hash of the ROTPK stored in the Trusted
  Key Storage registers to verify the ROTPK in the `plat_match_rotpk()`
  function. This prevents the correct establishment of the Chain of Trust at the
  first step in the Trusted Board Boot process.
- The version of the AEMv8 Base FVP used in this release resets the model
  instead of terminating its execution in response to a shutdown request using
  the PSCI `SYSTEM_OFF` API. This issue will be fixed in a future version of the
  model.
- GICv3 support is experimental. There are known issues with GICv3
  initialization in the TF-A.
- While this version greatly reduces the on-chip RAM requirements, there are
  further RAM usage enhancements that could be made.
- The firmware design documentation for the Test Secure-EL1 Payload (TSP) and
  its dispatcher (TSPD) is incomplete. Similarly for the PSCI section.
- The Juno-specific firmware design documentation is incomplete.

## [1.0.0](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/refs/tags/v0.4..refs/tags/v1.0) (2014-08-28)

### New features

- It is now possible to map higher physical addresses using non-flat virtual to
  physical address mappings in the MMU setup.

- Wider use is now made of the per-CPU data cache in BL3-1 to store:

  - Pointers to the non-secure and secure security state contexts.
  - A pointer to the CPU-specific operations.
  - A pointer to PSCI specific information (for example the current power
    state).
  - A crash reporting buffer.

- The following RAM usage improvements result in a BL3-1 RAM usage reduction
  from 96KB to 56KB (for FVP with TSPD), and a total RAM usage reduction across
  all images from 208KB to 88KB, compared to the previous release.

  - Removed the separate `early_exception` vectors from BL3-1 (2KB code size
    saving).
  - Removed NSRAM from the FVP memory map, allowing the removal of one (4KB)
    translation table.
  - Eliminated the internal `psci_suspend_context` array, saving 2KB.
  - Correctly dimensioned the PSCI `aff_map_node` array, saving 1.5KB in the FVP
    port.
  - Removed calling CPU mpidr from the bakery lock API, saving 160 bytes.
  - Removed current CPU mpidr from PSCI common code, saving 160 bytes.
  - Inlined the mmio accessor functions, saving 360 bytes.
  - Fully reclaimed all BL1 RW memory and BL2 memory on the FVP port by
    overlaying the BL3-1/BL3-2 NOBITS sections on top of these at runtime.
  - Made storing the FP register context optional, saving 0.5KB per context (8KB
    on the FVP port, with TSPD enabled and running on 8 CPUs).
  - Implemented a leaner `tf_printf()` function, allowing the stack to be
    greatly reduced.
  - Removed coherent stacks from the codebase. Stacks allocated in normal memory
    are now used before and after the MMU is enabled. This saves 768 bytes per
    CPU in BL3-1.
  - Reworked the crash reporting in BL3-1 to use less stack.
  - Optimized the EL3 register state stored in the `cpu_context` structure so
    that registers that do not change during normal execution are re-initialized
    each time during cold/warm boot, rather than restored from memory. This
    saves about 1.2KB.
  - As a result of some of the above, reduced the runtime stack size in all BL
    images. For BL3-1, this saves 1KB per CPU.

- PSCI SMC handler improvements to correctly handle calls from secure states and
  from AArch32.

- CPU contexts are now initialized from the `entry_point_info`. BL3-1 fully
  determines the exception level to use for the non-trusted firmware (BL3-3)
  based on the SPSR value provided by the BL2 platform code (or otherwise
  provided to BL3-1). This allows platform code to directly run non-trusted
  firmware payloads at either EL2 or EL1 without requiring an EL2 stub or OS
  loader.

- Code refactoring improvements:

  - Refactored `fvp_config` into a common platform header.
  - Refactored the fvp gic code to be a generic driver that no longer has an
    explicit dependency on platform code.
  - Refactored the CCI-400 driver to not have dependency on platform code.
  - Simplified the IO driver so it's no longer necessary to call `io_init()` and
    moved all the IO storage framework code to one place.
  - Simplified the interface the the TZC-400 driver.
  - Clarified the platform porting interface to the TSP.
  - Reworked the TSPD setup code to support the alternate BL3-2 initialization
    flow where BL3-1 generic code hands control to BL3-2, rather than expecting
    the TSPD to hand control directly to BL3-2.
  - Considerable rework to PSCI generic code to support CPU specific operations.

- Improved console log output, by:

  - Adding the concept of debug log levels.
  - Rationalizing the existing debug messages and adding new ones.
  - Printing out the version of each BL stage at runtime.
  - Adding support for printing console output from assembler code, including
    when a crash occurs before the C runtime is initialized.

- Moved up to the latest versions of the FVPs, toolchain, EDK2, kernel, Linaro
  file system and DS-5.

- On the FVP port, made the use of the Trusted DRAM region optional at build
  time (off by default). Normal platforms will not have such a "ready-to-use"
  DRAM area so it is not a good example to use it.

- Added support for PSCI `SYSTEM_OFF` and `SYSTEM_RESET` APIs.

- Added support for CPU specific reset sequences, power down sequences and
  register dumping during crash reporting. The CPU specific reset sequences
  include support for errata workarounds.

- Merged the Juno port into the master branch. Added support for CPU hotplug and
  CPU idle. Updated the user guide to describe how to build and run on the Juno
  platform.

### Issues resolved since last release

- Removed the concept of top/bottom image loading. The image loader now
  automatically detects the position of the image inside the current memory
  layout and updates the layout to minimize fragmentation. This resolves the
  image loader limitations of previously releases. There are currently no plans
  to support dynamic image loading.
- CPU idle now works on the publicized version of the Foundation FVP.
- All known issues relating to the compiler version used have now been resolved.
  This TF-A version uses Linaro toolchain 14.07 (based on GCC 4.9).

### Known issues

- GICv3 support is experimental. The Linux kernel patches to support this are
  not widely available. There are known issues with GICv3 initialization in the
  TF-A.

- While this version greatly reduces the on-chip RAM requirements, there are
  further RAM usage enhancements that could be made.

- The firmware design documentation for the Test Secure-EL1 Payload (TSP) and
  its dispatcher (TSPD) is incomplete. Similarly for the PSCI section.

- The Juno-specific firmware design documentation is incomplete.

- Some recent enhancements to the FVP port have not yet been translated into the
  Juno port. These will be tracked via the tf-issues project.

- The Linux kernel version referred to in the user guide has DVFS and HMP
  support disabled due to some known instabilities at the time of this release.
  A future kernel version will re-enable these features.

- DS-5 v5.19 does not detect Version 5.8 of the Cortex-A57-A53 Base FVPs in CADI
  server mode. This is because the `<SimName>` reported by the FVP in this
  version has changed. For example, for the Cortex-A57x4-A53x4 Base FVP, the
  `<SimName>` reported by the FVP is `FVP_Base_Cortex_A57x4_A53x4`, while DS-5
  expects it to be `FVP_Base_A57x4_A53x4`.

  The temporary fix to this problem is to change the name of the FVP in
  `sw/debugger/configdb/Boards/ARM FVP/Base_A57x4_A53x4/cadi_config.xml`. Change
  the following line:

  ```
  <SimName>System Generator:FVP_Base_A57x4_A53x4</SimName>
  ```

  to System Generator:FVP_Base_Cortex-A57x4_A53x4

  A similar change can be made to the other Cortex-A57-A53 Base FVP variants.

## [0.4.0](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/refs/tags/v0.3..refs/tags/v0.4) (2014-06-03)

### New features

- Makefile improvements:

  - Improved dependency checking when building.
  - Removed `dump` target (build now always produces dump files).
  - Enabled platform ports to optionally make use of parts of the Trusted
    Firmware (e.g. BL3-1 only), rather than being forced to use all parts. Also
    made the `fip` target optional.
  - Specified the full path to source files and removed use of the `vpath`
    keyword.

- Provided translation table library code for potential re-use by platforms
  other than the FVPs.

- Moved architectural timer setup to platform-specific code.

- Added standby state support to PSCI cpu_suspend implementation.

- SRAM usage improvements:

  - Started using the `-ffunction-sections`, `-fdata-sections` and
    `--gc-sections` compiler/linker options to remove unused code and data from
    the images. Previously, all common functions were being built into all
    binary images, whether or not they were actually used.
  - Placed all assembler functions in their own section to allow more unused
    functions to be removed from images.
  - Updated BL1 and BL2 to use a single coherent stack each, rather than one per
    CPU.
  - Changed variables that were unnecessarily declared and initialized as
    non-const (i.e. in the .data section) so they are either uninitialized (zero
    init) or const.

- Moved the Test Secure-EL1 Payload (BL3-2) to execute in Trusted SRAM by
  default. The option for it to run in Trusted DRAM remains.

- Implemented a TrustZone Address Space Controller (TZC-400) driver. A default
  configuration is provided for the Base FVPs. This means the model parameter
  `-C bp.secure_memory=1` is now supported.

- Started saving the PSCI cpu_suspend 'power_state' parameter prior to
  suspending a CPU. This allows platforms that implement multiple power-down
  states at the same affinity level to identify a specific state.

- Refactored the entire codebase to reduce the amount of nesting in header files
  and to make the use of system/user includes more consistent. Also split
  platform.h to separate out the platform porting declarations from the required
  platform porting definitions and the definitions/declarations specific to the
  platform port.

- Optimized the data cache clean/invalidate operations.

- Improved the BL3-1 unhandled exception handling and reporting. Unhandled
  exceptions now result in a dump of registers to the console.

- Major rework to the handover interface between BL stages, in particular the
  interface to BL3-1. The interface now conforms to a specification and is more
  future proof.

- Added support for optionally making the BL3-1 entrypoint a reset handler
  (instead of BL1). This allows platforms with an alternative image loading
  architecture to re-use BL3-1 with fewer modifications to generic code.

- Reserved some DDR DRAM for secure use on FVP platforms to avoid future
  compatibility problems with non-secure software.

- Added support for secure interrupts targeting the Secure-EL1 Payload (SP)
  (using GICv2 routing only). Demonstrated this working by adding an interrupt
  target and supporting test code to the TSP. Also demonstrated non-secure
  interrupt handling during TSP processing.

### Issues resolved since last release

- Now support use of the model parameter `-C bp.secure_memory=1` in the Base
  FVPs (see **New features**).
- Support for secure world interrupt handling now available (see **New
  features**).
- Made enough SRAM savings (see **New features**) to enable the Test Secure-EL1
  Payload (BL3-2) to execute in Trusted SRAM by default.
- The tested filesystem used for this release (Linaro AArch64 OpenEmbedded
  14.04) now correctly reports progress in the console.
- Improved the Makefile structure to make it easier to separate out parts of the
  TF-A for re-use in platform ports. Also, improved target dependency checking.

### Known issues

- GICv3 support is experimental. The Linux kernel patches to support this are
  not widely available. There are known issues with GICv3 initialization in the
  TF-A.
- Dynamic image loading is not available yet. The current image loader
  implementation (used to load BL2 and all subsequent images) has some
  limitations. Changing BL2 or BL3-1 load addresses in certain ways can lead to
  loading errors, even if the images should theoretically fit in memory.
- TF-A still uses too much on-chip Trusted SRAM. A number of RAM usage
  enhancements have been identified to rectify this situation.
- CPU idle does not work on the advertised version of the Foundation FVP. Some
  FVP fixes are required that are not available externally at the time of
  writing. This can be worked around by disabling CPU idle in the Linux kernel.
- Various bugs in TF-A, UEFI and the Linux kernel have been observed when using
  Linaro toolchain versions later than 13.11. Although most of these have been
  fixed, some remain at the time of writing. These mainly seem to relate to a
  subtle change in the way the compiler converts between 64-bit and 32-bit
  values (e.g. during casting operations), which reveals previously hidden bugs
  in client code.
- The firmware design documentation for the Test Secure-EL1 Payload (TSP) and
  its dispatcher (TSPD) is incomplete. Similarly for the PSCI section.

## [0.3.0](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/refs/tags/v0.2..refs/tags/v0.3) (2014-02-28)

### New features

- Support for Foundation FVP Version 2.0 added. The documented UEFI
  configuration disables some devices that are unavailable in the Foundation
  FVP, including MMC and CLCD. The resultant UEFI binary can be used on the
  AEMv8 and Cortex-A57-A53 Base FVPs, as well as the Foundation FVP.

  \:::\{note} The software will not work on Version 1.0 of the Foundation FVP.
  \:::

- Enabled third party contributions. Added a new contributing.md containing
  instructions for how to contribute and updated copyright text in all files to
  acknowledge contributors.

- The PSCI CPU_SUSPEND API has been stabilised to the extent where it can be
  used for entry into power down states with the following restrictions:

  - Entry into standby states is not supported.
  - The API is only supported on the AEMv8 and Cortex-A57-A53 Base FVPs.

- The PSCI AFFINITY_INFO api has undergone limited testing on the Base FVPs to
  allow experimental use.

- Required C library and runtime header files are now included locally in TF-A
  instead of depending on the toolchain standard include paths. The local
  implementation has been cleaned up and reduced in scope.

- Added I/O abstraction framework, primarily to allow generic code to load
  images in a platform-independent way. The existing image loading code has been
  reworked to use the new framework. Semi-hosting and NOR flash I/O drivers are
  provided.

- Introduced Firmware Image Package (FIP) handling code and tools. A FIP
  combines multiple firmware images with a Table of Contents (ToC) into a single
  binary image. The new FIP driver is another type of I/O driver. The Makefile
  builds a FIP by default and the FVP platform code expect to load a FIP from
  NOR flash, although some support for image loading using semi- hosting is
  retained.

  \:::\{note} Building a FIP by default is a non-backwards-compatible change. :::

  \:::\{note} Generic BL2 code now loads a BL3-3 (non-trusted firmware) image
  into DRAM instead of expecting this to be pre-loaded at known location. This
  is also a non-backwards-compatible change. :::

  \:::\{note} Some non-trusted firmware (e.g. UEFI) will need to be rebuilt so
  that it knows the new location to execute from and no longer needs to copy
  particular code modules to DRAM itself. :::

- Reworked BL2 to BL3-1 handover interface. A new composite structure
  (bl31_args) holds the superset of information that needs to be passed from BL2
  to BL3-1, including information on how handover execution control to BL3-2 (if
  present) and BL3-3 (non-trusted firmware).

- Added library support for CPU context management, allowing the saving and
  restoring of

  - Shared system registers between Secure-EL1 and EL1.
  - VFP registers.
  - Essential EL3 system registers.

- Added a framework for implementing EL3 runtime services. Reworked the PSCI
  implementation to be one such runtime service.

- Reworked the exception handling logic, making use of both SP_EL0 and SP_EL3
  stack pointers for determining the type of exception, managing general purpose
  and system register context on exception entry/exit, and handling SMCs. SMCs
  are directed to the correct EL3 runtime service.

- Added support for a Test Secure-EL1 Payload (TSP) and a corresponding
  Dispatcher (TSPD), which is loaded as an EL3 runtime service. The TSPD
  implements Secure Monitor functionality such as world switching and EL1
  context management, and is responsible for communication with the TSP.

  \:::\{note} The TSPD does not yet contain support for secure world interrupts.
  \:::

  \:::\{note} The TSP/TSPD is not built by default. :::

### Issues resolved since last release

- Support has been added for switching context between secure and normal worlds
  in EL3.
- PSCI API calls `AFFINITY_INFO` & `PSCI_VERSION` have now been tested (to a
  limited extent).
- The TF-A build artifacts are now placed in the `./build` directory and
  sub-directories instead of being placed in the root of the project.
- TF-A is now free from build warnings. Build warnings are now treated as
  errors.
- TF-A now provides C library support locally within the project to maintain
  compatibility between toolchains/systems.
- The PSCI locking code has been reworked so it no longer takes locks in an
  incorrect sequence.
- The RAM-disk method of loading a Linux file-system has been confirmed to work
  with the TF-A and Linux kernel version (based on version 3.13) used in this
  release, for both Foundation and Base FVPs.

### Known issues

The following is a list of issues which are expected to be fixed in the future
releases of TF-A.

- The TrustZone Address Space Controller (TZC-400) is not being programmed yet.
  Use of model parameter `-C bp.secure_memory=1` is not supported.
- No support yet for secure world interrupt handling.
- GICv3 support is experimental. The Linux kernel patches to support this are
  not widely available. There are known issues with GICv3 initialization in
  TF-A.
- Dynamic image loading is not available yet. The current image loader
  implementation (used to load BL2 and all subsequent images) has some
  limitations. Changing BL2 or BL3-1 load addresses in certain ways can lead to
  loading errors, even if the images should theoretically fit in memory.
- TF-A uses too much on-chip Trusted SRAM. Currently the Test Secure-EL1 Payload
  (BL3-2) executes in Trusted DRAM since there is not enough SRAM. A number of
  RAM usage enhancements have been identified to rectify this situation.
- CPU idle does not work on the advertised version of the Foundation FVP. Some
  FVP fixes are required that are not available externally at the time of
  writing.
- Various bugs in TF-A, UEFI and the Linux kernel have been observed when using
  Linaro toolchain versions later than 13.11. Although most of these have been
  fixed, some remain at the time of writing. These mainly seem to relate to a
  subtle change in the way the compiler converts between 64-bit and 32-bit
  values (e.g. during casting operations), which reveals previously hidden bugs
  in client code.
- The tested filesystem used for this release (Linaro AArch64 OpenEmbedded
  14.01) does not report progress correctly in the console. It only seems to
  produce error output, not standard output. It otherwise appears to function
  correctly. Other filesystem versions on the same software stack do not exhibit
  the problem.
- The Makefile structure doesn't make it easy to separate out parts of the TF-A
  for re-use in platform ports, for example if only BL3-1 is required in a
  platform port. Also, dependency checking in the Makefile is flawed.
- The firmware design documentation for the Test Secure-EL1 Payload (TSP) and
  its dispatcher (TSPD) is incomplete. Similarly for the PSCI section.

## [0.2.0](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/4b825dc642cb6eb9a060e54bf8d69288fbee4904..refs/tags/v0.2) (2013-10-25)

### New features

- First source release.
- Code for the PSCI suspend feature is supplied, although this is not enabled by
  default since there are known issues (see below).

### Issues resolved since last release

- The "psci" nodes in the FDTs provided in this release now fully comply with
  the recommendations made in the PSCI specification.

### Known issues

The following is a list of issues which are expected to be fixed in the future
releases of TF-A.

- The TrustZone Address Space Controller (TZC-400) is not being programmed yet.
  Use of model parameter `-C bp.secure_memory=1` is not supported.
- No support yet for secure world interrupt handling or for switching context
  between secure and normal worlds in EL3.
- GICv3 support is experimental. The Linux kernel patches to support this are
  not widely available. There are known issues with GICv3 initialization in
  TF-A.
- Dynamic image loading is not available yet. The current image loader
  implementation (used to load BL2 and all subsequent images) has some
  limitations. Changing BL2 or BL3-1 load addresses in certain ways can lead to
  loading errors, even if the images should theoretically fit in memory.
- Although support for PSCI `CPU_SUSPEND` is present, it is not yet stable and
  ready for use.
- PSCI API calls `AFFINITY_INFO` & `PSCI_VERSION` are implemented but have not
  been tested.
- The TF-A make files result in all build artifacts being placed in the root of
  the project. These should be placed in appropriate sub-directories.
- The compilation of TF-A is not free from compilation warnings. Some of these
  warnings have not been investigated yet so they could mask real bugs.
- TF-A currently uses toolchain/system include files like stdio.h. It should
  provide versions of these within the project to maintain compatibility between
  toolchains/systems.
- The PSCI code takes some locks in an incorrect sequence. This may cause
  problems with suspend and hotplug in certain conditions.
- The Linux kernel used in this release is based on version 3.12-rc4. Using this
  kernel with the TF-A fails to start the file-system as a RAM-disk. It fails to
  execute user-space `init` from the RAM-disk. As an alternative, the
  VirtioBlock mechanism can be used to provide a file-system to the kernel.

______________________________________________________________________

*Copyright (c) 2013-2022, Arm Limited and Contributors. All rights reserved.*

[mbed tls releases]: https://tls.mbed.org/tech-updates/releases
[pr#1002]: https://github.com/ARM-software/arm-trusted-firmware/pull/1002#issuecomment-312650193
[sdei specification]: http://infocenter.arm.com/help/topic/com.arm.doc.den0054a/ARM_DEN0054A_Software_Delegated_Exception_Interface.pdf
[tf-issue#501]: https://github.com/ARM-software/tf-issues/issues/501
