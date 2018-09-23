
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * This notice applies to any and all portions of this file
  * that are not between comment pairs USER CODE BEGIN and
  * USER CODE END. Other portions of this file, whether 
  * inserted by the user or by software development tools
  * are owned by their respective copyright owners.
  *
  * Copyright (c) 2018 STMicroelectronics International N.V. 
  * All rights reserved.
  *
  * Redistribution and use in source and binary forms, with or without 
  * modification, are permitted, provided that the following conditions are met:
  *
  * 1. Redistribution of source code must retain the above copyright notice, 
  *    this list of conditions and the following disclaimer.
  * 2. Redistributions in binary form must reproduce the above copyright notice,
  *    this list of conditions and the following disclaimer in the documentation
  *    and/or other materials provided with the distribution.
  * 3. Neither the name of STMicroelectronics nor the names of other 
  *    contributors to this software may be used to endorse or promote products 
  *    derived from this software without specific written permission.
  * 4. This software, including modifications and/or derivative works of this 
  *    software, must execute solely and exclusively on microcontroller or
  *    microprocessor devices manufactured by or for STMicroelectronics.
  * 5. Redistribution and use of this software other than as permitted under 
  *    this license is void and will automatically terminate your rights under 
  *    this license. 
  *
  * THIS SOFTWARE IS PROVIDED BY STMICROELECTRONICS AND CONTRIBUTORS "AS IS" 
  * AND ANY EXPRESS, IMPLIED OR STATUTORY WARRANTIES, INCLUDING, BUT NOT 
  * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A 
  * PARTICULAR PURPOSE AND NON-INFRINGEMENT OF THIRD PARTY INTELLECTUAL PROPERTY
  * RIGHTS ARE DISCLAIMED TO THE FULLEST EXTENT PERMITTED BY LAW. IN NO EVENT 
  * SHALL STMICROELECTRONICS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
  * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
  * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, 
  * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF 
  * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING 
  * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
  * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "stm32f4xx_hal.h"
#include "cmsis_os.h"
#include "lwip.h"

/* USER CODE BEGIN Includes */
#include "spi_flash.h"
#include "spiffs.h"
#include "cJSON.h"
#include "lwip/apps/tftp_server.h"
#include "di_monitor.h"

/* USER CODE END Includes */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc1;
DMA_HandleTypeDef hdma_adc1;

SPI_HandleTypeDef hspi3;
DMA_HandleTypeDef hdma_spi3_rx;
DMA_HandleTypeDef hdma_spi3_tx;

TIM_HandleTypeDef htim7;

osThreadId defaultTaskHandle;
osThreadId modbus_tcpHandle;
osThreadId ai_monitorHandle;
osThreadId di_monitorHandle;
osThreadId webserverHandle;
osTimerId GARP_TimerHandle;
osMutexId AI_DataAccessHandle;
osMutexId DI_DataAccessHandle;
osMutexId SpiffsMutexHandle;
osMutexId SpiFlashChipMutexHandle;
osMutexId WebServerFileMutexHandle;

/* USER CODE BEGIN PV */
/* Private variables ---------------------------------------------------------*/
EventGroupHandle_t xDiEventGroup;
EventGroupHandle_t xComEventGroup;
char SH_Z_002_SN[SH_Z_SN_LEN + 1];
extern struct netif gnetif;
extern spiffs SPI_FFS_fs;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_ADC1_Init(void);
static void MX_TIM7_Init(void);
static void MX_SPI3_Init(void);
void StartDefaultTask(void const * argument);
extern void start_modbus_tcp_server(void const * argument);
extern void start_ai_monitor(void const * argument);
extern void start_di_monitor(void const * argument);
extern void start_webserver(void const * argument);
void send_GARP(void const * argument);

/* USER CODE BEGIN PFP */
/* Private function prototypes -----------------------------------------------*/

/* USER CODE END PFP */

/* USER CODE BEGIN 0 */
//static void send_GARP(void const * argument) {
////	printf("send GARP timer enter\n");
//	etharp_gratuitous(&gnetif);
////	printf("send GARP timer leave\n");
//}

