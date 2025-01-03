/*
 * Copyright (c) 2024, Texas Instruments Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <ti_sci.h>

#include <assert.h>
#include <string.h>
#include <stdio.h>

#include <plat_scmi_def.h>

#include <drivers/scmi-msg.h>
#include <drivers/scmi.h>
#include <lib/utils.h>
#include <lib/utils_def.h>

#include <clk.h>
#include <device_wrapper.h>
#include <device.h>
#include <devices.h>
#include <clocks.h>
#include <device_clk.h>
#include <device_pm.h>


#define POWER_STATE_ON  (0 << 30)
#define POWER_STATE_OFF (1 << 30)

#define xstr(s) str(s)
#define str(s) #s
#define TRUNCATE_AM62LX_DEV_ _
#define TRUNCATE(s) TRUNCATE_ ## s

struct power_domain {
	char *name;
	uint32_t id;
};

#define AM62L_POWER_DOMAIN(dev_id, _name)                                     \
{									\
	.id	= dev_id,                                               \
                .name = _name,                                          \
}

static struct power_domain scmi_power_domains[] = {
        AM62L_POWER_DOMAIN(AM62LX_DEV_ADC0, str(ADC0)),
        AM62L_POWER_DOMAIN(AM62LX_DEV_MAIN_GPIOMUX_INTROUTER0, str(MAIN_GPIOMUX_INTROUTER0)),
        AM62L_POWER_DOMAIN(AM62LX_DEV_TIMESYNC_INTROUTER0, str(TIMESYNC_INTROUTER0)),
        AM62L_POWER_DOMAIN(AM62LX_DEV_CPSW0, str(CPSW0)),
        AM62L_POWER_DOMAIN(AM62LX_DEV_CPT2_AGGR0, str(CPT2_AGGR0)),
        AM62L_POWER_DOMAIN(AM62LX_DEV_CPT2_AGGR1, str(CPT2_AGGR1)),
        AM62L_POWER_DOMAIN(AM62LX_DEV_WKUP_CPT2_AGGR0, str(WKUP_CPT2_AGGR0)),
        AM62L_POWER_DOMAIN(AM62LX_DEV_STM0, str(STM0)),
        AM62L_POWER_DOMAIN(AM62LX_DEV_DEBUGSS_WRAP0, str(DEBUGSS_WRAP0)),
        AM62L_POWER_DOMAIN(AM62LX_DEV_DMASS0, str(DMASS0)),
        AM62L_POWER_DOMAIN(AM62LX_DEV_DMASS0_BCDMA_0, str(DMASS0_BCDMA_0)),
        AM62L_POWER_DOMAIN(AM62LX_DEV_DMASS0_PKTDMA_0, str(DMASS0_PKTDMA_0)),
        AM62L_POWER_DOMAIN(AM62LX_DEV_WKUP_DMASS0, str(WKUP_DMASS0)),
        AM62L_POWER_DOMAIN(AM62LX_DEV_WKUP_DMASS0_DTHE, str(WKUP_DMASS0_DTHE)),
        AM62L_POWER_DOMAIN(AM62LX_DEV_WKUP_DMASS0_XLCDMA_0, str(WKUP_DMASS0_XLCDMA_0)),
        AM62L_POWER_DOMAIN(AM62LX_DEV_TIMER0, str(TIMER0)),
        AM62L_POWER_DOMAIN(AM62LX_DEV_TIMER1, str(TIMER1)),
        AM62L_POWER_DOMAIN(AM62LX_DEV_TIMER2, str(TIMER2)),
        AM62L_POWER_DOMAIN(AM62LX_DEV_TIMER3, str(TIMER3)),
        AM62L_POWER_DOMAIN(AM62LX_DEV_WKUP_TIMER0, str(WKUP_TIMER0)),
        AM62L_POWER_DOMAIN(AM62LX_DEV_WKUP_TIMER1, str(WKUP_TIMER1)),
        AM62L_POWER_DOMAIN(AM62LX_DEV_WKUP_SMS_LITE0, str(WKUP_SMS_LITE0)),
        AM62L_POWER_DOMAIN(AM62LX_DEV_ECAP0, str(ECAP0)),
        AM62L_POWER_DOMAIN(AM62LX_DEV_ECAP1, str(ECAP1)),
        AM62L_POWER_DOMAIN(AM62LX_DEV_ECAP2, str(ECAP2)),
        AM62L_POWER_DOMAIN(AM62LX_DEV_ELM0, str(ELM0)),
        AM62L_POWER_DOMAIN(AM62LX_DEV_MMCSD1, str(MMCSD1)),
        AM62L_POWER_DOMAIN(AM62LX_DEV_MMCSD2, str(MMCSD2)),
        AM62L_POWER_DOMAIN(AM62LX_DEV_MMCSD0, str(MMCSD0)),
        AM62L_POWER_DOMAIN(AM62LX_DEV_EQEP0, str(EQEP0)),
        AM62L_POWER_DOMAIN(AM62LX_DEV_EQEP1, str(EQEP1)),
        AM62L_POWER_DOMAIN(AM62LX_DEV_EQEP2, str(EQEP2)),
        AM62L_POWER_DOMAIN(AM62LX_DEV_FSS0, str(FSS0)),
        AM62L_POWER_DOMAIN(AM62LX_DEV_GICSS0, str(GICSS0)),
        AM62L_POWER_DOMAIN(AM62LX_DEV_GPIO0, str(GPIO0)),
        AM62L_POWER_DOMAIN(AM62LX_DEV_GPIO2, str(GPIO2)),
        AM62L_POWER_DOMAIN(AM62LX_DEV_WKUP_GPIO0, str(WKUP_GPIO0)),
        AM62L_POWER_DOMAIN(AM62LX_DEV_GPMC0, str(GPMC0)),
        AM62L_POWER_DOMAIN(AM62LX_DEV_DSS_DSI0, str(DSS_DSI0)),
        AM62L_POWER_DOMAIN(AM62LX_DEV_DSS0, str(DSS0)),
        AM62L_POWER_DOMAIN(AM62LX_DEV_EPWM0, str(EPWM0)),
        AM62L_POWER_DOMAIN(AM62LX_DEV_EPWM1, str(EPWM1)),
        AM62L_POWER_DOMAIN(AM62LX_DEV_EPWM2, str(EPWM2)),
        AM62L_POWER_DOMAIN(AM62LX_DEV_LED0, str(LED0)),
        AM62L_POWER_DOMAIN(AM62LX_DEV_PBIST0, str(PBIST0)),
        AM62L_POWER_DOMAIN(AM62LX_DEV_WKUP_PBIST0, str(WKUP_PBIST0)),
        AM62L_POWER_DOMAIN(AM62LX_DEV_WKUP_VTM0, str(WKUP_VTM0)),
        AM62L_POWER_DOMAIN(AM62LX_DEV_MCAN0, str(MCAN0)),
        AM62L_POWER_DOMAIN(AM62LX_DEV_MCAN1, str(MCAN1)),
        AM62L_POWER_DOMAIN(AM62LX_DEV_MCAN2, str(MCAN2)),
        AM62L_POWER_DOMAIN(AM62LX_DEV_MCASP0, str(MCASP0)),
        AM62L_POWER_DOMAIN(AM62LX_DEV_MCASP1, str(MCASP1)),
        AM62L_POWER_DOMAIN(AM62LX_DEV_MCASP2, str(MCASP2)),
        AM62L_POWER_DOMAIN(AM62LX_DEV_I2C0, str(I2C0)),
        AM62L_POWER_DOMAIN(AM62LX_DEV_I2C1, str(I2C1)),
        AM62L_POWER_DOMAIN(AM62LX_DEV_I2C2, str(I2C2)),
        AM62L_POWER_DOMAIN(AM62LX_DEV_I2C3, str(I2C3)),
        AM62L_POWER_DOMAIN(AM62LX_DEV_WKUP_I2C0, str(WKUP_I2C0)),
        AM62L_POWER_DOMAIN(AM62LX_DEV_WKUP_GTC0, str(WKUP_GTC0)),
        AM62L_POWER_DOMAIN(AM62LX_DEV_WKUP_RTCSS0, str(WKUP_RTCSS0)),
        AM62L_POWER_DOMAIN(AM62LX_DEV_RTI0, str(RTI0)),
        AM62L_POWER_DOMAIN(AM62LX_DEV_RTI1, str(RTI1)),
        AM62L_POWER_DOMAIN(AM62LX_DEV_DEBUGSS0, str(DEBUGSS0)),
        AM62L_POWER_DOMAIN(AM62LX_DEV_MSRAM_96K0, str(MSRAM_96K0)),
        AM62L_POWER_DOMAIN(AM62LX_DEV_WKUP_PSRAM_64K0, str(WKUP_PSRAM_64K0)),
        AM62L_POWER_DOMAIN(AM62LX_DEV_ROM0, str(ROM0)),
        AM62L_POWER_DOMAIN(AM62LX_DEV_PSC0, str(PSC0)),
        AM62L_POWER_DOMAIN(AM62LX_DEV_WKUP_DEEPSLEEP_SOURCES0, str(WKUP_DEEPSLEEP_SOURCES0)),
        AM62L_POWER_DOMAIN(AM62LX_DEV_MCU_MCU_16FF0, str(MCU_MCU_16FF0)),
        AM62L_POWER_DOMAIN(AM62LX_DEV_COMPUTE_CLUSTER0_ARM_COREPACK_0, str(ARM_COREPACK_0)),
        AM62L_POWER_DOMAIN(AM62LX_DEV_DDR16SS0, str(DDR16SS0)),
        AM62L_POWER_DOMAIN(AM62LX_DEV_WKUP_DFTSS0, str(WKUP_DFTSS0)),
        AM62L_POWER_DOMAIN(AM62LX_DEV_MCSPI0, str(MCSPI0)),
        AM62L_POWER_DOMAIN(AM62LX_DEV_MCSPI1, str(MCSPI1)),
        AM62L_POWER_DOMAIN(AM62LX_DEV_MCSPI2, str(MCSPI2)),
        AM62L_POWER_DOMAIN(AM62LX_DEV_MCSPI3, str(MCSPI3)),
        AM62L_POWER_DOMAIN(AM62LX_DEV_TRNG_DRBG_EIP76D_WRAP0, str(TRNG_DRBG_EIP76D_WRAP0)),
        AM62L_POWER_DOMAIN(AM62LX_DEV_UART1, str(UART1)),
        AM62L_POWER_DOMAIN(AM62LX_DEV_UART2, str(UART2)),
        AM62L_POWER_DOMAIN(AM62LX_DEV_UART3, str(UART3)),
        AM62L_POWER_DOMAIN(AM62LX_DEV_UART4, str(UART4)),
        AM62L_POWER_DOMAIN(AM62LX_DEV_UART5, str(UART5)),
        AM62L_POWER_DOMAIN(AM62LX_DEV_UART6, str(UART6)),
        AM62L_POWER_DOMAIN(AM62LX_DEV_WKUP_UART0, str(WKUP_UART0)),
        AM62L_POWER_DOMAIN(AM62LX_DEV_USB0, str(USB0)),
        AM62L_POWER_DOMAIN(AM62LX_DEV_USB1, str(USB1)),
        AM62L_POWER_DOMAIN(AM62LX_DEV_DPHY_TX0, str(DPHY_TX0)),
        AM62L_POWER_DOMAIN(AM62LX_DEV_COMPUTE_CLUSTER0_CLKDIV_0, str(CLKDIV_0)),
        AM62L_POWER_DOMAIN(AM62LX_DEV_COMPUTE_CLUSTER0_PBIST_0, str(PBIST_0)),
        AM62L_POWER_DOMAIN(AM62LX_DEV_UART0, str(UART0)),
        AM62L_POWER_DOMAIN(AM62LX_DEV_BOARD0, str(BOARD0)),
        AM62L_POWER_DOMAIN(AM62LX_DEV_WKUP_GTCCLK_SEL_DEV_VD, str(WKUP_GTCCLK_SEL_DEV_VD)),
        AM62L_POWER_DOMAIN(AM62LX_DEV_WKUP_OBSCLK_MUX_SEL_DEV_VD, str(WKUP_OBSCLK_MUX_SEL_DEV_VD)),
        AM62L_POWER_DOMAIN(AM62LX_DEV_WKUP_CLKOUT_SEL_DEV_VD, str(WKUP_CLKOUT_SEL_DEV_VD)),
        AM62L_POWER_DOMAIN(AM62LX_DEV_OBSCLK0_MUX_SEL_DEV_VD, str(OBSCLK0_MUX_SEL_DEV_VD)),
        AM62L_POWER_DOMAIN(AM62LX_DEV_MAIN_USB0_ISO_VD, str(USB0_ISO_VD)),
        AM62L_POWER_DOMAIN(AM62LX_DEV_MAIN_USB1_ISO_VD, str(USB1_ISO_VD)),
        AM62L_POWER_DOMAIN(AM62LX_DEV_CLK_32K_RC_SEL_DEV_VD , str(CLK_32K_RC)),
};

size_t plat_scmi_pd_count(unsigned int agent_id __unused)
{
	return ARRAY_SIZE(scmi_power_domains);
}

const char *plat_scmi_pd_get_name(unsigned int agent_id __unused,
				  unsigned int pd_id)
{
	return scmi_power_domains[pd_id].name;
}

unsigned int plat_scmi_pd_get_state(unsigned int agent_id __unused,
				    unsigned int pd_id __unused)
{
        return scmi_handler_device_state_get(scmi_power_domains[pd_id].id);
}

int32_t plat_scmi_pd_set_state(unsigned int agent_id __unused,
			       unsigned int flags,
			       unsigned int pd_id,
			       unsigned int state)
{
	int ret = SCMI_SUCCESS;
	ret = scmi_handler_device_state_get(scmi_power_domains[pd_id].id);
	/*
	 * First, check if the device state even needs to be changed, otherwise do nothing and return
	 * SCMI_SUCCESS
	 */
        if(ret == POWER_STATE_ON && state == POWER_STATE_OFF) {
		VERBOSE("\n%s: Disabling PD: agent_id = %d, pd = %d, state to set = 0x%x\n",
		__func__, agent_id, pd_id, state);
		ret = scmi_handler_device_state_set_off(scmi_power_domains[pd_id].id);
        } else if(ret == POWER_STATE_OFF && state == POWER_STATE_ON){
			VERBOSE("\n%s: Enabling PD: agent_id = %d, pd = %d, state to set = 0x%x\n",
				__func__, agent_id, pd_id, state);
                        ret = scmi_handler_device_state_set_on(scmi_power_domains[pd_id].id);
        } else {
		ret = SCMI_SUCCESS;
        }

        return ret;
}
