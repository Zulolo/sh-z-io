#include "stm32f4xx_hal.h"
#include <string.h>
#include "cmsis_os.h"
#include "di_monitor.h"

static uint8_t bDI_Value[SH_Z_002_DI_BYTE_NUM]; 

void DI_get_DI_values(uint8_t* pDestBuf, uint8_t unDIValueLen) {
	// Mutex get
	
	memcpy(pDestBuf, bDI_Value, unDIValueLen);
	// Mutex release
}

void start_di_monitor(void const * argument) {
	while (1) {
		osDelay(10);
	}
}

