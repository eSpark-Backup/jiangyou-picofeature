#include "actuator_lib.h"

/**
 * @brief Construct a new Actuator:: Actuator object
 * 
 */
Actuator::Actuator() : _readSerialThreadGuard(_readSerialThread, &exitThread){
    _id = 1;
    #if defined (_WIN32) || defined (_WIN64)
        _port = "\\\\.\\COM1";
    #else
        _port = "/dev/ttyUSB0";
    #endif
}

/**
 * @brief Construct a new Actuator:: Actuator object
 * 
 * @param id    Actuator ID
 * @param port  Serial COM port e.g. COM1 or /dev/ttyUSB0
 */
Actuator::Actuator(int id, std::string port) : _readSerialThreadGuard(_readSerialThread, &exitThread) {
    _id = id;
    #if defined (_WIN32) || defined (_WIN64)
        _port = "\\\\.\\"+port;
    #else
        _port = port;
    #endif
}

/**
 * @brief   Connect to the actuator. Initializes the serial port. Starts the read thread.
 * 
 * @return true    Connected
 * @return false   Not connected
 */
bool Actuator::connect() {

    // Open serial port
    try
    {
        if(_serial.openDevice(_port.c_str(), 115200) != 1){
            consoleOut("ERROR: Could not open serial port\n", true);
            _lastError = SERIAL_OPEN_ERROR;
            return false;
        }
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
        _lastError = SERIAL_OPEN_ERROR;
        return false;
    }

    // Set DTR
    _serial.DTR(true);
    // Clear RTS
    _serial.RTS(false);

    // Flush the receiving buffer.
    _serial.flushReceiver();

    // Start readSerial thread
    _readSerialThread = std::thread(&Actuator::readSerialThread, this);
    
    // Wait for thread to start
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    return true;
}

/**
 * @brief   Disconnect from the actuator. Stops the read thread. Closes the serial port.
 * 
 * @return true     Disconnected (the serial library does not return confirmation of this)
 */
bool Actuator::disconnect() {
    if(exitThread)
        return true;
    
    // Stop readSerial thread
    exitThread = true;

    try
    {
        // Wait for thread to stop
        _readSerialThread.join();

        // Close serial port
        _serial.closeDevice();
    }
    catch(const std::exception& e)
    {
        _lastError = SERIAL_CLOSE_ERROR;
        std::cerr << e.what() << '\n';
        return false;
    }

    return true;
}

/**
 * @brief Check if serial connection is open. Doesn't uctually check if a actuator is responsive on the other end.
 * 
 * @return true 
 * @return false 
 */
bool Actuator::isConnected() {
    return _serial.isDeviceOpen();
}

/**
 * @brief Enable or disable debug output
 * 
 * @param enable    [bool] Enable or disable debug output
 * @return std::mutex* Pointer to the stdio mutex. Can be used to prevent debug output from being interupted by other threads.
 */
std::mutex * Actuator::enableDebug(bool enable) {
    _debugEnabled = enable;
    return &_stdioMutex;
}

/**
 * @brief Write to stdio. Thread safe.
 * 
 * @param data  [string] Data to write to stdio
 * @param debugOverride [bool] Override debug setting
 */
void Actuator::consoleOut(std::string data, bool debugOverride) {
    if(_debugEnabled || debugOverride) {
        _stdioMutex.lock();
        std::cout << data;
        _stdioMutex.unlock();
    }
}

/**
 * @brief Write to serial port.
 * 
 * @param   data     [string] Data to write to serial port
 * @return  true     Data written.
 * @return  false    Data not written
 */
bool Actuator::writeSerial(std::string data){
    consoleOut(">> " + data);
    try
    {
        if(_serial.writeString(data.c_str()) != 1){
            consoleOut("ERROR: Could not write to serial port\n", true);
            _lastError = SERIAL_WRITE_ERROR;
            return false;
        }
        return true;
    }
    catch(const std::exception& e)
    {
        std::cerr << "Caught EXCEPTION: " << e.what() << '\n';
        _lastError = SERIAL_WRITE_ERROR;
        return false;
    }
}

/**
 * @brief   Thread to read serial port.
 * 
 */
