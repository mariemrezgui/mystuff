#!/bin/bash

COLOR_RESET='\033[0m' # No Color
COLOR_BOLD_RED='\033[1;31m'
COLOR_BOLD_YELLOW='\033[1;33m'
COLOR_BOLD_WHITE='\033[1;37m'

SCRIPTNAME=$0
PARAMETER=$1
set -e # aborts script on first error

#VALIDATE INPUT
if [ $# -eq 1 ]; then
	if [[ "$PARAMETER" != "-no-free" && "$PARAMETER" != "-no-merge" && "$PARAMETER" != "-final" ]]; then
		echo -e "${COLOR_BOLD_RED}[ERROR] wrong input${COLOR_RESET}"
		echo -e "${COLOR_BOLD_YELLOW}USAGE: ${SCRIPTNAME} [ -no-free | -no-merge | -final ]${COLOR_RESET}"
		exit 1
	fi
elif [ -z $PARAMETER]; then
	echo -e "${COLOR_BOLD_RED}[ERROR] wrong input${COLOR_RESET}"
	echo -e "${COLOR_BOLD_YELLOW}USAGE: ${SCRIPTNAME} [ -no-free | -no-merge | -final ]${COLOR_RESET}"
	exit 1
fi

#PREPROCESSING
rm -f memory_map.bin
rm -f memory_map.hex
rm -f mymalloc.out

#PROCESSING
COMMAND="./mymalloc $PARAMETER 2>&1 | tee mymalloc.out"
eval $COMMAND

#POSTPROCESSING
od --address-radix=x --endian=little --width=24 --read-bytes 4096 --output-duplicates --format x8 memory_map.bin >> memory_map.hex

#WAIT FOR INPUT
echo -e "\n${COLOR_BOLD_WHITE}Press any key to compare the memory footprint with solution (left: yours, right: solution)${COLOR_RESET}"
read WOM

#COMPARE
if [[ "$PARAMETER" == "-no-free" ]]; then
	meld memory_map.hex memory_map_no_free.hex
elif [[ "$PARAMETER" == "-no-merge" ]]; then
	meld memory_map.hex memory_map_no_merge.hex
else
	meld memory_map.hex memory_map_final.hex
fi
