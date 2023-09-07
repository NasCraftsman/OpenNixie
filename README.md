# OpenNixie 

<img src="Docs/_DSC1764.JPG" width="300px"></a>
<img src="Docs/_DSC1766.JPG" width="300px"></a>
<img src="Docs/Nixie_Gif_Animation.gif" width="300px"></a>

OpenNixie was born with the aim of creating an open project development platform around all types of Nixie Tubes. A platform that condenses the future with the past. The IoT technology with the famous plasma tubes in a miniature and modular design.

OpenNixie is a BLE and Wifi platform for different applications. It was originally designed as a RTC with an Internet connection, but later became a universal platform with many applications. This system is based on the ESP32-WROVER-IE Module, includes a 170V power stage and can be powered and programmed directly through the USB-C connector.

Some of the features that the OpenNixie platform includes are:

- Dimensions are only 30x60mm
- Wifi and BLE connected
- Modular design: it can be used for different Nixie tubes shapes and applications
- USB Type-C for power and programming
- 4 1N-12A + RGB lights as a default Nixie extension board 
- 1 general pourpose button for control the platform
  

<a href="https://www.pcbway.com/project/shareproject/Open_IoT_Nixie_Platform_9b133654.html"><img src="https://www.pcbway.com/project/img/images/frompcbway-1220.png" alt="PCB from PCBWay" /></a>

## Multi Board Architecture
 
OpenNixie has been designed as a modular platform. This means that the control PCB can be connected to any PCB with Nixie tubes, so the applications are unlimited. At the moment the V2 version has been tested with a base for 4 1N-12A Nixies with RGB LEDs and SPI expansion.

## Warning

## GPIOs

Function | GPIO C3T | GPIO C3 | Mode
-------- | -------- | -------- | --------
LED | GPIO6 | GPIO6 (CLK) + GPIO7 (SDI) | Output
Latch* | GPIO3 | ** | Output
Button | GPIO5 | GPIO5 | Input
Charge Stat. | GPIO1  | ext. LED | Input
Bat Voltage | GPIO4 | GPIO4 | Input
Bat Voltage EN | -- | GPIO3 | Output

*Enabling the LDO can be done by pressing the button of the device or turning the latch high. In most use cases, the latch GPIO should be turned on as the first task of the Picoclick. Once the task is completed the device can be powered off by turning the latch off (e.g. pulling it low).

**The Picoclick C3 doesn't need a latching GPIO because it uses the embedded flash's power supply pin as a reference. It can be depowered with putting the ESP32 in deepsleep. To reduce the power consumption of the ESP32 this function will disable the power of the embedded flash (`VDD_SPI`) which in result will depower the Picoclick itself. The deepsleep calling function is only necessary to pulling the `VDD_SPI` line low, not to use the deepsleep mode of the ESP32.

## Board overview C3T (Battery connections)

<img src="Docs/Controller_diagram_bottom.jpg" width="500px"></a>
<img src="Docs/Controller_diagram_front.jpg" width="500px"></a>

## Flashing firmware to the ESP32 (C3T)

**- Press and hold the button during the complete flashing process! Otherwise the ESP32 will be loose power and the upload process will crash!**

**- A battery or a power supply has to be applied to the battery pads (3.5v - 5.5v) in order to flash the device!**

Except the above, the Picoclick behaves like a normal development board. No need to get the ESP32 into download mode or pressing any reset button.

## Cases for the C3T

Round design. Combined with two M2x8mm screws.

<img src="docs/pc3t_case2.jpg" width="250px"></a>

# FAQ

## My Picoclick C3T is not shown on the device list when I plug it in the computer - what can I do?
- Your Picoclick is well tested before shipping, so it is probably not broken. Make sure you have connected a decent power supply to the battery connections (3.5v - 5.5v). Battery polarity is shown above. The USB-C jack is only for charging the battery and to flash the ESP32 - not for powering the device.

## Media


