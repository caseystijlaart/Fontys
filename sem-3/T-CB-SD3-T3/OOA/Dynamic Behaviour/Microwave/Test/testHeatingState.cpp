#include "Microwave.h"

#include "MLight.h"
#include "MMotor.h"
#include "MSystem.h"
#include "MUserInterface.h"

using ::testing::Return;
using ::testing::_;

class TestHeatingState : public ::testing::Test
{
protected:
    TestHeatingState()
        : microwave(light, motor, system, ui)
    {
    }

    virtual ~TestHeatingState() {}

    MLight light;
    MMotor motor;
    MSystem system;
    MUserInterface ui;
    Microwave microwave;
};

TEST_F(TestHeatingState, test_time_up_event)
{

    EXPECT_CALL(motor, SetPower(0));
    EXPECT_CALL(ui, Ping());

    EXPECT_EQ(STATE_IDLE, microwave.HandleHeatingState(EV_TIME_UP));
}

TEST_F(TestHeatingState, test_open_door_event)
{
    EXPECT_CALL(motor, SetPower(0));
    EXPECT_CALL(ui, StopClock());
    EXPECT_CALL(light, On());
    EXPECT_EQ(STATE_OPEN_DOOR, microwave.HandleHeatingState(EV_DOOR_OPENED));
}