static void* tftp_file_open(const char* fname, const char* mode, u8_t write) {
	spiffs_file nFileHandle;
	osMutexWait(WebServerFileMutexHandle, osWaitForever);
	if (write) {
		nFileHandle = SPIFFS_open(&SPI_FFS_fs, fname, SPIFFS_CREAT | SPIFFS_TRUNC | SPIFFS_RDWR, 0);
		printf("errno %d\n", SPIFFS_errno(&SPI_FFS_fs));
	} else {
		nFileHandle = SPIFFS_open(&SPI_FFS_fs, fname, SPIFFS_RDONLY, 0);
	}
	if (nFileHandle <= 0 ) {
		osMutexRelease(WebServerFileMutexHandle);
		return NULL;
	} else {
		return ((void*)((uint32_t)nFileHandle));
	}	
}

static void tftp_file_close(void* handle) {
	SPIFFS_close(&SPI_FFS_fs, (spiffs_file)handle);
	osMutexRelease(WebServerFileMutexHandle);
}

static int tftp_file_read(void* handle, void* buf, int bytes) {
	int res;
	res = SPIFFS_read(&SPI_FFS_fs, (spiffs_file)handle, (u8_t *)buf, bytes);
	return res;
}

static int tftp_file_write(void* handle, struct pbuf* p) {
	return SPIFFS_write(&SPI_FFS_fs, (spiffs_file)handle, p->payload, p->len);
}

const struct tftp_context TFTP_Ctx = {.open = tftp_file_open, .close = tftp_file_close, .read = tftp_file_read, .write = tftp_file_write};

static int create_default_sh_z_002_info(void) {
	spiffs_file tFileDesc;
    char* pJsonString = NULL;
    cJSON* pSN = NULL;
	cJSON* pDI_ConfJsonWriter;
	
	tFileDesc = SPIFFS_open(&SPI_FFS_fs, SH_Z_002_INFO_FILE_NAME, SPIFFS_RDWR | SPIFFS_CREAT, 0);

	pDI_ConfJsonWriter = cJSON_CreateObject();
	if (pDI_ConfJsonWriter == NULL){
		SPIFFS_close(&SPI_FFS_fs, tFileDesc);
		printf("failed to create json root object.\n");
		return (-1);
	}

    pSN = cJSON_CreateString("123456789ABCDEFG");
    if (pSN == NULL){
		SPIFFS_close(&SPI_FFS_fs, tFileDesc);
		cJSON_Delete(pDI_ConfJsonWriter);
		printf("failed to create json latch set object.\n");
		return (-1);
    }	
	cJSON_AddItemToObject(pDI_ConfJsonWriter, DEVICE_SN_JSON_TAG, pSN);
	
	pJsonString = cJSON_Print(pDI_ConfJsonWriter);
    if (pJsonString == NULL){
		SPIFFS_close(&SPI_FFS_fs, tFileDesc);
		cJSON_Delete(pDI_ConfJsonWriter);
		printf("failed to digest json object.\n");
		return (-1);
    }
	
	if (SPIFFS_write(&SPI_FFS_fs, tFileDesc, pJsonString, strlen(pJsonString)) < 0 ) {
		SPIFFS_close(&SPI_FFS_fs, tFileDesc);
		cJSON_Delete(pDI_ConfJsonWriter);
		free(pJsonString);
		printf("failed to write digested json string to file.\n");
		return (-1);		
	}
	SPIFFS_close(&SPI_FFS_fs, tFileDesc);
	cJSON_Delete(pDI_ConfJsonWriter);
	free(pJsonString);
	return (0);	
}

static int load_sh_z_002_info(spiffs_file tFileDesc) {
    cJSON* pSN = NULL;
    cJSON* pDI_ConfJson;
	char cConfString[256];
	int nReadNum = SPIFFS_read(&SPI_FFS_fs, tFileDesc, cConfString, sizeof(cConfString));
	if ((nReadNum <= 0) || (sizeof(cConfString) == nReadNum )) {
		printf("%d char was read from conf file.\n", nReadNum);
		return (-1);		
	}

	pDI_ConfJson = cJSON_Parse(cConfString);
    if (pDI_ConfJson == NULL) {
        const char *error_ptr = cJSON_GetErrorPtr();
        if (error_ptr != NULL){
            printf("Error before: %s\n", error_ptr);
        }
        return (-1);
    }

    pSN = cJSON_GetObjectItemCaseSensitive(pDI_ConfJson, DEVICE_SN_JSON_TAG);
    if (cJSON_IsString(pSN)){
		strncpy(SH_Z_002_SN, pSN->valuestring, sizeof(SH_Z_002_SN));
		SH_Z_002_SN[SH_Z_SN_LEN] = '\0';
    }	
	
	cJSON_Delete(pDI_ConfJson);
	return 0;
	
}

