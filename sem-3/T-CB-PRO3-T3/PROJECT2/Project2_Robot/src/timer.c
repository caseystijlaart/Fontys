#include <stm32f3xx.h>
#include <stdlib.h>
#include <stdint.h>
#include "timer.h"

//////////////////////////////////////////
// Private functions
//////////////////////////////////////////
void SysTick_Init()
{
    SysTick->CTRL = 0;
    SysTick->LOAD = 16 - 1; // Results in a period of 1 microsecond

    NVIC_SetPriority(SysTick_IRQn, (1 << __NVIC_PRIO_BITS) - 1);

    SysTick->VAL = 0;
    SysTick->CTRL = 0x5;
}

void micro_Delay(int microSeconds)
{
    int ticks = 0;
    while (ticks < microSeconds)
    {
        if ((SysTick->CTRL & (1 << 16)) != 0)
        {
            ticks++;
            SysTick->CTRL ^= 0x10000;
        }
    }
}

uint32_t get_Time()
{
    return SysTick->VAL;
}
