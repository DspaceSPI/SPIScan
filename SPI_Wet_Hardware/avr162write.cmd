#sudo avrdude -p at90usb162 -F -P usb -c avrispmkii -U flash:w:Arduino-COMBINED-dfu-usbserial-atmega16u2-Mega2560-Rev3.hex -U lfuse:w:0xFF:m -U hfuse:w:0xD9:m -U efuse:w:0xF4:m -U lock:w:0x0F:m
sudo avrdude -p at90usb162 -F -P usb -c avrispmkii -U lfuse:w:0xFF:m -U hfuse:w:0xD9:m -U efuse:w:0xF4:m
sudo avrdude -p at90usb162 -F -P usb -c avrispmkii -U flash:w:arduino-16usb/Arduino-usbserial.hex

