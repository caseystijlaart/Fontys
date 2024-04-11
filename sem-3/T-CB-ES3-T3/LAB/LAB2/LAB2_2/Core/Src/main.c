/* USER CODE BEGIN Header */
/**
 * Name 	: 	Casey Stijlaart (466488)
 * Course	: 	ES3 - SysTick part 2
 * Date		:	11-09-2021
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * This program is made to toggle the built in LED on a nucleo F303RE. To toggle
  * the LED there is made use of the SysTick and the SysTick handler. Based on
  * these two functions, there is a delay created.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
//#include "system_stm32f3xx.h"

volatile int32_t TimeDelay;

void SystemClock_Config(void);

void SysTick_Init()
{
	SysTick->CTRL = 0;
	SysTick->LOAD = 16000 - 1;

	NVIC_SetPriority (SysTick_IRQn, (1<<__NVIC_PRIO_BITS) - 1);

	SysTick->VAL = 0;
	SysTick->CTRL |= 0x05;
}

void SysTick_LED()
{
	RCC->AHBENR |= RCC_AHBENR_GPIOAEN;
	GPIOA->MODER |= (1<<GPIO_MODER_MODER5_Pos);
	GPIOA->OTYPER &= 0x00;
	GPIOA->PUPDR &= 0x00;
}

void DelayMs(uint32_t n)
  {
  	TimeDelay = n;
  	while(TimeDelay != 0);

  }

int main(void)
{
  SystemClock_Config();
  SysTick_Init();
  SysTick_LED();

  HAL_Init();


  while (1)
  {
	  for(int i = 0; i < 3; i++)
	  	  {
	  		  GPIOA->ODR ^= GPIO_ODR_5;
	  		  DelayMs(2000);
	  	  }

	  for(int i = 0; i < 3; i++)
	  	  	  {
	  	  		  GPIOA->ODR ^= GPIO_ODR_5;
	  	  		  DelayMs(5000);
	  	  	  }

	  for(int i = 0; i < 3; i++)
	  	  	  {
	  	  		  GPIOA->ODR ^= GPIO_ODR_5;
	  	  		  DelayMs(2000);
	  	  	  }

  }


}


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
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
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
