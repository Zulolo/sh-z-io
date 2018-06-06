#include "stm32f4xx_hal.h"
#include "cmsis_os.h"
#include "string.h"
#include "ai_monitor.h"

extern osMutexId AI_DataAccessHandle;
extern ADC_HandleTypeDef hadc1;
__IO uint16_t unADCxConvertedValue[SH_Z_002_AI_NUM];

void AI_get_AI_values(uint16_t* pAI_Values) {
	osMutexWait(AI_DataAccessHandle, osWaitForever);
	memcpy(pAI_Values, (void *)unADCxConvertedValue, sizeof(unADCxConvertedValue));
	osMutexRelease(AI_DataAccessHandle);
}

void start_ai_monitor(void const * argument) {

	while (1) {
		osMutexWait(AI_DataAccessHandle, osWaitForever);
		if(HAL_ADC_Start_DMA(&hadc1, (uint32_t*)unADCxConvertedValue, 4) != HAL_OK) {
			osMutexRelease(AI_DataAccessHandle);
			/* Start Conversation Error */
			Error_Handler(); 
		}
		osMutexRelease(AI_DataAccessHandle);
		osDelay(500);
	}
}

