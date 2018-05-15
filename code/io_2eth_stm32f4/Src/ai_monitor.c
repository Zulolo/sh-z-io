#include "stm32f4xx_hal.h"
#include "cmsis_os.h"

extern ADC_HandleTypeDef hadc1;
__IO uint16_t unADCxConvertedValue[4];

void start_ai_monitor(void const * argument) {

	while (1) {
		if(HAL_ADC_Start_DMA(&hadc1, (uint32_t*)unADCxConvertedValue, 4) != HAL_OK) {
			/* Start Conversation Error */
			Error_Handler(); 
		}
		osDelay(100);
	}
}

