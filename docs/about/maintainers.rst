Project Maintenance
===================

Trusted Firmware-A (TF-A) is an open governance community project. All
contributions are ultimately merged by the maintainers listed below. Technical
ownership of most parts of the codebase falls on the code owners listed
below. An acknowledgement from these code owners is required before the
maintainers merge a contribution.

More details may be found in the `Project Maintenance Process`_ document.

.. |M| replace:: **Mail**
.. |G| replace:: **GitHub ID**
.. |F| replace:: **Files**

.. _maintainers:

Maintainers
-----------

:|M|: Dan Handley <dan.handley@arm.com>
:|G|: `danh-arm`_
:|M|: Soby Mathew <soby.mathew@arm.com>
:|G|: `soby-mathew`_
:|M|: Sandrine Bailleux <sandrine.bailleux@arm.com>
:|G|: `sandrine-bailleux-arm`_
:|M|: Alexei Fedorov <Alexei.Fedorov@arm.com>
:|G|: `AlexeiFedorov`_
:|M|: Manish Pandey <manish.pandey2@arm.com>
:|G|: `manish-pandey-arm`_
:|M|: Mark Dykes <mark.dykes@arm.com>
:|G|: `mardyk01`_
:|M|: Olivier Deprez <olivier.deprez@arm.com>
:|G|: `odeprez`_
:|M|: Bipin Ravi <bipin.ravi@arm.com>
:|G|: `bipinravi-arm`_
:|M|: Joanna Farley <joanna.farley@arm.com>
:|G|: `joannafarley-arm`_
:|M|: Julius Werner <jwerner@chromium.org>
:|G|: `jwerner-chromium`_
:|M|: Varun Wadekar <vwadekar@nvidia.com>
:|G|: `vwadekar`_
:|M|: Andre Przywara <andre.przywara@arm.com>
:|G|: `Andre-ARM`_
:|M|: Lauren Wehrmeister <Lauren.Wehrmeister@arm.com>
:|G|: `laurenw-arm`_
:|M|: Madhukar Pappireddy <Madhukar.Pappireddy@arm.com>
:|G|: `madhukar-Arm`_
:|M|: Raghu Krishnamurthy <raghu.ncstate@icloud.com>
:|G|: `raghuncstate`_
:|M|: Manish Badarkhe <manish.badarkhe@arm.com>
:|G|: `ManishVB-Arm`_


.. _code owners:

Code owners
-----------

Common Code
~~~~~~~~~~~

Armv7-A architecture port
^^^^^^^^^^^^^^^^^^^^^^^^^
:|M|: Etienne Carriere <etienne.carriere@linaro.org>
:|G|: `etienne-lms`_

Build Definitions for CMake Build System
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
:|M|: Javier Almansa Sobrino <Javier.AlmansaSobrino@arm.com>
:|G|: `javieralso-arm`_
:|M|: Chris Kay <chris.kay@arm.com>
:|G|: `CJKay`_
:|F|: /

Software Delegated Exception Interface (SDEI)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
:|M|: Mark Dykes <mark.dykes@arm.com>
:|G|: `mardyk01`_
:|F|: services/std_svc/sdei/

Trusted Boot
^^^^^^^^^^^^
:|M|: Sandrine Bailleux <sandrine.bailleux@arm.com>
:|G|: `sandrine-bailleux-arm`_
:|M|: Manish Pandey <manish.pandey2@arm.com>
:|G|: `manish-pandey-arm`_
:|M|: Manish Badarkhe <manish.badarkhe@arm.com>
:|G|: `ManishVB-Arm`_
:|F|: drivers/auth/

Secure Partition Manager Core (EL3 FF-A SPMC)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
:|M|: Marc Bonnici <marc.bonnici@arm.com>
:|G|: `marcbonnici`_
:|F|: services/std_svc/spm/el3_spmc/\*

Secure Partition Manager Dispatcher (SPMD)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
:|M|: Olivier Deprez <olivier.deprez@arm.com>
:|G|: `odeprez`_
:|M|: Manish Pandey <manish.pandey2@arm.com>
:|G|: `manish-pandey-arm`_
:|M|: Maksims Svecovs <maksims.svecovs@arm.com>
:|G|: `max-shvetsov`_
:|M|: Joao Alves <Joao.Alves@arm.com>
:|G|: `J-Alves`_
:|F|: services/std_svc/spmd/\*

Exception Handling Framework (EHF)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
:|M|: Manish Badarkhe <manish.badarkhe@arm.com>
:|G|: `ManishVB-Arm`_
:|F|: bl31/ehf.c

