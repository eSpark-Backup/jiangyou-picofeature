# Raspberry Pi Pico firmware upgrade feature
#### Client Name: Jiang You

## Project Requirements
Need to develop a feature that can flash Pico firmware through client's main software without press-hold the reset button on Pico.
During upgrade, the new firmware will be stored in a flash drive. and connect to the Host PC where our main software locates. Then the user shall be able to upgrade the firmware by clicking a press-button in GUI.

#### Proposed Sequence (Open to better ways)
1. Send reboot command
2. Disconnect from Serial
3. Wait for USB mass storage device to mount
4. Check if the mounted device is correct
5. Copy the firmware package onto the drive
6. Wait for COM port to appear
7. Connect to serial


The deliverable is :
Modification of firmware or SDK that can enable this upgrade feature.
