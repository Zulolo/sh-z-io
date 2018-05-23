#include "main.h"
#include "stm32f4xx_hal.h"
#include <string.h>
#include "cmsis_os.h"
#include "di_monitor.h"

#define DI_SAMPLE_INTERVAL			10
#define DI_SAMPLE_FILTER_TIME		(DI_SAMPLE_INTERVAL*5)

extern osMutexId DI_DataAccessHandle;
extern EventGroupHandle_t xDiEventGroup;
static uint32_t unDI_Value; 
// limitation can only use pins in same port, e.g. GPIOD 
// Also the pins should be continuously
static GPIO_TypeDef* DI_Port = GPIOD;
static const uint16_t DI_Pins[SH_Z_002_DI_NUM] = {DI_0_Pin, DI_1_Pin, DI_2_Pin, DI_3_Pin};

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	BaseType_t xHigherPriorityTaskWoken, xResult;
	if ((DI_0_Pin == GPIO_Pin) || (DI_1_Pin == GPIO_Pin) || 
		(DI_2_Pin == GPIO_Pin) || (DI_3_Pin == GPIO_Pin)) {
		/* xHigherPriorityTaskWoken must be initialised to pdFALSE. */
		xHigherPriorityTaskWoken = pdFALSE;

		/* Set bit 0 and bit 4 in xEventGroup. */
		xResult = xEventGroupSetBitsFromISR(xDiEventGroup, GPIO_Pin, &xHigherPriorityTaskWoken );

		/* Was the message posted successfully? */
		if( xResult != pdFAIL ) {
			portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
		}	  
	}
}

static uint32_t get_DI_values(void) {
	uint8_t unIndex;
	uint32_t unDI_ValueTemp = 0;

	assert_param(IS_DI_PIN_NUM(SH_Z_002_DI_NUM));
	
	for (unIndex = 0; unIndex < SH_Z_002_DI_NUM; unIndex++) {
		(GPIO_PIN_SET == HAL_GPIO_ReadPin(DI_Port, DI_Pins[unIndex])) ? 
			(SET_BIT(unDI_ValueTemp, DI_Pins[unIndex])) : 
			(CLEAR_BIT(unDI_ValueTemp, DI_Pins[unIndex]));
	}
	return unDI_ValueTemp;
}

uint32_t DI_get_DI_values(void) {
	uint32_t unTemp;
	osMutexWait(DI_DataAccessHandle, osWaitForever);
	unTemp = unDI_Value >> 8;
	osMutexRelease(DI_DataAccessHandle);
	return unTemp;
}

void start_di_monitor(void const * argument) {
	uint8_t unIndex;
//	EventBits_t uxBits;
	static BaseType_t bSomeThingHappened;
	static uint32_t unDI_ValueTemp;
	static uint16_t unDI_FilterTimer[SH_Z_002_DI_NUM];
	while (1) {
		xEventGroupWaitBits(xDiEventGroup, DI_0_Pin | DI_1_Pin | DI_2_Pin | DI_3_Pin, 
										pdTRUE, pdFALSE, osWaitForever );
		bSomeThingHappened = pdTRUE;
		while (pdTRUE == bSomeThingHappened) {
			unDI_ValueTemp = get_DI_values();
			bSomeThingHappened = pdFALSE;
			osMutexWait(DI_DataAccessHandle, osWaitForever);
			for (unIndex = 0; unIndex < SH_Z_002_DI_NUM; unIndex++) {
				if (READ_BIT(unDI_ValueTemp, DI_Pins[unIndex]) != READ_BIT(unDI_Value, DI_Pins[unIndex])) {
					unDI_FilterTimer[unIndex] += DI_SAMPLE_INTERVAL;
					bSomeThingHappened = pdTRUE;
				} else {
					unDI_FilterTimer[unIndex] = 0;
				}
			}
			for (unIndex = 0; unIndex < SH_Z_002_DI_NUM; unIndex++) {
				if (unDI_FilterTimer[unIndex] > DI_SAMPLE_FILTER_TIME) {
					(READ_BIT(unDI_ValueTemp, DI_Pins[unIndex]) == DI_Pins[unIndex]) ? (SET_BIT(unDI_Value, DI_Pins[unIndex])) : (CLEAR_BIT(unDI_Value, DI_Pins[unIndex]));
				}
			}		
			osMutexRelease(DI_DataAccessHandle);		
			osDelay(DI_SAMPLE_INTERVAL);
		}
	}
}

