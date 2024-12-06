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


# Give Permission To Pico Device

When serial port open it required permission, so following steps give permanent permission.

### Step# 1: Identify the Device- Plug in your device and run:

`ls -l /dev/ttyACM0`

### Step# 2: Check Device Attributes- Get attributes like idVendor and idProduct by running:

`udevadm info -a -n /dev/ttyACM0`

Look for the first block with idVendor and idProduct attributes.

### Step# 3: Create a udev Rule- Create a new udev rule file:

`sudo nano /etc/udev/rules.d/99-usb-serial.rules`

Add the following line (replace with your device's idVendor and idProduct values):
`ATTRS{idVendor}=="1234", ATTRS{idProduct}=="5678", MODE="0666"`

### Step# 4: Reload udev Rules- Reload udev rules and reconnect the device:

`sudo udevadm control --reload-rules` 

`sudo udevadm trigger`

### Step# 5: Verify- Check the permissions:

`ls -l /dev/ttyACM0`

