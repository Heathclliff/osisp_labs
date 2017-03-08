#!/bin/bash
ERR_LOG="/home/valery/labs_osisp/lab1//err.log"
exec 6>&2 2>$ERR_LOG
cd $2
for i in $(find . -type f | grep "$1" -rl "$PWD")
do
echo "$(readlink -f "$i") $(stat -c %s "$i")"
done
exec 2>&6 6>&-
sed "s/.[a-zA-Z ]*:/`basename $0`:/" < $ERR_LOG 1>&2
