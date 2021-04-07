# Generic Modbus RTU driver
Generic Modbus driver firmware for RS-485 Featherwing with a Particle Argon or Boron microcontroller.

Connect your Modbus RTU device to your phone using the Particle.io mobile app.

A generic app that lets you set/get any modbus holding register.

![RS-485 Featherwing](img/MRv0.8_ARGON.JPG)

## Components

* Modbus RTU device: default device id 1, 9600 baud rate, N81 - no partiy, 8 data bits, 1 stop bit.
* RS-485 Featherwing ([github](https://github.com/timshac/RS-485-FeatherWing), [tindie](https://www.tindie.com/products/hudsonsonoma/rs-485-modbus-controller-with-mobile-app/))
* Particle Argon or Boron ([particle.io](https://store.particle.io/collections/gen-3))

## Connect hardware

1. Wire the RS-485 Featherwing to the modbus RTU device.  Connect D1 <-> A and D2 <-> B
2. Insert the Boron or Argon into the RS-485 Featherwing
3. Power the RS-485 Featherwing with 5.2-36v DC (300mA @ 5.2vdc, or 150mA @ 24vdc), or via the usb port.
4. Connnect particle Boron (Cellular) or Argon (WiFi) to the Particle cloud using the Particle mobile app: [Android](https://play.google.com/store/apps/details?id=io.particle.android.app), [iOS](https://apps.apple.com/us/app/particle-iot/id991459054)

Then you are ready to flash the firmware

## Flash Firmware

Clone this repository, and install the [particle command line tools](https://docs.particle.io/tutorials/developer-tools/cli/)

```
# edit src/particle-generic-modbus.ino to set your device id, baud rate, and parity.
# default device id is 1, baurd rate 9600, parity 8N1

cd particle-generic-modbus
# compile and flash over WiFi or Cellular
particle flash device_name 

# or, for usb connected particle Boron
particle compile argon|boron --saveTo=out.bin
particle serial list
particle serial flash out.bin
```

## Monitor and Control your device
### Particle Functions

Open the Particle.io app on your phone to control your device. 

Get the value of the first register (0) as unsigned integer
```
GetOrSet_Register_Unsigned_regORregEQvalue_0_65535
0
```

Get the value of the first register (0) as signed integer

```
GetOrSet_Register_Signed_regORregEQvalue_-32767_32767
0
```

Set the value of register 8 as unsigned integer
```
GetOrSet_Register_Unsigned_regORregEQvalue_0_65535
8=25
```

Set the value of register 8 as signed integer
```
GetOrSet_Register_Unsigned_regORregEQvalue_0_65535
8=-1
```

Each particle command posts a JSON event to report its operation. The event is a single enty array. The event is posted to *GenericModbus/cmd_resp* For example:
```
[
  {
    "function": "set_signed",
    "command": "-625",
    "result": "-625",
    "error": 0
  }
]
```


### Particle Events

Once a minute a status update is posted to the event *GenericModbus/status*
```
{
  "0": 302,
  "1": 616,
  "2": 1024,
  "3": 0
}
```

Additional or specific registers can be included in the status report with a simple modification of the source code.

## Let's talk - Office Hours

Are you interested in this system of tools we are building? 

We would love to hear about what you are working on!

Please book a free 20 minute session - [Hudson Sonoma Office Hours](https://calendly.com/twbishop/hudson-sonoma-office-hours) - if you would like to talk about any of the following:

* Is my equipment compatible with this RS-485 Featherwing for remote control?
* Understanding what this is and why it might be useful for you
* Customizing the firmware or mobile app
* Feedback having used (or tried to use) this product