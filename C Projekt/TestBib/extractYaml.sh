#!/bin/bash
# important note: exec this script, don't just run it!

IFS=$'\n';
SEEN_TOKEN=false;
TOKEN="BSYAMLTOKENSTART";
for line in $($1 --yaml); do
	if [ "$SEEN_TOKEN" = "true" ]; then
		echo $line;
	elif [ "$line" = "$TOKEN" ]; then
		SEEN_TOKEN=true;
	fi
done
