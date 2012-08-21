CC = gcc
C++ = g++
_dspace.so:	scan.o dspace_wrap.o ardintf.o
	$(C++) -shared scan.o ardintf.o dspace_wrap.o -o _dspace.so -lpthread -lsane -ljpeg

test:	test.o scan.o
	$(CC) -o test test.o scan.o -lpthread -lsane -ljpeg -g


dspace_wrap.o:	dspace_wrap.c
	 $(C++) -c dspace_wrap.c -I/usr/include/python2.7  -fPIC -I../Arduino  -I../ArdInterface


dspace_wrap.c:	dspace.i
	swig -python -I../Arduino  -I../ArdInterface -includeall dspace.i 
	
test.o:	test.c
	$(C++) -c test.c -g
scan.o:	scan.c
	$(C++) -c scan.c -g  -fPIC
ardintf.o:	../ArdInterface/ardintf.c ../ArdInterface/ardintf.h ../Arduino/SPIproto.h
	$(C++) -c ../ArdInterface/ardintf.c -I ../ArdInterface -I ../Arduino -g -fPIC

clean:
	rm -f *.o *.pyc  dspace.py *.so dspace_wrap.c
