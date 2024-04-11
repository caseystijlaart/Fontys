/*
 * Distance.h
 *
 *  Created on: 15 Jan 2022
 *      Author: Casey
 */

#ifndef INC_DISTANCE_H_
#define INC_DISTANCE_H_

#define TRIG_PIN GPIO_PIN_8
#define TRIG_PORT GPIOA

uint8_t Distance = 0;

/**
 * @brief reads the current distance to the
 * closest object.
 *
 * @return int current distance to the closest
 * object.
 */
int HCSR04_Read(void);

#endif /* INC_DISTANCE_H_ */
