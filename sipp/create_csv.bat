#!/bin/bash
callerfile="uac.csv"
calleefile="uas.csv"

caller=1000
callee=2000
totalcaller=1000
echo "SEQUENTIAL" > $callerfile
echo "SEQUENTIAL" > $calleefile

while [ $caller != $callee ]
    do
        echo "$caller;$(($caller+$totalcaller));[authentication username=$caller password=$caller]" >> $callerfile
        echo "$(($caller+$totalcaller));$(($caller+$totalcaller));[authentication username=$(($caller+$totalcaller)) password=$(($caller+$totalcaller))]" >> $calleefile
        caller=$(($caller + 1))
    done;
