#include "Microwave.h"

#include "MLight.h"
#include "MMotor.h"
#include "MSystem.h"
#include "MUserInterface.h"

using ::testing::Return;
using ::testing::_;

class TestOpenState : public ::testing::Test
{
protected:
    TestOpenState()
        : microwave(light, motor, system, ui)
    {
    }

    virtual ~TestOpenState() {}

    MLight light;
    MMotor motor;
    MSystem system;
    MUserInterface ui;
    Microwave microwave;
};

TEST_F(TestOpenState, test_close_door_event_back_to_heating)
{
    States lastState = STATE_HEATING;
    
    EXPECT_CALL(motor, SetPower(800));
    EXPECT_CALL(ui, StartClock());
    EXPECT_CALL(light, Off());

    EXPECT_EQ(STATE_HEATING, microwave.HandleOpenState(EV_DOOR_OPENED, lastState));
}

TEST_F(TestOpenState, test_close_door_event_back_to_idle)
{
    States lastState = STATE_IDLE;

    EXPECT_CALL(motor, SetPower(800));
    EXPECT_CALL(ui, StartClock());
    EXPECT_CALL(light, Off());
    
    EXPECT_EQ(STATE_IDLE, microwave.HandleOpenState(EV_DOOR_OPENED, lastState));
}