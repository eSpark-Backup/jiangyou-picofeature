#include "serial_lib.h"
#include "QTSerial.h"

_QTSERIAL QT_SERIAL; //INSTANCE OF QT SERIAL LIBRARY

/**
 * @brief CONNECT TO THE SERIAL PORT
 * @param COMPORT
 * @return CONNECTED ----> true / !CONNECTED ----> false
 */
char serialib::openDevice(){
    if(QT_SERIAL.connectToSerialPort("COM1")){
        return true;
    }
    return false;
}
/**
 * @brief CHECK IF SERIAL PORT IS OPEN OR NOT
 * @return OPEN ---> true / !OPEN ---> false
 */
bool serialib::isDeviceOpen()
{
    if(QT_SERIAL.isSerialPortOpen()){
        return true;
    }
    return false;
}

/**
 * @brief DISCONNECT FROM THE CONNECTED SERIALDEVICE
 */
void serialib::closeDevice()
{
    QT_SERIAL.disconnectFromSerialPort();
}

//________________________________________
// ::: Read/Write operation on strings :::

/*!
     \brief     Write a string on the current serial port
     \param     receivedString : string to send on the port (must be terminated by '\0')
     \return     1 success
     \return    -1 error while writting data
  */
int serialib::writeString(const char *receivedString)
{
    QT_SERIAL.SendSerialData(receivedString);
    return 1;
}

std::string serialib::readString (){
    QT_SERIAL.readSerialData();
}
