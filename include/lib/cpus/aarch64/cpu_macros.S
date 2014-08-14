/*
 * Copyright (c) 2014, ARM Limited and Contributors. All rights reserved.
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

#define CPU_IMPL_PN_MASK	(MIDR_IMPL_MASK << MIDR_IMPL_SHIFT) | \
				(MIDR_PN_MASK << MIDR_PN_SHIFT)

	/*
	 * Define the offsets to the fields in cpu_ops structure.
	 */
	.struct 0
CPU_MIDR: /* cpu_ops midr */
	.space  8
/* Reset fn is needed in BL at reset vector */
#if IMAGE_BL1 || (IMAGE_BL31 && RESET_TO_BL31)
CPU_RESET_FUNC: /* cpu_ops reset_func */
	.space  8
#endif
#if IMAGE_BL31 /* The power down core and cluster is needed only in BL3-1 */
CPU_PWR_DWN_CORE: /* cpu_ops core_pwr_dwn */
	.space  8
CPU_PWR_DWN_CLUSTER: /* cpu_ops cluster_pwr_dwn */
	.space  8
#endif
#if (IMAGE_BL31 && CRASH_REPORTING)
CPU_REG_DUMP: /* cpu specific register dump for crash reporting */
	.space  8
#endif
CPU_OPS_SIZE = .

	/*
	 * Convenience macro to declare cpu_ops structure.
	 * Make sure the structure fields are as per the offsets
	 * defined above.
	 */
	.macro declare_cpu_ops _name:req, _midr:req, _noresetfunc = 0
	.section cpu_ops, "a"; .align 3
	.type cpu_ops_\_name, %object
	.quad \_midr
#if IMAGE_BL1 || (IMAGE_BL31 && RESET_TO_BL31)
	.if \_noresetfunc
	.quad 0
	.else
	.quad \_name\()_reset_func
	.endif
#endif
#if IMAGE_BL31
	.quad \_name\()_core_pwr_dwn
	.quad \_name\()_cluster_pwr_dwn
#endif
#if (IMAGE_BL31 && CRASH_REPORTING)
	.quad \_name\()_cpu_reg_dump
#endif
	.endm
