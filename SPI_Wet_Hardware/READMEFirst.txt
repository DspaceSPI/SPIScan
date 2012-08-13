READMEFirst.txt

// Procedural comments from Brian Paavo //

0)  Ensure that avrdude is installed on host computer system, should be automatically installed with Arduino IDE
1)  Open a terminal at the /SPI_Wet_Hardware level as command-line commands use relative file locations
2)  Connect an appropriate load (i.e. 2 series 3W LEDs) to LED terminal, note that the default is 100% duty cycle on so be sure to heatsink!
3)  Connect a load to Laser LED terminal (such as 50-250 mW parallel load)
4)  Supply 12 V power to wet end hardward PCB
5)  Connect Atmel AVR programmer (such as A VRISP mkII Main uni, A09-0002/05) to pins nearest USB hub, if connector placed properly programmer indicates good link
6)  Programme USB-serial chip with command line 1 from file main.cmd
7)  Move Atmel programmer to pins nearest ATMega 2560, check for good link
8)  Programme ATMega 2560 with command lines 2 and 3 from file main.cmd