void Actuator::readSerialThread() {
    int bytes_read;
    char buffer[1024];
    std::string response;

    consoleOut("INFO: Read serial thread started\n");

    // Loop until exitThread is set to true
    while (!exitThread) {
        // Wait for data to be available on the serial port
        while(_serial.available() < 1 && !exitThread){
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        if(exitThread){
            break;
        }

        // read from serial until the '\n' character is found
        bytes_read = _serial.readString(buffer, '\n', 1024, 1000);

        // Check for errors
        if(bytes_read < 1){
            if(bytes_read == 0){
                consoleOut("ERROR: Timeout while waiting for footer\n");
                _lastError = SERIAL_TIMEOUT_ERROR;
            }
            else{
                consoleOut("ERROR: Error while reading from serial port: " + std::to_string(bytes_read) + "\n");
                _lastError = SERIAL_READ_ERROR;
            }
            continue;
        }

        // store the response in a string
        response = std::string(buffer, bytes_read);

        // parse response -> @01 0 OK IDLE -- 0
        // look for '@' character
        if(response.find('@') != std::string::npos) {
            // response found. get the response from '@' onwards
            response = response.substr(response.find('@')+1);

            consoleOut("<< @" + response);

            // strip trailing '\r' and/or '\n' characters
            while(response.back() == '\r' || response.back() == '\n'){
                response.pop_back();
            }

            // split response into tokens
            std::vector<std::string> tokens;
            while(response.find(' ') != std::string::npos) {
                tokens.push_back(response.substr(0, response.find(' ')));
                response = response.substr(response.find(' ')+1);
            }
            tokens.push_back(response);

            // check if response has the correct number of tokens
            if(tokens.size() != 6) {
                consoleOut("ERROR: Invalid response\n");
                _lastError = RESPONSE_PARSE_ERROR;
                continue;
            }

            // check if the response is for this device
            if (tokens[0] == std::to_string(_id)) {
                // get the axis number
                int axis = std::stoi(tokens[1]);

                // was the last command successful
                if (tokens[2] == "OK") {
                    // command was successful and accepted
                    _commandMutex.lock();
                    _lastCommandStatus = SUCCESS;
                    _commandMutex.unlock();

                    consoleOut("INFO: Command was successful\n");
                }
                else if (tokens[2] == "RJ") {
                    // command was rejected
                    _commandMutex.lock();
                    _lastCommandStatus = REJECTED;
                    _commandMutex.unlock();

                    _lastError = COMMAND_REJECTED_ERROR;
                    consoleOut("ALERT: Command was Rejected\n");
                }

                // parse the current status of the device/axis
                if(tokens[3] == "IDLE")
                    _isAxisBusy[axis] = false;
                else
                    _isAxisBusy[axis] = true;
                consoleOut("INFO: Axis " + std::to_string(axis) + " is " + tokens[3] + "\n");

                // parse included warning and data
                _lastResponseWarning = tokens[4];
                _lastResponseData = tokens[5];
            }
        }
    }
    
    consoleOut("INFO: Read serial thread stopped\n");
}

/**
 * @brief Move the actuator relative to its current position
 * 
 * @param distance      [double] Distance to move the actuator in mm
 * @param axis          [int] Axis to move (0+). 0 = all axes
 * @param speed         [double] Speed to move the actuator in mm/s
 * @param acceleration  [double] Acceleration to move the actuator in mm/s^2
 * @param waitUntilIdle [bool] Wait until the actuator is idle before returning
 * @return true 
 * @return false 
 */
bool Actuator::moveRelative(double distance, int axis, double speed, double acceleration, bool waitUntilIdle) {

    // Assemble command
    std::string command = "/" + std::to_string(_id);
    if(axis > 0){
        command += " " + std::to_string(axis);
    }
    command += " move rel " + std::to_string(distance);
    if(speed > 0){
        command += " " + std::to_string(speed);
    }
    if(acceleration > 0){
        command += " " + std::to_string(acceleration);
    }
    command += "\n";

    // Set command status to pending and send the command
    _commandMutex.lock();
    _lastCommandStatus = PENDING;  
    if(!writeSerial(command)) {
        _commandMutex.unlock();
        return false;
    }
    _commandMutex.unlock();

    // Wait for command acknoledgement
    if(!waitForCommandAcknowledge()){
        return false;
    }

    // return immediately if waitUntilIdle is false
    if(!waitUntilIdle){
        return true;
    }

    // Wait until the actuator is idle
    waitUntilIdleFun(axis);

    return true;
}

/**
 * @brief Move the actuator to an absolute position. Measured from the home position.
 * 
 * @param position      [double] Position to move the actuator to in mm
 * @param axis          [int] Axis to move (0+). 0 = all axes
 * @param speed         [double] Speed to move the actuator in mm/s
 * @param acceleration  [double] Acceleration to move the actuator in mm/s^2
 * @param waitUntilIdle [bool] Wait until the actuator is idle before returning
 * @return true 
 * @return false 
 */
bool Actuator::moveAbsolute(double position, int axis, double speed, double acceleration, bool waitUntilIdle) {

    // Assemble command
    std::string command = "/" + std::to_string(_id);
    if(axis > 0){
        command += " " + std::to_string(axis);
    }
    command += " move abs " + std::to_string(position);
    if(speed > 0){
        command += " " + std::to_string(speed);
    }
    if(acceleration > 0){
        command += " " + std::to_string(acceleration);
    }
    command += "\n";

    // Set command status to pending and send the command
    _commandMutex.lock();
    _lastCommandStatus = PENDING;  
    if(!writeSerial(command)) {
        _commandMutex.unlock();
        return false;
    }
    _commandMutex.unlock();

    // Wait for command acknoledgement
    if(!waitForCommandAcknowledge()){
        return false;
    }

    // return immediately if waitUntilIdle is false
    if(!waitUntilIdle){
        return true;
    }

    // Wait until the actuator is idle
    waitUntilIdleFun(axis);

    return true;
}

/**
 * @brief Home the actuator.
 * 
 * @param axis          [int] Axis to home (0+). 0 = all axes
 * @param speed         [double] Speed to home the actuator in mm/s
 * @param waitUntilIdle [bool] Wait until the actuator is idle before returning
 * @return true 
 * @return false 
 */
bool Actuator::home(int axis, double speed, bool waitUntilIdle, int timeout){

    // Assemble command
    std::string command = "/" + std::to_string(_id);
    if(axis > 0){
        command += " " + std::to_string(axis);
    }
    command += " home";
    if(speed > 0){
        command += " " + std::to_string(speed);
    }
    command += "\n";

    // Set command status to pending and send the command
    _commandMutex.lock();
    _lastCommandStatus = PENDING;  
    if(!writeSerial(command)) {
        _commandMutex.unlock();
        return false;
    }
    _commandMutex.unlock();

    // Wait for command acknoledgement
    if(!waitForCommandAcknowledge()){
        return false;
    }

    // return immediately if waitUntilIdle is false
    if(!waitUntilIdle){
        return true;
    }

    // Wait until the actuator is idle
    return waitUntilIdleFun(axis, timeout);
}

/**
 * @brief Stop the actuator.
 * 
 * @param axis          [int] Axis to stop (0+). 0 = all axes
 * @param waitUntilIdle [bool] Wait until the actuator is idle before returning
 * @return true 
 * @return false 
 */
bool Actuator::stop(int axis, bool waitUntilIdle){

    // Assemble command
    std::string command = "/" + std::to_string(_id);
    if(axis > 0){
        command += " " + std::to_string(axis);
    }
    command += " stop\n";

    // Set command status to pending and send the command
    _commandMutex.lock();
    _lastCommandStatus = PENDING;  
    if(!writeSerial(command)) {
        _commandMutex.unlock();
        return false;
    }
    _commandMutex.unlock();

    // Wait for command acknoledgement
    if(!waitForCommandAcknowledge()){
        return false;
    }

    if(!waitUntilIdle){
        return true;
    }
    
    // Wait until the actuator is idle
    waitUntilIdleFun(axis);

    return true;
}

/**
 * @brief Check if actuator is homed.
 * 
 * @param axis  [int] Axis to check (0+). 0 = all axes
 * @return true 
 * @return false 
 */
bool Actuator::isHomed(int axis){
    
    // Assemble command
    std::string command = "/" + std::to_string(_id);
    if(axis > 0){
        command += " " + std::to_string(axis);
    }
    command += " get home\n";

    // Set command status to pending and send the command
    _commandMutex.lock();
    _lastCommandStatus = PENDING;  
    if(!writeSerial(command)) {
        _commandMutex.unlock();
        return false;
    }
    _commandMutex.unlock();

    // Wait for command acknoledgement
    if(!waitForCommandAcknowledge()){
        return false;
    }

    // Return the value
    if(_lastResponseData == "1"){
        return true;
    }
    else{
        return false;
    }
}

/**
 * @brief Check if actuator is busy.
 * 
 * @param axis  [int] Axis to check (0+). 0 = all axes
 * @return true 
 * @return false 
 */
bool Actuator::isBusy(int axis){
    
    // Assemble command
    std::string command = "/" + std::to_string(_id);
    if(axis > 0){
        command += " " + std::to_string(axis);
    }
    command += "\n";

    // Set command status to pending and send the command
    _commandMutex.lock();
    _lastCommandStatus = PENDING;  
    if(!writeSerial(command)) {
        _commandMutex.unlock();
        return false;
    }
    _commandMutex.unlock();

    // Wait for command acknoledgement
    if(!waitForCommandAcknowledge()){
        return false;
    }

    //Return the value
    return _isAxisBusy[axis];
}

/**
 * @brief Get the actuator position.
 * 
 * @param axis  [int] Axis to check (0+). 0 = all axes
 * @return std::vector<double>  Vector of positions
 */
std::vector<double> Actuator::getPosition(int axis){
    std::vector<double> positions;

    // Assemble command
    std::string command = "/" + std::to_string(_id);
    if(axis >= 0){
        command += " " + std::to_string(axis);
    }
    command += " get pos\n";

    // Set command status to pending and send the command
    _commandMutex.lock();
    _lastCommandStatus = PENDING;  
    if(!writeSerial(command)) {
        _commandMutex.unlock();
        positions.push_back(-1);
        return positions;
    }
    _commandMutex.unlock();

    // Wait for command acknoledgement
    if(!waitForCommandAcknowledge()){
        positions.push_back(-1);
        return positions;
    }

    // Split the response by ',' and return the values
    std::string response = _lastResponseData;
    // consoleOut("Response: " + response, true);
    std::string delimiter = ",";
    size_t pos = 0;
    std::string token;
    while ((pos = response.find(delimiter)) != std::string::npos) {
        token = response.substr(0, pos);
        positions.push_back(std::stod(token));
        response.erase(0, pos + delimiter.length());
    }
    positions.push_back(std::stod(response));

    return positions;
}

/**
 * @brief Get the analog value from a ADC channel.
 * 
 * @param channel   [int] Channel to read (0|1|2)
 * @return int 
 */
int Actuator::getAnalog(int channel){
    // Make sure the channel is valid
    if(channel < 0 || channel > 4){
        return -1;
    }

    // Assemble command
    std::string command = "/" + std::to_string(_id) + " 0";
    command += " get adc " + std::to_string(channel) + "\n";

    // Set command status to pending and send the command
    _commandMutex.lock();
    _lastCommandStatus = PENDING;  
    if(!writeSerial(command)) {
        _commandMutex.unlock();
        return -1;
    }
    _commandMutex.unlock();

    // Wait for command acknoledgement
    if(!waitForCommandAcknowledge()){
        return -1;
    }

    // Return the value
    return std::stoi(_lastResponseData);
}

double Actuator::getPicoTemperature()
{
    //Since the current PICO firmware is not compatible with the feature, calling this function causes firmware crash.
    int ADC_Voltage = getAnalog(4);
    double temperature = 27.0 - (ADC_Voltage - 0.706) / 0.001721;
    return temperature;//temperature;
}

std::string Actuator::getImagingProbe()
{
    const double ZeroOneThresh = 0.42;
    const double OneTwoThresh = 1.01;
    const double TwoThreeThresh = 1.44;
    const double ThreeFourThresh = 1.82;
    const double FourFiveThresh = 2.12;
    const double FiveSixThresh = 2.38;
    const double SixSevenThresh = 2.59;

    double vol = double(getAnalog(1)) / 4096 * 3.3;

    //Adding threshold conditionals

    std::string res;
    if (vol < ZeroOneThresh) {
        res = "Zero";
    } else if (vol >= ZeroOneThresh && vol < OneTwoThresh) {
        res = "One";
    } else if (vol >= OneTwoThresh && vol < TwoThreeThresh) {
        res = "Two";
    } else if (vol >= TwoThreeThresh && vol < ThreeFourThresh) {
        res = "Three";
    } else if (vol >= ThreeFourThresh && vol < FourFiveThresh) {
        res = "Four";
    } else if (vol >= FourFiveThresh && vol < FiveSixThresh) {
        res = "Five";
    } else if (vol >= FiveSixThresh && vol < SixSevenThresh) {
        res = "Six";
    } else {
        res = "Seven";
    }

    return res;
}

/**
 * @brief Get the number of axes attached to the actuator.
 * 
 * @return int 
 */
int Actuator::getAxisCount(){

    // Assemble command
    std::string command = "/" + std::to_string(_id) + " 0 get axis\n";

    // Set command status to pending and send the command
    _commandMutex.lock();
    _lastCommandStatus = PENDING;
    if(!writeSerial(command)) {
        _commandMutex.unlock();
        return -1;
    }
    _commandMutex.unlock();

    // Wait for command acknoledgement
    if(!waitForCommandAcknowledge()){
        return -1;
    }

    // Return the value
    return std::stoi(_lastResponseData);
}

/**
 * @brief Set the output state of a GPIO pin.
 * 
 * @param pin   [int] Pin number (0+)
 * @param value [int] Value to set (0 or 1)
 * @return true 
 * @return false 
 */
bool Actuator::setGPIO(int pin, bool value){
    // Check if value is valid
    if(value != 0 && value != 1){
        return false;
    }

    // Assemble command
    std::string command = "/" + std::to_string(_id) + " 0 set gpio " + std::to_string(pin) + " ";

    if(value){
        command += "1";
    }
    else{
        command += "0";
    }
    command += "\n";

    // Set command status to pending and send the command
    _commandMutex.lock();
    _lastCommandStatus = PENDING;
    if(!writeSerial(command)) {
        _commandMutex.unlock();
        return false;
    }
    _commandMutex.unlock();

    // Wait for command acknoledgement
    if(!waitForCommandAcknowledge()){
        return false;
    }

    // Return the value
    return true;
}

/**
 * @brief Get the state of a GPIO pin.
 * 
 * @param pin   [int] Pin number (0-28)
 * @return true 
 * @return false 
 */
bool Actuator::getGPIO(int pin){
    // Confirm pin is valid
    if(pin < 0 || pin > 28){
        return false;
    }

    // Assemble command
    std::string command = "/" + std::to_string(_id) + " 0 get gpio " + std::to_string(pin) + "\n";

    // Set command status to pending and send the command
    _commandMutex.lock();
    _lastCommandStatus = PENDING;
    if(!writeSerial(command)) {
        _commandMutex.unlock();
        return false;
    }
    _commandMutex.unlock();

    // Wait for command acknoledgement
    if(!waitForCommandAcknowledge()){
        return false;
    }

    // Return the value
    return std::stoi(_lastResponseData);
}

/**
 * @brief Wait to receive a response from the actuator for the last command.
 * 
 * @param timeout   [int] Timeout in milliseconds
 * @return true     OK response received
 * @return false    Timeout or error
 */
bool Actuator::waitForCommandAcknowledge(int timeout){
    auto start = std::chrono::high_resolution_clock::now();
    while(_lastCommandStatus == PENDING) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        auto end = std::chrono::high_resolution_clock::now();
        if(std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() > timeout){
            consoleOut("ERROR: Command timed out\n");
            _commandMutex.lock();
            _lastCommandStatus = TIMEOUT;
            _commandMutex.unlock();
            _lastError = ACKNOWLEGEMENT_TIMEOUT_ERROR;
            return false;
        }
    }
    if(_lastCommandStatus == REJECTED){
        return false;
    }
    return true;
}

