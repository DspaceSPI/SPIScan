CC = gcc
C++ = g++
OBJ = scan.o dspace_wrap.o ardintf.o startup.o gps.o savetiff.o 

_dspace.so:	$(OBJ)
	$(C++) -shared $(OBJ) -o _dspace.so -lpthread -lsane -ljpeg -ltiff

test:	test.o scan.o
	$(CC) -o test test.o scan.o -lpthread -lsane -ljpeg -g


dspace_wrap.o:	dspace_wrap.c
	 $(C++) -c dspace_wrap.c -I/usr/include/python2.7  -fPIC -I../Arduino  -I../ArdInterface


dspace_wrap.c:	dspace.i ../ArdInterface/ardintf.h  
	swig -python -I../Arduino  -I../ArdInterface -includeall dspace.i 
	
test.o:	test.c
	$(C++) -c test.c -g
startup.o:	scan.c
	$(C++) -c startup.c -g  -fPIC -I ../ArdInterface -I ../Arduino
gps.o:	gps.c savetiff.h
	$(C++) -c gps.c -g  -fPIC
scan.o:	scan.c savetiff.h
	$(C++) -c scan.c -g  -fPIC
savetiff.o:	savetiff.cpp savetiff.h
	$(C++) -c savetiff.cpp -g  -fPIC
ardintf.o:	../ArdInterface/ardintf.c ../ArdInterface/ardintf.h ../Arduino/SPIproto.h
	$(C++) -c ../ArdInterface/ardintf.c -I ../ArdInterface -I ../Arduino -g -fPIC

clean:
	rm -f *.o *.pyc  dspace.py *.so dspace_wrap.c
