#ifndef GETGPIOTEST_H
#define GETGPIOTEST_H

#include <QThread>
#include <QTimer>
#include <QObject>
#include <iostream>
#include <chrono>
#include <thread>
#include "actuator_lib.h"
#include <QMutex>
#include <QWaitCondition>

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
        : actuator_(actuator), interval_(interval), gpioTestsInProgress_(false)
    {
    }

    void waitForTests() {
        QMutexLocker locker(&mutex_);
        while (gpioTestsInProgress_) {
            condition_.wait(&mutex_);
        }
    }

    signals:
    void gpioTestsStarted();
    void gpioTestsFinished();

protected:
    void run() override {
        QTimer timer;
        timer.setInterval(interval_ * 1000); // Convert seconds to milliseconds
        QObject::connect(&timer, &QTimer::timeout, [&]() {
            {
                QMutexLocker locker(&mutex_);
                gpioTestsInProgress_ = true;
            }
            emit gpioTestsStarted(); // Notify that GPIO tests are about to start

            // Disconnect the actuator
            try {
                // Disconnect the actuator
                if (actuator_.isConnected()) {
                    std::cout << "Disconnecting actuator to run GPIO tests..." << std::endl;
                    actuator_.disconnect();
                }

            // Introduce a delay of 1 second before running GPIO tests
            std::this_thread::sleep_for(std::chrono::seconds(1));

            // Run GPIO tests
            runGPIOTests(&actuator_);

            // Introduce another delay of 1 second after running tests
            std::this_thread::sleep_for(std::chrono::seconds(1));

            // Reconnect the actuator
            std::cout << "Reconnecting actuator..." << std::endl;
            actuator_.connect();
            }
            catch (const std::exception& e) {
               std::cerr << "Caught EXCEPTION in GPIOTestWorker: " << e.what() << std::endl;
           }

            {
                QMutexLocker locker(&mutex_);
                gpioTestsInProgress_ = false;
            }
            emit gpioTestsFinished(); // Notify that GPIO tests are finished
            condition_.wakeAll(); // Wake up waiting threads
        });
        timer.start();
        exec(); // Start the event loop for QTimer
    }

private:
    Actuator& actuator_;
    int interval_;
    bool gpioTestsInProgress_;
    QMutex mutex_;
    QWaitCondition condition_;
};



#endif // GETGPIOTEST_H
