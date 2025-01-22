#include <drivers/delay_timer.h>

void osal_delay(uint64_t usecs)
{
	udelay((uint32_t)usecs);
}
