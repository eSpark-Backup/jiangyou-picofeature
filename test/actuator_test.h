//
// Created by muhammad-aman-khan on 12/4/24.
//

#ifndef ACTUATOR_TEST_H
#define ACTUATOR_TEST_H

#include "../actuator_lib.h"

#define VALID_SERIAL_PORT "/dev/ttyACM0"
#define INVALID_SERIAL_PORT "/dev/ttyACM1"

#define VALID_DEVICE_ID 1
#define INVALID_DEVICE_ID 2

// extern bool copyFile(const std::string& sourcePath, const std::string& destinationPath);
// extern std::string sourcePath;

class ActuatorTest : public testing::Test {
public: // Make GetGPIOTest public
    static void GetGPIOTest(int pin) {
        Actuator actuator(VALID_DEVICE_ID, VALID_SERIAL_PORT);
        actuator.connect();
        actuator.getLastError();
        actuator.setGPIO(pin, 1);
        EXPECT_EQ(actuator.getGPIO(pin), 1);
        EXPECT_EQ(actuator.getLastError(), _NO_ERROR);
        actuator.setGPIO(pin, 0);
        EXPECT_EQ(actuator.getGPIO(pin), 0);
        EXPECT_EQ(actuator.getLastError(), _NO_ERROR);
        actuator.disconnect();
    }
};

#endif //ACTUATOR_TEST_H
