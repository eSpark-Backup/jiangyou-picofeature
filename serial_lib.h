#ifndef SERIAL_LIB_H
#define SERIAL_LIB_H

#include <string>

class serialib
{
public:
    // Open a device
    char openDevice();
    // Check device opening state
    bool isDeviceOpen();
    // Close the current device
    void closeDevice();

    //________________________________________
    // ::: Read/Write operation on strings :::

    // Write a string
    int writeString (const char *String);

    // Read a string ()
    std::string readString ();
    // Return the number of bytes in the received buffer
    int available();

private:

};

#endif
