#!/bin/bash
CF=CF_INDEXED_2_BIT
./lv_img_conv.js --color-format $CF -i icon_headlight_auto -o src/img/display/lvgl8/icon_headlight_auto.c img/display/headlights_auto.png -f
./lv_img_conv.js --color-format $CF -i icon_headlight -o src/img/display/lvgl8/icon_headlight.c img/display/headlights.png -f
./lv_img_conv.js --color-format $CF -i icon_brake -o src/img/display/lvgl8/icon_brake.c img/display/brake.png -f
./lv_img_conv.js --color-format $CF -i icon_clock -o src/img/display/lvgl8/icon_clock.c img/display/clock.png -f
./lv_img_conv.js --color-format $CF -i icon_engine -o src/img/display/lvgl8/icon_engine.c img/display/engine.png -f
./lv_img_conv.js --color-format $CF -i icon_journey -o src/img/display/lvgl8/icon_journey.c img/display/journey.png -f 
./lv_img_conv.js --color-format $CF -i icon_temperature -o src/img/display/lvgl8/icon_temperature.c img/display/temperature.png -f
./lv_img_conv.js --color-format $CF -i icon_trip -o src/img/display/lvgl8/icon_trip.c img/display/trip.png -f
./lv_img_conv.js --color-format $CF -i battery_black -o src/img/display/lvgl8/battery_black.c img/display/battery_black.png -f

