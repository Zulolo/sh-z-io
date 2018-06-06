#ifndef _DI_MONITOR_H
#define _DI_MONITOR_H

#include "cmsis_os.h"
#include "sh_z_002.h"

#define GET_DI_BYTE_NUM(x)			((((x) % BITS_NUM_PER_BYTE) == 0) ? \
										((x) / BITS_NUM_PER_BYTE): \
										((x) / BITS_NUM_PER_BYTE + 1))
#define SH_Z_002_DI_BYTE_NUM		GET_DI_BYTE_NUM(SH_Z_002_DI_NUM)
#define IS_DI_PIN_NUM(NUM)          (((NUM) <= 32 ))

typedef struct {
	uint8_t bEnableCNT : 1;
	uint8_t bClearCNT : 1;
	uint8_t bClearOverflow : 1;
	uint8_t bLatchStatus : 1;
	uint8_t bLatchSet : 1;	// 1 means latch on rising edge, 0 on falling
	uint8_t NA : 3;
}DI_ConfTypeDef;

uint32_t DI_get_DI_values(void);
int DI_get_DI_conf(DI_ConfTypeDef * tDI_ConfBuf, uint32_t unBitOffset, uint32_t unBitNum);
#endif
