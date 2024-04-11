/*
 * wheel.c
 *
 *  Created on: 30 Nov 2021
 *      Author: casey
 */

#define _USE_MATH_DEFINES

#include <stdlib.h>
#include <stdint.h>
#include <stm32f3xx.h>
#include <stdbool.h>
#include <math.h>
#include "wheel.h"
#include "timer.h"

//////////////////////////////////////////
// Private functions
//////////////////////////////////////////
void TIM3_IRQHandler(void)
{
	TIM3->SR = ~TIM_SR_UIF; // Reset interrupt flag. Is needed, otherwise interrupt handler will be called again.
	adjust_Wheel();
}

void wheel_Init(wheel_Info wheel)
{
	int retPin = init_Pin(wheel);
	if (retPin == 0)
	{

		uint32_t ARR_Val = 0;
		if (wheel.tim_Instance == TIM2)
		{
			RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;
			TIM2->PSC = 0x00;

			ARR_Val = ((0.02 * wheel.clock_Speed) - 1);
			wheel.ARR_Val = ARR_Val;
			TIM2->ARR = ARR_Val;
			TIM2->CNT = 0x00;
			TIM2->CR1 |= TIM_CR1_CEN;

			if (wheel.channel == HAL_TIM_ACTIVE_CHANNEL_1)
			{
				TIM2->CCMR1 &= ~TIM_CCMR1_CC1S & ~TIM_CCMR1_OC1PE & ~TIM_CCMR1_CC2S & ~TIM_CCMR1_OC1M_3;
				TIM2->CCMR1 |= TIM_CCMR1_OC1M_1 | TIM_CCMR1_OC1M_2;
				TIM2->CCER |= TIM_CCER_CC1E;
			}
			else if (wheel.channel == HAL_TIM_ACTIVE_CHANNEL_2)
			{
				TIM2->CCMR1 &= ~TIM_CCMR1_CC2S & ~TIM_CCMR1_OC2PE & ~TIM_CCMR1_CC1S & ~TIM_CCMR1_OC2M_3;
				TIM2->CCMR1 |= TIM_CCMR1_OC2M_1 | TIM_CCMR1_OC2M_2;
				TIM2->CCER |= TIM_CCER_CC2E;
			}
			else if (wheel.channel == HAL_TIM_ACTIVE_CHANNEL_3)
			{
				TIM2->CCMR2 &= ~TIM_CCMR2_CC3S & ~TIM_CCMR2_OC3PE & ~TIM_CCMR2_CC4S & ~TIM_CCMR2_OC4M_3;
				TIM2->CCMR2 |= TIM_CCMR2_OC3M_1 | TIM_CCMR2_OC3M_2;
				TIM2->CCER |= TIM_CCER_CC3E;
			}
			else if (wheel.channel == HAL_TIM_ACTIVE_CHANNEL_4)
			{
				TIM2->CCMR2 &= ~TIM_CCMR2_CC4S & ~TIM_CCMR2_OC4PE & ~TIM_CCMR2_CC3S & ~TIM_CCMR2_OC3M_3;
				TIM2->CCMR2 |= TIM_CCMR2_OC4M_1 | TIM_CCMR2_OC4M_2;
				TIM2->CCER |= TIM_CCER_CC4E;
			}
		}
	}

	feedbackSignal_Init();
}

