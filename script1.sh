#! /bin/bash

log="log"
wordsCount=0

for fname in "$log"/*
do
    #metraw th leksi search se ola ta arxeia
    for word in `cat $fname`
    do
        if [ $word == "search" ] 
        then
            wordsCount=`expr $wordsCount + 1`
        fi
    done
done

echo "Search words in log files = $wordsCount"
