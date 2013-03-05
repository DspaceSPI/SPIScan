#!/bin/bash
cd /home/brian/pyramidstuff/MockSPIGUI2/templates/mjpg_streamer
echo "================================================"
echo "Hi, $USER!  Starting hi res video from cam 1, frame rate 1 per sec view on port 8090"
echo "================================================"
echo
./mjpg_streamer -i "./input_uvc.so -d /dev/video1 -r 1920x1080 -f 1 -y" -o "./output_file.so -f ./captures -d 6000" 
