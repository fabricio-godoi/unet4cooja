#!/bin/bash

# Check if it's sudo
if [ "$EUID" -ne 0 ]
  then echo "Please run as root"
  exit
fi

GCC_PATH=/home/user/ti/msp430_gcc/bin

source /etc/environment

string='My long string';

if [[ $PATH == *"GCC_PATH"* ]]
then
  echo "Already configured!"
else
  echo "PATH=\"$PATH:$GCC_PATH\"" > /etc/environment
		
  echo "To changes make effect system need to be rebooted, proced? (Y:n): "
  # Reboot?
  read -n 1 c
  if [ $c == "Y" ]; then rebot now; fi;
  if [ $c == "y" ]; then rebot now; fi;
fi

