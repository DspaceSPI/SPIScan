#!/bin/bash
echo
cd /home/brian/DspaceSPI/SPIScan/mjpg-streamer/captures
wget -O tries=10 --output-document=/home/brian/DspaceSPI/SPIScan/mjpg-streamer/captures/capture.jpg http://127.0.0.1:8090/?action=snapshot
sleep 2
killall mjpg_streamer
