#!/bin/bash

if [[ $# -le 0 ]]; then
    echo "usage: ./bashcmd.sh number"
    exit
fi

if [[ $1 -eq 1 ]]; then
    echo "set eth0 to promisc model"
    ifconfig eth0 promisc 
elif [[ $1 -eq 2 ]]; then
    echo "unset eth0 to promisc model"
    ifconfig eth0 -promisc 
fi

