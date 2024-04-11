/*
 * timer.h
 *
 * Created on: 20 dec 2021
 *      Author: casey
 *
 * header file for the public functions of the system
 * timer
 */

#ifndef INC_TIMER_H
#define INC_TIMER_H

#include <stdlib.h>
#include <stdint.h>

/**
 * @brief a delay function based on the system timer.
 * 
 *  @param microSeconds is the amount of delay that 
 * is wished for.
 */
void micro_Delay(int microSeconds);

/**
 * @brief Gets the current time based on the current
 * systemtimer value.
 * 
 * @return uint32_t current time.
 */
uint32_t get_Time();

/**
 * @brief initializes the system timer
 * to use for keeping track of the time.
 * 
 */
void SysTick_Init();

#endif