Realm Management Monitor Dispatcher (RMMD)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
:|M|: Javier Almansa Sobrino <javier.almansasobrino@arm.com>
:|G|: `javieralso-arm`_
:|F|: services/std_svc/rmmd/\*
:|F|: include/services/rmmd_svc.h
:|F|: include/services/rmm_core_manifest.h

Realm Management Extension (RME)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
:|M|: Bipin Ravi <bipin.ravi@arm.com>
:|G|: `bipinravi-arm`_
:|M|: Mark Dykes <mark.dykes@arm.com>
:|G|: `mardyk01`_
:|M|: Javier Almansa Sobrino <javier.almansasobrino@arm.com>
:|G|: `javieralso-arm`_

Drivers, Libraries and Framework Code
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Console API framework
^^^^^^^^^^^^^^^^^^^^^
:|M|: Julius Werner <jwerner@chromium.org>
:|G|: `jwerner-chromium`_
:|F|: drivers/console/
:|F|: include/drivers/console.h
:|F|: plat/common/aarch64/crash_console_helpers.S

coreboot support libraries
^^^^^^^^^^^^^^^^^^^^^^^^^^
:|M|: Julius Werner <jwerner@chromium.org>
:|G|: `jwerner-chromium`_
:|F|: drivers/coreboot/
:|F|: include/drivers/coreboot/
:|F|: include/lib/coreboot.h
:|F|: lib/coreboot/

eMMC/UFS drivers
^^^^^^^^^^^^^^^^
:|M|: Haojian Zhuang <haojian.zhuang@linaro.org>
:|G|: `hzhuang1`_
:|F|: drivers/partition/
:|F|: drivers/synopsys/emmc/
:|F|: drivers/synopsys/ufs/
:|F|: drivers/ufs/
:|F|: include/drivers/dw_ufs.h
:|F|: include/drivers/ufs.h
:|F|: include/drivers/synopsys/dw_mmc.h

JTAG DCC console driver
^^^^^^^^^^^^^^^^^^^^^^^
:M: Michal Simek <michal.simek@amd.com>
:G: `michalsimek`_
:M: Venkatesh Yadav Abbarapu <venkatesh.abbarapu@amd.com>
:G: `venkatesh`_
:F: drivers/arm/dcc/
:F: include/drivers/arm/dcc.h

Power State Coordination Interface (PSCI)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
:|M|: Javier Almansa Sobrino <Javier.AlmansaSobrino@arm.com>
:|G|: `javieralso-arm`_
:|M|: Madhukar Pappireddy <Madhukar.Pappireddy@arm.com>
:|G|: `madhukar-Arm`_
:|M|: Lauren Wehrmeister <Lauren.Wehrmeister@arm.com>
:|G|: `laurenw-arm`_
:|M|: Zelalem Aweke <Zelalem.Aweke@arm.com>
:|G|: `zelalem-aweke`_
:|F|: lib/psci/

DebugFS
^^^^^^^
:|M|: Olivier Deprez <olivier.deprez@arm.com>
:|G|: `odeprez`_
:|F|: lib/debugfs/

Firmware Configuration Framework (FCONF)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
:|M|: Madhukar Pappireddy <Madhukar.Pappireddy@arm.com>
:|G|: `madhukar-Arm`_
:|M|: Manish Badarkhe <manish.badarkhe@arm.com>
:|G|: `ManishVB-Arm`_
:|M|: Lauren Wehrmeister <Lauren.Wehrmeister@arm.com>
:|G|: `laurenw-arm`_
:|F|: lib/fconf/

Performance Measurement Framework (PMF)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
:|M|: Joao Alves <Joao.Alves@arm.com>
:|G|: `J-Alves`_
:|F|: lib/pmf/

Arm CPU libraries
^^^^^^^^^^^^^^^^^
:|M|: Lauren Wehrmeister <Lauren.Wehrmeister@arm.com>
:|G|: `laurenw-arm`_
:|F|: lib/cpus/

Reliability Availability Serviceabilty (RAS) framework
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
:|M|: Olivier Deprez <olivier.deprez@arm.com>
:|G|: `odeprez`_
:|M|: Manish Pandey <manish.pandey2@arm.com>
:|G|: `manish-pandey-arm`_
:|F|: lib/extensions/ras/

Activity Monitors Unit (AMU) extensions
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
:|M|: Alexei Fedorov <Alexei.Fedorov@arm.com>
:|G|: `AlexeiFedorov`_
:|M|: Chris Kay <chris.kay@arm.com>
:|G|: `CJKay`_
:|F|: lib/extensions/amu/

Memory Partitioning And Monitoring (MPAM) extensions
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
:|M|: Zelalem Aweke <Zelalem.Aweke@arm.com>
:|G|: `zelalem-aweke`_
:|F|: lib/extensions/mpam/

