#!/bin/sh

## This file is used to test folding functions and using item-list command

function hello1 {
    echo "Hello One!"
    x=2
    if [ $x -lt 3 ] ; then
        echo "x is small" ;
    else
        echo "x is larger than 2"
    fi
}   

hello2 () {
    echo "Hello Two!"
}

hello3 () 
{
    echo "Hello Three!"
}

hello1
hello2
hello3
