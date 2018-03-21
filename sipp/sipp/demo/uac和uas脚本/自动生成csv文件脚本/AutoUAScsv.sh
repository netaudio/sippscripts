#!/bin/bash
i=2000
rm uas.csv
echo "RANDOM" >uas.csv
while [ $i != 3000 ]
do
  i=$(($i+1))
  echo "$i;$i;[authentication username=$i password=1234]" >>uas.csv
done