Pointer Authentication (PAuth) and Branch Target Identification (BTI) extensions
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
:|M|: Alexei Fedorov <Alexei.Fedorov@arm.com>
:|G|: `AlexeiFedorov`_
:|M|: Zelalem Aweke <Zelalem.Aweke@arm.com>
:|G|: `zelalem-aweke`_
:|F|: lib/extensions/pauth/

Statistical Profiling Extension (SPE)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
:|M|: Zelalem Aweke <Zelalem.Aweke@arm.com>
:|G|: `zelalem-aweke`_
:|F|: lib/extensions/spe/

Standard C library
^^^^^^^^^^^^^^^^^^
:|M|: Alexei Fedorov <Alexei.Fedorov@arm.com>
:|G|: `AlexeiFedorov`_
:|F|: lib/libc/

Library At ROM (ROMlib)
^^^^^^^^^^^^^^^^^^^^^^^
:|M|: Madhukar Pappireddy <Madhukar.Pappireddy@arm.com>
:|G|: `madhukar-Arm`_
:|F|: lib/romlib/

Translation tables (``xlat_tables``) library
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
:|M|: Javier Almansa Sobrino <Javier.AlmansaSobrino@arm.com>
:|G|: `javieralso-arm`_
:|M|: Joao Alves <Joao.Alves@arm.com>
:|G|: `J-Alves`_
:|F|: lib/xlat\_tables_\*/

IO abstraction layer
^^^^^^^^^^^^^^^^^^^^
:|M|: Manish Pandey <manish.pandey2@arm.com>
:|G|: `manish-pandey-arm`_
:|M|: Olivier Deprez <olivier.deprez@arm.com>
:|G|: `odeprez`_
:|F|: drivers/io/

GIC driver
^^^^^^^^^^
:|M|: Alexei Fedorov <Alexei.Fedorov@arm.com>
:|G|: `AlexeiFedorov`_
:|M|: Manish Pandey <manish.pandey2@arm.com>
:|G|: `manish-pandey-arm`_
:|M|: Madhukar Pappireddy <Madhukar.Pappireddy@arm.com>
:|G|: `madhukar-Arm`_
:|M|: Olivier Deprez <olivier.deprez@arm.com>
:|G|: `odeprez`_
:|F|: drivers/arm/gic/

Message Handling Unit (MHU) driver
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
:|M|: David Vincze <david.vincze@arm.com>
:|G|: `davidvincze`_
:|F|: include/drivers/arm/mhu.h
:|F|: drivers/arm/mhu

Runtime Security Subsystem (RSS) comms driver
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
:|M|: David Vincze <david.vincze@arm.com>
:|G|: `davidvincze`_
:|F|: include/drivers/arm/rss_comms.h
:|F|: drivers/arm/rss

Libfdt wrappers
^^^^^^^^^^^^^^^
:|M|: Madhukar Pappireddy <Madhukar.Pappireddy@arm.com>
:|G|: `madhukar-Arm`_
:|M|: Manish Badarkhe <manish.badarkhe@arm.com>
:|G|: `ManishVB-Arm`_
:|F|: common/fdt_wrappers.c

Firmware Encryption Framework
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
:|M|: Sumit Garg <sumit.garg@linaro.org>
:|G|: `b49020`_
:|F|: drivers/io/io_encrypted.c
:|F|: include/drivers/io/io_encrypted.h
:|F|: include/tools_share/firmware_encrypted.h

Measured Boot
^^^^^^^^^^^^^
:|M|: Alexei Fedorov <Alexei.Fedorov@arm.com>
:|G|: `AlexeiFedorov`_
:|M|: Javier Almansa Sobrino <Javier.AlmansaSobrino@arm.com>
:|G|: `javieralso-arm`_
:|M|: Sandrine Bailleux <sandrine.bailleux@arm.com>
:|G|: `sandrine-bailleux-arm`_
:|F|: drivers/measured_boot
:|F|: include/drivers/measured_boot
:|F|: docs/components/measured_boot
:|F|: plat/arm/board/fvp/fvp\*_measured_boot.c

PSA Firmware Update
^^^^^^^^^^^^^^^^^^^
:|M|: Manish Badarkhe <manish.badarkhe@arm.com>
:|G|: `ManishVB-Arm`_
:|M|: Sandrine Bailleux <sandrine.bailleux@arm.com>
:|G|: `sandrine-bailleux-arm`_
:|F|: drivers/fwu
:|F|: include/drivers/fwu

Platform Security Architecture (PSA) APIs
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
:|M|: Sandrine Bailleux <sandrine.bailleux@arm.com>
:|G|: `sandrine-bailleux-arm`_
:|F|: include/lib/psa
:|F|: lib/psa

