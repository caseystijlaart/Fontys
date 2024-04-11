#include <stdlib.h>
#include <stdint.h>
#include <stm32f3xx.h>
#include "wheel.h"
#include "distance.h"
#include "timer.h"

int main(void)
{
    wheels_Init(wheel_1 , wheel_2);
    int curSpeed = 10;

    drive(wheel_1, wheel_2, curSpeed);

     while (1)
     {
        while (HCSR04_Read() <= 10) {
			turnRandom(wheel_1, wheel_2, curSpeed);
		}
     }
}
