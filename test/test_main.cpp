// TEST_MAIN_CPP

#include <gtest/gtest.h>
#include "../actuator_lib.h"
#include <string>
#include <chrono>
#include <iostream>
#include "qobject.h"

#include <thread>
#include "test_main.moc"
#include <QCoreApplication>
#include <filesystem>
#include "GetGPIOTest.h"

namespace fs = std::filesystem;


#define VALID_SERIAL_PORT "/dev/ttyACM0"
#define INVALID_SERIAL_PORT "/dev/ttyACM1"

#define VALID_DEVICE_ID 1
#define INVALID_DEVICE_ID 2

#define TEST_HOMING_FUNCTION    // Comment out this line to skip homing tests

// Function to copy a file
bool copyFile(const std::string& sourcePath, const std::string& destinationPath) {
    std::string command = "cp " + sourcePath + "/main.uf2" + " " + destinationPath;
    // Execute the command
    int result = system(command.c_str());

    if (result == 0) {
        return true;
    } else {
        return false;
    }

}

std::string sourcePath = fs::current_path();

// class ActuatorTest : public testing::Test {
// public: // Make GetGPIOTest public
//     static void GetGPIOTest(int pin) {
//         Actuator actuator(VALID_DEVICE_ID, VALID_SERIAL_PORT);
//         actuator.connect();
//         actuator.getLastError();
//         actuator.setGPIO(pin, 1);
//         EXPECT_EQ(actuator.getGPIO(pin), 1);
//         EXPECT_EQ(actuator.getLastError(), _NO_ERROR);
//         actuator.setGPIO(pin, 0);
//         EXPECT_EQ(actuator.getGPIO(pin), 0);
//         EXPECT_EQ(actuator.getLastError(), _NO_ERROR);
//         actuator.disconnect();
//     }
// };

// Manual Connect test
TEST(ActuatorTest, ManualConnectTest) {
    Actuator actuator(VALID_DEVICE_ID, VALID_SERIAL_PORT);
    EXPECT_EQ(actuator.connect(), 1);
    // disconnect
    actuator.disconnect();
}

// Manual Connect test with wrong port
TEST(ActuatorTest, ManualConnectTestWrongPort) {
    Actuator actuator(VALID_DEVICE_ID, INVALID_SERIAL_PORT);
    EXPECT_EQ(actuator.connect(), 0);
    EXPECT_EQ(actuator.getLastError(), SERIAL_OPEN_ERROR);
}

// Manual Connect test with wrong ID
TEST(ActuatorTest, ManualConnectTestWrongID) {
    Actuator actuator(INVALID_DEVICE_ID, VALID_SERIAL_PORT);
    EXPECT_EQ(actuator.connect(), 0);
}

// Auto Connect test
TEST(ActuatorTest, AutoConnectTest) {
    Actuator actuator(VALID_DEVICE_ID, "");
    EXPECT_EQ(actuator.connect(), 1);
    // disconnect
    actuator.disconnect();
}

// Disconnect test without connect
TEST(ActuatorTest, DisconnectTest) {
    Actuator actuator(VALID_DEVICE_ID, VALID_SERIAL_PORT);
    EXPECT_EQ(actuator.disconnect(), 0);
    EXPECT_EQ(actuator.getLastError(), SERIAL_CLOSE_ERROR);
}

// Disconnect test with connect
TEST(ActuatorTest, DisconnectTestWithConnect) {
    Actuator actuator(VALID_DEVICE_ID, VALID_SERIAL_PORT);
    actuator.connect();
    EXPECT_EQ(actuator.disconnect(), 1);
}

// Is Connected test
TEST(ActuatorTest, IsConnectedTest) {
    Actuator actuator(VALID_DEVICE_ID, VALID_SERIAL_PORT);
    EXPECT_EQ(actuator.isConnected(), 0);
    actuator.connect();
    EXPECT_EQ(actuator.isConnected(), 1);
    // disconnect
    actuator.disconnect();
}

// Get Axis Count test
TEST(ActuatorTest, GetAxisCountTest) {
    Actuator actuator(VALID_DEVICE_ID, VALID_SERIAL_PORT);
    actuator.connect();
    EXPECT_EQ(actuator.getAxisCount(), 2);
    // disconnect
    actuator.disconnect();
}

// Set Direction test
TEST(ActuatorTest, SetDirectionTest) {
    Actuator actuator(VALID_DEVICE_ID, VALID_SERIAL_PORT);
    actuator.connect();
    EXPECT_EQ(actuator.setDirection(1, 0), 1);
    // disconnect
    actuator.disconnect();
}

