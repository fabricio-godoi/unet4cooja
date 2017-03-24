#!/bin/bash

## Variables
CONTIKI_PATH=/home/user/contiki         # Select the specific path to contiki
MSPSIM_PATH=$CONTIKI_PATH/tools/mspsim  # Select the path to mspsim

## Copy alterated files to specifics locations
cp mspsim/MSP430f2617Config.java $CONTIKI_PATH/tools/mspsim/se/sics/mspsim/config/MSP430f2617Config.java
cp mspsim/BasicClockModule.java $CONTIKI_PATH/tools/mspsim/se/sics/mspsim/core/BasicClockModule.java
cp mspsim/MSP430Config.java $CONTIKI_PATH/tools/mspsim/se/sics/mspsim/config/MSP430Config.java

LOCAL_PATH=$(pwd);
cd $MSPSIM_PATH
make
cd $LOCAL_PATH

