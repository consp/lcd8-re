# LCD8 reverse engineering project

Since my LCD8 device fell appart due to the plastic becomming brittile (I guess it's not proper ABS, I'm switching to a bafang anyway) I decided to open it up and see what's going on.

## Board
Front:
![Front of PCB](https://github.com/consp/lcd8-re/blob/master/img/Front.png)
Back:
![Back of PCB](https://github.com/consp/lcd8-re/blob/master/img/Back-mirror.png)

### Chips

U1: MCU AT32F415RCT7, 256k flash
U2: C1117-3.3 3.3v LDO
U3: XLSEMI XL7015E1 DC/DC converter 0.8A 5-80V
U4: 1B0B0G Unknown IC, likely a I2C EEPROM 

Qn: Either G1 or BG (NPN or PNP fet)
Q?: A1043 Unknown transistor, likely NPN

### Notable things
- Bodge cap just north of the MCU
- Place for one more cap on +5V line
- Unconnected 2 pin connector connected to PC0, likely for a button
- Two large pads connected to GND/VBAT
- Two small pads connected by zener to GND/+5V likely for USB port *Note that the MCU can draw up to 400mA and the DC/DC is only rated for 0.6A outputting 5V.

## Display
I'm not 100% sure as I have not yet found the actual display. It looks like a 320x200 RGB driven display with 4 PC pins for sync/enable/clock and 16 data pins for parallel data via DMA transfers onto PB*.

![LCD connector](https://github.com/consp/lcd8-re/blob/master/img/lcd_connector.jpg)

Markings display: 
- LM35014B2
- HCC350-06A29
- LM-035CK45021N0-14B2-B1
Markings connector:
- MCU 16B
- RGB 16/18
- BJG-035CK4502N0

It looks like a RGB 16bit interface, the 4 PC8-11 lines being HSync, VSync and Display enable and clock (likely P8 or P9). Not enough pins are connected for the 8080 protocol.

## Software
The MCU is locked, writing will destroy the original data. BEWARE!

## Schematic
My poor attempt at drawing a schematic.
![Schematic](https://github.com/consp/lcd8-re/blob/master/img/schematic.svg?raw=true)

# License
All stuff not already licenced otherwise is only available under CC-BY-NC-4.0 https://creativecommons.org/licenses/by-nc/4.0/ . For commercial use contact me.