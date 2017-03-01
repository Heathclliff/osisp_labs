#!/bin/bash

for i in $(grep  $1  -rl $2)
do
du -b $i
done

