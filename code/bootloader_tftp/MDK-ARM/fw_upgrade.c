#include "main.h"
#include "stm32f4xx_hal.h"
#include "cmsis_os.h"
#include "spiffs.h"
#include "sh_z_002.h"
#include "fw_upgrade.h"

// VTOR can not be 0x08020010, only allowed 0x08020000
#define APPLICATION_ENTRY_ADDRESS				((uint32_t)0x08020000)		// start at sector 5, CRC include all other headers
#define APPLICATION_SIZE_W_ADDRESS				((uint32_t)0x08080000 - 0x04)		// size in word, includeing only fw itself, no header
#define APPLICATION_VERSION_ADDRESS				((APPLICATION_SIZE_W_ADDRESS) - 0x04)
#define APPLICATION_SLAVE_ID_ADDRESS			((APPLICATION_VERSION_ADDRESS) - 0x04)
#define APPLICATION_CRC_ADDRESS					((APPLICATION_SLAVE_ID_ADDRESS) - 0x04)

typedef struct {
	uint32_t unCRC;
	uint32_t unSlaveID;
	int32_t nVersion;
	uint32_t unFW_LengthInWord;
} fw_header;

#define INTERNAL_IMAGE_CRC_IN_HEAD			(*((uint32_t *)APPLICATION_CRC_ADDRESS))
#define INTERNAL_SLAVE_ID_IN_HEAD			(*((uint32_t *)APPLICATION_SLAVE_ID_ADDRESS))
#define INTERNAL_FW_LENGTH_W_IN_HEAD		(*((uint32_t *)APPLICATION_SIZE_W_ADDRESS))
#define INTERNAL_FW_VERSION_IN_HEAD			(*((int32_t *)APPLICATION_VERSION_ADDRESS))

#define MAX_FW_LENGTH_IN_WORD				(128*3*1024/4)
typedef void (*pFunction)(void);

extern EventGroupHandle_t xComEventGroup;
extern spiffs SPI_FFS_fs;
extern CRC_HandleTypeDef hcrc;

/************ fw image file name format ***********/
//fw_slaveid_vHEX.bin
// e.g. fw_002_v0100.bin  (explanation: 002 is the slave ID for SH-Z-002, 0100 is 0x0100, measn version 01.00)

