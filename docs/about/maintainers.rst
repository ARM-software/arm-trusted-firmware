Project Maintenance
===================

Trusted Firmware-A (TF-A) is an open governance community project. All
contributions are ultimately merged by the maintainers listed below. Technical
ownership of most parts of the codebase falls on the code owners listed
below. An acknowledgement from these code owners is required before the
maintainers merge a contribution.

More details may be found in the `Project Maintenance Process`_ document.


.. _maintainers:

Maintainers
-----------

:M: Dan Handley <dan.handley@arm.com>
:G: `danh-arm`_
:M: Soby Mathew <soby.mathew@arm.com>
:G: `soby-mathew`_
:M: Sandrine Bailleux <sandrine.bailleux@arm.com>
:G: `sandrine-bailleux-arm`_
:M: Alexei Fedorov <Alexei.Fedorov@arm.com>
:G: `AlexeiFedorov`_
:M: Manish Pandey <manish.pandey2@arm.com>
:G: `manish-pandey-arm`_
:M: Mark Dykes <mark.dykes@arm.com>
:G: `mardyk01`_
:M: Olivier Deprez <olivier.deprez@arm.com>
:G: `odeprez`_
:M: Bipin Ravi <bipin.ravi@arm.com>
:G: `bipinravi-arm`_
:M: Joanna Farley <joanna.farley@arm.com>
:G: `joannafarley-arm`_
:M: Julius Werner <jwerner@chromium.org>
:G: `jwerner-chromium`_
:M: Varun Wadekar <vwadekar@nvidia.com>
:G: `vwadekar`_
:M: Andre Przywara <andre.przywara@arm.com>
:G: `Andre-ARM`_
:M: Lauren Wehrmeister <Lauren.Wehrmeister@arm.com>
:G: `laurenw-arm`_
:M: Madhukar Pappireddy <Madhukar.Pappireddy@arm.com>
:G: `madhukar-Arm`_
:M: Raghu Krishnamurthy <raghu.ncstate@icloud.com>
:G: `raghuncstate`_


.. _code owners:

Code owners
-----------

Core Code
~~~~~~~~~

Armv7-A architecture port
^^^^^^^^^^^^^^^^^^^^^^^^^
:M: Etienne Carriere <etienne.carriere@linaro.org>
:G: `etienne-lms`_

Build Definitions for CMake Build System
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
:M: Javier Almansa Sobrino <Javier.AlmansaSobrino@arm.com>
:G: `javieralso-arm`_
:M: Chris Kay <chris.kay@arm.com>
:G: `CJkay`_
:F: /

Software Delegated Exception Interface (SDEI)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
:M: Mark Dykes <mark.dykes@arm.com>
:G: `mardyk01`_
:M: John Powell <John.Powell@arm.com>
:G: `john-powell-arm`_
:F: services/std_svc/sdei/

Trusted Boot
^^^^^^^^^^^^
:M: Sandrine Bailleux <sandrine.bailleux@arm.com>
:G: `sandrine-bailleux-arm`_
:M: Manish Pandey <manish.pandey2@arm.com>
:G: `manish-pandey-arm`_
:M: Manish Badarkhe <manish.badarkhe@arm.com>
:G: `ManishVB-Arm`_
:F: drivers/auth/

Secure Partition Manager (SPM)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
:M: Olivier Deprez <olivier.deprez@arm.com>
:G: `odeprez`_
:M: Manish Pandey <manish.pandey2@arm.com>
:G: `manish-pandey-arm`_
:M: Maksims Svecovs <maksims.svecovs@arm.com>
:G: `max-shvetsov`_
:M: Joao Alves <Joao.Alves@arm.com>
:G: `J-Alves`_
:F: services/std_svc/spm\*

Exception Handling Framework (EHF)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
:M: Manish Badarkhe <manish.badarkhe@arm.com>
:G: `ManishVB-Arm`_
:M: John Powell <John.Powell@arm.com>
:G: `john-powell-arm`_
:F: bl31/ehf.c


