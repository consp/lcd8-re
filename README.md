# LCD8 reverse engineering project

Since my LCD8 device fell appart due to the plastic becomming brittile (I guess it's not proper ABS, I'm switching to a bafang anyway) I decided to open it up and see what's going on.

## Board
Front:
![Front of PCB](https://github.com/consp/lcd8-re/blob/master/img/Front.png)
Back:
![Back of PCB](https://github.com/consp/lcd8-re/blob/master/img/Back-mirror.png)

### Chips

- U1: MCU AT32F415RCT7, 256k flash
- U2: C1117-3.3 3.3v LDO
- U3: XLSEMI XL7015E1 DC/DC converter 0.8A 5-80V
- U4: 1B0B0G Unknown IC, likely a I2C EEPROM 

- Qn: Either G1 or BG (NPN or PNP fet)
- Q?: A1043 Unknown transistor, likely NPN

### Notable things
- Bodge cap just north of the MCU
- Place for one more cap on power input on the DC/DC converter.
- Place for one more cap on both VDD and +5V.
- Unconnected 2 pin connector connected to PC0, likely for a non installed button. You could use it for a light sensor as PC0 is an ADC channel.
- Two large pads connected to GND/VBAT
- Two small pads connected by zener to GND/+VDD
- Power enable is a very simple "press button and get power", after which the MCU takes over by triggering PA4 (sense for on button is on PA6). This also means the button is at VBAT and the inputs of the other buttons are not protected so make sure they do not short out.

## Display
The display has the following characteristics (if my calculations are correct):
- 320x480 pixels
- 8080 interface with likely a ILI9488 chip, initial assumption about RGB interface was wrong.
- 16bit 2MHz data rate, though only for commands. Data is significantly slower for some reason.
- CA/PASEL commands are at 2MHz, rest is variable but around 750khz. Everything comes in PA/CASEL (10 cycles) followed by 1 cycle RAMWR and 36 cycles of data. This is partial guesswork as I do not have a logic analyser only a scope.
- Blocks are either send in 36 pixels (5:6:5 mode) or 32 pixels (6:6:6 mode)
- Screen is never updated completely, only partially after first draw.

Pinout likely control lines:
- Pin 9 (PC11) is CS
- Pin 10 (PC10) is DCx (Data/Command select)
- Pin 11 (PC9) is Write Strobe (WRx)
- Pin 12 (PC8) is likely Read Strobe (RDx) but is never used (always high)

![LCD connector](https://github.com/consp/lcd8-re/blob/master/img/lcd_connector.jpg)

Markings display: 
- LM35014B2
- HCC350-06A29
- LM-035CK45021N0-14B2-B1

Markings connector:
- MCU 16B
- RGB 16/18
- BJG-035CK4502N0

## Software
The MCU is locked, writing will destroy the original data. BEWARE!

## Schematic
My poor attempt at drawing a schematic.
![Schematic](https://github.com/consp/lcd8-re/blob/master/img/schematic.svg?raw=true)

# License
All stuff not already licenced otherwise is only available under CC-BY-NC-4.0 https://creativecommons.org/licenses/by-nc/4.0/ . For commercial use contact me.
