#include "main.h"
#include "stm32f4xx_hal.h"
#include "cmsis_os.h"
#include "spiffs.h"

#define APPLICATION_VERSION_ADDRESS			((uint32_t)0x08018000)
#define APPLICATION_SIZE_ADDRESS			((APPLICATION_VERSION_ADDRESS) + 0x04)
#define APPLICATION_CRC_ADDRESS				((APPLICATION_SIZE_ADDRESS) + 0x04)
#define APPLICATION_ENTRY_ADDRESS			((APPLICATION_CRC_ADDRESS) + 0x04)

typedef  void (*pFunction)(void);

extern EventGroupHandle_t xComEventGroup;
extern spiffs SPI_FFS_fs;

int FWU_app_valid(void) {
	return 1;
}

void FWU_run_app(void) {
	pFunction Jump_To_Application;
	uint32_t JumpAddress;
	if (((*(__IO uint32_t*)APPLICATION_ENTRY_ADDRESS) & 0x2FFE0000 ) == 0x20000000) { 
		/* Jump to user application */
		JumpAddress = *(__IO uint32_t*) (APPLICATION_ENTRY_ADDRESS + 4);
		Jump_To_Application = (pFunction) JumpAddress;
		/* Initialize user application's Stack Pointer */
		__set_MSP(*(__IO uint32_t*) APPLICATION_ENTRY_ADDRESS);
		SCB->VTOR = APPLICATION_ENTRY_ADDRESS;
		Jump_To_Application();
	}	
}

int FWU_check_upgrade_file(char* file_name, unsigned char unBufLen) {
	
	
}

void FWU_upgrade(char* file_name) {
	// read binary file from external flash 
	// and write to internal flash
	
}