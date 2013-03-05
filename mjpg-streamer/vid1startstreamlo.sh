#!/bin/bash
cd /home/brian/pyramidstuff/MockSPIGUI2/templates/mjpg_streamer
echo "================================================"
echo "Hi, $USER!  Starting 200x150 video from cam 1, frame rate 5 per sec view on port 8090"
echo "================================================"
echo
./mjpg_streamer -i "./input_uvc.so -d /dev/video1 -r 176x144 -f 5 -y" -o "./output_http.so -p 8090 -w ./www" 
