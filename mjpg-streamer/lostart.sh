#!/bin/bash
echo
cd /home/brian/DspaceSPI/SPIScan/mjpg-streamer
./mjpg_streamer -i "./input_uvc.so -d /dev/video1 -r 176x144 -f 5 -y" -o "./output_http.so -p 8090 -w ./www" 