Drivers, Libraries and Framework Code
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Console API framework
^^^^^^^^^^^^^^^^^^^^^
:M: Julius Werner <jwerner@chromium.org>
:G: `jwerner-chromium`_
:F: drivers/console/
:F: include/drivers/console.h
:F: plat/common/aarch64/crash_console_helpers.S

coreboot support libraries
^^^^^^^^^^^^^^^^^^^^^^^^^^
:M: Julius Werner <jwerner@chromium.org>
:G: `jwerner-chromium`_
:F: drivers/coreboot/
:F: include/drivers/coreboot/
:F: include/lib/coreboot.h
:F: lib/coreboot/

eMMC/UFS drivers
^^^^^^^^^^^^^^^^
:M: Haojian Zhuang <haojian.zhuang@linaro.org>
:G: `hzhuang1`_
:F: drivers/partition/
:F: drivers/synopsys/emmc/
:F: drivers/synopsys/ufs/
:F: drivers/ufs/
:F: include/drivers/dw_ufs.h
:F: include/drivers/ufs.h
:F: include/drivers/synopsys/dw_mmc.h

Power State Coordination Interface (PSCI)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
:M: Javier Almansa Sobrino <Javier.AlmansaSobrino@arm.com>
:G: `javieralso-arm`_
:M: Madhukar Pappireddy <Madhukar.Pappireddy@arm.com>
:G: `madhukar-Arm`_
:M: Lauren Wehrmeister <Lauren.Wehrmeister@arm.com>
:G: `laurenw-arm`_
:M: Zelalem Aweke <Zelalem.Aweke@arm.com>
:G: `zelalem-aweke`_
:F: lib/psci/

DebugFS
^^^^^^^
:M: Olivier Deprez <olivier.deprez@arm.com>
:G: `odeprez`_
:F: lib/debugfs/

Firmware Configuration Framework (FCONF)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
:M: Madhukar Pappireddy <Madhukar.Pappireddy@arm.com>
:G: `madhukar-Arm`_
:M: Manish Badarkhe <manish.badarkhe@arm.com>
:G: `ManishVB-Arm`_
:M: Lauren Wehrmeister <Lauren.Wehrmeister@arm.com>
:G: `laurenw-arm`_
:F: lib/fconf/

Performance Measurement Framework (PMF)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
:M: Joao Alves <Joao.Alves@arm.com>
:G: `J-Alves`_
:M: Jimmy Brisson <Jimmy.Brisson@arm.com>
:G: `theotherjimmy`_
:F: lib/pmf/

Arm CPU libraries
^^^^^^^^^^^^^^^^^
:M: Lauren Wehrmeister <Lauren.Wehrmeister@arm.com>
:G: `laurenw-arm`_
:M: John Powell <John.Powell@arm.com>
:G: `john-powell-arm`_
:F: lib/cpus/

Reliability Availability Serviceabilty (RAS) framework
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
:M: Olivier Deprez <olivier.deprez@arm.com>
:G: `odeprez`_
:M: Manish Pandey <manish.pandey2@arm.com>
:G: `manish-pandey-arm`_
:F: lib/extensions/ras/

Activity Monitors Unit (AMU) extensions
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
:M: Alexei Fedorov <Alexei.Fedorov@arm.com>
:G: `AlexeiFedorov`_
:F: lib/extensions/amu/

Memory Partitioning And Monitoring (MPAM) extensions
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
:M: Zelalem Aweke <Zelalem.Aweke@arm.com>
:G: `zelalem-aweke`_
:M: Jimmy Brisson <Jimmy.Brisson@arm.com>
:G: `theotherjimmy`_
:F: lib/extensions/mpam/

Pointer Authentication (PAuth) and Branch Target Identification (BTI) extensions
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
:M: Alexei Fedorov <Alexei.Fedorov@arm.com>
:G: `AlexeiFedorov`_
:M: Zelalem Aweke <Zelalem.Aweke@arm.com>
:G: `zelalem-aweke`_
:F: lib/extensions/pauth/