const uint32_t crc32_tab[] = {
	0x00000000, 0x77073096, 0xee0e612c, 0x990951ba, 0x076dc419, 0x706af48f,
	0xe963a535, 0x9e6495a3,	0x0edb8832, 0x79dcb8a4, 0xe0d5e91e, 0x97d2d988,
	0x09b64c2b, 0x7eb17cbd, 0xe7b82d07, 0x90bf1d91, 0x1db71064, 0x6ab020f2,
	0xf3b97148, 0x84be41de,	0x1adad47d, 0x6ddde4eb, 0xf4d4b551, 0x83d385c7,
	0x136c9856, 0x646ba8c0, 0xfd62f97a, 0x8a65c9ec,	0x14015c4f, 0x63066cd9,
	0xfa0f3d63, 0x8d080df5,	0x3b6e20c8, 0x4c69105e, 0xd56041e4, 0xa2677172,
	0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b,	0x35b5a8fa, 0x42b2986c,
	0xdbbbc9d6, 0xacbcf940,	0x32d86ce3, 0x45df5c75, 0xdcd60dcf, 0xabd13d59,
	0x26d930ac, 0x51de003a, 0xc8d75180, 0xbfd06116, 0x21b4f4b5, 0x56b3c423,
	0xcfba9599, 0xb8bda50f, 0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924,
	0x2f6f7c87, 0x58684c11, 0xc1611dab, 0xb6662d3d,	0x76dc4190, 0x01db7106,
	0x98d220bc, 0xefd5102a, 0x71b18589, 0x06b6b51f, 0x9fbfe4a5, 0xe8b8d433,
	0x7807c9a2, 0x0f00f934, 0x9609a88e, 0xe10e9818, 0x7f6a0dbb, 0x086d3d2d,
	0x91646c97, 0xe6635c01, 0x6b6b51f4, 0x1c6c6162, 0x856530d8, 0xf262004e,
	0x6c0695ed, 0x1b01a57b, 0x8208f4c1, 0xf50fc457, 0x65b0d9c6, 0x12b7e950,
	0x8bbeb8ea, 0xfcb9887c, 0x62dd1ddf, 0x15da2d49, 0x8cd37cf3, 0xfbd44c65,
	0x4db26158, 0x3ab551ce, 0xa3bc0074, 0xd4bb30e2, 0x4adfa541, 0x3dd895d7,
	0xa4d1c46d, 0xd3d6f4fb, 0x4369e96a, 0x346ed9fc, 0xad678846, 0xda60b8d0,
	0x44042d73, 0x33031de5, 0xaa0a4c5f, 0xdd0d7cc9, 0x5005713c, 0x270241aa,
	0xbe0b1010, 0xc90c2086, 0x5768b525, 0x206f85b3, 0xb966d409, 0xce61e49f,
	0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4, 0x59b33d17, 0x2eb40d81,
	0xb7bd5c3b, 0xc0ba6cad, 0xedb88320, 0x9abfb3b6, 0x03b6e20c, 0x74b1d29a,
	0xead54739, 0x9dd277af, 0x04db2615, 0x73dc1683, 0xe3630b12, 0x94643b84,
	0x0d6d6a3e, 0x7a6a5aa8, 0xe40ecf0b, 0x9309ff9d, 0x0a00ae27, 0x7d079eb1,
	0xf00f9344, 0x8708a3d2, 0x1e01f268, 0x6906c2fe, 0xf762575d, 0x806567cb,
	0x196c3671, 0x6e6b06e7, 0xfed41b76, 0x89d32be0, 0x10da7a5a, 0x67dd4acc,
	0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5, 0xd6d6a3e8, 0xa1d1937e,
	0x38d8c2c4, 0x4fdff252, 0xd1bb67f1, 0xa6bc5767, 0x3fb506dd, 0x48b2364b,
	0xd80d2bda, 0xaf0a1b4c, 0x36034af6, 0x41047a60, 0xdf60efc3, 0xa867df55,
	0x316e8eef, 0x4669be79, 0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236,
	0xcc0c7795, 0xbb0b4703, 0x220216b9, 0x5505262f, 0xc5ba3bbe, 0xb2bd0b28,
	0x2bb45a92, 0x5cb36a04, 0xc2d7ffa7, 0xb5d0cf31, 0x2cd99e8b, 0x5bdeae1d,
	0x9b64c2b0, 0xec63f226, 0x756aa39c, 0x026d930a, 0x9c0906a9, 0xeb0e363f,
	0x72076785, 0x05005713, 0x95bf4a82, 0xe2b87a14, 0x7bb12bae, 0x0cb61b38,
	0x92d28e9b, 0xe5d5be0d, 0x7cdcefb7, 0x0bdbdf21, 0x86d3d2d4, 0xf1d4e242,
	0x68ddb3f8, 0x1fda836e, 0x81be16cd, 0xf6b9265b, 0x6fb077e1, 0x18b74777,
	0x88085ae6, 0xff0f6a70, 0x66063bca, 0x11010b5c, 0x8f659eff, 0xf862ae69,
	0x616bffd3, 0x166ccf45, 0xa00ae278, 0xd70dd2ee, 0x4e048354, 0x3903b3c2,
	0xa7672661, 0xd06016f7, 0x4969474d, 0x3e6e77db, 0xaed16a4a, 0xd9d65adc,
	0x40df0b66, 0x37d83bf0, 0xa9bcae53, 0xdebb9ec5, 0x47b2cf7f, 0x30b5ffe9,
	0xbdbdf21c, 0xcabac28a, 0x53b39330, 0x24b4a3a6, 0xbad03605, 0xcdd70693,
	0x54de5729, 0x23d967bf, 0xb3667a2e, 0xc4614ab8, 0x5d681b02, 0x2a6f2b94,
	0xb40bbe37, 0xc30c8ea1, 0x5a05df1b, 0x2d02ef8d
};

static uint32_t crc32(uint32_t crc, const void *buf, size_t size) {
	const uint8_t *p = buf;
	crc = ~crc;
	while (size--)
		crc = crc32_tab[(crc ^ *p++) & 0xFF] ^ (crc >> 8);
	return ~crc;
}
 
//static uint32_t crc32(uint32_t crc, unsigned char *buf, size_t len)
//{
//    crc = ~crc;
//    while (len--) {
//        crc ^= *buf++;
//        crc = crc & 1 ? (crc >> 1) ^ 0xedb88320 : crc >> 1;
//        crc = crc & 1 ? (crc >> 1) ^ 0xedb88320 : crc >> 1;
//        crc = crc & 1 ? (crc >> 1) ^ 0xedb88320 : crc >> 1;
//        crc = crc & 1 ? (crc >> 1) ^ 0xedb88320 : crc >> 1;
//        crc = crc & 1 ? (crc >> 1) ^ 0xedb88320 : crc >> 1;
//        crc = crc & 1 ? (crc >> 1) ^ 0xedb88320 : crc >> 1;
//        crc = crc & 1 ? (crc >> 1) ^ 0xedb88320 : crc >> 1;
//        crc = crc & 1 ? (crc >> 1) ^ 0xedb88320 : crc >> 1;
//    }
//    return ~crc;
//}