System Control and Management Interface (SCMI) Server
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
:|M|: Etienne Carriere <etienne.carriere@st.com>
:|G|: `etienne-lms`_
:|M|: Peng Fan <peng.fan@nxp.com>
:|G|: `MrVan`_
:|F|: drivers/scmi-msg
:|F|: include/drivers/scmi\*

Max Power Mitigation Mechanism (MPMM)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
:|M|: Chris Kay <chris.kay@arm.com>
:|G|: `CJKay`_
:|F|: include/lib/mpmm/
:|F|: lib/mpmm/

Granule Protection Tables Library (GPT-RME)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
:|M|: Mark Dykes <mark.dykes@arm.com>
:|G|: `mardyk01`_
:|F|: lib/gpt_rme
:|F|: include/lib/gpt_rme

Platform Ports
~~~~~~~~~~~~~~

Allwinner ARMv8 platform port
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
:|M|: Andre Przywara <andre.przywara@arm.com>
:|G|: `Andre-ARM`_
:|M|: Samuel Holland <samuel@sholland.org>
:|G|: `smaeul`_
:|F|: docs/plat/allwinner.rst
:|F|: plat/allwinner/
:|F|: drivers/allwinner/

Amlogic Meson S905 (GXBB) platform port
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
:|M|: Andre Przywara <andre.przywara@arm.com>
:|G|: `Andre-ARM`_
:|F|: docs/plat/meson-gxbb.rst
:|F|: drivers/amlogic/
:|F|: plat/amlogic/gxbb/

Amlogic Meson S905x (GXL) platform port
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
:|M|: Remi Pommarel <repk@triplefau.lt>
:|G|: `remi-triplefault`_
:|F|: docs/plat/meson-gxl.rst
:|F|: plat/amlogic/gxl/

Amlogic Meson S905X2 (G12A) platform port
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
:|M|: Carlo Caione <ccaione@baylibre.com>
:|G|: `carlocaione`_
:|F|: docs/plat/meson-g12a.rst
:|F|: plat/amlogic/g12a/

Amlogic Meson A113D (AXG) platform port
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
:|M|: Carlo Caione <ccaione@baylibre.com>
:|G|: `carlocaione`_
:|F|: docs/plat/meson-axg.rst
:|F|: plat/amlogic/axg/

Arm FPGA platform port
^^^^^^^^^^^^^^^^^^^^^^
:|M|: Andre Przywara <andre.przywara@arm.com>
:|G|: `Andre-ARM`_
:|M|: Javier Almansa Sobrino <Javier.AlmansaSobrino@arm.com>
:|G|: `javieralso-arm`_
:|F|: plat/arm/board/arm_fpga

Arm FVP Platform port
^^^^^^^^^^^^^^^^^^^^^
:|M|: Manish Pandey <manish.pandey2@arm.com>
:|G|: `manish-pandey-arm`_
:|M|: Madhukar Pappireddy <Madhukar.Pappireddy@arm.com>
:|G|: `madhukar-Arm`_
:|F|: plat/arm/board/fvp

Arm Juno Platform port
^^^^^^^^^^^^^^^^^^^^^^
:|M|: Manish Pandey <manish.pandey2@arm.com>
:|G|: `manish-pandey-arm`_
:|M|: Chris Kay <chris.kay@arm.com>
:|G|: `CJKay`_
:|F|: plat/arm/board/juno

Arm Morello and N1SDP Platform ports
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
:|M|: Manoj Kumar <manoj.kumar3@arm.com>
:|G|: `manojkumar-arm`_
:|M|: Chandni Cherukuri <chandni.cherukuri@arm.com>
:|G|: `chandnich`_
:|F|: plat/arm/board/morello
:|F|: plat/arm/board/n1sdp

Arm Rich IoT Platform ports
^^^^^^^^^^^^^^^^^^^^^^^^^^^
:|M|: Abdellatif El Khlifi <abdellatif.elkhlifi@arm.com>
:|G|: `abdellatif-elkhlifi`_
:|M|: Vishnu Banavath <vishnu.banavath@arm.com>
:|G|: `vishnu-banavath`_
:|F|: plat/arm/board/corstone700
:|F|: plat/arm/board/a5ds
:|F|: plat/arm/board/corstone1000

Arm Reference Design platform ports
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
:|M|: Thomas Abraham <thomas.abraham@arm.com>
:|G|: `thomas-arm`_
:|M|: Vijayenthiran Subramaniam <vijayenthiran.subramaniam@arm.com>
:|G|: `vijayenthiran-arm`_
:|F|: plat/arm/css/sgi/
:|F|: plat/arm/board/rde1edge/
:|F|: plat/arm/board/rdn1edge/
:|F|: plat/arm/board/rdn2/
:|F|: plat/arm/board/rdv1/
:|F|: plat/arm/board/rdv1mc/
:|F|: plat/arm/board/sgi575/

