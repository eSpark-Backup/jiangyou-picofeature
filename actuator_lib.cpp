#include "actuator_lib.h"

/**
 * @brief Construct a new Actuator:: Actuator object
 * 
 */
Actuator::Actuator() : _readSerialThreadGuard(_readSerialThread, &exitThread), _serial("COM1", 115200, serial::Timeout::simpleTimeout(1000)) {
    _id = 1;
    _port = "COM1";
}

/**
 * @brief Construct a new Actuator:: Actuator object
 * 
 * @param id    Actuator ID
 * @param port  Serial COM port e.g. COM1 or /dev/ttyUSB0
 */
Actuator::Actuator(int id, std::string port) : _readSerialThreadGuard(_readSerialThread, &exitThread), _serial("", 115200, serial::Timeout::simpleTimeout(1000)) {
    _id = id;
    _port = port;
}

/**
 * @brief   Connect to the actuator. Initializes the serial port. Starts the read thread.
 * 
 * @return true    Connected
 * @return false   Not connected
 */
bool Actuator::connect() {
    
    // Check if a port was provided
    if(_port != "") {
        consoleOut("INFO: Port provided: " + _port + "\n");
        try{
            _serial.setPort(_port);
            _serial.open();
        }
        catch(const std::exception& e)
        {
            consoleOut("ERROR: Could not open serial port\n", true);
            _lastError = SERIAL_OPEN_ERROR;
            return false;
        }
        if(_serial.isOpen()){
            consoleOut("INFO: Connected to port: " + _port + "\n");
            
            // Set DTR
            _serial.setDTR(true);
            
            // Clear RTS
            _serial.setRTS(false);

            // Flush both Buffers
            _serial.flush();

            // Start readSerial thread
            exitThread = false;
            _readSerialThread = std::thread(&Actuator::readSerialThread, this);
            
            // Wait for thread to start
            std::this_thread::sleep_for(std::chrono::milliseconds(500));

            // Check if this is the correct device
            std::string version = getFirmwareVersion();
            if(version != "ERROR"){
                consoleOut("INFO: Connected to device version: " + version + "\n");
                return true;
            }
            else{
                consoleOut("ERROR: The device is not compatible.\n", true);
                _lastError = SERIAL_OPEN_ERROR;
                
                // Kill the read thread
                exitThread = true;
                _readSerialThread.join();
                _serial.close();
                
                return false;
            }
        }
        else{
            consoleOut("ERROR: Could not open serial port\n", true);
            _lastError = SERIAL_OPEN_ERROR;
            return false;
        }
    }

    consoleOut("INFO: No port provided. Will look for ports automatically.\n");

    // List available ports
    std::vector<serial::PortInfo> devices_found = serial::list_ports();
    for (const serial::PortInfo &device : devices_found) {
        consoleOut("INFO: Found device: " + device.port + " - " + device.description + " - " + device.hardware_id + "\n");
    }

    // If no ports are found return false
    if(devices_found.size() == 0){
        consoleOut("ERROR: No devices found\n", true);
        _lastError = SERIAL_OPEN_ERROR;
        return false;
    }

    // From the found ports, get all ports that have "Board CDC" in the description
    std::vector<serial::PortInfo> devices_found_filtered;
    std::copy_if(devices_found.begin(), devices_found.end(), std::back_inserter(devices_found_filtered), [](const serial::PortInfo &device) { return ((device.description.find("Board CDC") != std::string::npos) || (device.description.find("Raspberry Pi Pico") != std::string::npos)); });

    // If no ports are found return false
    if(devices_found_filtered.size() == 0){
        consoleOut("ERROR: No devices found\n", true);
        _lastError = SERIAL_OPEN_ERROR;
        return false;
    }

    // Iterate through the filtered ports and try to connect to the actuator
    for (const serial::PortInfo &device : devices_found_filtered) {
        consoleOut("INFO: Trying to connect to port: " + device.port + " - " + device.description + " - " + device.hardware_id + "\n");
        try{
            // _serial = serial::Serial(device.port, 115200, serial::Timeout::simpleTimeout(1000));
            _serial.setPort(device.port);
            _serial.open();
            // if(!_serial.isOpen()){
            //     _serial.open();
            // }
        }
        catch(const std::exception& e)
        {
            std::cerr << e.what() << '\n';
            _lastError = SERIAL_OPEN_ERROR;
            continue;
        }
        if(_serial.isOpen()){
            consoleOut("INFO: Connected to port: " + device.port + "\n");
            
            // Set DTR
            _serial.setDTR(true);
            
            // Clear RTS
            _serial.setRTS(false);

            // Flush both Buffers
            _serial.flush();

            // Start readSerial thread
            exitThread = false;
            _readSerialThread = std::thread(&Actuator::readSerialThread, this);
            
            // Wait for thread to start
            std::this_thread::sleep_for(std::chrono::milliseconds(500));

            // Check if this is the correct device
            std::string version = getFirmwareVersion();
            if(version != "ERROR"){
                consoleOut("INFO: Connected to device: " + version + "\n");
                _port = device.port;
                return true;
            }
            else{
                consoleOut("ERROR: Could not connect to device\n", true);
                _lastError = SERIAL_OPEN_ERROR;
                
                // Kill the read thread
                exitThread = true;
                _readSerialThread.join();
                _serial.close();
                
                continue;
            }
        }
        else{
            consoleOut("ERROR: Could not open serial port\n", true);
            _lastError = SERIAL_OPEN_ERROR;
            continue;
        }
    }

    // If no ports are found return false
    consoleOut("ERROR: No compatible devices found\n", true);
    _lastError = SERIAL_OPEN_ERROR;
    return false;
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
        _serial.close();
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
    return _serial.isOpen();
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
        qDebug() << data.c_str();
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
        if(_serial.write(data) != data.length()){
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
    std::string response;

    consoleOut("INFO: Read serial thread started\n");

    int consecutiveErrors = 0;

    // Loop until exitThread is set to true
    while (!exitThread) {
        // Wait for data to be available on the serial port
        try{
            while(_serial.available() < 1 && !exitThread){
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
            if(exitThread){
                break;
            }

            // read from serial until the '\n' character is found
            // bytes_read = _serial.readString(buffer, '\n', 1024, 1000);
            bytes_read = _serial.readline(response, 1024, std::string("\n"));

            consecutiveErrors = 0;
        }
        catch(const std::exception& e)
        {
            if(consecutiveErrors > 10){
                consoleOut("ERROR: Too many consecutive errors. Exiting read thread\n");
                _lastError = SERIAL_READ_ERROR;
                break;
            }

            consoleOut("ERROR: Error while reading from serial port: " + std::string(e.what()) + "\n");
            _lastError = SERIAL_READ_ERROR;

            consecutiveErrors++;

            continue;
        }

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
    
    if(!exitThread){
        consoleOut("ERROR: Read serial thread stopped unexpectedly\n");
    }
    else{
        consoleOut("INFO: Read serial thread stopped\n");
    }
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
    if(axis >= 0){
        command += " " + std::to_string(axis);
    }
    else{
        return false;
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
    if(axis >= 0){
        command += " " + std::to_string(axis);
    }
    else{
        return false;
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
    if(axis >= 0){
        command += " " + std::to_string(axis);
    }
    else{
        return false;
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
    if(axis >= 0){
        command += " " + std::to_string(axis);
    }
    else{
        return false;
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
    if(axis >= 0){
        command += " " + std::to_string(axis);
    }
    else{
        return false;
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
    if(axis >= 0){
        command += " " + std::to_string(axis);
    }
    else{
        return false;
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
    else{
        positions.push_back(-1);
        return positions;
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
 * @param channel   [int] Channel to read (0 to 4)
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
    int adv_val = getAnalog(4);
    float convertion_factor = 3.3f / (1 << 12);
    float voltage = adv_val * convertion_factor;
    double temperature = 27.0 - (voltage - 0.706) / 0.001721;
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

    int adv_val = getAnalog(4);
    float convertion_factor = 3.3f / (1 << 12);
    float voltage = adv_val * convertion_factor;
    // double vol = double(getAnalog(1)) / 4096 * 3.3;

    //Adding threshold conditionals

    std::string res;
    if (voltage < ZeroOneThresh) {
        res = "Zero";
    } else if (voltage >= ZeroOneThresh && voltage < OneTwoThresh) {
        res = "One";
    } else if (voltage >= OneTwoThresh && voltage < TwoThreeThresh) {
        res = "Two";
    } else if (voltage >= TwoThreeThresh && voltage < ThreeFourThresh) {
        res = "Three";
    } else if (voltage >= ThreeFourThresh && voltage < FourFiveThresh) {
        res = "Four";
    } else if (voltage >= FourFiveThresh && voltage < FiveSixThresh) {
        res = "Five";
    } else if (voltage >= FiveSixThresh && voltage < SixSevenThresh) {
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
    // // Check if value is valid
    // if(value != 0 && value != 1){
    //     return false;
    // }

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
    // // Confirm pin is valid
    // if(pin < 0 || pin > 28){
    //     return false;
    // }

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
    if(axis >= 0){
        command += " " + std::to_string(axis);
    }
    else{
        return false;
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
    if(axis >= 0){
        command += " " + std::to_string(axis);
    }
    else{
        return false;
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

void Actuator::resetPi(){
    // Assemble command
    std::string command = "/" + std::to_string(_id) + " 0 reset version\n";

    // Set command status to pending and send the command
    _commandMutex.lock();
    _lastCommandStatus = PENDING;
    if(!writeSerial(command)) {
        _commandMutex.unlock();
    }
    _commandMutex.unlock();

    // Wait for command acknoledgement
    if(!waitForCommandAcknowledge()){
    }
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
