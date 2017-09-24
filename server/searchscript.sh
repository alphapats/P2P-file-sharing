#!/bin/bash
var=$(grep -wc "$1" filelist.txt)
if [ $var -gt 0 ]
then
grep -w "$1" filelist.txt > searchresult.txt
else
echo "Sorry. Desired file not found" > searchresult.txt
fi

