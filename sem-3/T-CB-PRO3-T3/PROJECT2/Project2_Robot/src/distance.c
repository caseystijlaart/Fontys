/*
 * Distance.c
 *
 *  Created on: 15 Jan 2022
 *      Author: Casey
 */

#include <distance.h>

int HCSR04_Read(void) {
	HAL_GPIO_WritePin(TRIG_PORT, TRIG_PIN, GPIO_PIN_SET);
	micro_Delay(10);
	HAL_GPIO_WritePin(TRIG_PORT, TRIG_PIN, GPIO_PIN_RESET);
	__HAL_TIM_ENABLE_IT(&htim1, TIM_IT_CC1);
	return Distance;
}

void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim) {
	uint32_t inter1 = 0;
	uint32_t inter2 = 0;
	uint32_t time = 0;
	uint8_t captured = 0;

	if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1) {
		if (captured == 0) {
			inter1 = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_1);
			captured++;
			__HAL_TIM_SET_CAPTUREPOLARITY(htim, TIM_CHANNEL_1,
					TIM_INPUTCHANNELPOLARITY_FALLING);
		}

		else if (captured == 1)
				{
			inter2 = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_1);
			__HAL_TIM_SET_COUNTER(htim, 0);

			if (inter2 > inter1) {
				time = inter2 - inter1;
			} else if (inter1 > inter2) {
				time = (0xffff - inter1) + inter2;
			}

			Distance = time * 0.034 / 2;
			captured = 0;

			__HAL_TIM_SET_CAPTUREPOLARITY(htim, TIM_CHANNEL_1,
					TIM_INPUTCHANNELPOLARITY_RISING);
			__HAL_TIM_DISABLE_IT(&htim1, TIM_IT_CC1);
		}
	}
}