Statistical Profiling Extension (SPE)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
:M: Zelalem Aweke <Zelalem.Aweke@arm.com>
:G: `zelalem-aweke`_
:M: Jimmy Brisson <Jimmy.Brisson@arm.com>
:G: `theotherjimmy`_
:F: lib/extensions/spe/

Scalable Vector Extension (SVE)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
:M: Jimmy Brisson <Jimmy.Brisson@arm.com>
:G: `theotherjimmy`_
:F: lib/extensions/sve/

Standard C library
^^^^^^^^^^^^^^^^^^
:M: Alexei Fedorov <Alexei.Fedorov@arm.com>
:G: `AlexeiFedorov`_
:M: John Powell <John.Powell@arm.com>
:G: `john-powell-arm`_
:F: lib/libc/

Library At ROM (ROMlib)
^^^^^^^^^^^^^^^^^^^^^^^
:M: Madhukar Pappireddy <Madhukar.Pappireddy@arm.com>
:G: `madhukar-Arm`_
:F: lib/romlib/

Translation tables (``xlat_tables``) library
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
:M: Javier Almansa Sobrino <Javier.AlmansaSobrino@arm.com>
:G: `javieralso-arm`_
:M: Joao Alves <Joao.Alves@arm.com>
:G: `J-Alves`_
:F: lib/xlat\_tables_\*/

IO abstraction layer
^^^^^^^^^^^^^^^^^^^^
:M: Manish Pandey <manish.pandey2@arm.com>
:G: `manish-pandey-arm`_
:M: Olivier Deprez <olivier.deprez@arm.com>
:G: `odeprez`_
:F: drivers/io/

GIC driver
^^^^^^^^^^
:M: Alexei Fedorov <Alexei.Fedorov@arm.com>
:G: `AlexeiFedorov`_
:M: Manish Pandey <manish.pandey2@arm.com>
:G: `manish-pandey-arm`_
:M: Madhukar Pappireddy <Madhukar.Pappireddy@arm.com>
:G: `madhukar-Arm`_
:M: Olivier Deprez <olivier.deprez@arm.com>
:G: `odeprez`_
:F: drivers/arm/gic/

Libfdt wrappers
^^^^^^^^^^^^^^^
:M: Madhukar Pappireddy <Madhukar.Pappireddy@arm.com>
:G: `madhukar-Arm`_
:M: Manish Badarkhe <manish.badarkhe@arm.com>
:G: `ManishVB-Arm`_
:F: common/fdt_wrappers.c

Firmware Encryption Framework
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
:M: Sumit Garg <sumit.garg@linaro.org>
:G: `b49020`_
:F: drivers/io/io_encrypted.c
:F: include/drivers/io/io_encrypted.h
:F: include/tools_share/firmware_encrypted.h

Measured Boot
^^^^^^^^^^^^^
:M: Alexei Fedorov <Alexei.Fedorov@arm.com>
:G: `AlexeiFedorov`_
:M: Javier Almansa Sobrino <Javier.AlmansaSobrino@arm.com>
:G: `javieralso-arm`_
:F: drivers/measured_boot
:F: include/drivers/measured_boot
:F: plat/arm/board/fvp/fvp_measured_boot.c

Platform Ports
~~~~~~~~~~~~~~

Allwinner ARMv8 platform port
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
:M: Andre Przywara <andre.przywara@arm.com>
:G: `Andre-ARM`_
:M: Samuel Holland <samuel@sholland.org>
:G: `smaeul`_
:F: docs/plat/allwinner.rst
:F: plat/allwinner/
:F: drivers/allwinner/

Amlogic Meson S905 (GXBB) platform port
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
:M: Andre Przywara <andre.przywara@arm.com>
:G: `Andre-ARM`_
:F: docs/plat/meson-gxbb.rst
:F: drivers/amlogic/
:F: plat/amlogic/gxbb/

Amlogic Meson S905x (GXL) platform port
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
:M: Remi Pommarel <repk@triplefau.lt>
:G: `remi-triplefault`_
:F: docs/plat/meson-gxl.rst
:F: plat/amlogic/gxl/

