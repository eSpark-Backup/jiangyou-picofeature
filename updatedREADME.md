# PICO-SDK SETUP:

To resolve this error, you need to provide the `PICO_SDK_PATH`. Therefore, the `pico-sdk` folder should be present in the project directory.  
![Image](https://github.com/user-attachments/assets/3374b800-3f59-4b82-ba6e-6d4fb61575ee)

### Following Steps for `pico_sdk` Setup:
Step#1 -     `cd path/to/jiangyou-picofeature`                   

Step#2 -     `git clone https://github.com/raspberrypi/pico-sdk.git`     

Step#3 -     `cd pico-sdk`      

Step#4 -     `git submodule update --init`   

Step#5 -     `cd`     

Step#6 -     `export PICO_SDK_PATH=/path/to/pico-sdk`    

Step#7 -     `cd path/to/firmware/build`    

Step#8 -     `cmake ..`     

Step#9 -     `make -j4`     


### CMake:
 ![Image](https://github.com/user-attachments/assets/7f053163-e31b-492f-99ee-13ce7c59ee00)
 
### Build Complete:
![image](https://github.com/user-attachments/assets/40f5bf93-8ced-43af-9213-fbb75d19312b)


# Giving Permanent Permission to Pico Device

When the serial port is opened, permission is required. Follow these steps to give permanent permission to the Pico device.

### Step# 1: Identify the Device
Plug in your device and run the following command:
```bash
ls -l /dev/ttyACM0
```
### Step# 2: Check Device Attributes 
Get attributes like idVendor and idProduct by running:
```bash
udevadm info -a -n /dev/ttyACM0
```
Look for the first block with idVendor and idProduct attributes.

### Step# 3: Create a udev Rule
Create a new udev rule file:
```bash
sudo nano /etc/udev/rules.d/99-usb-serial.rules
```
Add the following line (replace with your device's idVendor and idProduct values):
```bash
ATTRS{idVendor}=="1234", ATTRS{idProduct}=="5678", MODE="0666"
```
### Step# 4: Reload udev Rules
Reload udev rules and reconnect the device:

```bash
sudo udevadm control --reload-rules 
sudo udevadm trigger
```
### Step# 5: Verify
Check the permissions:
```bash
ls -l /dev/ttyACM0
```
# Commands to Run Before Building

Before starting the build process, ensure the necessary packages and tools are installed by running the following commands:

```bash
# Update package list
sudo apt update

# Install essential tools
sudo apt install cmake
sudo apt install build-essential

# Install Qt5 development libraries
sudo apt install qt5-qmake qtbase5-dev qtchooser qtbase5-dev-tools libqt5webkit5-dev libqt5svg5-dev

# Install Google Test and compile it
sudo apt-get install libgtest-dev
sudo apt-get install cmake
cd /usr/src/gtest
sudo cmake .
sudo make
sudo cp libgtest*.a /usr/lib

# Install additional libraries
sudo apt install libboost-all-dev libssl-dev libprotobuf-dev protobuf-compiler

# Install Git
sudo apt install git

# Install C++ compilers
sudo apt install g++ gcc
```
