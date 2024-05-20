#include "QTSerial.h"
#include "QDebug"
QSerialPort QSerialPort;

/**
 * @brief CONNECT TO THE SERIAL PORT
 * @param COMPORT
 * @return CONNECTED ----> true / !CONNECTED ----> false
 */
bool _QTSERIAL::connectToSerialPort(std::string comPort){
    comPort = "";
    //TO DO : convert the comport string const qstring to pass in function below
    QSerialPort.setPortName("COM1");
    QSerialPort.setBaudRate(QSerialPort::Baud115200);
    QSerialPort.setDataBits(QSerialPort::Data8);
    QSerialPort.setParity(QSerialPort::NoParity);
    QSerialPort.setStopBits(QSerialPort::OneStop);
    QSerialPort.setFlowControl(QSerialPort::NoFlowControl);

    if (QSerialPort.open(QIODevice::ReadWrite)) {
        qDebug() << "COM 1 OPENED";
        // [FOR DEBUG] TEST SENDING SAMPLE DATA ()
        // QSerialPort.write("TEST SENDING SAMPLE DATA While connection\n");
        return true;
    } else {
        qDebug() << "Failed to open serial port.";
        return false;
    }
    return false;
}

/**
 * @brief DISCONNECT FROM THE CONNECTED SERIALDEVICE
 */
void _QTSERIAL::disconnectFromSerialPort(){
    QSerialPort.close();
}

/**
 * @brief CHECK IF THE DATA IS AVILABLE TO READ
 * @return DATA AVILABLE TO READ ---> true / !DATA AVILABLE TO READ ---> false
 */
bool _QTSERIAL::SerialDataAvailable(){
    if (QSerialPort.bytesAvailable()>0){
        //IF DATA IS AVAILABLE THEN WAIT FOR 10 MILISECOND AND RETURN true
        // QSerialPort.waitForReadyRead(10);
        // qDebug() << "NO OF BYTES AVAILABLE TO READ" << QSerialPort.bytesAvailable();
        return true;
    }
    return false;
}

/**
 * @brief CHECK IF THE NUKBER OF BYTES AVILABLE TO READ
 * @return NUMBER OF BYTES AVAILABLE TO READ
 */
int _QTSERIAL::number_Of_Bytes_Available_To_Read(){
    return QSerialPort.bytesAvailable();
}
/**
 * @brief SEND DATA OVER SERIAL PORT
 */
void _QTSERIAL::SendSerialData(const char *receivedString){
    QSerialPort.write(receivedString);
}

char _QTSERIAL::readChar(unsigned int timeOut_ms){
    QElapsedTimer timer;
    timer.start();
    while (timer.elapsed() < timeOut_ms) { // Continue looping until timeout
        if (QSerialPort.bytesAvailable() > 0) { // Check if bytes are available to read
            QByteArray byteData = QSerialPort.read(1); // Read a single byte
            if (!byteData.isEmpty()) {
                char character = byteData.at(0); // Convert the byte to character
                // qDebug()<<"READ char";
                // qDebug()<<character;
                return character; // Return the character read
            }
        }
    }
    // qDebug()<<"TIME OUT READING CHARACTER \n";
    return 0; // Return 0 if couldn't read before timeout
}

/**
 * @brief READ AVAILABLE SERIAL DATA
 * @return DATA READ BY SERIAL PORT
 */
std::string _QTSERIAL::readSerialData(){
    QByteArray readArray;
    readArray=QSerialPort.readAll();
    QSerialPort.write(readArray);
    // qDebug()<<"Data available to Read";
    // qDebug()<<readArray;

    //CONVERT Q BYTE ARAY INTO STD::STRING AND RETURN IT
    QString str = QString::fromUtf8(readArray);
    std::string stdStr = str.toStdString();
    return stdStr;
}
/**
     @brief Read a string from the serial device (with timeout)
     @param receivedString : string read on the serial device
     @param finalChar : final char of the string
     @param maxNbBytes : maximum allowed number of bytes read
     @param timeOut_ms : delay of timeout before giving up the reading (optional)
     @return  >0 success, return the number of bytes read
     @return  0 timeout is reached
     @return -1 error while setting the Timeout
     @return -2 error while reading the byte
     @return -3 MaxNbBytes is reached
  */
int _QTSERIAL::readSerialData(char *receivedString,char finalChar,unsigned int maxNbBytes,unsigned int timeOut_ms){
    // NUMBER OF BYTES TO BE READ
    unsigned int    nbBytes=0;

    // TIMER FOR TIMEOUT
    QElapsedTimer         timer;
    timer.start();

    // While the buffer is not full
    while (nbBytes<maxNbBytes)
    {
        // Check if timeout is reached
        if (timer.elapsed()>timeOut_ms)
        {
            // Add the end caracter
            receivedString[nbBytes]=0;
            // qDebug() << "TIME OUT , ELAPSED TIME "<< timer.elapsed();
            // qDebug() << "Returning ";
            return 0;
        }

        //read 1 byte and store it in charRead
        char tempCharRead = readChar(20);
        receivedString[nbBytes] = tempCharRead;

        // If a byte has been received / read character is not null
        if (tempCharRead != 0)
        {
            // Check if the character received is the final one
            if (receivedString[nbBytes]==finalChar)
            {
                // Final character: add the end character 0
                receivedString  [++nbBytes]=0;
                // Return the number of bytes read
                return nbBytes;
            }
            // This is not the final character, just increase the number of bytes read
            nbBytes++;
        }
        // else{
        //     qDebug() << "Null character read while reading character";
        //     return tempCharRead;
        // }
    }
    //BUFFER FULL
    return -3;
}




/**
 * @brief CHECK IF SERIAL PORT IS OPEN OR NOT
 * @return OPEN ---> true / !OPEN ---> false
 */
bool _QTSERIAL::isSerialPortOpen(){
    if(!QSerialPort.isOpen()){
        return false;
    }
    else if(QSerialPort.isOpen()){
        return true;
    }
    return false;
}