Arm Total Compute platform port
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
:|M|: Arunachalam Ganapathy <arunachalam.ganapathy@arm.com>
:|G|: `arugan02`_
:|M|: Usama Arif <usama.arif@arm.com>
:|G|: `uarif1`_
:|F|: plat/arm/board/tc

HiSilicon HiKey and HiKey960 platform ports
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
:|M|: Haojian Zhuang <haojian.zhuang@linaro.org>
:|G|: `hzhuang1`_
:|F|: docs/plat/hikey.rst
:|F|: docs/plat/hikey960.rst
:|F|: plat/hisilicon/hikey/
:|F|: plat/hisilicon/hikey960/

HiSilicon Poplar platform port
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
:|M|: Shawn Guo <shawn.guo@linaro.org>
:|G|: `shawnguo2`_
:|F|: docs/plat/poplar.rst
:|F|: plat/hisilicon/poplar/

Intel SocFPGA platform ports
^^^^^^^^^^^^^^^^^^^^^^^^^^^^
:|M|: Sieu Mun Tang <sieu.mun.tang@intel.com>
:|G|: `sieumunt`_
:|M|: Benjamin Jit Loon Lim <jit.loon.lim@intel.com>
:|G|: `BenjaminLimJL`_
:|F|: plat/intel/soc/
:|F|: drivers/intel/soc/

MediaTek platform ports
^^^^^^^^^^^^^^^^^^^^^^^
:|M|: Rex-BC Chen <rex-bc.chen@mediatek.com>
:|G|: `mtk-rex-bc-chen`_
:|M|: Leon Chen <leon.chen@mediatek.com>
:|G|: `leon-chen-mtk`_
:|F|: docs/plat/mt\*.rst
:|F|: plat/mediatek/

Marvell platform ports and SoC drivers
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
:|M|: Konstantin Porotchkin <kostap@marvell.com>
:|G|: `kostapr`_
:|F|: docs/plat/marvell/
:|F|: plat/marvell/
:|F|: drivers/marvell/
:|F|: tools/marvell/

NVidia platform ports
^^^^^^^^^^^^^^^^^^^^^
:|M|: Varun Wadekar <vwadekar@nvidia.com>
:|G|: `vwadekar`_
:|F|: docs/plat/nvidia-tegra.rst
:|F|: include/lib/cpus/aarch64/denver.h
:|F|: lib/cpus/aarch64/denver.S
:|F|: plat/nvidia/

NXP i.MX 7 WaRP7 platform port and SoC drivers
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
:|M|: Bryan O'Donoghue <bryan.odonoghue@linaro.org>
:|G|: `bryanodonoghue`_
:|M|: Jun Nie <jun.nie@linaro.org>
:|G|: `niej`_
:|F|: docs/plat/warp7.rst
:|F|: plat/imx/common/
:|F|: plat/imx/imx7/
:|F|: drivers/imx/timer/
:|F|: drivers/imx/uart/
:|F|: drivers/imx/usdhc/

NXP i.MX 8 platform port
^^^^^^^^^^^^^^^^^^^^^^^^
:|M|: Peng Fan <peng.fan@nxp.com>
:|G|: `MrVan`_
:|F|: docs/plat/imx8.rst
:|F|: plat/imx/

NXP i.MX8M platform port
^^^^^^^^^^^^^^^^^^^^^^^^
:|M|: Jacky Bai <ping.bai@nxp.com>
:|G|: `JackyBai`_
:|F|: docs/plat/imx8m.rst
:|F|: plat/imx/imx8m/

NXP QorIQ Layerscape common code for platform ports
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
:|M|: Pankaj Gupta <pankaj.gupta@nxp.com>
:|G|: `pangupta`_
:|M|: Jiafei Pan <jiafei.pan@nxp.com>
:|G|: `JiafeiPan`_
:|F|: docs/plat/nxp/
:|F|: plat/nxp/
:|F|: drivers/nxp/
:|F|: tools/nxp/

NXP SoC Part LX2160A and its platform port
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
:|M|: Pankaj Gupta <pankaj.gupta@nxp.com>
:|G|: `pangupta`_
:|F|: plat/nxp/soc-lx2160a
:|F|: plat/nxp/soc-lx2160a/lx2162aqds
:|F|: plat/nxp/soc-lx2160a/lx2160aqds
:|F|: plat/nxp/soc-lx2160a/lx2160ardb

NXP SoC Part LS1028A and its platform port
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
:|M|: Jiafei Pan <jiafei.pan@nxp.com>
:|G|: `JiafeiPan`_
:|F|: plat/nxp/soc-ls1028a
:|F|: plat/nxp/soc-ls1028a/ls1028ardb

