#!/bin/bash
CF=RGB888
COMPRESS=
# COMPRESS=--compress RLE 
python3 thirdparty/lvgl/scripts/LVGLImage.py --ofmt C --cf $CF -o src/img/display/lvgl9 img/display/icon_headlight_auto.png 
python3 thirdparty/lvgl/scripts/LVGLImage.py --ofmt C --cf $CF -o src/img/display/lvgl9 img/display/icon_headlight_auto_enabled.png 
python3 thirdparty/lvgl/scripts/LVGLImage.py --ofmt C --cf $CF -o src/img/display/lvgl9 img/display/icon_headlight.png 
python3 thirdparty/lvgl/scripts/LVGLImage.py --ofmt C --cf $CF -o src/img/display/lvgl9 img/display/icon_headlight_enabled.png 
python3 thirdparty/lvgl/scripts/LVGLImage.py --ofmt C --cf $CF -o src/img/display/lvgl9 img/display/icon_brake.png 
python3 thirdparty/lvgl/scripts/LVGLImage.py --ofmt C --cf $CF -o src/img/display/lvgl9 img/display/icon_clock.png 
python3 thirdparty/lvgl/scripts/LVGLImage.py --ofmt C --cf $CF -o src/img/display/lvgl9 img/display/icon_engine.png 
python3 thirdparty/lvgl/scripts/LVGLImage.py --ofmt C --cf $CF -o src/img/display/lvgl9 img/display/icon_journey.png  
python3 thirdparty/lvgl/scripts/LVGLImage.py --ofmt C --cf $CF -o src/img/display/lvgl9 img/display/icon_temperature.png 
python3 thirdparty/lvgl/scripts/LVGLImage.py --ofmt C --cf $CF -o src/img/display/lvgl9 img/display/icon_trip.png 
python3 thirdparty/lvgl/scripts/LVGLImage.py --ofmt C --cf $CF -o src/img/display/lvgl9 img/display/icon_energy.png 
python3 thirdparty/lvgl/scripts/LVGLImage.py --ofmt C --cf $CF -o src/img/display/lvgl9 img/display/battery_black.png 

