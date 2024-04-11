/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2022 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "string.h"
#include <stdbool.h>
#include <stdio.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define maxStrings 8
#define maxStringSize 6
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
UART_HandleTypeDef huart2;

/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = { .name = "defaultTask",
		.stack_size = 128 * 4, .priority = (osPriority_t) osPriorityNormal, };
/* Definitions for sprinkMutex1 */
osMutexId_t sprinkMutex1Handle;
const osMutexAttr_t sprinkMutex1_attributes = { .name = "sprinkMutex1" };
/* Definitions for sprinkMutex2 */
osMutexId_t sprinkMutex2Handle;
const osMutexAttr_t sprinkMutex2_attributes = { .name = "sprinkMutex2" };
/* Definitions for sprinkMutex3 */
osMutexId_t sprinkMutex3Handle;
const osMutexAttr_t sprinkMutex3_attributes = { .name = "sprinkMutex3" };
/* Definitions for sprinkMutex4 */
osMutexId_t sprinkMutex4Handle;
const osMutexAttr_t sprinkMutex4_attributes = { .name = "sprinkMutex4" };
/* Definitions for sprinkMutex5 */
osMutexId_t sprinkMutex5Handle;
const osMutexAttr_t sprinkMutex5_attributes = { .name = "sprinkMutex5" };
/* Definitions for sceneMutex1 */
osMutexId_t sceneMutex1Handle;
const osMutexAttr_t sceneMutex1_attributes = { .name = "sceneMutex1" };
/* Definitions for sceneMutex2 */
osMutexId_t sceneMutex2Handle;
const osMutexAttr_t sceneMutex2_attributes = { .name = "sceneMutex2" };
/* Definitions for sceneMutex3 */
osMutexId_t sceneMutex3Handle;
const osMutexAttr_t sceneMutex3_attributes = { .name = "sceneMutex3" };
/* USER CODE BEGIN PV */
char scene1[maxStrings][maxStringSize] = { "1on", "wait5", "2on", "1off",
		"wait5", "2off", "wait5" };
char scene2[maxStrings][maxStringSize] = { "3on", "wait5", "2on", "3off",
		"wait5", "2off", "wait5" };
char scene3[maxStrings][maxStringSize] = { "4on", "5on", "wait5", "4off",
		"wait5", "5off", "wait5" };

char scene[20][maxStringSize] = { };

osThreadId_t scene1Handle;
const osThreadAttr_t scene1_attributes = { .name = "scene1", .stack_size = 64
		* 4, .priority = (osPriority_t) osPriorityNormal, };

osThreadId_t scene2Handle;
const osThreadAttr_t scene2_attributes = { .name = "scene2", .stack_size = 64
		* 4, .priority = (osPriority_t) osPriorityNormal, };

osThreadId_t scene3Handle;
const osThreadAttr_t scene3_attributes = { .name = "scene3", .stack_size = 64
		* 4, .priority = (osPriority_t) osPriorityNormal, };

/* Definitions for myQueue01 */
osMessageQueueId_t myQueue01Handle;
const osMessageQueueAttr_t myQueue01_attributes = { .name = "myQueue01" };
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);
void StartDefaultTask(void *argument);

/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void ledsConfig(void) {
	//sprinkler 1 : B5
	RCC->AHBENR |= RCC_AHBENR_GPIOBEN;
	GPIOB->MODER |= (1 << GPIO_MODER_MODER5_Pos);
	GPIOB->OTYPER &= 0x00;
	GPIOB->PUPDR &= 0x00;

	//sprinkler 2 : A7 + sprinkler 3 : A10 + sprinkler 4 : A8 + sprinkler 5 : A9
	RCC->AHBENR |= RCC_AHBENR_GPIOAEN;
	GPIOA->MODER |= (1 << GPIO_MODER_MODER10_Pos) | (1 << GPIO_MODER_MODER8_Pos)
			| (1 << GPIO_MODER_MODER9_Pos) | (1 << GPIO_MODER_MODER7_Pos);
	GPIOA->OTYPER &= 0x00;
	GPIOA->PUPDR &= 0x00;
}