NXP SoC Part LS1043A and its platform port
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
:|M|: Jiafei Pan <jiafei.pan@nxp.com>
:|G|: `JiafeiPan`_
:|F|: plat/nxp/soc-ls1043a
:|F|: plat/nxp/soc-ls1043a/ls1043ardb

NXP SoC Part LS1046A and its platform port
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
:|M|: Jiafei Pan <jiafei.pan@nxp.com>
:|G|: `JiafeiPan`_
:|F|: plat/nxp/soc-ls1046a
:|F|: plat/nxp/soc-ls1046a/ls1046ardb
:|F|: plat/nxp/soc-ls1046a/ls1046afrwy
:|F|: plat/nxp/soc-ls1046a/ls1046aqds

NXP SoC Part LS1088A and its platform port
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
:|M|: Jiafei Pan <jiafei.pan@nxp.com>
:|G|: `JiafeiPan`_
:|F|: plat/nxp/soc-ls1088a
:|F|: plat/nxp/soc-ls1088a/ls1088ardb
:|F|: plat/nxp/soc-ls1088a/ls1088aqds

QEMU platform port
^^^^^^^^^^^^^^^^^^
:|M|: Jens Wiklander <jens.wiklander@linaro.org>
:|G|: `jenswi-linaro`_
:|F|: docs/plat/qemu.rst
:|F|: plat/qemu/

QTI platform port
^^^^^^^^^^^^^^^^^
:|M|: Saurabh Gorecha <sgorecha@codeaurora.org>
:|G|: `sgorecha`_
:|M|: Lachit Patel <lpatel@codeaurora.org>
:|G|: `lachitp`_
:|M|: Sreevyshanavi Kare <skare@codeaurora.org>
:|G|: `sreekare`_
:|M|: QTI TF Maintainers <qti.trustedfirmware.maintainers@codeaurora.org>
:|F|: docs/plat/qti.rst
:|F|: plat/qti/

QTI MSM8916 platform port
^^^^^^^^^^^^^^^^^^^^^^^^^
:|M|: Stephan Gerhold <stephan@gerhold.net>
:|G|: `stephan-gh`_
:|M|: Nikita Travkin <nikita@trvn.ru>
:|G|: `TravMurav`_
:|F|: docs/plat/qti-msm8916.rst
:|F|: plat/qti/msm8916/

Raspberry Pi 3 platform port
^^^^^^^^^^^^^^^^^^^^^^^^^^^^
:|M|: Ying-Chun Liu (PaulLiu) <paul.liu@linaro.org>
:|G|: `grandpaul`_
:|F|: docs/plat/rpi3.rst
:|F|: plat/rpi/rpi3/
:|F|: plat/rpi/common/
:|F|: drivers/rpi3/
:|F|: include/drivers/rpi3/

Raspberry Pi 4 platform port
^^^^^^^^^^^^^^^^^^^^^^^^^^^^
:|M|: Andre Przywara <andre.przywara@arm.com>
:|G|: `Andre-ARM`_
:|F|: docs/plat/rpi4.rst
:|F|: plat/rpi/rpi4/
:|F|: plat/rpi/common/
:|F|: drivers/rpi3/
:|F|: include/drivers/rpi3/

Renesas rcar-gen3 platform port
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
:|M|: Jorge Ramirez-Ortiz  <jramirez@baylibre.com>
:|G|: `ldts`_
:|M|: Marek Vasut <marek.vasut@gmail.com>
:|G|: `marex`_
:|F|: docs/plat/rcar-gen3.rst
:|F|: plat/renesas/common
:|F|: plat/renesas/rcar
:|F|: drivers/renesas/common
:|F|: drivers/renesas/rcar
:|F|: tools/renesas/rcar_layout_create

Renesas RZ/G2 platform port
^^^^^^^^^^^^^^^^^^^^^^^^^^^
:|M|: Biju Das <biju.das.jz@bp.renesas.com>
:|G|: `bijucdas`_
:|M|: Marek Vasut <marek.vasut@gmail.com>
:|G|: `marex`_
:|M|: Lad Prabhakar <prabhakar.mahadev-lad.rj@bp.renesas.com>
:|G|: `prabhakarlad`_
:|F|: docs/plat/rz-g2.rst
:|F|: plat/renesas/common
:|F|: plat/renesas/rzg
:|F|: drivers/renesas/common
:|F|: drivers/renesas/rzg
:|F|: tools/renesas/rzg_layout_create

RockChip platform port
^^^^^^^^^^^^^^^^^^^^^^
:|M|: Tony Xie <tony.xie@rock-chips.com>
:|G|: `TonyXie06`_
:|G|: `rockchip-linux`_
:|M|: Heiko Stuebner <heiko@sntech.de>
:|G|: `mmind`_
:|M|: Julius Werner <jwerner@chromium.org>
:|G|: `jwerner-chromium`_
:|F|: plat/rockchip/