// Address invalid axis
TEST(ActuatorTest, AddressInvalidAxis) {
    Actuator actuator(VALID_DEVICE_ID, VALID_SERIAL_PORT);
    actuator.connect();
    actuator.getLastError();
    actuator.setDirection(1, 3);
    EXPECT_EQ(actuator.getLastError(), COMMAND_REJECTED_ERROR);
    // disconnect
    actuator.disconnect();
}

// isHomed test
TEST(ActuatorTest, IsHomedTest) {
    Actuator actuator(VALID_DEVICE_ID, VALID_SERIAL_PORT);
    actuator.connect();
    actuator.getLastError();
    actuator.isHomed(0);
    EXPECT_EQ(actuator.getLastError(), _NO_ERROR);
    // disconnect
    actuator.disconnect();
}

#ifdef TEST_HOMING_FUNCTION

// Home test without wait
TEST(ActuatorTest, HomeTest) {
    Actuator actuator(VALID_DEVICE_ID, VALID_SERIAL_PORT);
    actuator.connect();
    EXPECT_EQ(actuator.home(1, 0, false, -1), 1);
    actuator.stop(1, true);
    // disconnect
    actuator.disconnect();
}

// Home test with wait
TEST(ActuatorTest, HomeTestWithWait) {
    Actuator actuator(VALID_DEVICE_ID, VALID_SERIAL_PORT);
    actuator.connect();
    EXPECT_EQ(actuator.home(1, 0, true, -1), 1);
    // wait half a second
    usleep(500000);
    EXPECT_EQ(actuator.isHomed(1), 1);
    // disconnect
    actuator.disconnect();
}

#endif

void runGPIOTests(Actuator& actuator) {
    std::cout << "Testing GPIO..." << std::endl;
    ActuatorTest::GetGPIOTest(0);
    ActuatorTest::GetGPIOTest(1);
    ActuatorTest::GetGPIOTest(11);
    ActuatorTest::GetGPIOTest(28);
}

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

void runGPIOTests(Actuator* actuator) {
    std::cout << "Testing GPIO..." << std::endl;
    ActuatorTest::GetGPIOTest(0);
    ActuatorTest::GetGPIOTest(1);
    ActuatorTest::GetGPIOTest(11);
    ActuatorTest::GetGPIOTest(28);
}

TEST(ActuatorTest, MoveRelativeTest) {
    Actuator actuator(VALID_DEVICE_ID, VALID_SERIAL_PORT);

    int iteration_Control = 10; // Set the value of iteration
    int timer_interrupt = 3; // Set the interrupt value in seconds
    int Total_Hours = 2; // Set the total hours to run the test

    auto StartTime = std::chrono::steady_clock::now(); // Start of the entire test

    int argc = 0;
    char* argv[] = { nullptr };
    QCoreApplication app(argc, argv);

    GPIOTestWorker gpioWorker(actuator, timer_interrupt);
    gpioWorker.start();

    while (true) {
        std::cout << "Testing moveRelative command..." << std::endl;
        try {
            actuator.connect();

            for (int i = 0; i < iteration_Control; i++) {
                auto movRelStartTime = std::chrono::steady_clock::now();

                EXPECT_EQ(actuator.moveRelative(0.5, 1, 100, 1000, true), true);

                auto movRelEndTime = std::chrono::steady_clock::now();
                auto movRelDuration = std::chrono::duration_cast<std::chrono::milliseconds>(movRelEndTime - movRelStartTime);

                std::cout << "MoveRelative iteration " << i + 1 << " took "
                          << movRelDuration.count() << " ms." << std::endl;

                std::this_thread::sleep_for(std::chrono::seconds(1));
            }

            actuator.disconnect();

            auto EndTime = std::chrono::steady_clock::now();
            auto ElapsedTime = std::chrono::duration_cast<std::chrono::hours>(EndTime - StartTime);
            if (ElapsedTime.count() >= Total_Hours) {
                std::cout << "Total hours of running is: " << ElapsedTime.count() << std::endl;
                break;
            }
        } catch (const std::exception& e) {
            std::cerr << "Caught EXCEPTION: " << e.what() << std::endl;
            break; // Exit loop on exception
        }
    }

    gpioWorker.quit(); // Request the QThread to quit
    gpioWorker.wait(); // Wait for the QThread to finish
}


