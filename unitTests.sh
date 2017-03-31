#!/bin/bash

cxxCmdLine=`make -s getCompileLine`

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
    if ! make -s buildunit
    then
       echo OS fail at compile time on test $b2
       exit 1
    fi
    outfile=${f%.*}.out
    timeout 3s make -s runqemud

    o=${f%.*}.out
    if ! cmp logqemu.txt $o
    then
        echo Test $b2 failed : wrong output
        exit 1
    fi
done

exit 0
