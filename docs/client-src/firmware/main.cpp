#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "hardware/gpio.h"
#include "hardware/adc.h"
#include "AccelStepper/AccelStepper.h"
#include <string>
#include <vector>

#define FIRMWARE_VERSION "1.6"

#define STEPS_PER_REV 3200.00
#define SPEED_MM_PER_SEC 10.00
#define DEVICE_ADDRESS 1
#define AXIS_COUNT 2
#define SERIAL_READ_FREQUENCY 200

float mm_per_rev[AXIS_COUNT] = {1.27, 1.27};

const uint LED_PIN = 25;

const uint STEP_PIN = 2;
const uint DIR_PIN = 3;

const uint STEP_PIN_2 = 4;
const uint DIR_PIN_2 = 5;

const uint home_pins[AXIS_COUNT] = {6, 7};
const uint end_pins[AXIS_COUNT] = {8, 9};

bool isDeviceBusy = false;
bool isAxisBusy[AXIS_COUNT];

AccelStepper stepper1(AccelStepper::DRIVER, STEP_PIN, DIR_PIN);
AccelStepper stepper2(AccelStepper::DRIVER, STEP_PIN_2, DIR_PIN_2);

AccelStepper *steppers[AXIS_COUNT] = {&stepper1, &stepper2};

bool isAxisHomed[AXIS_COUNT];
bool isAxisHoming[AXIS_COUNT];
bool isAxisBeyondHome[AXIS_COUNT];
bool isAxisBeyondEnd[AXIS_COUNT];

float axisSpeed[AXIS_COUNT];
float homingSpeed[AXIS_COUNT];

uint64_t rpm_to_delay_us(int rpm){
    uint64_t delay = (60.00 * 1000.00 * 1000.00) / ((float)rpm * 200.00);
    return delay;
}

float rpm_to_steps_per_sec(float rpm){
    return (rpm * STEPS_PER_REV) / 60.0;
}

float mm_per_sec_to_steps_per_sec(float mm_per_sec, float _mm_per_rev){
    return (mm_per_sec * STEPS_PER_REV) / _mm_per_rev;
}

float mm_to_steps(float mm, float _mm_per_rev){
    return (mm * STEPS_PER_REV) / _mm_per_rev;
}

float steps_to_mm(float steps, float _mm_per_rev){
    return (steps * _mm_per_rev) / STEPS_PER_REV;
}

