#!/bin/bash
i=1000
j=3853652000
rm uac.csv
echo "RANDOM" >uac.csv
while [ $i != 2000 ]
do
  echo "$i;$j;[authentication username=$i password=1234]" >>uac.csv
  i=$(($i+1))
  j=$(($j+1))
done
