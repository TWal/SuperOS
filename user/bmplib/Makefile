
CXX = ../../g++-supos
#CXX = g++


all: libBMP.a

BMP.o: BMP.cpp BMP.h
	$(CXX) -c BMP.cpp -o BMP.o

libBMP.a: BMP.o
	ar rcs libBMP.a BMP.o

clean:
	rm -f BMP.o
	rm -f libBMP.a