int init_Pin(wheel_Info wheel)
{
	bool AFR = false;
	if ((wheel.pin_Number <= 7) && (wheel.pin_Number >= 0))
	{
		AFR = true;
	}
	else if ((wheel.pin_Number <= 15) && (wheel.pin_Number >= 8))
	{
		AFR = false;
	}
	else
	{
		return -1;
	}

	if (wheel.GPIO == GPIOA)
	{
		RCC->AHBENR |= RCC_AHBENR_GPIOAEN;
		GPIOA->MODER |= (0b10 << (2 * wheel.pin_Number));
		if (AFR == true)
		{
			GPIOA->AFR[0] = (AFR << (4 * wheel.pin_Number));
		}
		else if (AFR == false)
		{
			GPIOA->AFR[1] = (AFR << (4 * wheel.pin_Number));
		}

		GPIOA->OTYPER &= 0x00;
		GPIOA->PUPDR &= 0x00;
	}
	else if (wheel.GPIO == GPIOB)
	{
		RCC->AHBENR |= RCC_AHBENR_GPIOBEN;
		GPIOB->MODER |= (0b10 << (2 * wheel.pin_Number));

		if (AFR == true)
		{
			GPIOB->AFR[0] = (AFR << (4 * wheel.pin_Number));
		}
		else if (AFR == false)
		{
			GPIOB->AFR[1] = (AFR << (4 * wheel.pin_Number));
		}

		GPIOB->OTYPER &= 0x00;
		GPIOB->PUPDR &= 0x00;
	}
	else if (wheel.GPIO == GPIOC)
	{
		RCC->AHBENR |= RCC_AHBENR_GPIOCEN;
		GPIOC->MODER |= (0b10 << (2 * wheel.pin_Number));

		if (AFR == true)
		{
			GPIOC->AFR[0] = (AFR << (4 * wheel.pin_Number));
		}
		else if (AFR == false)
		{
			GPIOC->AFR[1] = (AFR << (4 * wheel.pin_Number));
		}

		GPIOC->OTYPER &= 0x00;
		GPIOC->PUPDR &= 0x00;
	}
	else
	{
		return -1;
	}
	return 0;
}

uint32_t calc_CCR_Speed(float speed, wheel_Info wheel)
{
	float R = 60.0 * speed;
	float circum = 7.0 * M_PI;
	float RPM = R / circum;
	float tControl = 0;

	if ((speed < 0) && (speed > -51.31))
	{
		float perM = (wheel.max_backward_tControl - wheel.min_backward_tControl) / 140;
		tControl = wheel.max_backward_tControl + (RPM * perM);
	}
	else if ((speed >= 0) && (speed < 51.31))
	{
		float perM = (wheel.max_forward_tControl - wheel.min_forward_tControl) / 140;
		tControl = wheel.min_forward_tControl + (RPM * perM);
	}
	uint32_t CCR = ((tControl / 1000) / 20) * wheel.ARR_Val;
	return CCR;
}

int rotate(wheel_Info wheel, float speed)
{
	uint32_t CCR = calc_CCR_Speed(speed, wheel);

	if (wheel.channel == HAL_TIM_ACTIVE_CHANNEL_1)
	{
		wheel.tim_Instance->CCR1 = CCR;
	}
	else if (wheel.channel == HAL_TIM_ACTIVE_CHANNEL_2)
	{
		wheel.tim_Instance->CCR2 = CCR;
	}
	else if (wheel.channel == HAL_TIM_ACTIVE_CHANNEL_3)
	{
		wheel.tim_Instance->CCR3 = CCR;
	}
	else if (wheel.channel == HAL_TIM_ACTIVE_CHANNEL_4)
	{
		wheel.tim_Instance->CCR4 = CCR;
	}
	else
	{
		return -1;
	}
	return 0;
}

int generateRandomNumber()
{
	int rndSeed = get_Time();
	srand(rndSeed);

	int number = 0;
	while (number == 0)
	{
		number = rand() % 180 + (-90);
	}
	return number;
}

void adjust_Wheel()
{
	float wheel1 = get_Feedback(GPIOA, GPIO_PIN_7);
	float wheel2 = get_Feedback(GPIOA, GPIO_PIN_11);

	if (wheel1 != wheel2)
	{
		int newCCR = wheel1 / 0.02 * (320000 - 1);
		wheel_2.tim_Instance->CCR1 |= newCCR;
	}
}

float get_Feedback(GPIO_TypeDef *GPIO, uint8_t pinNr)
{
	int unitsFC = 360;
	int dutyScale = 1000;
	int dcMin = 29;
	int dcMax = 971;

	int dc, theta, thetaP, tHigh, tLow;
	tLow = (!(HAL_GPIO_ReadPin(GPIO, pinNr)));
	tHigh = (HAL_GPIO_ReadPin(GPIO, pinNr));

	dc = (dutyScale * tHigh) / (tHigh + tLow);
	return dc;
}

