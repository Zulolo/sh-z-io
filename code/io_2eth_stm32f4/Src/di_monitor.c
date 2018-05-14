#include "stm32f4xx_hal.h"
#include <string.h>
#include "cmsis_os.h"
#include "di_monitor.h"

extern osMutexId DI_DataAccessHandle;
static uint8_t bDI_Value[SH_Z_002_DI_BYTE_NUM]; 
static GPIO_TypeDef* DI_ports[SH_Z_002_DI_NUM] = {DI_0_GPIO_Port, DI_1_GPIO_Port, DI_2_GPIO_Port, DI_3_GPIO_Port};
static uint16_t DI_pins[SH_Z_002_DI_NUM] = {DI_0_Pin, DI_1_Pin, DI_2_Pin, DI_3_Pin};

static uint32_t get_DI_values(void) {
	assert_param(IS_DI_PIN_NUM(SH_Z_002_DI_NUM));
	
}

void DI_get_DI_values(uint8_t* pDestBuf, uint8_t unDIValueLen) {
	osMutexWait(DI_DataAccessHandle, osWaitForever);
	
	memcpy(pDestBuf, bDI_Value, unDIValueLen);
	osMutexRelease(DI_DataAccessHandle);
}

void start_di_monitor(void const * argument) {
	while (1) {
		osDelay(10);
	}
}

