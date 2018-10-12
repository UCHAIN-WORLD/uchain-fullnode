#!/bin/bash

MYIP=`curl ifconfig.me`
TARGET=~/.UChain/uc.conf

mkdir -p ~/.UChain/

if [ ! -f ${TARGET} ]; then
    cp ../etc/uc.conf ${TARGET}
    sed -i 's/127.0.0.1:8707/0.0.0.0:8707/g' ${TARGET}
    sed -i "/\[network\]/a self=${MYIP}:5678" ${TARGET}
fi