void feedbackSignal_Init(void)
{
	// Wheel 1
	RCC->APB1ENR |= RCC_APB1ENR_TIM3EN;
	TIM3->PSC |= 16 - 1;
	TIM3->CCMR1 |= TIM_CCMR1_CC1S_1;
	TIM3->CCER &= ~(TIM_CCER_CC1NP_Pos) & ~(TIM_CCER_CC1P_Pos);
	TIM3->CCER |= TIM_CCER_CC1E_Pos;

	TIM3->CCMR1 |= TIM_CCMR1_CC2S_1;
	TIM3->CCER |= TIM_CCER_CC2P_Pos;
	TIM3->CCER &= ~(TIM_CCER_CC2NE_Pos);
	TIM3->CCER |= TIM_CCER_CC2E_Pos;
	TIM3->SMCR &= ~(TIM_SMCR_TS_1) & ~(TIM_SMCR_SMS_1) & ~(TIM_SMCR_SMS_0);
	TIM3->SMCR |= TIM_SMCR_TS_2 | TIM_SMCR_TS_0 | TIM_SMCR_SMS_2;

	TIM3->CR1 |= 0x00;

	setInterrupt();

	// wheel 2
	RCC->APB1ENR |= RCC_APB1ENR_TIM4EN;
	TIM4->PSC |= 16 - 1;
	TIM4->CCMR1 |= TIM_CCMR1_CC1S_1;
	TIM4->CCER &= ~(TIM_CCER_CC1NP_Pos) & ~(TIM_CCER_CC1P_Pos);
	TIM4->CCER |= TIM_CCER_CC1E_Pos;

	TIM4->CCMR1 |= TIM_CCMR1_CC2S_1;
	TIM4->CCER |= TIM_CCER_CC2P_Pos;
	TIM4->CCER &= ~(TIM_CCER_CC2NE_Pos);
	TIM4->CCER |= TIM_CCER_CC2E_Pos;
	TIM4->SMCR &= ~(TIM_SMCR_TS_1) & ~(TIM_SMCR_SMS_1) & ~(TIM_SMCR_SMS_0);
	TIM4->SMCR |= TIM_SMCR_TS_2 | TIM_SMCR_TS_0 | TIM_SMCR_SMS_2;

	pinsFeedBack_Init();
}

void setInterrupt()
{
	TIM3->EGR |= TIM_EGR_UG;
	TIM3->SR &= ~TIM_SR_UIF;
	TIM3->ARR = 100;
	NVIC_EnableIRQ(TIM3_IRQn);
	TIM3->DIER |= TIM_DIER_UIE;
	TIM3->CR1 |= TIM_CR1_CEN;
}

void pinsFeedBack_Init(void)
{
	// Side One
	RCC->AHBENR |= RCC_AHBENR_GPIOAEN;
	GPIOA->MODER |= GPIO_MODER_MODER7_1;
	GPIOA->AFR[0] |= (0b10 << 24);
	GPIOA->OTYPER &= 0x00;
	GPIOA->PUPDR &= 0x00;

	RCC->AHBENR |= RCC_AHBENR_GPIOAEN;
	GPIOA->MODER |= GPIO_MODER_MODER11_1;
	GPIOA->AFR[1] |= (0b1010 << 12);
	GPIOA->OTYPER &= 0x00;
	GPIOA->PUPDR &= 0x00;
}

//////////////////////////////////////////
// Public functions
//////////////////////////////////////////
void wheels_Init(wheel_Info wheel1, wheel_Info wheel2)
{
	wheel_Init(wheel1);
	wheel_Init(wheel2);
}

int drive(wheel_Info wheel, wheel_Info wheel2, float speed)
{
	int ret1 = rotate(wheel, speed);
	int ret2 = rotate(wheel2, -(speed));

	if (ret1 == 1 || ret2 == 1)
	{
		return -1;
	}
	return 0;
}

int turnRandom(wheel_Info wheel_1, wheel_Info wheel_2, float curSpeed)
{
	int angle = generateRandomNumber();
	float circumference = 7.0 * M_PI;
	float arcLength = circumference * (90 / 360.0);

	// to make it turn more smoothly and accurate, the speed is set
	// to 10cm per second.
	float turnDuration = arcLength * 5;

	if ((angle < 90) && (angle > 0))
	{
		rotate(wheel_1, 0);
		rotate(wheel_2, -5);
		micro_Delay((uint32_t)ceil(turnDuration));
	}
	else if ((angle > -90) && (angle < 0))
	{
		rotate(wheel_2, 0);
		rotate(wheel_1, 5);
		micro_Delay((uint32_t)ceil(turnDuration));
	}
	else
	{
		return -1;
	}
	drive(wheel_1, wheel_2, curSpeed);
	return 0;
}
