/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
unsigned long startTime = 0;
unsigned long stopTime = 0;

unsigned long startTime1 = 0;
unsigned long stopTime1 = 0;

void SystemClock_Config(void);

volatile int int_0;
void EXTI0_IRQHandler(void)
{
    EXTI->PR |= EXTI_PR_PR0;
    int_0++;

}

volatile int int_1;
void EXTI3_IRQHandler(void)
{
    EXTI->PR |= EXTI_PR_PR0;
    int_1++;

}


void SysCfg_Button0(void)
{
	// Button on PC0
	RCC->AHBENR |= RCC_AHBENR_GPIOCEN;
	GPIOC->MODER &= ~(GPIO_MODER_MODER0);
	GPIOC->PUPDR |= GPIO_PUPDR_PUPDR0_0;

	RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;
	SYSCFG->EXTICR[0] |= SYSCFG_EXTICR1_EXTI0_PC;

	EXTI->RTSR = EXTI_RTSR_TR0;
	EXTI->FTSR = EXTI_FTSR_TR0;

	NVIC_SetPriority(EXTI0_IRQn, 1);
	EXTI->IMR = EXTI_IMR_MR0;
	NVIC_EnableIRQ(EXTI0_IRQn);
}

void SysCfg_Button1(void)
{
	// Button on PB3
	RCC->AHBENR |= RCC_AHBENR_GPIOBEN;
	GPIOB->MODER &= ~(GPIO_MODER_MODER3);
	GPIOB->PUPDR |= GPIO_PUPDR_PUPDR3_0;

	RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;
	SYSCFG->EXTICR[0] |= SYSCFG_EXTICR1_EXTI3_PB;

	EXTI->RTSR = EXTI_RTSR_TR3;
	EXTI->FTSR = EXTI_FTSR_TR3;

	NVIC_SetPriority(EXTI3_IRQn, 1);
	EXTI->IMR = EXTI_IMR_MR3;
	NVIC_EnableIRQ(EXTI3_IRQn);
}

void SysCfg_LEDS(void)
{
	RCC->AHBENR |= RCC_AHBENR_GPIOAEN;
	GPIOA->MODER |= (1<<GPIO_MODER_MODER8_Pos) | (1<<GPIO_MODER_MODER9_Pos);
	GPIOA->OTYPER &= 0x00;
	GPIOA->PUPDR &= 0x00;
}

int main(void)
{
  SystemClock_Config();
  SysCfg_LEDS();
  SysCfg_Button0();
  SysCfg_Button1();
  HAL_Init();

  int lastInt_0 = 0;
  int lastInt_1 = 0;


	while (1)
	{
		if(int_0 != lastInt_0)
		{
			if ( int_0 % 2 == 1)
			{
				startTime = HAL_GetTick();
			}
			else if ( int_0 % 2 == 0)
			{
				stopTime = HAL_GetTick();

				if (stopTime - startTime > 500)
				{
					GPIOA->ODR &= ~GPIO_ODR_8;
				}
				else if ((stopTime - startTime > 20) && (stopTime - startTime < 500))
				{
					GPIOA->ODR |= GPIO_ODR_8;
				}
			}
			lastInt_0 = int_0;
		}

		if(int_1 != lastInt_1)
		{
			if ( int_1 % 2 == 1)
			{
				startTime1 = HAL_GetTick();
			}
			else if ( int_1 % 2 == 0)
			{
				stopTime1 = HAL_GetTick();
				if (stopTime1 - startTime1 > 500)
				{
					GPIOA->ODR &= ~GPIO_ODR_9;
				}
				else if ((stopTime1 - startTime1 > 20) && (stopTime - startTime < 500))
				{
					GPIOA->ODR |= GPIO_ODR_9;
				}
			}
			lastInt_1 = int_1;
		}
	}
}

  /* USER CODE END 3 */


/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL2;
  RCC_OscInitStruct.PLL.PREDIV = RCC_PREDIV_DIV1;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
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
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