// response -> @01 0 OK IDLE -- 0
void send_response(int axis, bool success, std::string warning = "--", std::string data = "0"){
    std::string response = "@" + std::to_string(DEVICE_ADDRESS) + " " + std::to_string(axis) + " ";
    
    if(success){
        response += "OK";
    }else{
        response += "RJ";
    }
    response += " ";

    if(axis == 0){
        std::string state = "IDLE";
        for(int i = 0; i < AXIS_COUNT; i++){
            if(steppers[i]->isRunning()){
                state = "BUSY";
                break;
            }
        }
        response += state;
    }
    else{
        if(steppers[axis-1]->isRunning()){
            response += "BUSY";
        }
        else{
            response += "IDLE";
        }
    }
    response += " ";

    response += warning;
    response += " ";

    response += data;
    response += "\n";

    printf("%s", response.c_str());
}
/* reset function of Rasbpi PIco*/
void reset_pico()
{
    reset_usb_boot(0, 0);
}
/**
COMMAND FORMAT:
/device_address axis_number command parameter parameter \n
*/
void get_commands_from_serial(){
    int c = getchar_timeout_us(0);
    if (c == PICO_ERROR_TIMEOUT) {
        return;
    }

    // get the text until '/' is found
    while(c != '/'){
        c = getchar_timeout_us(10);
        if (c == PICO_ERROR_TIMEOUT) {
            return;
        }
    }

    // get the command until '\n' or '\r' is found
    std::string command = "";
    while(c != '\n' && c != '\r'){
        command += (char)c;
        c = getchar_timeout_us(10);
        if (c == PICO_ERROR_TIMEOUT) {
            printf("Timeout waiting for footer\n");
            return;
        }
    }

    printf("%s\n", command.c_str());

    // remove the '/' from the command
    command = command.substr(1);

    // split command into tokens
    std::vector<std::string> tokens;
    while(command.find(' ') != std::string::npos) {
        tokens.push_back(command.substr(0, command.find(' ')));
        command = command.substr(command.find(' ')+1);
    }
    tokens.push_back(command);

    // printf("Tokens: %d\n", tokens.size());
    // for(int i = 0; i < tokens.size(); i++){
    //     printf("%s\n", tokens[i].c_str());
    // }

    // TODO: Add a mechanism to test if device address and axis number tokens are integers.
    // Else, the program will crash if the tokens are not integers.
    
    int device_address = -1;
    try{
        device_address = std::stoi(tokens[0]);
    }catch(std::exception e){
        printf("Invalid device address\n");
        send_response(0, false, "BADDATA");
        return;
    }
    if(device_address != DEVICE_ADDRESS){
        printf("Wrong device address\n");
        return;
    }

    int axis_number = -1;
    try{
        axis_number = std::stoi(tokens[1]);
    }
    catch(std::exception e){
        printf("Invalid axis number\n");
        send_response(axis_number, false, "BADDATA");
        return;
    }
    if(axis_number < 0 || axis_number > AXIS_COUNT){
        printf("Invalid axis number\n");
        send_response(0, false, "BADDATA");
        return;
    }

    if(tokens.size() < 3){
        send_response(axis_number, true);
        return;
    }

    if(tokens[2] == "home"){
        if(tokens.size() > 3){
            printf("Homing Speed is included\n");
            float speed = -mm_per_sec_to_steps_per_sec(std::stof(tokens[3]), mm_per_rev[0]);
            if(axis_number == 0){
                for(int i = 0; i < AXIS_COUNT; i++){
                    homingSpeed[i] = speed;
                }
            }
            else{
                float speed = -mm_per_sec_to_steps_per_sec(std::stof(tokens[3]), mm_per_rev[axis_number-1]);
                homingSpeed[axis_number-1] = speed;
            }
        }

        if(axis_number == 0){
            for(int i = 0; i < AXIS_COUNT; i++){
                steppers[i]->move(-1);
                isAxisHomed[i] = false;
                isAxisHoming[i] = true;
            }
        }
        else{
            steppers[axis_number-1]->move(-1);
            isAxisHomed[axis_number-1] = false;
            isAxisHoming[axis_number-1] = true;
        }
        send_response(axis_number, true);
        return;
    }
    else if(tokens[2] == "get"){
        if(tokens[3] == "pos"){
            if(axis_number == 0){
                std::string data = "";
                for(int i = 0; i < AXIS_COUNT; i++){
                    data += std::to_string(steps_to_mm(steppers[i]->currentPosition(), mm_per_rev[i]));
                    data += ",";
                }
                data = data.substr(0, data.size()-1);
                send_response(axis_number, true, "--", data);
            }
            else{
                send_response(axis_number, true, "--", std::to_string(steps_to_mm(steppers[axis_number-1]->currentPosition(), mm_per_rev[axis_number-1])));
            }
        }
        else if(tokens[3] == "home"){
            if(axis_number == 0){
                std::string data = "";
                for(int i = 0; i < AXIS_COUNT; i++){
                    data += std::to_string(isAxisHomed[i]);
                    data += ",";
                }
                data = data.substr(0, data.size()-1);
                send_response(axis_number, true, "--", data);
            }
            else{
                send_response(axis_number, true, "--", std::to_string(isAxisHomed[axis_number-1]));
            }
        }
        else if(tokens[3] == "axis"){
            send_response(axis_number, true, "--", std::to_string(AXIS_COUNT));
        }
        else if(tokens[3] == "adc"){
            if(tokens.size() > 4){
                int adc_channel = std::stoi(tokens[4]);
                if(adc_channel < 0 || adc_channel > 4){
                    send_response(axis_number, false, "BADDATA");
                }
                else{
                    adc_select_input(adc_channel);
                    send_response(axis_number, true, "--", std::to_string(adc_read()));
                }
            }
            else{
                send_response(axis_number, false, "BADDATA");
            }
        }
        else if(tokens[3] == "gpio"){
            if(tokens.size() > 4){
                int pin = std::stoi(tokens[4]);
                if(pin < 0 || pin > 28){
                    send_response(axis_number, false, "BADDATA");
                }
                else{
                    send_response(axis_number, true, "--", std::to_string(gpio_get(pin)));
                }
            }
            else{
                send_response(axis_number, false, "BADDATA");
            }
        }
        else if(tokens[3] == "version"){
            send_response(axis_number, true, "--", FIRMWARE_VERSION);
        }
        else{
            send_response(axis_number, false, "BADDATA");
        }
    }
    else if(tokens[2] == "stop"){
        if(axis_number == 0){
            for(int i = 0; i < AXIS_COUNT; i++){
                if(isAxisHoming[i]){
                    isAxisHoming[i] = false;
                    steppers[i]->setSpeed(0);
                    steppers[i]->moveTo(steppers[i]->currentPosition());
                }
                else{
                    steppers[i]->stop();
                }
            }
        }
        else{
            if(isAxisHoming[axis_number-1]){
                isAxisHoming[axis_number-1] = false;
                steppers[axis_number-1]->setSpeed(0);
                steppers[axis_number-1]->moveTo(steppers[axis_number-1]->currentPosition());
            }
            else{
                steppers[axis_number-1]->stop();
            }
        }
        send_response(axis_number, true);
    }
    else if(tokens[2] == "move"){
        if(tokens.size() < 5){
            send_response(axis_number, false, "BADDATA");
            return;
        }

        std::string move_type = tokens[3];
        float move_value = std::stof(tokens[4]);

        if(tokens.size() > 5){
            float speed_value = std::stof(tokens[5]);
            if(axis_number == 0){
                for(int i = 0; i < AXIS_COUNT; i++){
                    axisSpeed[i] = mm_per_sec_to_steps_per_sec(speed_value, mm_per_rev[i]);
                }
            }
            else{
                axisSpeed[axis_number-1] = mm_per_sec_to_steps_per_sec(speed_value, mm_per_rev[axis_number-1]);
            }
            if(tokens.size() > 6){
                float accel_value = std::stof(tokens[6]);
                if(axis_number == 0){
                    for(int i = 0; i < AXIS_COUNT; i++){
                        steppers[i]->setAcceleration(mm_per_sec_to_steps_per_sec(accel_value, mm_per_rev[i]));
                    }
                }
                else{
                    steppers[axis_number-1]->setAcceleration(mm_per_sec_to_steps_per_sec(accel_value, mm_per_rev[axis_number-1]));
                }
            }
        }

        if(move_type == "abs"){
            if(axis_number == 0){
                for(int i = 0; i < AXIS_COUNT; i++){
                    steppers[i]->moveTo(mm_to_steps(move_value, mm_per_rev[i]));
                }
            }
            else{
                steppers[axis_number-1]->moveTo(mm_to_steps(move_value, mm_per_rev[axis_number-1]));
            }
            send_response(axis_number, true);
        }
        else if(move_type == "rel"){
            if(axis_number == 0){
                for(int i = 0; i < AXIS_COUNT; i++){
                    steppers[i]->move(mm_to_steps(move_value, mm_per_rev[i]));
                }
            }
            else{
                steppers[axis_number-1]->move(mm_to_steps(move_value, mm_per_rev[axis_number-1]));
            }
            send_response(axis_number, true);
        }
        else{
            send_response(axis_number, false, "BADDATA");
            return;
        }
    }
    else if(tokens[2] == "set"){
        if(tokens.size() < 5){
            send_response(axis_number, false, "BADDATA");
            return;
        }
        
        if(tokens[3] == "gpio"){
            if(tokens.size() < 6){
                send_response(axis_number, false, "BADDATA");
                return;
            }
            int pin = std::stoi(tokens[4]);
            if(pin < 0 || pin > 28){
                send_response(axis_number, false, "BADDATA");
                return;
            }
            send_response(axis_number, true);
            int value = std::stoi(tokens[5]);
            gpio_init(pin);
            gpio_set_dir(pin, GPIO_OUT);
            gpio_put(pin, value);
        }
        else if(tokens[3] == "pos"){
            if(tokens.size() < 5){
                send_response(axis_number, false, "BADDATA");
                return;
            }
            float pos = std::stof(tokens[4]);
            if(axis_number == 0){
                for(int i = 0; i < AXIS_COUNT; i++){
                    steppers[i]->setCurrentPosition(mm_to_steps(pos, mm_per_rev[i]));
                    isAxisHomed[i] = true;
                }
            }
            else{
                steppers[axis_number-1]->setCurrentPosition(mm_to_steps(pos, mm_per_rev[axis_number-1]));
                isAxisHomed[axis_number-1] = true;
            }
            send_response(axis_number, true);
        }
        else if(tokens[3] == "dir"){
            if(tokens.size() < 5){
                send_response(axis_number, false, "BADDATA");
                return;
            }
            int dir = std::stoi(tokens[4]);
            if(dir != 0 && dir != 1){
                send_response(axis_number, false, "BADDATA");
                return;
            }
            if(axis_number == 0){
                for(int i = 0; i < AXIS_COUNT; i++){
                    steppers[i]->setPinsInverted(dir);
                }
            }
            else{
                steppers[axis_number-1]->setPinsInverted(dir);
            }
            send_response(axis_number, true);
        }
    }
     else if (tokens[2] == "reset")
    {
        reset_pico();
        send_response(axis_number, true);
    }
    else{
        send_response(axis_number, false, "BADDATA");
        return;
    }
}

