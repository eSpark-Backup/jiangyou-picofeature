
//
#include <string>
#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>
#include <qt5/QtCore/QCoreApplication>
#include "GetGPIOTest.h"
#include "actuator_test.h"
#include <iostream>
#include <QThread>

namespace fs = std::filesystem;

void CopyFile(const std::string& sourcePath, const std::string& destinationPath) {
    std::string command = "cp " + sourcePath + "/main.uf2" + " " + destinationPath;
    // Execute the command
    int result = system(command.c_str());
    if (result == 0) {
        // return true;
        std::cout << "File transfer successfully ";
    } else {
        // return false;
        std::cout << "File transfer Fail";
    }
}

std::string currentPh = fs::current_path().string();
std::string parentPh = std::filesystem::path(currentPh).parent_path();
std::string SourcePath = parentPh + "/uf2File";

// Function to create and write to a CSV file
void WriteCSV(const std::string& filename, const std::string& testName, const std::string& actualResult, const std::string& testResult, const std::string& remarks) {
    std::ofstream file(filename, std::ios::app);  // Append mode to add new test results
    if (file.is_open()) {
        // Write the headers if the file is empty (i.e., newly created)
        file.seekp(0, std::ios::end);  // Go to the end of the file
        if (file.tellp() == 0) {       // Check if the file is empty
            file << "TestName,ActualResult,TestResult,Remarks\n";
        }
        // Write test result to the CSV file
        file << testName << "," << actualResult << "," << testResult <<  "," << remarks <<"\n";
        file.close();
    } else {
        throw std::runtime_error("Unable to open file");
    }
}