STM32MP1 platform port
^^^^^^^^^^^^^^^^^^^^^^
:|M|: Yann Gautier <yann.gautier@st.com>
:|G|: `Yann-lms`_
:|F|: docs/plat/stm32mp1.rst
:|F|: drivers/st/
:|F|: fdts/stm32\*
:|F|: include/drivers/st/
:|F|: include/dt-bindings/\*/stm32\*
:|F|: plat/st/
:|F|: tools/stm32image/

Synquacer platform port
^^^^^^^^^^^^^^^^^^^^^^^
:|M|: Sumit Garg <sumit.garg@linaro.org>
:|G|: `b49020`_
:|F|: docs/plat/synquacer.rst
:|F|: plat/socionext/synquacer/

Texas Instruments platform port
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
:|M|: Nishanth Menon <nm@ti.com>
:|G|: `nmenon`_
:|F|: docs/plat/ti-k3.rst
:|F|: plat/ti/

UniPhier platform port
^^^^^^^^^^^^^^^^^^^^^^
:|M|: Orphan
:|F|: docs/plat/socionext-uniphier.rst
:|F|: plat/socionext/uniphier/

Xilinx platform port
^^^^^^^^^^^^^^^^^^^^
:|M|: Michal Simek <michal.simek@amd.com>
:|G|: `michalsimek`_
:|M|: Venkatesh Yadav Abbarapu <venkatesh.abbarapu@amd.com>
:|G|: `venkatesh`_
:|F|: docs/plat/xilinx-zynqmp.rst
:|F|: plat/xilinx/


Secure Payloads and Dispatchers
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

OP-TEE dispatcher
^^^^^^^^^^^^^^^^^
:|M|: Jens Wiklander <jens.wiklander@linaro.org>
:|G|: `jenswi-linaro`_
:|F|: docs/components/spd/optee-dispatcher.rst
:|F|: services/spd/opteed/

TLK
^^^^^^^^^^^^^^^^^^^^^^^^^^
:|M|: Varun Wadekar <vwadekar@nvidia.com>
:|G|: `vwadekar`_
:|F|: docs/components/spd/tlk-dispatcher.rst
:|F|: include/bl32/payloads/tlk.h
:|F|: services/spd/tlkd/

Trusty secure payloads
^^^^^^^^^^^^^^^^^^^^^^^^^^
:|M|: Arve Hjønnevåg <arve@android.com>
:|G|: `arve-android`_
:|M|: Marco Nelissen <marcone@google.com>
:|G|: `marcone`_
:|M|: Varun Wadekar <vwadekar@nvidia.com>
:|G|: `vwadekar`_
:|F|: docs/components/spd/trusty-dispatcher.rst
:|F|: services/spd/trusty/


Test Secure Payload (TSP)
^^^^^^^^^^^^^^^^^^^^^^^^^
:|M|: Manish Badarkhe <manish.badarkhe@arm.com>
:|G|: `ManishVB-Arm`_
:|F|: bl32/tsp/
:|F|: services/spd/tspd/

Tools
~~~~~

Fiptool
^^^^^^^
:|M|: Joao Alves <Joao.Alves@arm.com>
:|G|: `J-Alves`_
:|F|: tools/fiptool/

Cert_create tool
^^^^^^^^^^^^^^^^
:|M|: Sandrine Bailleux <sandrine.bailleux@arm.com>
:|G|: `sandrine-bailleux-arm`_
:|F|: tools/cert_create/

Encrypt_fw tool
^^^^^^^^^^^^^^^
:|M|: Sumit Garg <sumit.garg@linaro.org>
:|G|: `b49020`_
:|F|: tools/encrypt_fw/

Sptool
^^^^^^
:|M|: Manish Pandey <manish.pandey2@arm.com>
:|G|: `manish-pandey-arm`_
:|M|: Joao Alves <Joao.Alves@arm.com>
:|G|: `J-Alves`_
:|F|: tools/sptool/

Build system
^^^^^^^^^^^^
:|M|: Manish Pandey <manish.pandey2@arm.com>
:|G|: `manish-pandey-arm`_
:|F|: Makefile
:|F|: make_helpers/

Threat Model
~~~~~~~~~~~~~
:|M|: Zelalem Aweke <Zelalem.Aweke@arm.com>
:|G|: `zelalem-aweke`_
:|M|: Sandrine Bailleux <sandrine.bailleux@arm.com>
:|G|: `sandrine-bailleux-arm`_
:|M|: Joanna Farley <joanna.farley@arm.com>
:|G|: `joannafarley-arm`_
:|M|: Raghu Krishnamurthy <raghu.ncstate@icloud.com>
:|G|: `raghuncstate`_
:|M|: Varun Wadekar <vwadekar@nvidia.com>
:|G|: `vwadekar`_
:|F|: docs/threat_model/