static void sh_z_002_info_init(void) {
	spiffs_file tFileDesc;
	
	tFileDesc = SPIFFS_open(&SPI_FFS_fs, SH_Z_002_INFO_FILE_NAME, SPIFFS_RDONLY, 0);
	if (tFileDesc < 0) {
		// file not exist, save default configuration
		create_default_sh_z_002_info();
	} else {
		// file exist, not first time run
		load_sh_z_002_info(tFileDesc);
		SPIFFS_close(&SPI_FFS_fs, tFileDesc);
	}	
}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  *
  * @retval None
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration----------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */
	
  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_ADC1_Init();
  MX_TIM7_Init();
  MX_SPI3_Init();
  /* USER CODE BEGIN 2 */

  /* USER CODE END 2 */

  /* Create the mutex(es) */
  /* definition and creation of AI_DataAccess */
  osMutexDef(AI_DataAccess);
  AI_DataAccessHandle = osMutexCreate(osMutex(AI_DataAccess));

  /* definition and creation of DI_DataAccess */
  osMutexDef(DI_DataAccess);
  DI_DataAccessHandle = osMutexCreate(osMutex(DI_DataAccess));

  /* definition and creation of SpiffsMutex */
  osMutexDef(SpiffsMutex);
  SpiffsMutexHandle = osMutexCreate(osMutex(SpiffsMutex));

  /* definition and creation of SpiFlashChipMutex */
  osMutexDef(SpiFlashChipMutex);
  SpiFlashChipMutexHandle = osMutexCreate(osMutex(SpiFlashChipMutex));

  /* definition and creation of WebServerFileMutex */
  osMutexDef(WebServerFileMutex);
  WebServerFileMutexHandle = osMutexCreate(osMutex(WebServerFileMutex));

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  xDiEventGroup = xEventGroupCreate();
  if( xDiEventGroup == NULL ) {
    printf("DI event group create failed.\n");
  }
  xComEventGroup = xEventGroupCreate();
  if( xComEventGroup == NULL ) {
    printf("Communication event group create failed.\n");
  }  
  
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* Create the timer(s) */
  /* definition and creation of GARP_Timer */
  osTimerDef(GARP_Timer, send_GARP);
  GARP_TimerHandle = osTimerCreate(osTimer(GARP_Timer), osTimerPeriodic, NULL);

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
	osTimerStart(GARP_TimerHandle, 5000);

  /* USER CODE END RTOS_TIMERS */

  /* Create the thread(s) */
  /* definition and creation of defaultTask */
  osThreadDef(defaultTask, StartDefaultTask, osPriorityNormal, 0, 256);
  defaultTaskHandle = osThreadCreate(osThread(defaultTask), NULL);

  /* definition and creation of modbus_tcp */
  osThreadDef(modbus_tcp, start_modbus_tcp_server, osPriorityIdle, 0, 256);
  modbus_tcpHandle = osThreadCreate(osThread(modbus_tcp), NULL);

  /* definition and creation of ai_monitor */
  osThreadDef(ai_monitor, start_ai_monitor, osPriorityIdle, 0, 512);
  ai_monitorHandle = osThreadCreate(osThread(ai_monitor), NULL);

  /* definition and creation of di_monitor */
  osThreadDef(di_monitor, start_di_monitor, osPriorityIdle, 0, 512);
  di_monitorHandle = osThreadCreate(osThread(di_monitor), NULL);

  /* definition and creation of webserver */
  osThreadDef(webserver, start_webserver, osPriorityIdle, 0, 256);
  webserverHandle = osThreadCreate(osThread(webserver), NULL);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  
  
  /* USER CODE END RTOS_QUEUES */
 

  /* Start scheduler */
  osKernelStart();
  
  /* We should never get here as control is now taken by the scheduler */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */

  while (1)
  {

  /* USER CODE END WHILE */

  /* USER CODE BEGIN 3 */

  }
  /* USER CODE END 3 */

}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{

  RCC_OscInitTypeDef RCC_OscInitStruct;
  RCC_ClkInitTypeDef RCC_ClkInitStruct;

    /**Configure the main internal regulator output voltage 
    */
  __HAL_RCC_PWR_CLK_ENABLE();

  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

    /**Initializes the CPU, AHB and APB busses clocks 
    */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = 16;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 168;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 7;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Initializes the CPU, AHB and APB busses clocks 
    */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Configure the Systick interrupt time 
    */
  HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000);

    /**Configure the Systick 
    */
  HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

  /* SysTick_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(SysTick_IRQn, 15, 0);
}

/* ADC1 init function */
static void MX_ADC1_Init(void)
{

  ADC_ChannelConfTypeDef sConfig;

    /**Configure the global features of the ADC (Clock, Resolution, Data Alignment and number of conversion) 
    */
  hadc1.Instance = ADC1;
  hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV4;
  hadc1.Init.Resolution = ADC_RESOLUTION_12B;
  hadc1.Init.ScanConvMode = ENABLE;
  hadc1.Init.ContinuousConvMode = DISABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 4;
  hadc1.Init.DMAContinuousRequests = ENABLE;
  hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time. 
    */
  sConfig.Channel = ADC_CHANNEL_3;
  sConfig.Rank = 1;
  sConfig.SamplingTime = ADC_SAMPLETIME_3CYCLES;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time. 
    */
  sConfig.Channel = ADC_CHANNEL_4;
  sConfig.Rank = 2;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time. 
    */
  sConfig.Channel = ADC_CHANNEL_5;
  sConfig.Rank = 3;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time. 
    */
  sConfig.Channel = ADC_CHANNEL_6;
  sConfig.Rank = 4;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

}

