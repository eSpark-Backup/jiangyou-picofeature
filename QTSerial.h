#ifndef QTSERIAL_H
#define QTSERIAL_H

/*******LIBRRAY FOR CONVERTING SERIAL LIB OF CPP TO QTSERIAL*******/
// TO DO : implement check if serial port is open in every function


#include <QSerialPort>
#include <QSerialPortInfo>
#include <QElapsedTimer>

//IMPLEMENT A FUNCTION FOR READING CHAR WITH TIMEOUT

class _QTSERIAL {
public:
    bool connectToSerialPort(std::string comPort);
    void disconnectFromSerialPort();
    bool SerialDataAvailable();
    int number_Of_Bytes_Available_To_Read();
    void SendSerialData(const char *receivedString);
    char readChar(unsigned int timeOut_ms);
    std::string readSerialData();
    int readSerialData(char *receivedString,char finalChar,unsigned int maxNbBytes,unsigned int timeOut_ms);
    bool isSerialPortOpen();
private:

};

#endif

