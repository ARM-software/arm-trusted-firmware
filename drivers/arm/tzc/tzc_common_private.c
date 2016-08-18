/*
 * Copyright (c) 2016, ARM Limited and Contributors. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * Neither the name of ARM nor the names of its contributors may be used
 * to endorse or promote products derived from this software without specific
 * prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <arch.h>
#include <arch_helpers.h>
#include <mmio.h>
#include <tzc_common.h>

#define DEFINE_TZC_COMMON_WRITE_ACTION(fn_name, macro_name)		\
	static inline void _tzc##fn_name##_write_action(		\
					uintptr_t base,			\
					tzc_action_t action)		\
	{								\
		mmio_write_32(base + TZC_##macro_name##_ACTION_OFF,	\
			action);					\
	}

#define DEFINE_TZC_COMMON_WRITE_REGION_BASE(fn_name, macro_name)	\
	static inline void _tzc##fn_name##_write_region_base(		\
					uintptr_t base,			\
					int region_no,			\
					unsigned long long region_base)	\
	{								\
		mmio_write_32(base +					\
			TZC_REGION_OFFSET(				\
				TZC_##macro_name##_REGION_SIZE,		\
				region_no) +				\
			TZC_##macro_name##_REGION_BASE_LOW_0_OFFSET,	\
			(uint32_t)region_base);				\
		mmio_write_32(base +					\
			TZC_REGION_OFFSET(				\
				TZC_##macro_name##_REGION_SIZE,		\
				region_no) +				\
			TZC_##macro_name##_REGION_BASE_HIGH_0_OFFSET,	\
			(uint32_t)(region_base >> 32));			\
	}

#define DEFINE_TZC_COMMON_WRITE_REGION_TOP(fn_name, macro_name)		\
	static inline void _tzc##fn_name##_write_region_top(		\
					uintptr_t base,			\
					int region_no,			\
					unsigned long long region_top)	\
	{								\
		mmio_write_32(base +					\
			TZC_REGION_OFFSET				\
				(TZC_##macro_name##_REGION_SIZE,	\
				region_no) +				\
			TZC_##macro_name##_REGION_TOP_LOW_0_OFFSET,	\
			(uint32_t)region_top);			\
		mmio_write_32(base +					\
			TZC_REGION_OFFSET(				\
				TZC_##macro_name##_REGION_SIZE,		\
				region_no) +				\
			TZC_##macro_name##_REGION_TOP_HIGH_0_OFFSET,	\
			(uint32_t)(region_top >> 32));		\
	}

#define DEFINE_TZC_COMMON_WRITE_REGION_ATTRIBUTES(fn_name, macro_name)	\
	static inline void _tzc##fn_name##_write_region_attributes(	\
						uintptr_t base,		\
						int region_no,		\
						unsigned int attr)	\
	{								\
		mmio_write_32(base +					\
			TZC_REGION_OFFSET(				\
				TZC_##macro_name##_REGION_SIZE,		\
				region_no) +				\
			TZC_##macro_name##_REGION_ATTR_0_OFFSET,	\
			attr);						\
	}

#define DEFINE_TZC_COMMON_WRITE_REGION_ID_ACCESS(fn_name, macro_name)	\
	static inline void _tzc##fn_name##_write_region_id_access(	\
						uintptr_t base,		\
						int region_no,		\
						unsigned int val)	\
	{								\
		mmio_write_32(base +					\
			TZC_REGION_OFFSET(				\
				TZC_##macro_name##_REGION_SIZE,		\
				region_no) +				\
			TZC_##macro_name##_REGION_ID_ACCESS_0_OFFSET,	\
			val);						\
	}

/*
 * It is used to program region 0 ATTRIBUTES and ACCESS register.
 */
