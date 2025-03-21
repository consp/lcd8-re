# LCD8 reverse engineering project

Since my LCD8 device fell appart due to the plastic becomming brittile (I guess it's not proper ABS, I'm switching to a bafang anyway) I decided to open it up and see what's going on.

In the end I'm writing my own firmware for it because reasons.

## Board
Front:
![Front of PCB](https://github.com/consp/lcd8-re/blob/master/img/Front.png)
Back:
![Back of PCB](https://github.com/consp/lcd8-re/blob/master/img/Back-mirror.png)

### Chips

- U1: MCU AT32F415RCT7, 256k flash
- U2: C1117-3.3 3.3v LDO
- U3: XLSEMI XL7015E1 DC/DC converter 0.8A 5-80V
- U4: 1B0B0G Unknown IC, ~~likely~~ a I2C EEPROM (likely 4k) acts like a standard EEPROM with 2 byte read and 3 byte write. Address is 0x50 looking at the first byte. Only page 1 is ever read.

- Qn: Either G1 or BG (NPN or PNP fet)
- Q?: A1043 Unknown transistor, likely NPN

### Notable things
- Beware! The Electrolithic caps might be snot-glued in.
- Bodge cap just north of the MCU
- Place for one more cap on power input on the DC/DC converter.
- Place for one more cap on both VDD and +5V.
- Unconnected 2 pin connector connected to PC0, likely for a non installed button. You could use it for a light sensor as PC0 is an ADC channel.
- Two large pads connected to GND/VBAT, on the USB versions this is where the USB DC/DC converter is connected.
- Two small pads connected by zener to GND/VDD, likely to supply the chip if a RTC low freq. crystal is provided or by using the CINT clock.
- There is a NTC inside the button pcb on channel PC1 (could also be PC2 if installed upside down). It's read normally and acts as a button when pulled to ground.
- Power enable is a very simple "press button and get power", after which the MCU takes over by triggering PA4 (sense for on button is on PA6). This also means the button is at VBAT and the inputs of the other buttons are not protected so make sure they do not short out.
- Power enable will not work with an external sink (e.g. a debugger) attached. 
- The MCU does not like to be power glitched. This will destroy any data in flash memory. (experimentally verified, repeatedly)

## Display
The display has the following characteristics:
- 320x480 pixels
- 8080 interface with a ILI9488 chip as display driver.
- 16bit parallel i8080 bus
- 256k colors, recommended to use 64k as the bus is 16bit.

Pinout control lines:
- Pin 9 (PC11) is CS
- Pin 10 (PC10) is DCx (Data/Command select)
- Pin 11 (PC9) is Write Strobe (WRx)
- Pin 12 (PC8) is Read Strobe (RDx)

![LCD connector](https://github.com/consp/lcd8-re/blob/master/img/lcd_connector.jpg)

Markings display: 
- LM35014B2
- HCC350-06A29
- LM-035CK45021N0-14B2-B1

Markings connector:
- MCU 16B
- RGB 16/18
- BJG-035CK4502N0

The CS is not needed but used by the original firmware. The source contains the gamma setup input copied via logic analyser. The original firmware never reads from the chip but the lines are attached and work.
With no delay states from the MCU the ILI9488 can do ~10MHz when pushing data from memory (~10ns write burst, 90ns delay). In practice it is less but still about 4x the speed of the original firmware.

## Software
The MCU is locked, writing will destroy the original data. BEWARE!

* LVGL 8.4 for most gui parts, can be used 9.x uses a lot more memory resulting is possible tearing (the framebuffer is 32 lines and less results in partial writes of components).
* Raw write for the large letters as lvgl will make buffers for them which eat all the memory (which is not needed anyway).

## Schematic
My poor attempt at drawing a schematic.
![Schematic](https://github.com/consp/lcd8-re/blob/master/img/schematic.svg?raw=true)

# License
All stuff not already licenced otherwise is only available under CC-BY-NC-4.0 https://creativecommons.org/licenses/by-nc/4.0/ . For commercial use contact me.