Amlogic Meson S905X2 (G12A) platform port
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
:M: Carlo Caione <ccaione@baylibre.com>
:G: `carlocaione`_
:F: docs/plat/meson-g12a.rst
:F: plat/amlogic/g12a/

Amlogic Meson A113D (AXG) platform port
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
:M: Carlo Caione <ccaione@baylibre.com>
:G: `carlocaione`_
:F: docs/plat/meson-axg.rst
:F: plat/amlogic/axg/

Arm FPGA platform port
^^^^^^^^^^^^^^^^^^^^^^
:M: Andre Przywara <andre.przywara@arm.com>
:G: `Andre-ARM`_
:M: Javier Almansa Sobrino <Javier.AlmansaSobrino@arm.com>
:G: `javieralso-arm`_
:F: plat/arm/board/arm_fpga

Arm System Guidance for Infrastructure / Mobile FVP platforms
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
:M: Nariman Poushin <nariman.poushin@linaro.org>
:G: `npoushin`_
:M: Thomas Abraham <thomas.abraham@arm.com>
:G: `thomas-arm`_
:F: plat/arm/css/sgi/
:F: plat/arm/css/sgm/
:F: plat/arm/board/sgi575/
:F: plat/arm/board/sgm775/

HiSilicon HiKey and HiKey960 platform ports
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
:M: Haojian Zhuang <haojian.zhuang@linaro.org>
:G: `hzhuang1`_
:F: docs/plat/hikey.rst
:F: docs/plat/hikey960.rst
:F: plat/hisilicon/hikey/
:F: plat/hisilicon/hikey960/

HiSilicon Poplar platform port
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
:M: Shawn Guo <shawn.guo@linaro.org>
:G: `shawnguo2`_
:F: docs/plat/poplar.rst
:F: plat/hisilicon/poplar/

Intel SocFPGA platform ports
^^^^^^^^^^^^^^^^^^^^^^^^^^^^
:M: Tien Hock Loh <tien.hock.loh@intel.com>
:G: `thloh85-intel`_
:M: Hadi Asyrafi <muhammad.hadi.asyrafi.abdul.halim@intel.com>
:G: mabdulha
:F: plat/intel/soc
:F: drivers/intel/soc/

MediaTek platform ports
^^^^^^^^^^^^^^^^^^^^^^^
:M: Yidi Lin (林以廸) <yidi.lin@mediatek.com>
:G: `mtk09422`_
:F: plat/mediatek/

Marvell platform ports and SoC drivers
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
:M: Konstantin Porotchkin <kostap@marvell.com>
:G: `kostapr`_
:F: docs/plat/marvell/
:F: plat/marvell/
:F: drivers/marvell/
:F: tools/marvell/

NVidia platform ports
^^^^^^^^^^^^^^^^^^^^^
:M: Varun Wadekar <vwadekar@nvidia.com>
:G: `vwadekar`_
:F: docs/plat/nvidia-tegra.rst
:F: include/lib/cpus/aarch64/denver.h
:F: lib/cpus/aarch64/denver.S
:F: plat/nvidia/

NXP QorIQ Layerscape platform ports
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
:M: Jiafei Pan <jiafei.pan@nxp.com>
:G: `qoriq-open-source`_
:F: docs/plat/ls1043a.rst
:F: plat/layerscape/

NXP i.MX 7 WaRP7 platform port and SoC drivers
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
:M: Bryan O'Donoghue <bryan.odonoghue@linaro.org>
:G: `bryanodonoghue`_
:M: Jun Nie <jun.nie@linaro.org>
:G: `niej`_
:F: docs/plat/warp7.rst
:F: plat/imx/common/
:F: plat/imx/imx7/
:F: drivers/imx/timer/
:F: drivers/imx/uart/
:F: drivers/imx/usdhc/

NXP i.MX 8 platform port
^^^^^^^^^^^^^^^^^^^^^^^^
:M: Anson Huang <Anson.Huang@nxp.com>
:G: `Anson-Huang`_
:F: docs/plat/imx8.rst
:F: plat/imx/