void FWU_run_app(void) {
	pFunction Jump_To_Application;
	uint32_t JumpAddress;
	if (((*(__IO uint32_t*)APPLICATION_ENTRY_ADDRESS) & 0x2FFE0000 ) == 0x20000000) { 
		/* Jump to user application */
		JumpAddress = *(__IO uint32_t*) (APPLICATION_ENTRY_ADDRESS + 4);
		Jump_To_Application = (pFunction) JumpAddress;
		/* Initialize user application's Stack Pointer */
		taskENTER_CRITICAL();
		__disable_irq();
		__set_MSP(*(__IO uint32_t*) APPLICATION_ENTRY_ADDRESS);
		SCB->VTOR = APPLICATION_ENTRY_ADDRESS;
		Jump_To_Application();
	}	
}

// all fw image in FS lower than nLatestVersion wil be removed
// if nLatestVersion == MAX_INT, all fw image will be removed
static void remove_fw_image_in_FS(int nLatestVersion) {
	spiffs_DIR d;
	struct spiffs_dirent e;
	struct spiffs_dirent *pe = &e;
	int nVersionInFileName;
	fw_header tFW_Header;
	spiffs_file fd = -1;
	
	SPIFFS_opendir(&SPI_FFS_fs, "/", &d);
	while ((pe = SPIFFS_readdir(&d, pe)) != NULL) {
		if (0 == strncmp("fw_002_v", (char *)pe->name, strlen("fw_002_v"))) {
			// found one
			// get version from file name
			if (sscanf((char *)pe->name, "fw_002_v%x.bin", &nVersionInFileName) == 1) {
				fd = SPIFFS_open(&SPI_FFS_fs, (char *)pe->name, SPIFFS_RDONLY, 0);
				if (fd >= 0) {
					if (SPIFFS_read(&SPI_FFS_fs, fd, (u8_t *)(&tFW_Header), sizeof(tFW_Header)) > 0) {					
						SPIFFS_close(&SPI_FFS_fs, fd);
						if ((tFW_Header.unSlaveID != SH_Z_002_SLAVE_ID) || (tFW_Header.nVersion < nLatestVersion)) {
							// if CRC is not OK or image is not for this type of device, delete the file
							SPIFFS_remove(&SPI_FFS_fs, (char *)pe->name);
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

static int get_fw_version_internal(void) {
	static uint32_t unCRCValue;
	// check CRC
	if (INTERNAL_FW_LENGTH_W_IN_HEAD > MAX_FW_LENGTH_IN_WORD) {
		return (-1);
	}
	unCRCValue = crc32(0xFFFFFFFF, (u8_t *)APPLICATION_SLAVE_ID_ADDRESS, sizeof(fw_header) - 4);	// header is at end of flash
	unCRCValue = crc32(unCRCValue, (u8_t *)APPLICATION_ENTRY_ADDRESS, INTERNAL_FW_LENGTH_W_IN_HEAD * 4);	// include most of header
	//unCRCValue = HAL_CRC_Calculate(&hcrc, (uint32_t *)APPLICATION_SLAVE_ID_ADDRESS, INTERNAL_FW_LENGTH_W_IN_HEAD - 1);
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
	while ((pe = SPIFFS_readdir(&d, pe)) != NULL) {
		if (0 == strncmp("fw_002_v", (char *)pe->name, strlen("fw_002_v"))) {
			// found one
			// get version from file name
			if (sscanf((char *)pe->name, "fw_002_v%x.bin", &nVersionInFileName) == 1) {
				fd = SPIFFS_open(&SPI_FFS_fs, (char *)pe->name, SPIFFS_RDONLY, 0);
				if (fd >= 0) {
					if (SPIFFS_read(&SPI_FFS_fs, fd, (u8_t *)(&tFW_Header), sizeof(tFW_Header)) > 0) {
						// check file crc
						unCalculatedCRC = crc32(0xFFFFFFFF, (uint8_t *)(&(tFW_Header.unSlaveID)), sizeof(tFW_Header) - sizeof(tFW_Header.unCRC));
						//unCalculatedCRC = HAL_CRC_Calculate(&hcrc, (uint32_t *)(&(tFW_Header.unSlaveID)), (sizeof(tFW_Header) - sizeof(tFW_Header.unCRC))/sizeof(uint32_t));
						while ((nReadCNT = SPIFFS_read(&SPI_FFS_fs, fd, (u8_t *)unBuf, sizeof(unBuf))) > 0 ) {
							unCalculatedCRC = crc32(unCalculatedCRC, (u8_t *)unBuf, nReadCNT);
							//unCalculatedCRC = HAL_CRC_Accumulate(&hcrc, unBuf, nReadCNT/sizeof(uint32_t));
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
	return nLatestVersion;
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
		remove_fw_image_in_FS(0x7FFFFFFF);
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

// return -1 means upgrade failed
// elsewise internal fw version
int FWU_upgrade(char* file_name) {
	// read binary file from external flash 
	// and write to internal flash
	FLASH_EraseInitTypeDef EraseInitStruct;
	fw_header tFW_Herdaer;
	uint32_t SectorError = 0;
	uint32_t unBuf[64];
	int nReadCNT;	
	uint32_t unFlashAddress;
	uint32_t unIndex;
	spiffs_file fd = -1;
	
	taskENTER_CRITICAL();
	//Erase
	HAL_FLASH_Unlock();
	EraseInitStruct.TypeErase = FLASH_TYPEERASE_SECTORS;
	EraseInitStruct.VoltageRange = FLASH_VOLTAGE_RANGE_3;
	EraseInitStruct.Sector = 5;
	EraseInitStruct.NbSectors = 3;
	if(HAL_FLASHEx_Erase(&EraseInitStruct, &SectorError) != HAL_OK) { 
		HAL_FLASH_Lock();
		taskEXIT_CRITICAL();
		printf("Erase flash failed\n");
		return (-1);
	}
	
	taskEXIT_CRITICAL();
	
	__HAL_FLASH_DATA_CACHE_DISABLE();
	__HAL_FLASH_INSTRUCTION_CACHE_DISABLE();

	__HAL_FLASH_DATA_CACHE_RESET();
	__HAL_FLASH_INSTRUCTION_CACHE_RESET();

	__HAL_FLASH_INSTRUCTION_CACHE_ENABLE();
	__HAL_FLASH_DATA_CACHE_ENABLE();
	
	//program
	fd = SPIFFS_open(&SPI_FFS_fs, file_name, SPIFFS_RDONLY, 0);
	if (fd >= 0) {
		// program header to end of flash
		unFlashAddress = APPLICATION_CRC_ADDRESS;
		nReadCNT = SPIFFS_read(&SPI_FFS_fs, fd, (u8_t *)(&tFW_Herdaer), sizeof(tFW_Herdaer));
		for (unIndex = 0; unIndex < (nReadCNT/sizeof(uint32_t)); unIndex++) {
			taskENTER_CRITICAL();
			HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, unFlashAddress, *((uint32_t *)(&tFW_Herdaer) + unIndex));
			taskEXIT_CRITICAL();
			unFlashAddress += 4;
		}
		// program fw (VTOR can not be 0x08020010, only allowed 0x08020000)
		unFlashAddress = APPLICATION_ENTRY_ADDRESS;		
		while ((nReadCNT = SPIFFS_read(&SPI_FFS_fs, fd, (u8_t *)unBuf, sizeof(unBuf))) > 0 ) {
			for (unIndex = 0; unIndex < (nReadCNT/sizeof(uint32_t)); unIndex++) {
				taskENTER_CRITICAL();
				HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, unFlashAddress, unBuf[unIndex]);
				taskEXIT_CRITICAL();
				unFlashAddress += 4;
			}
		}						
		SPIFFS_close(&SPI_FFS_fs, fd);
	}	
	HAL_FLASH_Lock();
	
	// validate
	return get_fw_version_internal();
}

void FWU_backup_fw(void){
	char file_name[64];
	spiffs_file fd = -1;
	
	sprintf(file_name, "fw_%03u_v%04X.bin", SH_Z_002_SLAVE_ID, INTERNAL_FW_VERSION_IN_HEAD);
	fd = SPIFFS_open(&SPI_FFS_fs, file_name, SPIFFS_RDWR | SPIFFS_CREAT, 0);
	if (fd >= 0) {
		SPIFFS_write(&SPI_FFS_fs, fd, (u8_t *)APPLICATION_CRC_ADDRESS, INTERNAL_FW_LENGTH_W_IN_HEAD * sizeof(uint32_t));					
		SPIFFS_close(&SPI_FFS_fs, fd);
	}		
}
	