// move Absolute test
TEST(ActuatorTest, MoveAbsoluteTest) {
    Actuator actuator(VALID_DEVICE_ID, VALID_SERIAL_PORT);
    actuator.connect();
    EXPECT_EQ(actuator.moveAbsolute(1, 1, 100, 1000, true), 1);
    // disconnect
    actuator.disconnect();
}

// get Position test
TEST(ActuatorTest, GetPositionTest) {
    Actuator actuator(VALID_DEVICE_ID, VALID_SERIAL_PORT);
    actuator.connect();
    EXPECT_NEAR(actuator.getPosition(1)[0], 1.0, 0.1);
    // disconnect
    actuator.disconnect();
}

// get Position acuracy test
TEST(ActuatorTest, GetPositionAcuracyTest) {
    Actuator actuator(VALID_DEVICE_ID, VALID_SERIAL_PORT);
    actuator.connect();
    double position = actuator.getPosition(1)[0];
    actuator.moveRelative(0.5, 1, 100, 1000, true);
    // wait half a second
    usleep(500000);
    EXPECT_NEAR(actuator.getPosition(1)[0], position + 0.5, 0.1);
    // disconnect
    actuator.disconnect();
}

// set Position test
TEST(ActuatorTest, SetPositionTest) {
    Actuator actuator(VALID_DEVICE_ID, VALID_SERIAL_PORT);
    actuator.connect();
    actuator.setPosition(20, 1);
    EXPECT_NEAR(actuator.getPosition(1)[0], 20.0, 0.1);
    // disconnect
    actuator.disconnect();
}

// isBusy test
TEST(ActuatorTest, IsBusyTest) {
    Actuator actuator(VALID_DEVICE_ID, VALID_SERIAL_PORT);
    actuator.connect();
    actuator.getLastError();
    actuator.isBusy(0);
    EXPECT_EQ(actuator.getLastError(), _NO_ERROR);
    // disconnect
    actuator.disconnect();
}

// stop test
TEST(ActuatorTest, StopTest) {
    Actuator actuator(VALID_DEVICE_ID, VALID_SERIAL_PORT);
    actuator.connect();
    actuator.moveRelative(100, 1, 100, 1000, false);
    // wait half a second
    usleep(500000);
    EXPECT_EQ(actuator.isBusy(1), 1);
    EXPECT_EQ(actuator.stop(1, true), 1);
    // wait half a second
    usleep(500000);
    EXPECT_EQ(actuator.isBusy(1), 0);
    // disconnect
    actuator.disconnect();
}

// get Firmware Version test
TEST(ActuatorTest, GetFirmwareVersionTest) {
    Actuator actuator(VALID_DEVICE_ID, VALID_SERIAL_PORT);
    actuator.connect();
    EXPECT_STREQ(actuator.getFirmwareVersion().c_str(), "1.6");
    // disconnect
    actuator.disconnect();
}

// get Pico Temperature test
TEST(ActuatorTest, GetPicoTemperatureTest) {
    Actuator actuator(VALID_DEVICE_ID, VALID_SERIAL_PORT);
    actuator.connect();
    EXPECT_NEAR(actuator.getPicoTemperature(), 25.0, 10.0);
    // disconnect
    actuator.disconnect();
}

// pico reset test
class ActuatorTestResetPi : public testing::TestWithParam<int> {};

TEST_P(ActuatorTestResetPi, ResetPiTest) {
    Actuator actuator(VALID_DEVICE_ID, VALID_SERIAL_PORT);
    EXPECT_EQ(actuator.connect(), 1);
    actuator.resetPi();
    EXPECT_EQ(actuator.connect(), 0);
    actuator.disconnect();

    std::string user = std::getenv("USER");
    std::string destinationDir = "/media/" + user + "/RPI-RP2";
    EXPECT_TRUE(copyFile(sourcePath, destinationDir));
    sleep(1);
    EXPECT_EQ(actuator.connect(), 1);
    actuator.disconnect();
}

INSTANTIATE_TEST_SUITE_P(
        ActuatorTestResetPiInstances,
        ActuatorTestResetPi,
        ::testing::Range(0, 300)
);

// set GPIO test
class ActuatorTestSetGPIOValid : public testing::TestWithParam<int> {};

