#!/bin/bash

# Configuration
DEFAULT_COOJA_DIR="/home/user/contiki/tools/cooja";
NEW_RADIO_LOGGER="./RadioLogger.java";
NEW_UNET_ANALYZER="./MicroNetAnalyzer.java";
if [ ! -f $NEW_RADIO_LOGGER ]; then echo "Missing 'RadioLogger.java' in installation folder!"; fi
if [ ! -f $NEW_UNET_ANALYZER ]; then echo "Missing 'MicroNetAnalyzer.java' in installation folder!"; fi

# Input methods
if [ "$1" = "default" ]; then
COOJA_DIR=$DEFAULT_COOJA_DIR;
elif [ "$#" -lt 1 ] || [ "$1" = "help" ] || [ "$1" = "-h" ] || [[ ! -d "$1" ]]; then
echo "Please inform the cooja directory!";
echo "Example1: install.sh /home/user/contiki/tools/cooja";
echo "Example2: install.sh default";
echo "          'default' is /home/user/contiki/tools/cooja";
else
COOJA_DIR="$1";
fi

# Check if the dir ends with '/'
if [[ ! $COOJA_DIR =~ .*\/$ ]]; then
COOJA_DIR+="/";
fi

# Check if it's realy the Cooja directory
RADIO_LOGGER="$COOJA_DIR""java/org/contikios/cooja/plugins/RadioLogger.java";
ANALYZERS="$COOJA_DIR""java/org/contikios/cooja/plugins/analyzers/";
if [[ ! -f $RADIO_LOGGER ]] || [[ ! -d $ANALYZERS ]]; then
echo "Wrong directory, please check if all Cooja project is present!";
exit 1;
fi


# Create a backup before replacing the files
DATE=`date +%Y%m%d%H%M`;
BACKUP="./backup_$DATE";
if [[ ! -d "$BACKUP" ]]; then mkdir "$BACKUP"; fi
cp $RADIO_LOGGER "$BACKUP";
cp $ANALYZERS"MicroNetAnalyzer.java" "$BACKUP";
echo "Backup created at $BACKUP";

#Install new files
cp $NEW_RADIO_LOGGER $RADIO_LOGGER;
cp $NEW_UNET_ANALYZER $ANALYZERS;

echo "Installation successful, reload the Cooja simulator!";
echo "";