NXP i.MX8M platform port
^^^^^^^^^^^^^^^^^^^^^^^^
:M: Jacky Bai <ping.bai@nxp.com>
:G: `JackyBai`_
:F: docs/plat/imx8m.rst
:F: plat/imx/imx8m/

QEMU platform port
^^^^^^^^^^^^^^^^^^
:M: Jens Wiklander <jens.wiklander@linaro.org>
:G: `jenswi-linaro`_
:F: docs/plat/qemu.rst
:F: plat/qemu/

QTI platform port
^^^^^^^^^^^^^^^^^
:M: Saurabh Gorecha <sgorecha@codeaurora.org>
:G: `sgorecha`_
:M: Debasish Mandal <dmandal@codeaurora.org>
:M: QTI TF Maintainers <qti.trustedfirmware.maintainers@codeaurora.org>
:F: docs/plat/qti.rst
:F: plat/qti/

Raspberry Pi 3 platform port
^^^^^^^^^^^^^^^^^^^^^^^^^^^^
:M: Ying-Chun Liu (PaulLiu) <paul.liu@linaro.org>
:G: `grandpaul`_
:F: docs/plat/rpi3.rst
:F: plat/rpi/rpi3/
:F: plat/rpi/common/
:F: drivers/rpi3/
:F: include/drivers/rpi3/

Raspberry Pi 4 platform port
^^^^^^^^^^^^^^^^^^^^^^^^^^^^
:M: Andre Przywara <andre.przywara@arm.com>
:G: `Andre-ARM`_
:F: docs/plat/rpi4.rst
:F: plat/rpi/rpi4/
:F: plat/rpi/common/
:F: drivers/rpi3/
:F: include/drivers/rpi3/

Renesas rcar-gen3 platform port
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
:M: Jorge Ramirez-Ortiz  <jramirez@baylibre.com>
:G: `ldts`_
:M: Marek Vasut <marek.vasut@gmail.com>
:G: `marex`_
:F: docs/plat/rcar-gen3.rst
:F: plat/renesas/rcar
:F: drivers/renesas/rcar
:F: tools/renesas/rcar_layout_create

RockChip platform port
^^^^^^^^^^^^^^^^^^^^^^
:M: Tony Xie <tony.xie@rock-chips.com>
:G: `TonyXie06`_
:G: `rockchip-linux`_
:M: Heiko Stuebner <heiko@sntech.de>
:G: `mmind`_
:F: plat/rockchip/

STM32MP1 platform port
^^^^^^^^^^^^^^^^^^^^^^
:M: Yann Gautier <yann.gautier@st.com>
:G: `Yann-lms`_
:F: docs/plat/stm32mp1.rst
:F: drivers/st/
:F: fdts/stm32\*
:F: include/drivers/st/
:F: include/dt-bindings/\*/stm32\*
:F: plat/st/
:F: tools/stm32image/

Synquacer platform port
^^^^^^^^^^^^^^^^^^^^^^^
:M: Sumit Garg <sumit.garg@linaro.org>
:G: `b49020`_
:F: docs/plat/synquacer.rst
:F: plat/socionext/synquacer/

Texas Instruments platform port
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
:M: Andrew F. Davis <afd@ti.com>
:G: `glneo`_
:F: docs/plat/ti-k3.rst
:F: plat/ti/

UniPhier platform port
^^^^^^^^^^^^^^^^^^^^^^
:M: Orphan
:F: docs/plat/socionext-uniphier.rst
:F: plat/socionext/uniphier/

Xilinx platform port
^^^^^^^^^^^^^^^^^^^^
:M: Siva Durga Prasad Paladugu <siva.durga.paladugu@xilinx.com>
:G: `sivadur`_
:F: docs/plat/xilinx-zynqmp.rst
:F: plat/xilinx/


Secure Payloads and Dispatchers
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

OP-TEE dispatcher
^^^^^^^^^^^^^^^^^
:M: Jens Wiklander <jens.wiklander@linaro.org>
:G: `jenswi-linaro`_
:F: docs/components/spd/optee-dispatcher.rst
:F: services/spd/opteed/

