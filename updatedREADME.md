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