void sprinklers(int sprinklerNr, bool state) {

	switch (sprinklerNr) {
	case 1:
		if (state) {
			osMutexAcquire(sprinkMutex1Handle, osWaitForever);
			GPIOB->ODR |= GPIO_ODR_5;
		} else {
			GPIOB->ODR &= ~(GPIO_ODR_5);
			osMutexRelease(sprinkMutex1Handle);
		}
		break;

	case 2:
		if (state) {
			osMutexAcquire(sprinkMutex2Handle, osWaitForever);
			GPIOA->ODR |= GPIO_ODR_7;
		} else {
			GPIOA->ODR &= ~(GPIO_ODR_7);
			osMutexRelease(sprinkMutex2Handle);
		}
		break;

	case 3:
		if (state) {
			osMutexAcquire(sprinkMutex3Handle, osWaitForever);
			GPIOA->ODR |= GPIO_ODR_10;
		} else {
			GPIOA->ODR &= ~(GPIO_ODR_10);
			osMutexRelease(sprinkMutex3Handle);
		}
		break;

	case 4:
		if (state) {
			osMutexAcquire(sprinkMutex4Handle, osWaitForever);
			GPIOA->ODR |= GPIO_ODR_8;
		} else {
			GPIOA->ODR &= ~(GPIO_ODR_8);
			osMutexRelease(sprinkMutex4Handle);
		}
		break;

	case 5:
		if (state) {
			osMutexAcquire(sprinkMutex5Handle, osWaitForever);
			GPIOA->ODR |= GPIO_ODR_9;
		} else {
			GPIOA->ODR &= ~(GPIO_ODR_9);
			osMutexRelease(sprinkMutex5Handle);
		}
		break;
	}
}

void executeScene(void *argument) {

	osMutexAcquire(sceneMutex1Handle, osWaitForever);
	int i = 0;
	int j = 0;
	while (osMessageQueueGetCount(myQueue01Handle) != 0) {

		osMessageQueueGet(myQueue01Handle, (uint8_t*) &scene[i][j], 0, 0);
		if (j == maxStringSize) {
			i++;
			j = 0;
		}
		j++;
	}

	for (int j = 0; j < 2; j++) {
		for (int i = 0; i < maxStrings; i++) {

			if (strcmp(scene[i], "1on") == 0) {
				sprinklers(1, true);
			}

			if (strcmp(scene[i], "wait5") == 0) {
				osDelay(500);
			}

			if (strcmp(scene[i], "1off") == 0) {
				sprinklers(1, false);
			}

			if (strcmp(scene[i], "2on") == 0) {
				sprinklers(2, true);
			}

			if (strcmp(scene[i], "2off") == 0) {
				sprinklers(2, false);
			}

			if (strcmp(scene[i], "3on") == 0) {
				sprinklers(3, true);
			}

			if (strcmp(scene[i], "3off") == 0) {
				sprinklers(3, false);
			}

			if (strcmp(scene[i], "4on") == 0) {
				sprinklers(4, true);
			}

			if (strcmp(scene[i], "4off") == 0) {
				sprinklers(4, false);
			}

			if (strcmp(scene[i], "5off") == 0) {
				sprinklers(5, false);
			}
			if (strcmp(scene[i], "5on") == 0) {
				sprinklers(5, true);
			}
		}
	}
	osMutexRelease(sceneMutex1Handle);
	osThreadTerminate(osThreadGetId());

}

void excecuteScene1(void *argument) {
	osMutexAcquire(sceneMutex1Handle, osWaitForever);
	for (int j = 0; j < 2; j++) {
		for (int i = 0; i < maxStrings; i++) {

			if (strcmp(scene[i], "1on") == 0) {
				sprinklers(1, true);
			}

			if (strcmp(scene[i], "1off") == 0) {
				sprinklers(1, false);
			}

			if (strcmp(scene[i], "2on") == 0) {
				sprinklers(2, true);
			}

			if (strcmp(scene[i], "2off") == 0) {
				sprinklers(2, false);
			}

			if (strcmp(scene[i], "3on") == 0) {
				sprinklers(3, true);
			}

			if (strcmp(scene[i], "3off") == 0) {
				sprinklers(3, false);
			}
		}
	}
	osMutexRelease(sceneMutex1Handle);
	osThreadTerminate(osThreadGetId());
}

void excecuteScene2(void *argument) {
	osMutexAcquire(sceneMutex2Handle, osWaitForever);

	for (int j = 0; j < 3; j++) {

		for (int i = 0; i < maxStrings; i++) {

			if (strcmp(scene[i], "3on") == 0) {
				sprinklers(3, true);
			}

			if (strcmp(scene[i], "wait5") == 0) {
				osDelay(500);
			}

			if (strcmp(scene[i], "3off") == 0) {
				sprinklers(3, false);
			}

			if (strcmp(scene[i], "2on") == 0) {
				sprinklers(2, true);
			}

			if (strcmp(scene[i], "2off") == 0) {
				sprinklers(2, false);
			}
		}
	}
	osMutexRelease(sceneMutex2Handle);
	osThreadTerminate(osThreadGetId());
}

