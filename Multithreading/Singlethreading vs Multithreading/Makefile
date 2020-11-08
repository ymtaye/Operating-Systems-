proj4: proj4.o
	cc  -Wall -pthread proj4.o -o proj4
proj4.o: proj4.c SerialA.h
	cc -c proj4.c -o proj4.o
clean:
	rm -f proj4.o