Conventional Changelog Extensions
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
:|M|: Chris Kay <chris.kay@arm.com>
:|G|: `CJKay`_
:|F|: tools/conventional-changelog-tf-a

.. _AlexeiFedorov: https://github.com/AlexeiFedorov
.. _Andre-ARM: https://github.com/Andre-ARM
.. _Anson-Huang: https://github.com/Anson-Huang
.. _bijucdas: https://github.com/bijucdas
.. _bryanodonoghue: https://github.com/bryanodonoghue
.. _b49020: https://github.com/b49020
.. _carlocaione: https://github.com/carlocaione
.. _danh-arm: https://github.com/danh-arm
.. _davidvincze: https://github.com/davidvincze
.. _etienne-lms: https://github.com/etienne-lms
.. _glneo: https://github.com/glneo
.. _grandpaul: https://github.com/grandpaul
.. _hzhuang1: https://github.com/hzhuang1
.. _JackyBai: https://github.com/JackyBai
.. _jenswi-linaro: https://github.com/jenswi-linaro
.. _jwerner-chromium: https://github.com/jwerner-chromium
.. _kostapr: https://github.com/kostapr
.. _lachitp: https://github.com/lachitp
.. _ldts: https://github.com/ldts
.. _marex: https://github.com/marex
.. _masahir0y: https://github.com/masahir0y
.. _michalsimek: https://github.com/michalsimek
.. _mmind: https://github.com/mmind
.. _MrVan: https://github.com/MrVan
.. _mtk-rex-bc-chen: https://github.com/mtk-rex-bc-chen
.. _leon-chen-mtk: https://github.com/leon-chen-mtk
.. _niej: https://github.com/niej
.. _npoushin: https://github.com/npoushin
.. _prabhakarlad: https://github.com/prabhakarlad
.. _remi-triplefault: https://github.com/repk
.. _rockchip-linux: https://github.com/rockchip-linux
.. _sandrine-bailleux-arm: https://github.com/sandrine-bailleux-arm
.. _sgorecha: https://github.com/sgorecha
.. _shawnguo2: https://github.com/shawnguo2
.. _smaeul: https://github.com/smaeul
.. _soby-mathew: https://github.com/soby-mathew
.. _sreekare: https://github.com/sreekare
.. _stephan-gh: https://github.com/stephan-gh
.. _sieumunt: https://github.com/sieumunt
.. _BenjaminLimJL: https://github.com/BenjaminLimJL
.. _thomas-arm: https://github.com/thomas-arm
.. _TonyXie06: https://github.com/TonyXie06
.. _TravMurav: https://github.com/TravMurav
.. _vwadekar: https://github.com/vwadekar
.. _venkatesh: https://github.com/vabbarap
.. _Yann-lms: https://github.com/Yann-lms
.. _manish-pandey-arm: https://github.com/manish-pandey-arm
.. _mardyk01: https://github.com/mardyk01
.. _odeprez: https://github.com/odeprez
.. _bipinravi-arm: https://github.com/bipinravi-arm
.. _joannafarley-arm: https://github.com/joannafarley-arm
.. _ManishVB-Arm: https://github.com/ManishVB-Arm
.. _max-shvetsov: https://github.com/max-shvetsov
.. _javieralso-arm: https://github.com/javieralso-arm
.. _laurenw-arm: https://github.com/laurenw-arm
.. _zelalem-aweke: https://github.com/zelalem-aweke
.. _J-Alves: https://github.com/J-Alves
.. _madhukar-Arm: https://github.com/madhukar-Arm
.. _raghuncstate: https://github.com/raghuncstate
.. _CJKay: https://github.com/cjkay
.. _nmenon: https://github.com/nmenon
.. _manojkumar-arm: https://github.com/manojkumar-arm
.. _chandnich: https://github.com/chandnich
.. _abdellatif-elkhlifi: https://github.com/abdellatif-elkhlifi
.. _vishnu-banavath: https://github.com/vishnu-banavath
.. _vijayenthiran-arm: https://github.com/vijayenthiran-arm
.. _arugan02: https://github.com/arugan02
.. _uarif1: https://github.com/uarif1
.. _pangupta: https://github.com/pangupta
.. _JiafeiPan: https://github.com/JiafeiPan
.. _arve-android: https://github.com/arve-android
.. _marcone: https://github.com/marcone
.. _marcbonnici: https://github.com/marcbonnici

.. _Project Maintenance Process: https://developer.trustedfirmware.org/w/collaboration/project-maintenance-process/
