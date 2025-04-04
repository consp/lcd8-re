# Gui based on lvgl for eBike displays running the EBiCS firmware

Features:
* customisable (obviously ...)
* Faster update rates than any factory device
* Custom communication protocol to be able to edit settings of the controller on the fly

## Supported displays

- [KT LCD8H](src/hal/lcd8h/README.md)
  - OC mode runs the device at ~250mhz which it likely can do (ymmv, disable OC if you use the internal timer or experiment).
    - A external clock source is required (8MHz crystal with two ~18pf 0603 caps) for higher clockspeeds.
  - Fast ILI9844 communication, original device runs at about 2MH max but the ILI9844 is capable of a lot more, the current implementation does >7MHz at 250MHz clock and about 3-4MHz at 144MHz.
    - Tested with 20cm cable and no issues experienced
- [EB04](src/hal/eb04/README.md)
  - TBD when it arrives

## Known issues

- KT LCD8H
  - Default internal clock is not very accurate and the baudrate generator does weird things over 72mhz, you might need to tweak the baudrate multiplier to your display if you use 57k6 baud.
  - No support for a clock since the displays do not have battery and 32khz crystal. You can install a 32khz crystal if you want but there is no deepsleep support due to lack of battery.
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
