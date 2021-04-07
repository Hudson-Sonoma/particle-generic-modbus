##Overview##
This is an Arduino class library for communicating with Modbus slaves over RS232/485 (via RTU protocol). **Updated to support Arduino 1.0.**

##Features##
The following Modbus functions have been implemented:

Discrete Coils/Flags

  * 0x01 - Read Coils
  * 0x02 - Read Discrete Inputs
  * 0x05 - Write Single Coil
  * 0x0F - Write Multiple Coils

Registers

  * 0x03 - Read Holding Registers
  * 0x04 - Read Input Registers
  * 0x06 - Write Single Register
  * 0x10 - Write Multiple Registers
  * 0x16 - Mask Write Register
  * 0x17 - Read Write Multiple Registers

##Uploading Sketches##
Arduinos prior to the Mega have one serial port which must be connected to USB (FTDI) for uploading sketches and to the RS232/485 device/network for running sketches. You will need to disconnect pin 0 (RX) while uploading sketches. After a successful upload, you can reconnect pin 0.

##Hardware##
This library has been tested with an Arduino [Duemilanove](http://www.arduino.cc/en/Main/ArduinoBoardDuemilanove), PHOENIX CONTACT [nanoLine](http://www.phoenixcontact.com/automation/34197.htm) controller, connected via RS485 using a Maxim [MAX488EPA](http://www.maxim-ic.com/quick_view2.cfm/qv_pk/1111) transceiver.

##Installation##
* Arduino 17 (or later):

  Determine the location of your sketchbook by selecting _File > Preferences_ from within the Arduino IDE. If you don't already have a _libraries_ folder within your sketchbook, create one and unzip the archive there. See [this](http://arduino.cc/blog/?p=313) for more information.

* Arduino 16 (or earlier):

  Download the zip file, extract and copy the _ModbusMaster_ folder to _ARDUINO\_HOME/hardware/libraries_. If you are upgrading from a previous version, be sure to delete ModbusMaster.o.

##Support##
Please report any bugs on the [Issue Tracker](/4-20ma/ModbusMaster/issues).

##Questions/Feedback##
I can be contacted at 4-20ma at wvfans dot net.

##Example##
The library contains a few sketches that demonstrate use of the ModbusMaster library. You can find these in the [examples](/4-20ma/ModbusMaster/tree/master/examples/) folder.

##Acknowledgements##
_Project inspired by [Arduino Modbus Master](http://sites.google.com/site/jpmzometa/arduino-mbrt/arduino-modbus-master)._

##Port to Spark Core##
Adapted for Spark Core by Paul Kourany, March 14, 2014

##Port to Particle Photon##
Further modifications and port to Particle Photon by Anurag Chugh, July 5, 2016
Changes:
1. Replaced spaces with tabs for indentation
2. If the initial bytes received in response to a transmitted frame are "zeros", keep discarding untl a non-zero byte is received
3. Assign an use a TX_Enable pin to switch RS485 driver from receiving to transmitting.
4. If debug is enabled, print TX and RX frames on Serial. Beware, enabling this messes up the timings for RS485 Transactions, causing them to fail.
5. Empty the receive before beginning Modbus transaction by repeatedly calling read() until available() returns 0.


