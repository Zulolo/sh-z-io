#include "stm32f4xx_hal.h"
#include <string.h>
#include "cmsis_os.h"
#include "di_monitor.h"

#define DI_SAMPLE_INTERVAL			20
#define DI_SAMPLE_FILTER_TIME		(DI_SAMPLE_INTERVAL*5)

extern osMutexId DI_DataAccessHandle;
static uint32_t unDI_Value; 
static GPIO_TypeDef* DI_Ports[SH_Z_002_DI_NUM] = {DI_0_GPIO_Port, DI_1_GPIO_Port, DI_2_GPIO_Port, DI_3_GPIO_Port};
static uint16_t DI_Pins[SH_Z_002_DI_NUM] = {DI_0_Pin, DI_1_Pin, DI_2_Pin, DI_3_Pin};

static uint32_t get_DI_values(void) {
	uint8_t unIndex;
	uint32_t unDI_ValueTemp = 0;

	assert_param(IS_DI_PIN_NUM(SH_Z_002_DI_NUM));
	
	for (unIndex = 0; unIndex < SH_Z_002_DI_NUM; unIndex++) {
		(GPIO_PIN_SET == HAL_GPIO_ReadPin(DI_Ports[unIndex], DI_Pins[unIndex])) ? 
			(SET_BIT(unDI_ValueTemp, ((0x01U) << unIndex))) : 
			(CLEAR_BIT(unDI_ValueTemp, ((0x01U) << unIndex)));
	}
	return unDI_ValueTemp;
}

uint32_t DI_get_DI_values(void) {
	uint32_t unTemp;
	osMutexWait(DI_DataAccessHandle, osWaitForever);
	unTemp = unDI_Value;
	osMutexRelease(DI_DataAccessHandle);
	return unTemp;
}

void start_di_monitor(void const * argument) {
	uint8_t unIndex;
	static uint32_t unDI_ValueTemp, unBitMask;
	static uint16_t unDI_FilterTimer[SH_Z_002_DI_NUM];
	while (1) {
		unDI_ValueTemp = get_DI_values();
		osMutexWait(DI_DataAccessHandle, osWaitForever);
		for (unIndex = 0; unIndex < SH_Z_002_DI_NUM; unIndex++) {
			unBitMask = ((0x01U) << unIndex);
			if (READ_BIT(unDI_ValueTemp, unBitMask) != READ_BIT(unDI_Value, unBitMask)) {
				unDI_FilterTimer[unIndex] += DI_SAMPLE_INTERVAL;
			} else {
				unDI_FilterTimer[unIndex] = 0;
			}
		}
		for (unIndex = 0; unIndex < SH_Z_002_DI_NUM; unIndex++) {
			if (unDI_FilterTimer[unIndex] > DI_SAMPLE_FILTER_TIME) {
				unBitMask = ((0x01U) << unIndex);
				(READ_BIT(unDI_ValueTemp, unBitMask) == unBitMask) ? (SET_BIT(unDI_Value, unBitMask)) : (CLEAR_BIT(unDI_Value, unBitMask));
			}
		}		
		osMutexRelease(DI_DataAccessHandle);		
		osDelay(DI_SAMPLE_INTERVAL);
	}
}

