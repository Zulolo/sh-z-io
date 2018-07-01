#include "stm32f4xx_hal.h"
#include "cmsis_os.h"
#include "string.h"
#include "ai_monitor.h"
#include "spiffs.h"
#include "cJSON.h"

#define AI_CONF_FILE_NAME				"ai_conf"
#define AI_CONF_CHANNEL_JSON_TAG		"ai_chn"
#define AI_CONF_CHN_INDEX_JSON_TAG		"chn_i"
#define AI_CONF_HIGH_THLD_JSON_TAG		"high_thld"
#define AI_CONF_LOW_THLD_JSON_TAG		"low_thld"
#define AI_CONF_HSTRCL_MAX_JSON_TAG		"hstrcl_max"
#define AI_CONF_HSTRCL_MIN_JSON_TAG		"hstrcl_min"

extern spiffs SPI_FFS_fs;
extern osMutexId AI_DataAccessHandle;
extern ADC_HandleTypeDef hadc1;
static __IO uint16_t unADCxConvertedValue[SH_Z_002_AI_NUM];

static int32_t nCurrentValue[SH_Z_002_AI_NUM];	// with two digital. e.g. 8.54mA is 854 in register
static int32_t nCurrentHighThreshold[SH_Z_002_AI_NUM] = {0x7FFFFFFF, 0x7FFFFFFF, 0x7FFFFFFF, 0x7FFFFFFF};
static int32_t nCurrentLowThreshold[SH_Z_002_AI_NUM]; 
static int32_t nCurrentHstrclMax[SH_Z_002_AI_NUM];
static int32_t nCurrentHstrclMin[SH_Z_002_AI_NUM] = {0x7FFFFFFF, 0x7FFFFFFF, 0x7FFFFFFF, 0x7FFFFFFF}; 
static uint32_t bAI_LowAlarm;
static uint32_t bAI_HighAlarm;

void AI_get_AI_values(uint16_t* pAI_Values) {
	osMutexWait(AI_DataAccessHandle, osWaitForever);
	memcpy(pAI_Values, (void *)unADCxConvertedValue, sizeof(unADCxConvertedValue));
	osMutexRelease(AI_DataAccessHandle);
}

void AI_get_AI_current(int32_t* pAI_Current) {
	osMutexWait(AI_DataAccessHandle, osWaitForever);
	memcpy(pAI_Current, (void *)nCurrentValue, sizeof(nCurrentValue));
	osMutexRelease(AI_DataAccessHandle);
}

void AI_get_AI_current_high_thrld(int32_t* pAI_Current) {
	osMutexWait(AI_DataAccessHandle, osWaitForever);
	memcpy(pAI_Current, (void *)nCurrentHighThreshold, sizeof(nCurrentHighThreshold));
	osMutexRelease(AI_DataAccessHandle);
}

void AI_get_AI_current_low_thrld(int32_t* pAI_Current) {
	osMutexWait(AI_DataAccessHandle, osWaitForever);
	memcpy(pAI_Current, (void *)nCurrentLowThreshold, sizeof(nCurrentLowThreshold));
	osMutexRelease(AI_DataAccessHandle);
}

void AI_get_AI_current_hstrcl_max(int32_t* pAI_Current) {
	osMutexWait(AI_DataAccessHandle, osWaitForever);
	memcpy(pAI_Current, (void *)nCurrentHstrclMax, sizeof(nCurrentHstrclMax));
	osMutexRelease(AI_DataAccessHandle);
}

void AI_get_AI_current_hstrcl_min(int32_t* pAI_Current) {
	osMutexWait(AI_DataAccessHandle, osWaitForever);
	memcpy(pAI_Current, (void *)nCurrentHstrclMin, sizeof(nCurrentHstrclMin));
	osMutexRelease(AI_DataAccessHandle);
}

void AI_set_AI_current_high_thrld(int32_t* pAI_Current) {
	osMutexWait(AI_DataAccessHandle, osWaitForever);
	memcpy(nCurrentHighThreshold, (void *)pAI_Current, sizeof(nCurrentHighThreshold));
	osMutexRelease(AI_DataAccessHandle);
}

void AI_set_AI_current_low_thrld(int32_t* pAI_Current) {
	osMutexWait(AI_DataAccessHandle, osWaitForever);
	memcpy(nCurrentLowThreshold, (void *)pAI_Current, sizeof(nCurrentLowThreshold));
	osMutexRelease(AI_DataAccessHandle);
}

void AI_set_AI_low_alarm(uint32_t unValue) {
	osMutexWait(AI_DataAccessHandle, osWaitForever);
	bAI_LowAlarm = unValue;
	osMutexRelease(AI_DataAccessHandle);	
}

void AI_set_AI_high_alarm(uint32_t unValue) {
	osMutexWait(AI_DataAccessHandle, osWaitForever);
	bAI_HighAlarm = unValue;
	osMutexRelease(AI_DataAccessHandle);	
}