bool Actuator::waitUntilIdleFun(int axis, int timeout)
{
    if(timeout == -1){
        while(_isAxisBusy[axis]);
        return true;
    }
    else {
        auto start = std::chrono::high_resolution_clock::now();
        while(_isAxisBusy[axis]) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            auto end = std::chrono::high_resolution_clock::now();
            if(std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() > timeout){
                consoleOut("ERROR: WaitUntilIdleFun timed out\n");
                return false;
            }
        }
        return true;
    }
}

/**
 * @brief Set the actuator position. It also sets the actuator to homed.
 * 
 * @param position  [double] Position to set
 * @param axis      [int] Axis to set (0+). 0 = all axes
 * @return true 
 * @return false 
 */
bool Actuator::setPosition(double position, int axis){
    // Assemble command
    std::string command = "/" + std::to_string(_id);
    if(axis > 0){
        command += " " + std::to_string(axis);
    }
    command += " set pos " + std::to_string(position) + "\n";

    // Set command status to pending and send the command
    _commandMutex.lock();
    _lastCommandStatus = PENDING;
    if(!writeSerial(command)) {
        _commandMutex.unlock();
        return false;
    }
    _commandMutex.unlock();

    // Wait for command acknoledgement
    if(!waitForCommandAcknowledge()){
        return false;
    }

    // Return the value
    return true;
}