#define DEFINE_TZC_COMMON_CONFIGURE_REGION0(fn_name)			\
	void _tzc##fn_name##_configure_region0(uintptr_t base,		\
			   tzc_region_attributes_t sec_attr,		\
			   unsigned int ns_device_access)		\
	{								\
		assert(base);						\
		VERBOSE("TrustZone : Configuring region 0 "		\
			"(TZC Interface Base=%p sec_attr=0x%x,"		\
			" ns_devs=0x%x)\n", (void *)base,		\
			sec_attr, ns_device_access);			\
									\
		/* Set secure attributes on region 0 */			\
		_tzc##fn_name##_write_region_attributes(base, 0,	\
			sec_attr << TZC_REGION_ATTR_SEC_SHIFT);		\
									\
		/***************************************************/	\
		/* Specify which non-secure devices have permission*/	\
		/* to access region 0.				   */	\
		/***************************************************/	\
		_tzc##fn_name##_write_region_id_access(base,		\
						0,			\
						ns_device_access);	\
	}

/*
 * It is used to program a region from 1 to 8 in the TrustZone controller.
 * NOTE:
 * Region 0 is special; it is preferable to use
 * ##fn_name##_configure_region0 for this region (see comment for
 * that function).
 */
#define DEFINE_TZC_COMMON_CONFIGURE_REGION(fn_name)			\
	void _tzc##fn_name##_configure_region(uintptr_t base,		\
				unsigned int filters,			\
				int region_no,				\
				unsigned long long region_base,		\
				unsigned long long region_top,		\
				tzc_region_attributes_t sec_attr,	\
				unsigned int nsaid_permissions)	\
	{								\
		assert(base);						\
		VERBOSE("TrustZone : Configuring region "		\
			"(TZC Interface Base: %p, region_no = %d)"	\
			"...\n", (void *)base, region_no);		\
		VERBOSE("TrustZone : ... base = %llx, top = %llx,"	\
			"\n", region_base, region_top);\
		VERBOSE("TrustZone : ... sec_attr = 0x%x,"		\
			" ns_devs = 0x%x)\n",				\
			sec_attr, nsaid_permissions);			\
									\
		/***************************************************/	\
		/* Inputs look ok, start programming registers.    */	\
		/* All the address registers are 32 bits wide and  */	\
		/* have a LOW and HIGH				   */	\
		/* component used to construct an address up to a  */	\
		/* 64bit.					   */	\
		/***************************************************/	\
		_tzc##fn_name##_write_region_base(base,			\
					region_no, region_base);	\
		_tzc##fn_name##_write_region_top(base,			\
					region_no, region_top);		\
									\
		/* Enable filter to the region and set secure attributes */\
		_tzc##fn_name##_write_region_attributes(base,		\
				region_no,				\
				(sec_attr << TZC_REGION_ATTR_SEC_SHIFT) |\
				(filters << TZC_REGION_ATTR_F_EN_SHIFT));\
									\
		/***************************************************/	\
		/* Specify which non-secure devices have permission*/	\
		/* to access this region.			   */	\
		/***************************************************/	\
		_tzc##fn_name##_write_region_id_access(base,		\
						region_no,		\
						nsaid_permissions);	\
	}

#if DEBUG
static unsigned int _tzc_read_peripheral_id(uintptr_t base)
{
	unsigned int id;

	id = mmio_read_32(base + PID0_OFF);
	/* Masks DESC part in PID1 */
	id |= ((mmio_read_32(base + PID1_OFF) & 0xF) << 8);

	return id;
}

#ifdef AARCH32
static unsigned long long _tzc_get_max_top_addr(int addr_width)
{
	/*
	 * Assume at least 32 bit wide address and initialize the max.
	 * This function doesn't use 64-bit integer arithmetic to avoid
	 * having to implement additional compiler library functions.
	 */
	unsigned long long addr_mask = 0xFFFFFFFF;
	uint32_t *addr_ptr = (uint32_t *)&addr_mask;

	assert(addr_width >= 32);

	/* This logic works only on little - endian platforms */
	assert((read_sctlr() & SCTLR_EE_BIT) == 0);

	/*
	 * If required address width is greater than 32, populate the higher
	 * 32 bits of the 64 bit field with the max address.
	 */
	if (addr_width > 32)
		*(addr_ptr + 1) = ((1 << (addr_width - 32)) - 1);

	return addr_mask;
}
#else
#define _tzc_get_max_top_addr(addr_width)\
	(UINT64_MAX >> (64 - (addr_width)))
#endif /* AARCH32 */

#endif
