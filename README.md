# Gui based on lvgl9 for eBikes using a VESC motor controller 

Features:
* customisable (obviously ...)
* Faster update rates than any factory device
* Uses LVGL
* Has MCU with sufficient power and memory to do everything I want it to without too much problems.

## who is this for?

Anyone who wants to build their own eBike display and wants an example. This project is provided as is. No commercial usage is allowed of any kind unless stated explicitly otherwise.

### Who is this not for?

Anyone who expects a ready made product without effort.

## Supported platforms 

![Schematic](/img/pcb.jpg)
![Image](/img/DISPLAY.jpg)
- Custom display:
  - Transflective LCD
  - 4 button support
  - Persistent Real time clock (if battery added) 
  - CAN/UART capable
  - Fast
  - Be able to connect to phone to show nav updates (via custom [gadgetbridge app](https://github.com/consp/Gadgetbridge))
  - Connect and use VESC messages ([custom version needed](https://github.com/consp/bldc), [custom tooling needed](https://github.com/consp/vesc-tool))
  - STM32H743
  - Should have enough power to output 1.5A on the 5V rail to USB

## Known issues
Probably many, there is a bug in the PCB making the ADC unreliable when a battery is installed.

# Attribution
Temperature icon: [Temperature Vectors by Vecteezy](https://www.vecteezy.com/free-vector/temperature)
Engine icon: [Heat icons created by HAJICON - Flaticon](https://www.flaticon.com/free-icons/heatv)
Brake icon: [Brake icons created by Smashicons - Flaticon](https://www.flaticon.com/free-icons/brake)
Trip icon: [Trip icons created by improstudio - Flaticon](https://www.flaticon.com/free-icons/trip)
Other icons: [Uicons - Flaticon](https://www.flaticon.com/uicons)
Headlight and headlight auto: [Headlight icons created by TravisAvery - Flaticon](https://www.flaticon.com/free-icon/headlights_9104236)

# License
All stuff not already licenced otherwise is only available under CC-BY-NC-4.0 https://creativecommons.org/licenses/by-nc/4.0/ . For commercial use contact me. No waranty of any kind is provided. No claims of any kind can be made.

# Contribution

If, for some reason, you want to add something or make a pull request: It's fine to do so but don't expect a prompt response. Any LLM based code or other generated files will be rejected without warning.
If, for some reason, you have a question: Use the discussion part of github. Do not expect a prompt response.
