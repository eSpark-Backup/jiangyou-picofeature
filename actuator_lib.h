// ACTUATOR_LIB_H
#ifndef ACTUATOR_LIB_H
#define ACTUATOR_LIB_H

// #include <windows.h>
#include "serialib.h"
#include <iostream>
#include <string>
#include <chrono>
#include <thread>
#include <mutex>
#include <vector>
#include <atomic>

#define MAX_AXIS_COUNT 5
#define COMMAND_TIMEOUT 5000

enum COMMAND_EXECUTION_STATUS {
    PENDING,
    SUCCESS,
    REJECTED,
    TIMEOUT
};

enum EXECUTION_ERRORS {
    _NO_ERROR,
    SERIAL_READ_ERROR,
    SERIAL_WRITE_ERROR,
    SERIAL_OPEN_ERROR,
    SERIAL_CLOSE_ERROR,
    SERIAL_TIMEOUT_ERROR,
    COMMAND_REJECTED_ERROR,
    ACKNOWLEGEMENT_TIMEOUT_ERROR,
    RESPONSE_PARSE_ERROR
};

class ThreadGuard
{
public:
    explicit ThreadGuard(std::thread& t_, bool *exitThread_) : t(t_) { exitThread = exitThread_; }
    ~ThreadGuard()
    {
        if(t.joinable())
        {
            *exitThread = true;
            t.join();
        }
    }
    ThreadGuard(ThreadGuard const&) = delete;
    ThreadGuard& operator=(ThreadGuard const&) = delete;
private:
    std::thread& t;
    bool *exitThread;
};

class Actuator {
public:
    Actuator();
    Actuator(int id, std::string port);

    std::mutex * enableDebug(bool enable);
    
    bool connect();
    bool disconnect();
    bool isConnected();

    int getAxisCount();

    bool setDirection(int direction, int axis = 0);

    bool home(int axis = 0, double speed = 0, bool waitUntilIdle = false, int timeout = -1);
    bool isHomed(int axis = 0);

    bool stop(int axis = 0, bool waitUntilIdle = false);
    bool isBusy(int axis = 0);

    bool moveRelative(double distance, int axis = 0, double speed = 0, double acceleration = 0, bool waitUntilIdle = false);
    bool moveAbsolute(double position, int axis = 0, double speed = 0, double acceleration = 0, bool waitUntilIdle = false);

    std::vector<double> getPosition(int axis = 0);
    bool setPosition(double position, int axis = 0);

    int getAnalog(int channel = 0);

    bool setGPIO(int pin, bool value);
    bool getGPIO(int pin);

    std::string getFirmwareVersion();
    double getPicoTemperature();

    void resetPi();

    std::string getImagingProbe();

    EXECUTION_ERRORS getLastError();
    std::string getErrorString(EXECUTION_ERRORS error);

private:
    bool writeSerial(std::string data);
    void readSerialThread();
    void consoleOut(std::string data, bool debugOverride = false);
    bool waitForCommandAcknowledge(int timeout = COMMAND_TIMEOUT);
    bool waitUntilIdleFun(int axis, int timeout = -1);//milliseconds

    bool _debugEnabled = true;

    int _id;
    std::string _port;

    std::atomic_bool _isAxisBusy[MAX_AXIS_COUNT];
    COMMAND_EXECUTION_STATUS _lastCommandStatus;
    std::string _lastResponseWarning;
    std::string _lastResponseData;

    bool exitThread = false;
    // std::mutex _serialMutex;
    std::mutex _stdioMutex;
    std::mutex _commandMutex;
    std::thread _readSerialThread;

    ThreadGuard _readSerialThreadGuard;

    serialib _serial;

    EXECUTION_ERRORS _lastError;
};

#endif // ACTUATOR_LIB_H
