#include "main.h"
#include "stm32f4xx_hal.h"
#include "cmsis_os.h"
#include "spiffs.h"
#include "sh_z_002.h"
#include "fw_upgrade.h"

#define APPLICATION_CRC_ADDRESS				((uint32_t)0x08018000)
#define APPLICATION_SLAVE_ID_ADDRESS		((APPLICATION_CRC_ADDRESS) + 0x04)
#define APPLICATION_VERSION_ADDRESS			((APPLICATION_SLAVE_ID_ADDRESS) + 0x04)
#define APPLICATION_SIZE_ADDRESS			((APPLICATION_VERSION_ADDRESS) + 0x04)		// size in word
#define APPLICATION_ENTRY_ADDRESS			((APPLICATION_SIZE_ADDRESS) + 0x04)

typedef struct {
	uint32_t unCRC;
	uint32_t unSlaveID;
	int32_t nVersion;
	uint32_t unFW_LengthInWord;
} fw_header;

#define INTERNAL_IMAGE_CRC_IN_HEAD			(*((uint32_t *)APPLICATION_CRC_ADDRESS))
#define INTERNAL_SLAVE_ID_IN_HEAD			(*((uint32_t *)APPLICATION_SLAVE_ID_ADDRESS))
#define INTERNAL_FW_LENGTH_IN_HEAD			(*((uint32_t *)APPLICATION_SIZE_ADDRESS))
#define INTERNAL_FW_VERSION_IN_HEAD			(*((int32_t *)APPLICATION_VERSION_ADDRESS))

typedef void (*pFunction)(void);

extern EventGroupHandle_t xComEventGroup;
extern spiffs SPI_FFS_fs;
extern CRC_HandleTypeDef hcrc;

/************ fw image file name format ***********/
//fw_slaveid_vHEX.bin
// e.g. fw_002_v0100.bin  (explanation: 002 is the slave ID for SH-Z-002, 0100 is 0x0100, measn version 01.00)

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

// all fw image in FS lower than nLatestVersion wil be removed
// if nLatestVersion == 0, all fw image will be removed
static void remove_fw_image_in_FS(int nLatestVersion) {
	
}

static int get_fw_version_internal(void) {
	uint32_t unCRCValue;
	// check CRC
	unCRCValue = HAL_CRC_Calculate(&hcrc, (uint32_t *)APPLICATION_ENTRY_ADDRESS, INTERNAL_FW_LENGTH_IN_HEAD);
	// if CRC OK, read and return version
	if ((unCRCValue == INTERNAL_IMAGE_CRC_IN_HEAD) && (SH_Z_002_SLAVE_ID == INTERNAL_SLAVE_ID_IN_HEAD)) {
		return INTERNAL_FW_VERSION_IN_HEAD;
	} else {
		return (-1);
	}
}

// during traversal all the files in fs, error fw image will be removed
// After execute this function, only the latest image will be reserved
// return the latest version of image in FS, -1 means no image found.
static int find_latest_fw_image_in_fs(char* pFileName, unsigned char unBufLen) {
	spiffs_DIR d;
	struct spiffs_dirent e;
	struct spiffs_dirent *pe = &e;
	int nVersionInFileName, nLatestVersion;
	fw_header tFW_Header;
	uint32_t unCalculatedCRC;
	uint32_t unBuf[64];
	int nReadCNT;
	
	spiffs_file fd = -1;
	nLatestVersion = -1;
	SPIFFS_opendir(&SPI_FFS_fs, "/", &d);
	while ((pe = SPIFFS_readdir(&d, pe))) {
		if (0 == strncmp("fw_002_v", (char *)pe->name, strlen("fw_002_v"))) {
			// found one
			// get version from file name
			if (sscanf((char *)pe->name, "fw_002_v%x.bin", &nVersionInFileName) == 1) {
				fd = SPIFFS_open(&SPI_FFS_fs, (char *)pe->name, SPIFFS_RDONLY, 0);
				if (fd >= 0) {
					if (SPIFFS_read(&SPI_FFS_fs, fd, (u8_t *)(&tFW_Header), sizeof(tFW_Header)) > 0) {
						// check file crc
						unCalculatedCRC = HAL_CRC_Calculate(&hcrc, (uint32_t *)(&(tFW_Header.unSlaveID)), sizeof(tFW_Header) - sizeof(tFW_Header.unCRC));
						while ((nReadCNT = SPIFFS_read(&SPI_FFS_fs, fd, (u8_t *)unBuf, sizeof(unBuf))) > 0 ) {
							unCalculatedCRC = HAL_CRC_Accumulate(&hcrc, unBuf, nReadCNT/sizeof(uint32_t));
						}						
						SPIFFS_close(&SPI_FFS_fs, fd);
						if ((unCalculatedCRC != tFW_Header.unCRC) || (tFW_Header.unSlaveID != SH_Z_002_SLAVE_ID)) {
							// if CRC is not OK or image is not for this type of device, delete the file
							SPIFFS_remove(&SPI_FFS_fs, (char *)pe->name);
						} else {
							if (nVersionInFileName == tFW_Header.nVersion) {
								if (tFW_Header.nVersion > nLatestVersion) {
									if (nLatestVersion >= 0) {
										SPIFFS_remove(&SPI_FFS_fs, pFileName);
									}
									nLatestVersion = tFW_Header.nVersion;								
									strncpy(pFileName, (char *)pe->name, unBufLen);
									pFileName[unBufLen - 1] = '\0';
								}
							} else {
								SPIFFS_remove(&SPI_FFS_fs, (char *)pe->name);
							}
						}
					} else {
						SPIFFS_close(&SPI_FFS_fs, fd);
						printf("errno %i\n", SPIFFS_errno(&SPI_FFS_fs));
					}
				}			
			}
		}
	}
	SPIFFS_closedir(&d);

}

// After execute this function, all image file older than internal flash will be removed
// FW_INTERNAL_FS_MATCH_LATEST Already latest image in internal flash
// FW_FS_NEWER found some new image
// NO_FW_INTERNAL_NO_FW_FS no fw found neither running nor in FS
// VALID_FW_INTERNAL_NO_FW_FS no image found in FS, but there is one running
fw_status FWU_check_upgrade_file(char* pFileName, unsigned char unBufLen) {
	int unFW_VersionInternal, unFW_VersionFS;
	unFW_VersionInternal = get_fw_version_internal();
	unFW_VersionFS = find_latest_fw_image_in_fs(pFileName, unBufLen);
	if ((unFW_VersionInternal < 0) && (unFW_VersionFS < 0)) {
		// neither fw was found in internal flash nor in FS
		pFileName[0] = 0;
		return NO_FW_INTERNAL_NO_FW_FS;
	} else if (unFW_VersionInternal > unFW_VersionFS) {
		pFileName[0] = 0;
		remove_fw_image_in_FS(0);
		return VALID_FW_INTERNAL_NO_FW_FS;
	} else if (unFW_VersionInternal < unFW_VersionFS) {
		return FW_FS_NEWER;
	} else if (unFW_VersionInternal == unFW_VersionFS) {
		return FW_INTERNAL_FS_MATCH_LATEST;
	} else {
		pFileName[0] = 0;
		printf("What the fxxk?\n");
		return NO_FW_INTERNAL_NO_FW_FS;
	}
}

void FWU_upgrade(char* file_name) {
	// read binary file from external flash 
	// and write to internal flash
	
}