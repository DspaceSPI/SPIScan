#!/bin/bash
cd /home/brian/pyramidstuff/MockSPIGUI2/templates/mjpg_streamer
echo "================================================"
echo "Hi, $USER!  Starting 200x150 video from cam 0, frame rate 5 per sec view on port 8091"
echo "================================================"
echo
./mjpg_streamer -i "./input_uvc.so -d /dev/video0 -r 200x150 -f 1 -y" -o "./output_http.so -p 8091 -w ./www" 
