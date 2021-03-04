#!/bin/bash
# important note: exec this script, don't just run it!

IFS=$'\n';
SEEN_TOKEN=false;
TOKEN="BSENVTOKENSTART";
CMD="$1 --env 2>/dev/null"
for line in $(eval "$CMD"); do
	#echo "DEBUG ($SEEN_TOKEN): $line";
	if [ "$SEEN_TOKEN" = "true" ]; then
		#echo "EVAL: $line";
		eval $line;
	elif [[ "$line" == *"$TOKEN"* ]]; then
		#echo "TOKEN: $line";
		SEEN_TOKEN=true;
	fi
done
