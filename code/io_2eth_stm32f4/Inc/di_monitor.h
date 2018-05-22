#ifndef _DI_MONITOR_H
#define _DI_MONITOR_H

#include "cmsis_os.h"
#include "sh_z_002.h"

#define SH_Z_002_DI_NUM				4
#define GET_DI_BYTE_NUM(x)			((((x) % BITS_NUM_PER_BYTE) == 0) ? \
										((x) / BITS_NUM_PER_BYTE): \
										((x) / BITS_NUM_PER_BYTE + 1))
#define SH_Z_002_DI_BYTE_NUM		GET_DI_BYTE_NUM(SH_Z_002_DI_NUM)
#define IS_DI_PIN_NUM(NUM)          (((NUM) <= 32 ))

uint32_t DI_get_DI_values(void);
#endif