void excecuteScene3(void *argument) {
	osMutexAcquire(sceneMutex3Handle, osWaitForever);
	for (int j = 0; j < 5; j++) {
		for (int i = 0; i < maxStrings; i++) {

			if (strcmp(scene[i], "4on") == 0) {
				sprinklers(4, true);
			}

			if (strcmp(scene[i], "wait5") == 0) {
				osDelay(500);
			}

			if (strcmp(scene[i], "4off") == 0) {
				sprinklers(4, false);
			}

			if (strcmp(scene[i], "5off") == 0) {
				sprinklers(5, false);
			}
			if (strcmp(scene[i], "5on") == 0) {
				sprinklers(5, true);
			}
		}
	}
	osMutexRelease(sceneMutex3Handle);
	osThreadTerminate(osThreadGetId());
}
/* USER CODE END 0 */

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void) {
	/* USER CODE BEGIN 1 */

	/* USER CODE END 1 */

	/* MCU Configuration--------------------------------------------------------*/

	/* Reset of all peripherals, Initializes the Flash interface and the Systick. */
	HAL_Init();

	/* USER CODE BEGIN Init */
	ledsConfig();
	/* USER CODE END Init */

	/* Configure the system clock */
	SystemClock_Config();

	/* USER CODE BEGIN SysInit */

	/* USER CODE END SysInit */

	/* Initialize all configured peripherals */
	MX_GPIO_Init();
	MX_USART2_UART_Init();
	/* USER CODE BEGIN 2 */

	/* USER CODE END 2 */

	/* Init scheduler */
	osKernelInitialize();
	/* Create the mutex(es) */
	/* creation of sprinkMutex1 */
	sprinkMutex1Handle = osMutexNew(&sprinkMutex1_attributes);

	/* creation of sprinkMutex2 */
	sprinkMutex2Handle = osMutexNew(&sprinkMutex2_attributes);

	/* creation of sprinkMutex3 */
	sprinkMutex3Handle = osMutexNew(&sprinkMutex3_attributes);

	/* creation of sprinkMutex4 */
	sprinkMutex4Handle = osMutexNew(&sprinkMutex4_attributes);

	/* creation of sprinkMutex5 */
	sprinkMutex5Handle = osMutexNew(&sprinkMutex5_attributes);

	/* creation of sceneMutex1 */
	sceneMutex1Handle = osMutexNew(&sceneMutex1_attributes);

	/* creation of sceneMutex2 */
	sceneMutex2Handle = osMutexNew(&sceneMutex2_attributes);

	/* creation of sceneMutex3 */
	sceneMutex3Handle = osMutexNew(&sceneMutex3_attributes);

	/* USER CODE BEGIN RTOS_MUTEX */
	/* add mutexes, ... */
	/* USER CODE END RTOS_MUTEX */

	/* USER CODE BEGIN RTOS_SEMAPHORES */
	/* add semaphores, ... */
	/* USER CODE END RTOS_SEMAPHORES */

	/* USER CODE BEGIN RTOS_TIMERS */
	/* start timers, add new ones, ... */
	/* USER CODE END RTOS_TIMERS */

	/* USER CODE BEGIN RTOS_QUEUES */
	myQueue01Handle = osMessageQueueNew(50, sizeof(char),
			&myQueue01_attributes);
	/* USER CODE END RTOS_QUEUES */

	/* Create the thread(s) */
	/* creation of defaultTask */
	defaultTaskHandle = osThreadNew(StartDefaultTask, NULL,
			&defaultTask_attributes);

	/* USER CODE BEGIN RTOS_THREADS */
	/* add threads, ... */

	/* USER CODE END RTOS_THREADS */

	/* USER CODE BEGIN RTOS_EVENTS */
	/* add events, ... */
	/* USER CODE END RTOS_EVENTS */

	/* Start scheduler */
	osKernelStart();

	/* We should never get here as control is now taken by the scheduler */
	/* Infinite loop */
	/* USER CODE BEGIN WHILE */
	while (1) {
		/* USER CODE END WHILE */

		/* USER CODE BEGIN 3 */
	}
	/* USER CODE END 3 */
}

