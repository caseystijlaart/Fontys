/*
 * wheel.h
 *
 *  Created on: 30 Nov 2021
 *      Author: casey
 *
 *  This configuration is only meant for timer 2
 *  in combination with the possible pins from port
 *  A, B and C.
 */

#ifndef INC_WHEELS_H_
#define INC_WHEELS_H_

#include <stdlib.h>
#include <stdint.h>
#include <stm32f3xx.h>
#include <stm32f3xx_hal_tim.h>
#include <math.h>

#define AF1 ((uint16_t)0b0001)
#define AF2 ((uint16_t)0b0010)
#define AF3 ((uint16_t)0b0011)
#define AF4 ((uint16_t)0b0100)
#define AF5 ((uint16_t)0b0101)
#define AF6 ((uint16_t)0b0110)
#define AF7 ((uint16_t)0b0111)
#define AF8 ((uint16_t)0b1000)
#define AF9 ((uint16_t)0b1001)
#define AF10 ((uint16_t)0b1010)

/**
 * @brief the necessary information for the
 * wheel in order to initialise it and rotate.
 *
 */
typedef struct
{
	GPIO_TypeDef *GPIO;
	uint16_t pin_Number;
	uint32_t clock_Speed;
	TIM_TypeDef *tim_Instance;
	HAL_TIM_ActiveChannel channel;
	uint32_t ARR_Val;
	uint32_t AFR_Val;
	uint32_t min_forward_tControl;
	uint32_t max_forward_tControl;
	uint32_t min_backward_tControl;
	uint32_t max_backward_tControl;
} wheel_Info;

wheel_Info wheel_1 = {
    GPIOB, 3, 16000000,
    TIM2, HAL_TIM_ACTIVE_CHANNEL_2, 319999, AF1, 1530, 1730, 1255, 1473};

wheel_Info wheel_2 = {
    GPIOA, 0, 16000000,
    TIM2, HAL_TIM_ACTIVE_CHANNEL_1, 319999, AF1, 1530, 1730, 1255, 1473};

/**
 * @brief initialises the wheels as PWM mode 1
 * based on the used timer, pin, channel and
 * clockspeed.
 *
 * @param wheel1 the first wheel instance for which
 * the initialisation is meant.
 * @param wheel2 the seconds wheel instance for which
 * the initialisation is meant.
 */
void wheels_Init(wheel_Info wheel1, wheel_Info wheel2);

/**
	 * @brief initialises the used pin as
	 * alternative pin mode.
	 *
	 * @param wheel
	 * @return int returns -1 if ran into an error,
	 * else returns 0;
	 */
int init_Pin(wheel_Info wheel);

/**
 * @brief calculated the needed CCR value for
 * the goal speed.
 *
 * @param speed the rotation speed for the wheel.
 * @param wheel the instance for which the CCR
 * will be calculated based on its tControl values.
 * @return uint32_t with the calculated CCR value.
 */
uint32_t calc_CCR_Speed(float speed, wheel_Info wheel);

/**
 * @brief sets the rotation speed for the wheels.
 * 
 * @param wheel the first wheel which rotates on the set
 * speed.
 * @param wheel2 the second wheel which rotates on the set
 * speed.
 * @param speed the rotation speed of the wheels in cm/s.
 * @return int int returns -1 if ran into an error,
 * else returns 0;
 */
int drive(wheel_Info wheel, wheel_Info wheel2, float speed);

/**
 * @brief turns the robot to a random direction,
 * based on the randomly generated number. If
 * the generated number if negative, the robot
 * will turn to the left and when positive it
 * will turn to the right. After turning, it will
 * resume in turning straight forward.
 *
 * @param wheel_1 the first wheel which rotate
 * on the set speed.
 * @param wheel_2 the second wheel which rotate
 * on the set speed.
 * @return int returns -1 if ran into an error,
 * else returns 0;
 */
int turnRandom(wheel_Info wheel_1, wheel_Info wheel_2, float curSpeed);

/**
 * @brief generates a random number between 90
 * and -90, excluding 0.
 *
 * @return int generated number.
 */
int generateRandomNumber();

/**
 * @brief sets the feedback pwm signal.
 *
 * @param GPIO is the used port.
 * @param pinNr is the used pin number.
 */
float get_Feedback(GPIO_TypeDef *GPIO, uint8_t pinNr);

/**
 * @brief Initialises the timer 3 for the feedback control.
 */
void feedbackSignal_Init(void);

/**
 * @brief Initialises the used pins as alternate functions
 * for the feedback control.
 */
void pinsFeedBack_Init(void);

/**
 * @brief Checks the current FeedBack control signals and
 * compares them. If needed e new ccr value will be send to
 * readjust a wheel.
 * 
 */
void adjust_Wheel();

#endif /* INC_WHEELS_H_ */