TLK/Trusty secure payloads
^^^^^^^^^^^^^^^^^^^^^^^^^^
:M: Varun Wadekar <vwadekar@nvidia.com>
:G: `vwadekar`_
:F: docs/components/spd/tlk-dispatcher.rst
:F: docs/components/spd/trusty-dispatcher.rst
:F: include/bl32/payloads/tlk.h
:F: services/spd/tlkd/
:F: services/spd/trusty/

Test Secure Payload (TSP)
^^^^^^^^^^^^^^^^^^^^^^^^^
:M: Manish Badarkhe <manish.badarkhe@arm.com>
:G: `ManishVB-Arm`_
:F: bl32/tsp/
:F: services/spd/tspd/

Tools
~~~~~

Fiptool
^^^^^^^
:M: Joao Alves <Joao.Alves@arm.com>
:G: `J-Alves`_
:F: tools/fiptool/

Cert_create tool
^^^^^^^^^^^^^^^^
:M: Sandrine Bailleux <sandrine.bailleux@arm.com>
:G: `sandrine-bailleux-arm`_
:F: tools/cert_create/

Encrypt_fw tool
^^^^^^^^^^^^^^^
:M: Sumit Garg <sumit.garg@linaro.org>
:G: `b49020`_
:F: tools/encrypt_fw/

Sptool
^^^^^^
:M: Manish Pandey <manish.pandey2@arm.com>
:G: `manish-pandey-arm`_
:F: tools/sptool/

Build system
^^^^^^^^^^^^
:M: Manish Pandey <manish.pandey2@arm.com>
:G: `manish-pandey-arm`_
:F: Makefile
:F: make_helpers/

.. _AlexeiFedorov: https://github.com/AlexeiFedorov
.. _Andre-ARM: https://github.com/Andre-ARM
.. _Anson-Huang: https://github.com/Anson-Huang
.. _bryanodonoghue: https://github.com/bryanodonoghue
.. _b49020: https://github.com/b49020
.. _carlocaione: https://github.com/carlocaione
.. _danh-arm: https://github.com/danh-arm
.. _etienne-lms: https://github.com/etienne-lms
.. _glneo: https://github.com/glneo
.. _grandpaul: https://github.com/grandpaul
.. _hzhuang1: https://github.com/hzhuang1
.. _JackyBai: https://github.com/JackyBai
.. _jenswi-linaro: https://github.com/jenswi-linaro
.. _jwerner-chromium: https://github.com/jwerner-chromium
.. _kostapr: https://github.com/kostapr
.. _ldts: https://github.com/ldts
.. _marex: https://github.com/marex
.. _masahir0y: https://github.com/masahir0y
.. _mmind: https://github.com/mmind
.. _mtk09422: https://github.com/mtk09422
.. _niej: https://github.com/niej
.. _npoushin: https://github.com/npoushin
.. _qoriq-open-source: https://github.com/qoriq-open-source
.. _remi-triplefault: https://github.com/repk
.. _rockchip-linux: https://github.com/rockchip-linux
.. _sandrine-bailleux-arm: https://github.com/sandrine-bailleux-arm
.. _sgorecha: https://github.com/sgorecha
.. _shawnguo2: https://github.com/shawnguo2
.. _sivadur: https://github.com/sivadur
.. _smaeul: https://github.com/smaeul
.. _soby-mathew: https://github.com/soby-mathew
.. _thloh85-intel: https://github.com/thloh85-intel
.. _thomas-arm: https://github.com/thomas-arm
.. _TonyXie06: https://github.com/TonyXie06
.. _vwadekar: https://github.com/vwadekar
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
.. _theotherjimmy: https://github.com/theotherjimmy
.. _J-Alves: https://github.com/J-Alves
.. _madhukar-Arm: https://github.com/madhukar-Arm
.. _john-powell-arm: https://github.com/john-powell-arm
.. _raghuncstate: https://github.com/raghuncstate
.. _CJKay: https://github.com/cjkay

.. _Project Maintenance Process: https://developer.trustedfirmware.org/w/collaboration/project-maintenance-process/
