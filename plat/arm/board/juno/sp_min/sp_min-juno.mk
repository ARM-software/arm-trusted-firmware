#
# Copyright (c) 2016, ARM Limited and Contributors. All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
# Redistributions of source code must retain the above copyright notice, this
# list of conditions and the following disclaimer.
#
# Redistributions in binary form must reproduce the above copyright notice,
# this list of conditions and the following disclaimer in the documentation
# and/or other materials provided with the distribution.
#
# Neither the name of ARM nor the names of its contributors may be used
# to endorse or promote products derived from this software without specific
# prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
# LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
# CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
# SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
# INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
# CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
# ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
# POSSIBILITY OF SUCH DAMAGE.
#

# SP_MIN source files specific to JUNO platform
BL32_SOURCES	+=	lib/cpus/aarch32/cortex_a53.S		\
			lib/cpus/aarch32/cortex_a57.S		\
			lib/cpus/aarch32/cortex_a72.S		\
			plat/arm/board/juno/juno_pm.c		\
			plat/arm/board/juno/juno_topology.c	\
			plat/arm/css/common/css_pm.c		\
			plat/arm/css/common/css_topology.c	\
			plat/arm/soc/common/soc_css_security.c	\
			plat/arm/css/drivers/scp/css_pm_scpi.c	\
			plat/arm/css/drivers/scpi/css_mhu.c	\
			plat/arm/css/drivers/scpi/css_scpi.c	\
			${JUNO_GIC_SOURCES}			\
			${JUNO_INTERCONNECT_SOURCES}		\
			${JUNO_SECURITY_SOURCES}

include plat/arm/common/sp_min/arm_sp_min.mk
