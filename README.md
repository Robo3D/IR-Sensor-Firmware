R2 IR Sensor 
==================
###(Based on the OrmerodSensorBoard)

This is a heavily modified version of the sensor firmware, which increases speed of the sensor. Firstly, the start time is instant instead of around 15 seconds I observed in original version. Secondly, due to the increased responsiveness - on Anet A8 with glass bed I'm getting 0.002 mm error instead of 0.15 mm in the original code. It produces only digital output: just on/off values instead of trying to autodetect digital or analog output to produce.

---

The IR Sensor PCBA uses two IR emitter diodes and one phototransistor to sense the nozzle distance from the build plate.  The circuit is based off Dave Crocker's (dc42) Mini Differential IR board, along with his V1 firmware.  

This folder contains the firmware sources, including the compiled .hex file. The firmware is compiled in Atmel Studio (with the ATTiny library installed). The .pdp files are for doing static checking or formal verification of the firmware using Escher C++ Verifier.  

To program the ATTiny25 mcu, we are using a Tiny Programmer (or JTAGICE MKII) and avrdude to flash the .hex file to the MCU.  

```avrdude -c <programmertype> -p t25 -U flash:w:MiniLedSensor.hex```

To write the fuse value (low fuse):
```avrdude -c <programmertype> -p t25 -U lfuse:w:0xe2:m```

For the schematics for the R2 variant of Dave's board, email developer@robo3d.com.

###Robo does not sell these boards individually!

If you want to get your hands on the latest version of Dave's Mini IR sensor, please check out his blog and follow the links to the appropriate store: https://miscsolutions.wordpress.com/mini-height-sensor-board/