uint32_t AI_get_AI_low_alarm(void) {
	uint32_t unTemp;
	osMutexWait(AI_DataAccessHandle, osWaitForever);
	unTemp = bAI_LowAlarm;
	osMutexRelease(AI_DataAccessHandle);
	return unTemp;
}

uint32_t AI_get_AI_high_alarm(void) {
	uint32_t unTemp;
	osMutexWait(AI_DataAccessHandle, osWaitForever);
	unTemp = bAI_HighAlarm;
	osMutexRelease(AI_DataAccessHandle);
	return unTemp;
}

static int ai_conf_wr_conf_file(spiffs_file tFileDesc) {
	uint8_t index;
    char* pJsonString = NULL;
	cJSON* pAIChannels = NULL;
	cJSON* pArrayMember = NULL;
	
	pAIChannels = cJSON_CreateArray();
	if (pAIChannels == NULL){
		printf("failed to create json array object.\n");
		return (-1);
	}
	
//	pAIChannels = cJSON_AddArrayToObject(pAI_ConfJsonWriter, AI_CONF_CHANNEL_JSON_TAG);
//    if (pAIChannels == NULL) {
//		cJSON_Delete(pAI_ConfJsonWriter);
//		printf("failed to create json ai channels object.\n");
//		return (-1);
//    }
    for (index = 0; index < SH_Z_002_AI_NUM; index++) {
		cJSON_AddItemToArray(pAIChannels, pArrayMember = cJSON_CreateObject()); 
//        pArrayMember = cJSON_CreateObject();
		// index
        if (cJSON_AddNumberToObject(pArrayMember, AI_CONF_CHN_INDEX_JSON_TAG, index) == NULL){
			cJSON_Delete(pAIChannels);
			printf("failed to create ai conf json index object.\n");
			return (-1);
        }
		// high threshold
        if(cJSON_AddNumberToObject(pArrayMember, AI_CONF_HIGH_THLD_JSON_TAG, nCurrentHighThreshold[index]) == NULL){
			cJSON_Delete(pAIChannels);
			printf("failed to create json high threshold value object.\n");
			return (-1);
        }
		// low threshold
		if(cJSON_AddNumberToObject(pArrayMember, AI_CONF_LOW_THLD_JSON_TAG, nCurrentLowThreshold[index]) == NULL){
			cJSON_Delete(pAIChannels);
			printf("failed to create json low threshold value object.\n");
			return (-1);
        }
		// historical max
		if(cJSON_AddNumberToObject(pArrayMember, AI_CONF_HSTRCL_MAX_JSON_TAG, nCurrentHstrclMax[index]) == NULL){
			cJSON_Delete(pAIChannels);
			printf("failed to create json historical max value object.\n");
			return (-1);
        }
		// historical min
		if(cJSON_AddNumberToObject(pArrayMember, AI_CONF_HSTRCL_MIN_JSON_TAG, nCurrentHstrclMin[index]) == NULL){
			cJSON_Delete(pAIChannels);
			printf("failed to create json historical min value object.\n");
			return (-1);
        }
//        cJSON_AddItemToArray(pAIChannels, pArrayMember);
    }
	
	pJsonString = cJSON_Print(pAIChannels);
    if (pJsonString == NULL){
		cJSON_Delete(pAIChannels);
		printf("failed to digest json object.\n");
		return (-1);
    }
	
	if (SPIFFS_write(&SPI_FFS_fs, tFileDesc, pJsonString, strlen(pJsonString)) < 0 ) {
		cJSON_Delete(pAIChannels);
		free(pJsonString);
		printf("failed to write digested json string to file.\n");
		return (-1);		
	}
	
	cJSON_Delete(pAIChannels);
	free(pJsonString);
	return (0);
}

