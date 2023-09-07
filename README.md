# OpenNixie 

<img src="Docs/_DSC1764.jpg" width="300px"></a>
<img src="Docs/_DSC1766.jpg" width="300px"></a>

The Picoclick C3 as well as its little brother (the C3T, T = tiny) are the successors of the well known [Picoclick](https://github.com/makermoekoe/Picoclick). The name is related to its new processor: the [ESP32-C3](https://www.espressif.com/sites/default/files/documentation/esp32-c3_datasheet_en.pdf) single core RISC-V 160MHz CPU.

The Picoclick is a tiny WiFi and BLE IoT button for several applications. Originally designed for smart home things, the Picoclick can also be used as an actuator for IFTTT automations or as an MQTT device. It is based on the single core ESP32-C3 RISC-V processor and therefore comes with tons of useful features.
With dimensions of only 10.5mm by 18mm, the C3T is not only the smallest one in the family of the Picoclicks, it is also the smallest device I have created so far.

The [Youtube video of the C3T](https://www.youtube.com/watch?v=t-50w3RsUlg) shows the assembly and soldering process, the optimization of the boot up time, the power measurements as well as some useful applications of the Picoclick.

Here are some specs of the Picoclick C3T:
- Dimensions are only 10.5x18mm
- Ultra low standby current due to latching circuit (no sleep mode needed)
- Single button & LED interface (WS2812 RGB LED)
- USB Type-C for charging the battery and flashing the ESP32-C3

<a href="https://www.tindie.com/stores/makermoekoe/?ref=offsite_badges&utm_source=sellers_makermoekoe&utm_medium=badges&utm_campaign=badge_large"><img src="https://d2ss6ovg47m0r5.cloudfront.net/badges/tindie-larges.png" alt="I sell on Tindie" width="200" height="104"></a>

## Multi Board Architecture 
holi2
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