/* SPI3 init function */
static void MX_SPI3_Init(void)
{

  /* SPI3 parameter configuration*/
  hspi3.Instance = SPI3;
  hspi3.Init.Mode = SPI_MODE_MASTER;
  hspi3.Init.Direction = SPI_DIRECTION_2LINES;
  hspi3.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi3.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi3.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi3.Init.NSS = SPI_NSS_SOFT;
  hspi3.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
  hspi3.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi3.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi3.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi3.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi3) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

}

/* TIM7 init function */
static void MX_TIM7_Init(void)
{

  TIM_MasterConfigTypeDef sMasterConfig;

  htim7.Instance = TIM7;
  htim7.Init.Prescaler = 0;
  htim7.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim7.Init.Period = 0;
  if (HAL_TIM_Base_Init(&htim7) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim7, &sMasterConfig) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

}

/** 
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void) 
{
  /* DMA controller clock enable */
  __HAL_RCC_DMA1_CLK_ENABLE();
  __HAL_RCC_DMA2_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA1_Stream0_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Stream0_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(DMA1_Stream0_IRQn);
  /* DMA1_Stream5_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Stream5_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(DMA1_Stream5_IRQn);
  /* DMA2_Stream0_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA2_Stream0_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(DMA2_Stream0_IRQn);

}

/** Configure pins as 
        * Analog 
        * Input 
        * Output
        * EVENT_OUT
        * EXTI
*/
static void MX_GPIO_Init(void)
{

  GPIO_InitTypeDef GPIO_InitStruct;

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(SPI_FLASH_CS_GPIO_Port, SPI_FLASH_CS_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOD, RELAY_0_Pin|RELAY_1_Pin|RELAY_2_Pin|RELAY_3_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : DI_0_Pin DI_1_Pin DI_2_Pin DI_3_Pin */
  GPIO_InitStruct.Pin = DI_0_Pin|DI_1_Pin|DI_2_Pin|DI_3_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  /*Configure GPIO pin : SPI_FLASH_CS_Pin */
  GPIO_InitStruct.Pin = SPI_FLASH_CS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(SPI_FLASH_CS_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : RELAY_0_Pin RELAY_1_Pin RELAY_2_Pin RELAY_3_Pin */
  GPIO_InitStruct.Pin = RELAY_0_Pin|RELAY_1_Pin|RELAY_2_Pin|RELAY_3_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI9_5_IRQn, 12, 0);
  HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);

  HAL_NVIC_SetPriority(EXTI15_10_IRQn, 12, 0);
  HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used 
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void const * argument)
{
  /* init code for LWIP */
  MX_LWIP_Init();

  /* USER CODE BEGIN 5 */
	printf("Main app default task start.\n");
	spiffs_init();
	sh_z_002_info_init();
	tftp_init(&TFTP_Ctx);	
  /* Infinite loop */
  for(;;)
  {
    osDelay(5000);
  }
  /* USER CODE END 5 */ 
}

/* send_GARP function */
void send_GARP(void const * argument)
{
  /* USER CODE BEGIN send_GARP */
  etharp_gratuitous(&gnetif);
  /* USER CODE END send_GARP */
}

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM1 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM1) {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */

  /* USER CODE END Callback 1 */
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @param  file: The file name as string.
  * @param  line: The line in file as a number.
  * @retval None
  */
void _Error_Handler(char *file, int line)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  while(1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{ 
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
