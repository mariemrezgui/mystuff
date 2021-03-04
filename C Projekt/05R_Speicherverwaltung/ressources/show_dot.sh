#!/bin/bash
if [[ $# -lt 1 ]]; then
	echo "Usage: $0 <map.bin>";
	exit 1;
fi

make && \
	./dotmalloc "$1" | tee out.dot; \
	dot -Tpng -o out.png out.dot; \
	xdg-open out.png;
