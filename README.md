# Crawler-Link
by E. Aaltonen 2023

## First fully working prototype version for testing and evaluation

The aim of this project is to use an on-board MCU for remote control of secondary accessories on an RC truck and for accessing telemetry data gathered by the system through WiFi. This first implementation mainly involves the control of LED lighting on the truck.

The present solution comprises four devices:
- Device A: ESP32C3, for physical user interface, incorporated in the RC transmitter
- Device B: ESP32C3, the main MCU for controlling onboard hardware
- Device C: ESP8266, as a WiFi Access Point (onboard)
- Client device (e.g. a smartphone) to access the HTML interface broadcasted by Device C

Devices A and B are linked through a one-way ESP-NOW connection (A as the initiator and B as the respondent). Devices B and C are linked via I2C (C being the master). Device C serves as a WiFi Access Point for the client device.


## Behaviour: Device A

Pushbuttons connected to Device A control the following functions on Device B:
- Button A: step up lighting level (4 levels: off, parking, low-beam, high-beam)
- Button B: step down lighting level
- Button C: toggle left blinkers
- Button D: toggle right blinkers
- Button E: toggle brake lights (logical brake light state)
- Button F: toggle reverse lights

Physical buttons E and F will be assigned for other purposes in future versions.

Battery supply voltage is connected to an ADC pin with a voltage divider circuit, such that the voltage value can be calculated based on the analog input (the maximum input is 3.3V).

When a button is pushed, Device A transmits a struct consisting of an integer for buttons and a float value for voltage. The value of buttons A-F is exhibited by setting the corresponding bit high:<br>
bits/buttons<br>
7 6 5 4 3 2 1 0<br>
- - F E B A D C

The present behaviour is very simple; the device transmits a message every time a button is pressed. The transmission encoding itself supports pressing multiple buttons simultaneously.


## Behaviour: Device B

Device B has 6 digital outputs, with the following functions:
- LEDs, left blinkers
- LEDs, right blinkers
- LEDs, headlights
- LED, left tail light
- LED, right tail light
- LEDs, reverse lights
These 3.3V outputs are connected to transistors (BC547C), which in turn switch the relevant LED circuits (powered from a 5-volt source) on and off.

Pins D4 and D5 are reserved for the I2C bus.

Because this RC model is a Ford Bronco 1979, front blinkers and tail lights are controlled in accordance with the North American tradition: all corner lights are lit in dim mode when lighting level 1 or above is switched on. When brake is activated, the rear lights light up with full brightness, and when lifted, the previous intensity ("base brightness") based on the lighting level (off or dim) is restored. The blinking function blinks the corresponding LEDs with a similar logic, between full brightness and the base brightness. The behaviour of rear lights in the emergency blinker mode, with both blinkers activated, is identical to pumping the brake repeatedly. When this mode is entered by activating first one blinker and then the other one, the timing of the second blinker matches that of the first one.

The program simulates traditional light bulbs by increasing or decreasing the brightess level over an adjusted onset or offset period.


## Behaviour: Device C

The behaviour of Device C is mostly experimental at this stage. In essence, it establishes a WiFi network for the client device to interface. The SSID is "ESP8266 Thing " plus the two last bytes of its MAC address. The network password is hard-coded in the source.

Device C builds a single HTML page accessible at http://192.168.4.1/k/ on the network. Requests from the client browser are handled with the GET method, whereby a request for 192.168.4.1/k/1 results in control value "1" (toggle left blinker) etc.

Buttons visible on the screen have the same function as the physical devices on Device A and deliver the same functionality. The user is provided feedback on active functionalities by relevant CSS classes as well as a small JS function to "blink" indicators when the blinkers are activated. Another JS function reloads the /0 page after a time interval to update status.

The HTML page is also intended to display the voltage value passed on from Device A as well as the voltage level of the on-board battery. This feature is not implemented in the present solution. ESP8266 only has one ADC pin and could therefore only read the overall supply voltage (with an adequte voltage divider, as the ADC pin is only rated for 3.3 volts). If more relevant information (per-cell voltage) is desired, this chip would need to be replaced with an ESP32 or the readings should be taken on Device B.

When an HTTP request is made, Device C passes on the command to Device B through the I2C bus with a one-byte message, encoded in a manner similar to the integer part transmitted by Device A. 

After this, Device C requests a status update from Device B. Bits 2 and 3 of the message are now assigned to the numeral value of the lighting status (00, 01, 10 and 11). 

This sequence should to be extended from 1 byte to e.g. 5 bytes for future use, to accommodate the float value (4 bytes) for Tx battery voltage.


The source code for Device C is from an earlier project written in the Arduino IDE, while devices A and B are developed with PlatformIO. When changing over to PlatformIO, some changes are necessary, such as renaming pins D0 etc. (as printed on the device) as 16 etc. (for GPIO16 etc., as presented in the appropriate pinout diagram).


## Capabilities of Device C

In this implementation, the WiFi service is taking up most of Device C's resources. Running the LED soft blinker functions on the same chip wasn't feasible. This chip has a handsome number of IO pins, though, and it could be used for simple accessories with no timing or computing requirements.


## Coming up

Features to be implemented in the next stages:
- battery voltage readings to be displayed on the HTML page
- read indication for brake and reverse from the RC Rx speed control signal or the ESC-to-motor signal
- RPM data (digital) from a magnet sensor (on the spur gear)
- motor and ESC temperatures
- position data (x/y axis tilt, compass direction) with a sensor module
