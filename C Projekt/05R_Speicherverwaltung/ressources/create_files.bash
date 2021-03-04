#!/bin/bash

#this script creates .bin, .hex and .out files for all configurations

rm -f *.out *.hex *.bin

# no free
./mymalloc -no-free &> mymalloc_no_free.out
od --address-radix=x --endian=little --width=24 --read-bytes 4096 --output-duplicates --format x8 memory_map.bin >> memory_map_no_free.hex
mv memory_map.bin memory_map_no_free.bin 

# no merge
./mymalloc -no-merge &> mymalloc_no_merge.out
od --address-radix=x --endian=little --width=24 --read-bytes 4096 --output-duplicates --format x8 memory_map.bin >> memory_map_no_merge.hex
mv memory_map.bin memory_map_no_merge.bin

# final
./mymalloc -final &> mymalloc_final.out
od --address-radix=x --endian=little --width=24 --read-bytes 4096 --output-duplicates --format x8 memory_map.bin >> memory_map_final.hex
mv memory_map.bin memory_map_final.bin

echo "if you want copy files to ../ressources   write: yes"
read USER_INPUT
if [[ ${USER_INPUT} == "yes" ]]; then
cp *.bin *.hex *.out ../ressources
echo "files copied"
else
echo "finished"
fi
