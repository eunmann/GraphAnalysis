#!/bin/sh

file_name=output
current_time=$(date "+%Y.%m.%d-%H:%M:%S")
file_ext=.txt
final_name=$file_name.$current_time$file_ext

./main > $final_name