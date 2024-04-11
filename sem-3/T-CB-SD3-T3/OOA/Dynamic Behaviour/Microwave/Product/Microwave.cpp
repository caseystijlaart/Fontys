#include "Microwave.h"

#include <iostream>

Microwave::Microwave(ILight& light, IMotor& motor, ISystem& system, IUserInterface& ui)
    : currentState(STATE_IDLE)
    , light(light)
    , motor(motor)
    , system(system)
    , ui(ui)
{
}

States Microwave::HandleIdleState(Events ev)
{
    States result = STATE_IDLE;
    
    switch (ev)
    {
    case EV_START:
        ui.GetRequestedPower();
        motor.SetPower(800);
        ui.StartClock();
        result = STATE_HEATING; 
        break;

    case EV_DOOR_OPENED:
        motor.SetPower(0);
        ui.StopClock();
        light.On();
        result = STATE_OPEN_DOOR;
        break;

    default:
        // ignored event, nothing to do here
        break;
    }

    return result;
}

States Microwave::HandleHeatingState(Events ev)
{
    States result = STATE_HEATING;

    switch (ev)
    {
    case EV_TIME_UP:
        motor.SetPower(0);
        ui.Ping();
        result = STATE_IDLE;
        break;

    case EV_DOOR_OPENED:
        motor.SetPower(0);
        ui.StopClock();
        light.On();
        result = STATE_OPEN_DOOR;
        break;

    default:
        // ignored event, nothing to do here
        break;
    }
    return result;
}

States Microwave::HandleOpenState(Events ev, States lastState)
{
    States result = STATE_OPEN_DOOR;

    switch (ev)
    {
    case EV_DOOR_OPENED:
        
        motor.SetPower(800);
        ui.StartClock();
        light.Off();

        if(lastState == STATE_HEATING)
        {
            result = STATE_HEATING;
        } else if (lastState == STATE_IDLE) {
            result = STATE_IDLE;
        }
        break;

    default:
        // ignored event, nothing to do here
        break;
    }
    return result;
}

void Microwave::HandleEvent(Events ev)
{
    switch (currentState)
    {
    case STATE_IDLE:
        currentState = HandleIdleState(ev);
        break;

    case STATE_OPEN_DOOR:
        currentState = HandleOpenState(ev, currentState);
        break;

    case STATE_HEATING:
        currentState = HandleHeatingState(ev);
        break;

    default:
        std::cerr << "ERROR: illegal/unhandled state with number: " << currentState;
        break;
    };
}
