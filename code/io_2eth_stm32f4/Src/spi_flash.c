#include "main.h"
#include "stm32f4xx_hal.h"
#include "cmsis_os.h"

#define SPI_FLASH_CHIP_BUSY_BIT				(0x01U)	
extern SPI_HandleTypeDef hspi3;
extern osMutexId SpiFlashChipMutexHandle;
extern EventGroupHandle_t xComEventGroup;;

void HAL_SPI_TxRxCpltCallback(SPI_HandleTypeDef *hspi) {
	BaseType_t xHigherPriorityTaskWoken, xResult;
	if (hspi == &hspi3) {
		/* xHigherPriorityTaskWoken must be initialised to pdFALSE. */
		xHigherPriorityTaskWoken = pdFALSE;

		xResult = xEventGroupSetBitsFromISR(xComEventGroup, SPI_FLASH_CHIP_DMA_TX_RX, &xHigherPriorityTaskWoken );

		/* Was the message posted successfully? */
		if( xResult != pdFAIL ) {
			portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
		}	  
	}
}

static void write_SPI_flash_chip(uint8_t unCMD, uint8_t* pData, uint32_t unDataLen) {
	
}

static void read_SPI_flash_chip(uint8_t unCMD, uint8_t* pData, uint32_t unDataLen) {
	
}

static uint32_t get_SPI_flash_chip_status(void) {
	
}

#define IS_SPI_FLASH_CHIP_BUSY			(((get_SPI_flash_chip_status() & SPI_FLASH_CHIP_BUSY_BIT) == 0) ? (pdFALSE):(pdTRUE))

void spi_flash_erase_block(uint32_t addr) {
	
}

void spi_flash_erase_chip(void) {
	// get mutex to use spi resource
	osMutexWait(SpiFlashChipMutexHandle, osWaitForever);
	// wait until spi flash chip is not BUSY
	
	// start spi DMA transmission
	
	// wait until spi trans finish
	
	// wait until spi flash chip is not BUSY
	
	// release spi resource
	osMutexRelease(SpiFlashChipMutexHandle);
}

void spi_flash_read(uint32_t addr, uint32_t size, uint8_t *dst) {
	
}

void spi_flash_write(uint32_t addr, uint32_t size, uint8_t *dst) {
	
}


