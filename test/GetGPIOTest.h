#ifndef GETGPIOTEST_H
#define GETGPIOTEST_H

#include <QThread>
#include <QTimer>
#include <QObject>
#include <iostream>
#include <chrono>
#include <thread>
#include "actuator_lib.h"

#define VALID_SERIAL_PORT "/dev/ttyACM0"
#define INVALID_SERIAL_PORT "/dev/ttyACM1"
#define VALID_DEVICE_ID 1
#define INVALID_DEVICE_ID 2

class Actuator;

// Declare the function with pointer parameter
void runGPIOTests(Actuator* actuator);

class GPIOTestWorker : public QThread {
    Q_OBJECT

public:
    GPIOTestWorker(Actuator& actuator, int interval)
        : actuator_(actuator), interval_(interval) {}

protected:
    void run() override {
        QTimer timer;
        timer.setInterval(interval_ * 1000); // Convert seconds to milliseconds
        QObject::connect(&timer, &QTimer::timeout, [&]() {
            runGPIOTests(&actuator_);
        });
        timer.start();
        exec(); // Start the event loop for QTimer
    }

private:
    Actuator& actuator_;
    int interval_;
};



#endif // GETGPIOTEST_H
