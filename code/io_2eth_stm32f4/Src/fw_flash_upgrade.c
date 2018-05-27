#include "stm32f4xx_hal.h"

void fw_flash_program(char* pFW_FileName, unsigned int unFW_FileSize, unsigned int unFW_FlashWriteAddr) {
	// disable all interrupt
	
	// disable all other OS function to prevent task switch
	
	// enter critical place, do it and only do it
	
	// enable internal flash write/erase
	
	// erase internal flash
	
	// 
	// reboot
}

