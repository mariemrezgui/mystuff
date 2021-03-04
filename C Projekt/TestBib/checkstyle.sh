#!/bin/bash

STYLES=(A1 A2y A4)
INDENTS=(s2 s4 t)
HAPPY=false
EXTENSION="${1##*.}"
BASENAME=$(basename -- $1)
TMPDIR="/tmp/astyle"
cp "$1" "$1.bak"
besterror="-1"
bestfit_style=""
bestfit_indents=""
mkdir -p "$TMPDIR"
for a in ${STYLES[*]}; do
	for i in ${INDENTS[*]}; do
		OUTPUT=$(astyle -j "-$a" "-$i" $1)
		FORMAT=$(echo $OUTPUT | grep "Formatted")
		if [ "$FORMAT" = "" ]; then
			HAPPY=true
		else
			#meld "$1.bak" "$1"
			:;
		fi
		NEWFILE="${BASENAME%.*}.$a.$i.$EXTENSION"
		cp "$1" "$TMPDIR/$NEWFILE"
		cp "$1.bak" "$1"
		diff -d --color=always "$1" "$TMPDIR/$NEWFILE" > "$TMPDIR/$NEWFILE.diff"
		error=$(cat "$TMPDIR/$NEWFILE.diff" | wc -l)
		if [ "$besterror" -lt 0 ] || [ "$error" -lt "$besterror" ]; then
			besterror="$error"
			bestfit_style="$a"
			bestfit_indents="$i"
		fi
		#echo "$BASENAME($a, $i)=$error" 
		rm "$1.orig" 2>/dev/null
	done
done
rm "$1.bak"
if [ "$HAPPY" != "true" ]; then
	echo "bestfit($BASENAME): $bestfit_style $bestfit_indents $besterror"
	cat "$TMPDIR/${BASENAME%.*}.$bestfit_style.$bestfit_indents.$EXTENSION.diff"
	echo -e "\e[31;1mEs wurden Fehler oder Inkonsistenzen in der Einrückung oder Klammerung von $1 gefunden.\e[0m"
	echo -e "\e[31;1mEin Änderungsvorschlag zur Behebung wurde über dieser Mitteilung als diff ausgegeben.\e[0m"
	exit 1;
fi
rm -rf $TMPDIR
exit 0;