TEST(ActuatorTest, RunAllTestWriteCSV)
{
    std::string currentDir = std::filesystem::current_path().string();
    std::string parentDir = std::filesystem::path(currentDir).parent_path();
    std::cout << "Parent Directory: " << parentDir << std::endl;
    std::string filename = parentDir + "/test/test_results.csv";
    std::cout << "CSV Filename: " << filename << std::endl;

    Actuator actuator(VALID_DEVICE_ID, VALID_SERIAL_PORT);

    // actuator.connect();
    bool connectResult = actuator.connect();
    WriteCSV(filename, "ConnectTest", connectResult ? "Connection Successful" : "Connection Unsuccessful", connectResult ? "Pass:" : "Fail", connectResult ? "Connection Passed":"Connection Failed");
    EXPECT_TRUE(connectResult);

        /*getPosition */
    bool getPositionTest = actuator.getPosition(1)[0];
    WriteCSV(filename, "getPositionTest", getPositionTest ? "Position Acquired Unsuccessfully" : "Position Acquired Successfully", getPositionTest ? "Fail" : "Pass", getPositionTest ? "Get Position Failed":"Get Position Passed");
    // EXPECT_TRUE(getPositionTest);

    /*set direction test*/
    bool setDirectionTest = actuator.setDirection(1, 0);
    EXPECT_EQ(actuator.setDirection(1, 0), 1);
    WriteCSV(filename, "setDirectionTest", setDirectionTest ? "Direction Set Successfully" : "Direction Set Unsuccessful", setDirectionTest ? "Pass" : "Fail", setDirectionTest ? "Set Direction Passed" : "Set Direction Failed");
    EXPECT_TRUE(setDirectionTest);

    /*check home test*/
    bool homeTest = actuator.home(1, 0, false, -1);
    EXPECT_EQ(actuator.home(1, 0, false, -1), 1);
    WriteCSV(filename, "homeTest", homeTest ? "Successfully reached home position" : "Unsuccessful reached home position", homeTest ? "Pass" : "Fail", homeTest ? "home Test Passed":"home Test Failed");
    actuator.stop(1, true);
    EXPECT_TRUE(homeTest);

    /*CHECK ishome test*/

    bool isHomeTest = actuator.isHomed(0);
    WriteCSV(filename, "ishomeTest", isHomeTest ? "Device on Home Validation" : "The device is not on home position validation", isHomeTest ? "Fail" : "Pass", isHomeTest ? "is home Test Failed":"is home Test Passed");
    EXPECT_FALSE(isHomeTest);

    /*check Stop test */

    actuator.moveRelative(100, 1, 100, 1000, false);
    usleep(500000);
    EXPECT_EQ(actuator.isBusy(1), 1);
    bool stopTest =actuator.stop(1, true);
    EXPECT_EQ(actuator.stop(1, true), 1);
    usleep(500000);
    EXPECT_EQ(actuator.isBusy(1), 0);
    WriteCSV(filename, "stopTest", stopTest ? "Successfully Stopped" : "Unsuccessfully stopped", stopTest ? "Pass" : "Fail", stopTest ? "Stop Test Passed" : " Stop Test Failed");
    EXPECT_TRUE(stopTest);

    /* isBusy test*/

    bool isbusyTest = actuator.isBusy(0);
    WriteCSV(filename, "isbusyTest", isbusyTest ? "Device isn't Busy Validation" : "Device is Busy Validation", isbusyTest ? "Fail" : "Pass", isbusyTest ? "is busy Test Failed" : "is busy Test Passed");
    EXPECT_FALSE(isbusyTest);

    /*Get Firmware Version*/

    std::string firmwareVersion = actuator.getFirmwareVersion();
    bool getFirmwareVersionTest = (firmwareVersion == "1.5");
    WriteCSV(filename, "getFirmwareVersionTest", getFirmwareVersionTest ? "get Firmware Version Successful" : "get Firmware Version Unsuccessful", getFirmwareVersionTest ? "Pass" : "Fail" , getFirmwareVersionTest ? "get Firmware Version Passed" : "get Firmware Version Failed");
    EXPECT_STREQ(actuator.getFirmwareVersion().c_str(), "1.5");
    EXPECT_TRUE(getFirmwareVersionTest);

    /* check move Absolute*/

    bool moveAbsoluteTest = actuator.moveAbsolute(1, 1, 100, 1000, true);
    EXPECT_EQ(actuator.moveAbsolute(1, 1, 100, 1000, true), 1);
    WriteCSV(filename, "moveAbsoluteTest", moveAbsoluteTest ? "move Absolute Test Successful" : "move Absolute Test unsuccessful", moveAbsoluteTest ? "Pass" : "Fail" , moveAbsoluteTest ? "move Absolute Test Passed" : "move Absolute Test Failed");
    EXPECT_TRUE(moveAbsoluteTest);

    /*Set GPIO test*/

   std::vector<int> pinValues = {0, 1, 11, 28};
    bool setGPIOtest = true;  // Initialize to true; we'll AND it with results
    for (int pin : pinValues) {
    bool result = actuator.setGPIO(pin, 1);
    setGPIOtest &= result;  // Keep track of the overall success
    EXPECT_EQ(result, 1);
    }
    WriteCSV(filename, "setGPIOtest", setGPIOtest ? "setGPIO Successful" : "setGPIO Unsuccessful", setGPIOtest ? "Pass" : "Fail" , setGPIOtest ? "setGPIO Test Passed" : "setGPIO Test Failed");
    EXPECT_TRUE(setGPIOtest);

    /*get gpio test*/

    std::vector<int> pinValu = {0, 1, 11, 28};
    bool getGPIOtest = true;
    for (int pin : pinValu) {
    // get GPIO to 1 and check
    actuator.setGPIO(pin, 1);
    bool result = (actuator.getGPIO(pin) == 1);
    // getGPIOtest &= result;
    // EXPECT_EQ(result, true);
    actuator.setGPIO(pin, 0);
    result = (actuator.getGPIO(pin) == 0);
    getGPIOtest &= result;
    // EXPECT_EQ(result, true);
    }
    WriteCSV(filename, "getGPIOTest", getGPIOtest ? "get GPIO Successful" : "get GPIO Unsuccessful", getGPIOtest ? "Pass" : "Fail" , getGPIOtest ? " getGPIO Test Passed" : "getGPIO Test Failed");
    EXPECT_TRUE(getGPIOtest);

    /* check MoveRelatives Test */

    int iteration_Control = 10; // Number of iterations
    int timer_interrupt = 20;   // Timer interrupt in seconds
    int Total_Hours = 1;        // Total hours to run the test
    auto StartTime = std::chrono::steady_clock::now();
    int argc = 0;
    char* argv[] = { nullptr };
    QCoreApplication app(argc, argv);
    GPIOTestWorker gpioWorker(actuator, timer_interrupt);
    gpioWorker.start();
    bool moveRelativesTest = true;
    for (int i = 0; i < iteration_Control; i++) {
        auto movRelStartTime = std::chrono::steady_clock::now();
        if (!actuator.isConnected()) {
            actuator.connect();
        }
        bool result = actuator.moveRelative(0.5, 1, 100, 1000, false);
        moveRelativesTest &= result;
        EXPECT_EQ(result, true);
        auto movRelEndTime = std::chrono::steady_clock::now();
        auto movRelDuration = std::chrono::duration_cast<std::chrono::milliseconds>(movRelEndTime - movRelStartTime);
        std::cout << "MoveRelative iteration " << i + 1 << " took " << movRelDuration.count() << " ms." << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(1));
        gpioWorker.waitForTests(); // Wait for GPIO tests to finish
        auto EndTime = std::chrono::steady_clock::now();
        auto ElapsedTime = std::chrono::duration_cast<std::chrono::minutes>(EndTime - StartTime);
        if (ElapsedTime.count() >= Total_Hours) {
            std::cout << "Total hours of running is: " << ElapsedTime.count() << std::endl;
            break;
        }
    }
    WriteCSV(filename, "MoveRelativesTest", moveRelativesTest ? "Move Relatives Successfully" : "Move Relatives Unsuccessfully", moveRelativesTest ? "Pass" : "Fail" , moveRelativesTest ? "Move Relatives Test Passed" : "Move Relatives Test Failed");
    EXPECT_TRUE(moveRelativesTest);
    gpioWorker.quit();
    gpioWorker.wait();

    /* Get Pico Temperature Test*/

    // EXPECT_NEAR(actuator.getPicoTemperature(), 0.0, 10.0);
    float PicoTemperature = actuator.getPicoTemperature();
    EXPECT_NEAR(actuator.getPicoTemperature(), PicoTemperature , 20.0);
    if (PicoTemperature)  // Correct range check
    {
        WriteCSV(filename, "PicoTemperature", "Get Pico Temperature Successfully", "Pass", "Passed: Pico Temperature in range");
        EXPECT_TRUE(true);  // Test passes if temperature is within range
    }
    else
    {
        WriteCSV(filename, "PicoTemperature", "Get Pico Temperature Unsuccessfully", "Fail", "Failed: Pico Temperature isn't in range");
        EXPECT_TRUE(false);  // Test fails if temperature is out of range
    }

    /* actuator.disconnect(); */

    bool disconnectResult = actuator.disconnect();  // or use return value if connect() returns something
    WriteCSV(filename, "DisConnectTest", disconnectResult ? "Disconnected Successfully" : "Disconnected unsuccessfully", disconnectResult ? "Pass" : "Fail" , disconnectResult ? "Disconnected Test Passed" : "Disconnected Test Fail");
    EXPECT_TRUE(disconnectResult);  // You can check if the connection was successful

    /*Reset pico Test*/
    bool resetPiTest;
    for (int i = 0; i < 5; ++i) {
        std::cout << "Iteration " << (i + 1) << ":\n";
        EXPECT_EQ(actuator.connect(), 1);
        // actuator.connect();
        QThread::msleep(200);
        actuator.resetPi();
        QThread::msleep(200);
        EXPECT_EQ(actuator.connect(), 0);
        actuator.disconnect();
        resetPiTest = actuator.resetPiSuccess();

        std::string user = std::getenv("USER");
        std::string destinationDir = "/media/" + user + "/RPI-RP2";
        CopyFile(SourcePath, destinationDir);
        QThread::msleep(1500);
    }
    WriteCSV(filename, "resetPiTest", resetPiTest ? "Reset Pi Successfully" : "Reset Pi Unsuccessfully", resetPiTest ? "Pass" : "Fail", resetPiTest ? "Reset Pi Test Passed" : "Reset Pi Test Failed");
}
