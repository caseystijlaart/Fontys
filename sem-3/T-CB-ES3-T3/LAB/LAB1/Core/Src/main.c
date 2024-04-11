/* USER CODE START Header */
/**
 * @assignment 		: LAB01 - LED PushButton
 * @student			: Casey Stijlaart (466488)
 * @course			: ES3 - Fontys University of Applied Science
 * @date  			: 05-09-2021
*/
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body

  ******************************************************************************
  * @attention
  *
  * This program is based on the given assignment LAB_01. With this program a LED
  * will turn on when button0 is pressed. When button1 is pressed the other LED
  * will blink. Regarding to both of the scenario's, the LED's turn of when the
  * button is released. Lastly, when both of the buttons are pressed at the same
  * time, the LED's blink SOS in morse code. The LED's keep blinking accordingly
  * until the buttons and the SOS message has ended.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

void SystemClock_Config(void);

int main(void)
{
   HAL_Init();

   RCC->AHBENR |= RCC_AHBENR_GPIOAEN | RCC_AHBENR_GPIOBEN | RCC_AHBENR_GPIOCEN;

   GPIOA->MODER &= ~(GPIO_MODER_MODER9);
   GPIOA->PUPDR |= GPIO_PUPDR_PUPDR9_0;

   GPIOB->MODER &= ~(GPIO_MODER_MODER5);
   GPIOB->PUPDR |= GPIO_PUPDR_PUPDR5_0;

   GPIOC->MODER |= (1<<GPIO_MODER_MODER4_Pos) | (1<<GPIO_MODER_MODER7_Pos);
   GPIOC->OTYPER &= 0x00;
   GPIOC->PUPDR &= 0x00;


   int pressed1 = 1;
   int pressed0 = 1;

  SystemClock_Config();

  while (1)
  {
	  // reading Button0
	  if( ((GPIOA->IDR & GPIO_IDR_9) >> 9) == 0)
	  {
	  	  pressed0 = 0;
	  } else {
	  	  pressed0 = 1;
	  }

	  // reading Button1
	  if(((GPIOB->IDR & GPIO_IDR_5) >> 5) == 0)
	  {
		  pressed1 = 0;
	  } else {
		  pressed1 = 1;
	  }

	  // SOS morse Code
	  if((pressed1 == 0) && (pressed0 == 0))
	  {

		  for (int i = 0; i < 3; i++)
		  {
			  GPIOC->ODR |= GPIO_ODR_7 | GPIO_ODR_4;
			  HAL_Delay(250);
			  GPIOC->ODR &= ~(GPIO_ODR_7) & ~(GPIO_ODR_4);
			  HAL_Delay(250);
		  }

		  for (int i = 0; i < 3; i++)
		  {
			  GPIOC->ODR |= GPIO_ODR_7 | GPIO_ODR_4;
			  HAL_Delay(500);
			  GPIOC->ODR &= ~(GPIO_ODR_7) & ~(GPIO_ODR_4);
			  HAL_Delay(250);
		  }

		  for (int i = 0; i < 3; i++)
		  {
			  GPIOC->ODR |= GPIO_ODR_7 | GPIO_ODR_4;
			  HAL_Delay(250);
			  GPIOC->ODR &= ~(GPIO_ODR_7) & ~(GPIO_ODR_4);
			  HAL_Delay(250);
		 }

		  HAL_Delay(1000);

		  // blinking LED
	  } else if(pressed1 == 0) {
		  GPIOC->ODR ^= GPIO_ODR_7;
		  HAL_Delay(400);

		  // LED on
	  } else if (pressed0 == 0) {
		  GPIOC->ODR |= GPIO_ODR_4;

		  // both LED's off.
	  } else {
		  GPIOC->ODR &= ~(GPIO_ODR_4);
		  GPIOC->ODR &= ~(GPIO_ODR_7);
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
