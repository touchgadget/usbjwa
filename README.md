# USBJWA -- USB Joystick Web App

USBJWA (USB Joystick Web App) turns a tablet or any device with a Web
browser into a USB joystick. Anyone having difficulties using a regular
joystick may find a touch joystick easier to use.

![System Block Diagram](./images/usbjwa_system_diag.gif)

![Screen capture of joystick in browser window](./images/Joystick_Web_App.gif)

The joystick web app displays a 4x8 grid in the browser window. The locations
of touch or mouse events are sent via a web socket back to the ESP32 S2 web
server. The server translates locations to USB HID joystick buttons and sends
them out the USB HID joystick interface.

The ESP32 S2 Saola board is programmed using the Arduino IDE. No soldering is
required. The browser communicates to the ESP32 S2 using WiFi. The ESP32 S2
communicates with the other computer using USB HID.

## Hardware

### USB
This should work on any ESP32 S2 board but has only been tested on the
Espressif Saola and DevKitC boards. Regular ESP32 boards do not have native USB
hardware so will not work.

The ESP32 S2 DevKitC includes a connector for the USB interface as well as the
UART interface. No wires or extra connectors are required.

![Picture of Espressif ESP32 S2 DevKitC board](https://docs.espressif.com/projects/esp-idf/en/latest/esp32s2/_images/esp32-s2-devkitc-1-v1-isometric.png)

On the Saola board, the USB micro connector is connected to a CP2102 USB Serial
chip. The USB data is on pin 19 and 20. A separate USB connector or cable must
be connected to pins 19, 20, GND, and 5V. Do not use both connectors at the
same time. If there is no protection diode on the 5V pin, board may be damaged.
Connect to the built-in USB micro connector to program the ESP32 S2. Disconnect
the cable then plug it into the other USB micro connector to test the USB
feature.

ESP32 S2 Saola      |USB micro connector
--------------------|---------------------
GND                 |GND
5V                  |VBUS
19 (USB D-)         |D-
20 (USB D+)         |D+
not connected       |ID

In the following photo, the USB connector is a SparkFun micro USB breakout
board connected to the ESP32 S2 Saola board using Dupont wires.

![ESPS2 S2 Saola board with external USB connector](./images/esp32s2_usb.jpg)

* [ESP32-S2-Saola-1](https://docs.espressif.com/projects/esp-idf/en/latest/esp32s2/hw-reference/esp32s2/user-guide-saola-1-v1.2.html)
* [ESP32-S2-DevKitC-1](https://docs.espressif.com/projects/esp-idf/en/latest/esp32s2/hw-reference/esp32s2/user-guide-s2-devkitc-1.html)
* [USB Device Driver](https://docs.espressif.com/projects/esp-idf/en/latest/esp32s2/api-reference/peripherals/usb_device.html)

### Joystick Analog Input

#### Saola Board

The ADC1 inputs inputs are reserved for capacitive touch so the joystick X,Y
outputs are connected to ADC2 analog inputs.

|Pin    |Label  |Function           |Joystick
--------|-------|-------------------|--------
|17     |15     |GPIO15, ADC2_CH4   |Xout arduino(A14)
|18     |16     |GPIO16, ADC2_CH5   |Yout arduino(A15)
|19     |17     |GPIO17, INPUT_PULL |Sel
|1      |3V3    |3.3V power         |2.5V (see below for divider)
|42     |GND    |Ground             |GND

https://docs.espressif.com/projects/esp-idf/en/latest/esp32s2/api-reference/peripherals/adc.html

Note: The maximum input value to an analog input pin is 2.5V. A 3.3K Ohm
resistor is used to divide the 3.3 voltage.

```
3.3V
  |
3.3K Ohm resistor
  |
  o-- Joystick VCC
  |
 10K Ohm joystick pot
  |
  Gnd

2.5V(Vout) = 3.3V(Vin) * 10K Ohm / (3.3K + 10K Ohm)
```

## Software

* [Arduino IDE 1.8.16](https://www.arduino.cc/en/software)
* [Arduino ESP32 2.0.0 or newer](https://docs.espressif.com/projects/arduino-esp32/en/latest/)

The following libraries can be installed using the IDE Library Manager.

* [WebSockets by Markus Sattler](https://github.com/Links2004/arduinoWebSockets)
* [ArduinoJson by Benoit Blanchon](https://arduinojson.org/)
* [WiFiManager by tzapu/tablatronix](https://github.com/tzapu/WiFiManager)
