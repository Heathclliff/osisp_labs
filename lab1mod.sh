#!/bin/bash
ERR_LOG="/home/valery/labs_osisp/lab1//err.log"
exec 6>&2 2>$ERR_LOG
IFS=$'\n'
for i in $(find $2 -type f  | grep "$1" -rl "$2")
do
echo "$i $(stat -c %s" "%A "$i")" 
done
exec 2>&6 6>&-
sed "s/.[a-zA-Z ]*:/`basename $0`:/" < $ERR_LOG 1>&2