/**
 * @brief Set the Axis rotation direction. Default is 0.
 * 
 * @param direction [int] Direction to set. 0 = normal, 1 = inverted
 * @param axis      [int] Axis to set (0+). 0 = all axes
 * @return true 
 * @return false 
 */
bool Actuator::setDirection(int direction, int axis){
    // Check if the direction is valid
    if(direction < 0 || direction > 1){
        return false;
    }

    // Assemble command
    std::string command = "/" + std::to_string(_id);
    if(axis > 0){
        command += " " + std::to_string(axis);
    }
    command += " set dir " + std::to_string(direction) + "\n";

    // Set command status to pending and send the command
    _commandMutex.lock();
    _lastCommandStatus = PENDING;
    if(!writeSerial(command)) {
        _commandMutex.unlock();
        return false;
    }
    _commandMutex.unlock();

    // Wait for command acknoledgement
    if(!waitForCommandAcknowledge()){
        return false;
    }

    // Return the value
    return true;
}

/**
 * @brief Get the firmware version.
 *
 * @return std::string returns "ERROR" on error
 */
std::string Actuator::getFirmwareVersion(){
    // Assemble command
    std::string command = "/" + std::to_string(_id) + " 0 get version\n";

    // Set command status to pending and send the command
    _commandMutex.lock();
    _lastCommandStatus = PENDING;
    if(!writeSerial(command)) {
        _commandMutex.unlock();
        return "ERROR";
    }
    _commandMutex.unlock();

    // Wait for command acknoledgement
    if(!waitForCommandAcknowledge()){
        return "ERROR";
    }

    // Return the value
    return _lastResponseData;
}

/**
 * @brief   Get the last error code.
 *          Clears the error code.
 * 
 * @return  EXECUTION_ERRORS
 */
EXECUTION_ERRORS Actuator::getLastError(){
    EXECUTION_ERRORS error = _lastError;
    _lastError = _NO_ERROR;
    return error;
}

/**
 * @brief Convert error code to string.
 * 
 * @param error [EXECUTION_ERRORS] Error code
 * @return std::string 
 */
std::string Actuator::getErrorString(EXECUTION_ERRORS error){
    switch(error){
        case _NO_ERROR:
            return "No error";
        case SERIAL_READ_ERROR:
            return "Serial read error";
        case SERIAL_WRITE_ERROR:
            return "Serial write error";
        case SERIAL_OPEN_ERROR:
            return "Serial open error";
        case SERIAL_CLOSE_ERROR:
            return "Serial close error";
        case SERIAL_TIMEOUT_ERROR:
            return "Serial timeout error";
        case COMMAND_REJECTED_ERROR:
            return "Command rejected error";
        case ACKNOWLEGEMENT_TIMEOUT_ERROR:
            return "Acknowlegement timeout error";
        case RESPONSE_PARSE_ERROR:
            return "Response parse error";
        default:
            return "Unknown error";
    }
}
