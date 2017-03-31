#!/bin/bash

cxxCmdLine=`make -s getCompileLine`

sudo echo "Super unit test system :"

for f in unitTests/*.cpp
do
    b=`basename $f`
    b2=${b%.*}
    echo Testing $b2
    if ! $cxxCmdLine -c $f -o out/unittest.o
    then
        echo test $b2 failed to compile
        exit 1
    fi
    if ! make -s buildunit > /dev/null
    then
       echo OS fail at compile time on test $b2
       exit 1
    fi
    outfile=${f%.*}.out
    # Change the 2s if the system is too slow to boot
    timeout --foreground 2s make -s runqemuu >logqemu.txt 2>/dev/null

    o=${f%.*}.out
    if ! cmp logqemu.txt $o
    then
        echo Test $b2 failed : wrong output
        #exit 1
    fi
done

rm kernel.elf out/unittest.o out/kmain.cpp.o

exit 0
