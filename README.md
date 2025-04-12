# Gui based on lvgl for eBike displays running the EBiCS firmware

Features:
* customisable (obviously ...)
* Faster update rates than any factory device
* Custom communication protocol to be able to edit settings of the controller on the fly

## Supported displays

- [KT LCD8H](src/hal/lcd8h/README.md)
  - OC mode runs the device at 200mhz with external crystal.
    - A external clock source is required (8MHz crystal with two ~18pf 0603 caps) for higher clockspeeds.
  - Fast ILI9844 communication, original device runs at about 2MHz max but the ILI9844 is capable of a lot more, the current implementation does ~14MHz at 200MHz bitbanged, and ~12MHz with DMA.
    - Tested with 20cm cable and no issues experienced
    - Should get you 30 FPS easily.
- [EB04](src/hal/eb04/README.md)
  - I was hoping for it to be a bogstandard ARM cortex-M but it's a MediaTek chip with missing SDK due to mediatek being mediatek.

## Known issues

- KT LCD8H
  - No support for a clock since the displays do not have battery and 32khz crystal. You can install a 32khz crystal if you want but there is no deepsleep support due to lack of battery and the way it's implemented.
  - No support for light automation since no sensor present.
  - Baudrates over 57k6 are not recommended due to the way it's implemented. This causes the "low" signal to be significantly shorter than required on higher baudrates missing the measurepoint.
  - Baudrates over 9k6 need the bodge cap removed. I do not know why it is installed as the signal is fine without it.

# Attribution
Temperature icon: [Temperature Vectors by Vecteezy](https://www.vecteezy.com/free-vector/temperature)
Engine icon: [Heat icons created by HAJICON - Flaticon](https://www.flaticon.com/free-icons/heatv)
Brake icon: [Brake icons created by Smashicons - Flaticon](https://www.flaticon.com/free-icons/brake)
Trip icon: [Trip icons created by improstudio - Flaticon](https://www.flaticon.com/free-icons/trip)
Other icons: [Uicons - Flaticon](https://www.flaticon.com/uicons)
Headlight and headlight auto: [Headlight icons created by TravisAvery - Flaticon](https://www.flaticon.com)

# License
All stuff not already licenced otherwise is only available under CC-BY-NC-4.0 https://creativecommons.org/licenses/by-nc/4.0/ . For commercial use contact me.