TEST_P(ActuatorTestSetGPIOValid, SetGPIOTest) {
    Actuator actuator(VALID_DEVICE_ID, VALID_SERIAL_PORT);
    actuator.connect();
    actuator.getLastError();
    int pin = GetParam();
    EXPECT_EQ(actuator.setGPIO(pin, 1), 1);
    EXPECT_EQ(actuator.getLastError(), _NO_ERROR);
    // disconnect
    actuator.disconnect();
}

INSTANTIATE_TEST_CASE_P(PinValues, ActuatorTestSetGPIOValid, testing::Values(0, 1, 11, 28));

// set GPIO test with invalid pin
class ActuatorTestSetGPIOInvalid : public testing::TestWithParam<int> {};

TEST_P(ActuatorTestSetGPIOInvalid, SetGPIOTestInvalidPin) {
    Actuator actuator(VALID_DEVICE_ID, VALID_SERIAL_PORT);
    actuator.connect();
    actuator.getLastError();
    int pin = GetParam();
    actuator.setGPIO(pin, 1);
    EXPECT_EQ(actuator.getLastError(), COMMAND_REJECTED_ERROR);
    // disconnect
    actuator.disconnect();
}

INSTANTIATE_TEST_CASE_P(PinValues, ActuatorTestSetGPIOInvalid, testing::Values(-1, -10, 29, 40));

// get GPIO test
class ActuatorTestGetGPIOValid : public testing::TestWithParam<int> {};

TEST_P(ActuatorTestGetGPIOValid, GetGPIOTest) {
    Actuator actuator(VALID_DEVICE_ID, VALID_SERIAL_PORT);
    actuator.connect();
    actuator.getLastError(); 
    int pin = GetParam();
    actuator.setGPIO(pin, 1);
    EXPECT_EQ(actuator.getGPIO(pin), 1);
    EXPECT_EQ(actuator.getLastError(), _NO_ERROR);

    actuator.setGPIO(pin, 0);
    EXPECT_EQ(actuator.getGPIO(pin), 0);
    EXPECT_EQ(actuator.getLastError(), _NO_ERROR);

    // disconnect
    actuator.disconnect();
}

INSTANTIATE_TEST_CASE_P(PinValues, ActuatorTestGetGPIOValid, testing::Values(0, 1, 11, 28));

// get GPIO test with invalid pin
class ActuatorTestGetGPIOInvalid : public testing::TestWithParam<int> {};

TEST_P(ActuatorTestGetGPIOInvalid, GetGPIOTestInvalidPin) {
    Actuator actuator(VALID_DEVICE_ID, VALID_SERIAL_PORT);
    actuator.connect();
    actuator.getLastError();
    int pin = GetParam();
    actuator.getGPIO(pin);
    EXPECT_EQ(actuator.getLastError(), COMMAND_REJECTED_ERROR);
    // disconnect
    actuator.disconnect();
}

INSTANTIATE_TEST_CASE_P(PinValues, ActuatorTestGetGPIOInvalid, testing::Values(-1, -10, 29, 40));

// get Analog test
class ActuatorTestAnalogValid : public testing::TestWithParam<int> {};

TEST_P(ActuatorTestAnalogValid, GetAnalogTest) {
    Actuator actuator(VALID_DEVICE_ID, VALID_SERIAL_PORT);
    actuator.connect();
    actuator.getLastError();
    int pin = GetParam();
    // Expect a 16 bit value
    EXPECT_GE(actuator.getAnalog(pin), 0);
    EXPECT_EQ(actuator.getLastError(), _NO_ERROR);
    EXPECT_LE(actuator.getAnalog(pin), 65535);
    EXPECT_EQ(actuator.getLastError(), _NO_ERROR);
    // disconnect
    actuator.disconnect();
}

INSTANTIATE_TEST_CASE_P(PinValues, ActuatorTestAnalogValid, testing::Values(0, 1, 2, 3, 4));

// get Analog test with invalid channel
class ActuatorTestAnalogInvalid : public testing::TestWithParam<int> {};

TEST_P(ActuatorTestAnalogInvalid, GetAnalogTest) {
    Actuator actuator(VALID_DEVICE_ID, VALID_SERIAL_PORT);
    actuator.connect();
    int pin = GetParam();
    EXPECT_EQ(actuator.getAnalog(pin), -1);
    // disconnect
    actuator.disconnect();
}

INSTANTIATE_TEST_CASE_P(PinValues, ActuatorTestAnalogInvalid, testing::Values(-1, -10, 5, 6, 10));

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    // ::testing::GTEST_FLAG(break_on_failure) = true;
    return RUN_ALL_TESTS();
}




/*
/1 0

*/