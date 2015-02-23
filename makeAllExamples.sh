#!/bin/bash

for exampleFolder in $(ls -1d *)
do
    if [[ $exampleFolder != *"example"* ]]
    then
        continue;
    fi
    echo "COMPILING PROJECT: " $exampleFolder
    cd $exampleFolder
    make
    ret=$?
    if [ $ret -ne 0 ]; then
        echo $exampleFolder "COMPILE FAIL" 
    else
        echo $exampleFolder "COMPILE SUCCESS" 
    fi
    cd ../
done