void serial_thread(){
    //multicore_fifo_pop_blocking();
    while(true){
        get_commands_from_serial();
        sleep_ms(10);
    }
}

int main() {
    // Initialize Serial
    stdio_init_all();

    // Initialize on-board LED
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    // Initialize ADC
    adc_init();
    adc_gpio_init(26);
    adc_gpio_init(27);
    adc_gpio_init(28);
    adc_gpio_init(29);

    adc_set_temp_sensor_enabled(true);

    multicore_launch_core1(serial_thread);
    //multicore_fifo_pop_blocking();

    bool lastAxisBusy[AXIS_COUNT];

    for(uint8_t i = 0; i < AXIS_COUNT; i++){
        isAxisBusy[i] = false;
        isAxisHomed[i] = false;
        homingSpeed[i] = -mm_per_sec_to_steps_per_sec(5, mm_per_rev[i]);
        isAxisHoming[i] = false;
        lastAxisBusy[i] = false;
        isAxisBeyondHome[i] = false;
        isAxisBeyondEnd[i] = false;

        gpio_init(home_pins[i]);
        gpio_set_dir(home_pins[i], GPIO_IN);
        gpio_set_pulls(home_pins[i], true, false);

        gpio_init(end_pins[i]);
        gpio_set_dir(end_pins[i], GPIO_IN);
        gpio_set_pulls(end_pins[i], true, false);

        steppers[i]->setMinPulseWidth(8);
        steppers[i]->setAcceleration(500);

        axisSpeed[i] = 200;
        steppers[i]->setMaxSpeed(axisSpeed[i]);
    }

    while (1) {
        for(uint8_t i = 0; i < AXIS_COUNT; i++){

            if(gpio_get(home_pins[i]) == 0){
                isAxisBeyondHome[i] = true;
                steppers[i]->setSpeed(-homingSpeed[i]);
                steppers[i]->runSpeed();
                steppers[i]->moveTo(steppers[i]->currentPosition());
                isAxisBusy[i] = true;
            }
            else if(isAxisBeyondHome[i]){
                isAxisBeyondHome[i] = false;
                isAxisHoming[i] = false;
                isAxisHomed[i] = true;
                steppers[i]->setCurrentPosition(0);
                steppers[i]->setSpeed(0);
                steppers[i]->moveTo(steppers[i]->currentPosition());
                isAxisBusy[i] = false;
            }

            if(gpio_get(end_pins[i]) == 0){
                isAxisBeyondEnd[i] = true;
                steppers[i]->setSpeed(homingSpeed[i]);
                steppers[i]->runSpeed();
                steppers[i]->moveTo(steppers[i]->currentPosition());
                isAxisBusy[i] = true;
            }
            else if(isAxisBeyondEnd[i]){
                isAxisBeyondEnd[i] = false;
                steppers[i]->setSpeed(0);
                steppers[i]->moveTo(steppers[i]->currentPosition());
                isAxisBusy[i] = false;
            }

            // can't move stepper beyond home position
            // if(isAxisHomed[i] && steppers[i]->targetPosition() < 0){
            //     steppers[i]->moveTo(0);
            // }

            // run the steppers
            if(isAxisHoming[i]){
                if(gpio_get(home_pins[i]) == 1){
                    if(steppers[i]->speed() != homingSpeed[i]){
                        steppers[i]->setSpeed(homingSpeed[i]);
                    }
                    if(steppers[i]->maxSpeed() != homingSpeed[i]){
                        steppers[i]->setMaxSpeed(homingSpeed[i]);
                    }
                    if(steppers[i]->runSpeed()){
                        isAxisBusy[i] = true;
                        gpio_put(LED_PIN, 1);
                    }
                }
            }
            else if(!isAxisBeyondHome[i] && !isAxisBeyondEnd[i]){
                if(steppers[i]->maxSpeed() != axisSpeed[i]){
                    steppers[i]->setMaxSpeed(axisSpeed[i]);
                }
                if(steppers[i]->run()){
                    isAxisBusy[i] = true;
                    gpio_put(LED_PIN, 1);
                }
                else{
                    isAxisBusy[i] = false;
                    gpio_put(LED_PIN, 0);
                }
            }

            // send status if axis busy state changed to false
            for(uint8_t i = 0; i < AXIS_COUNT; i++){
                if(lastAxisBusy[i] && !isAxisBusy[i]){
                    send_response(i+1, true);
                    bool allAxisIdle = true;
                    for(uint8_t i = 0; i < AXIS_COUNT; i++){
                        if(isAxisBusy[i]){
                            allAxisIdle = false;
                            break;
                        }
                    }
                    if(allAxisIdle){
                        send_response(0, true);
                    }
                }
                lastAxisBusy[i] = isAxisBusy[i];
            }
        }
    }
}

/**
 
/1 1 set gpio 12 1

/1 0 move rel 10

/1 1

 */
