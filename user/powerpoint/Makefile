CXX = ../../g++-supos
#CXX = g++

all : powerpoint

powerpoint: main.cpp ../bmplib/libBMP.a
	$(CXX) -I ../bmplib main.cpp -L ../bmplib -lBMP -o powerpoint

clean:
	rm -f powerpoint