static int ai_conf_load(spiffs_file tFileDesc) {
    cJSON* pHighThreshold = NULL;
	cJSON* pLowThreshold = NULL;
    cJSON* pHstrclMax = NULL;
	cJSON* pHstrclMin = NULL;
	cJSON* pIndex = NULL;
	cJSON* pArrayMember = NULL;
    cJSON* pAI_ConfJson;
	uint8_t index;
	char cConfString[512];
	int nReadNum = SPIFFS_read(&SPI_FFS_fs, tFileDesc, cConfString, sizeof(cConfString));
	if ((nReadNum <= 0) || (sizeof(cConfString) == nReadNum )) {
		printf("%d char was read from conf file.\n", nReadNum);
		return (-1);		
	}

	pAI_ConfJson = cJSON_Parse(cConfString);
    if (pAI_ConfJson == NULL) {
        const char *error_ptr = cJSON_GetErrorPtr();
        if (error_ptr != NULL){
            printf("Error before: %s\n", error_ptr);
        }
        return (-1);
    }
	for (index = 0 ; index < cJSON_GetArraySize(pAI_ConfJson) ; index++) {
		pArrayMember = cJSON_GetArrayItem(pAI_ConfJson, index);
        pHighThreshold = cJSON_GetObjectItemCaseSensitive(pArrayMember, AI_CONF_HIGH_THLD_JSON_TAG);
        pLowThreshold = cJSON_GetObjectItemCaseSensitive(pArrayMember, AI_CONF_LOW_THLD_JSON_TAG);
        pHstrclMax = cJSON_GetObjectItemCaseSensitive(pArrayMember, AI_CONF_HSTRCL_MAX_JSON_TAG);
        pHstrclMin = cJSON_GetObjectItemCaseSensitive(pArrayMember, AI_CONF_HSTRCL_MIN_JSON_TAG);
		pIndex = cJSON_GetObjectItemCaseSensitive(pArrayMember, AI_CONF_CHN_INDEX_JSON_TAG);
		
        if (!cJSON_IsNumber(pHighThreshold) || !cJSON_IsNumber(pLowThreshold) || !cJSON_IsNumber(pHstrclMax) ||
			 !cJSON_IsNumber(pHstrclMin) || !cJSON_IsNumber(pIndex)) {
			cJSON_Delete(pAI_ConfJson);
			printf("Error with parsing ai channel data.\n");
			return (-1);
        }
        if ((pIndex->valueint >= SH_Z_002_AI_NUM) || (pIndex->valueint < 0)) {
			cJSON_Delete(pAI_ConfJson);
			printf("AI channel index error.\n");
			return (-1);
        }
		
		nCurrentHighThreshold[pIndex->valueint] = pHighThreshold->valueint;
		nCurrentLowThreshold[pIndex->valueint] = pLowThreshold->valueint;
		nCurrentHstrclMax[pIndex->valueint] = pHstrclMax->valueint;
		nCurrentHstrclMin[pIndex->valueint] = pHstrclMin->valueint;
	}
	
	cJSON_Delete(pAI_ConfJson);
	return 0;
}

static int ai_save_conf(void) {
	spiffs_file tFileDesc;
	tFileDesc = SPIFFS_open(&SPI_FFS_fs, AI_CONF_FILE_NAME, SPIFFS_RDWR | SPIFFS_CREAT, 0);
	if (tFileDesc < 0) {
		printf("failed to open di configuration file.\n");
		return (-1);
	} else {
		if (ai_conf_wr_conf_file(tFileDesc) < 0) {
			printf("failed to write default ai configuration value to file.\n");
			SPIFFS_close(&SPI_FFS_fs, tFileDesc);
			return (-1);
		} else {
			SPIFFS_close(&SPI_FFS_fs, tFileDesc);
			return (0);				
		}
	}
}

static int ai_conf_initialize(void) {
	spiffs_file tFileDesc;
	tFileDesc = SPIFFS_open(&SPI_FFS_fs, AI_CONF_FILE_NAME, SPIFFS_RDONLY, 0);
	if (tFileDesc < 0) {
		// file not exist, save default configuration
		return ai_save_conf();
	} else {
		// file exist, not first time run
		if (ai_conf_load(tFileDesc) < 0) {
			printf("failed to load ai configuration from file.\n");
			SPIFFS_close(&SPI_FFS_fs, tFileDesc);
			return ai_save_conf();			
		} else {
			SPIFFS_close(&SPI_FFS_fs, tFileDesc);
			return (0);	
		}
	}	
}	

void start_ai_monitor(void const * argument) {
	static uint8_t i;
	
	while(SPI_FFS_fs.mounted != 1) {
		osDelay(100);
	}
	ai_conf_initialize();
	while (1) {
		osMutexWait(AI_DataAccessHandle, osWaitForever);
		if(HAL_ADC_Start_DMA(&hadc1, (uint32_t*)unADCxConvertedValue, 4) != HAL_OK) {
			osMutexRelease(AI_DataAccessHandle);
			/* Start Conversation Error */
			Error_Handler(); 
		}
		
		for (i = 0; i < SH_Z_002_AI_NUM; i++) {
//			nCurrentValue[i] = (uint16_t)((((double)unADCxConvertedValue[i]) * 3.3 * 1000 * 100) / (4096 * 100));
			nCurrentValue[i] = (uint16_t)((((double)unADCxConvertedValue[i]) * 3300) / 4096);
			if (nCurrentValue[i] > nCurrentHighThreshold[i]) {
				SET_BIT(bAI_HighAlarm, 0x01 << i);
			}
			if (nCurrentValue[i] < nCurrentLowThreshold[i]) {
				SET_BIT(bAI_LowAlarm, 0x01 << i);
			}
			if (nCurrentValue[i] > nCurrentHstrclMax[i]) {
				nCurrentHstrclMax[i] = nCurrentValue[i];
			}
			if (nCurrentValue[i] < nCurrentHstrclMin[i]) {
				nCurrentHstrclMin[i] = nCurrentValue[i];
			}
		}
		osMutexRelease(AI_DataAccessHandle);
		
		osDelay(500);
	}
}