/**
 * @brief System Clock Configuration
 * @retval None
 */
void SystemClock_Config(void) {
	RCC_OscInitTypeDef RCC_OscInitStruct = { 0 };
	RCC_ClkInitTypeDef RCC_ClkInitStruct = { 0 };
	RCC_PeriphCLKInitTypeDef PeriphClkInit = { 0 };

	/** Initializes the RCC Oscillators according to the specified parameters
	 * in the RCC_OscInitTypeDef structure.
	 */
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
	RCC_OscInitStruct.HSIState = RCC_HSI_ON;
	RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
	if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
		Error_Handler();
	}
	/** Initializes the CPU, AHB and APB buses clocks
	 */
	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
			| RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

	if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK) {
		Error_Handler();
	}
	PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART2;
	PeriphClkInit.Usart2ClockSelection = RCC_USART2CLKSOURCE_PCLK1;
	if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK) {
		Error_Handler();
	}
}

/**
 * @brief USART2 Initialization Function
 * @param None
 * @retval None
 */
static void MX_USART2_UART_Init(void) {

	/* USER CODE BEGIN USART2_Init 0 */

	/* USER CODE END USART2_Init 0 */

	/* USER CODE BEGIN USART2_Init 1 */

	/* USER CODE END USART2_Init 1 */
	huart2.Instance = USART2;
	huart2.Init.BaudRate = 9600;
	huart2.Init.WordLength = UART_WORDLENGTH_8B;
	huart2.Init.StopBits = UART_STOPBITS_1;
	huart2.Init.Parity = UART_PARITY_NONE;
	huart2.Init.Mode = UART_MODE_TX_RX;
	huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	huart2.Init.OverSampling = UART_OVERSAMPLING_16;
	huart2.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
	huart2.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
	if (HAL_UART_Init(&huart2) != HAL_OK) {
		Error_Handler();
	}
	/* USER CODE BEGIN USART2_Init 2 */

	/* USER CODE END USART2_Init 2 */

}

/**
 * @brief GPIO Initialization Function
 * @param None
 * @retval None
 */
static void MX_GPIO_Init(void) {

	/* GPIO Ports Clock Enable */
	__HAL_RCC_GPIOA_CLK_ENABLE();

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
void StartDefaultTask(void *argument) {
	/* USER CODE BEGIN 5 */
	/* Infinite loop */
	uint8_t in[1] = { 0 };

	for (;;) {
		HAL_UART_Receive(&huart2, in, 1, HAL_MAX_DELAY);

		if (in[0] == '1') {
			for (int i = 0; i < (maxStrings); i++) {
				for (int j = 0; j < maxStringSize; j++) {
					osMessageQueuePut(myQueue01Handle, &scene1[i][j], 1, 0);
				}
			}
			scene1Handle = osThreadNew(executeScene, NULL, &scene1_attributes);
		} else if (in[0] == '2') {
			for (int i = 0; i < (maxStrings); i++) {
				for (int j = 0; j < maxStringSize; j++) {
					osMessageQueuePut(myQueue01Handle, &scene2[i][j], 1, 0);
				}
			}
			scene2Handle = osThreadNew(executeScene, NULL, &scene2_attributes);
		} else if (in[0] == '3') {
			for (int i = 0; i < (maxStrings); i++) {
				for (int j = 0; j < maxStringSize; j++) {
					osMessageQueuePut(myQueue01Handle, &scene3[i][j], 1, 0);
				}
			}
			scene3Handle = osThreadNew(executeScene, NULL, &scene3_attributes);
		}
	}
	osThreadTerminate(osThreadGetId());
	/* USER CODE END 5 */
}

/**
 * @brief  Period elapsed callback in non blocking mode
 * @note   This function is called  when TIM6 interrupt took place, inside
 * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
 * a global variable "uwTick" used as application time base.
 * @param  htim : TIM handle
 * @retval None
 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
	/* USER CODE BEGIN Callback 0 */

	/* USER CODE END Callback 0 */
	if (htim->Instance == TIM6) {
		HAL_IncTick();
	}
	/* USER CODE BEGIN Callback 1 */

	/* USER CODE END Callback 1 */
}

/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void Error_Handler(void) {
	/* USER CODE BEGIN Error_Handler_Debug */
	/* User can add his own implementation to report the HAL error return state */
	__disable_irq();
	while (1) {
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
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

