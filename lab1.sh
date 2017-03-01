#!/bin/bash
cd $2
for i in $(grep  $1  -rl   "$PWD")
do
du -b  $i  
done

#find $(readlink -f $2) -regex /$1/ -printf "%h/%f %s\n" 
