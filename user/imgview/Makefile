
CXX = ../../g++-supos
#CXX = g++

all : imgview

imgview: main.cpp ../bmplib/BMP.a
	$(CXX) -I ../bmplib main.cpp -L ../bmplib -lBMP -o imgview

clean:
	rm -f imgview
