#include <plat/common/platform.h>

#include <sunxi_cpucfg_ncat.h>

static inline bool sunxi_cpucfg_has_per_cluster_regs(void)
{
	return (plat_get_soc_revision() != 2);
}
