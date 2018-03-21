@echo off
rem "C:\Program Files (x86)\Sipp_3.1\sipp.exe" -sn uac 192.168.27.149:5060 -inf data.csv  -i 192.168.27.1  -s 1000 -sf uas.xml

 sipp.exe -sn uas 192.168.27.149:5060 -inf uas.csv  -i 192.168.27.1  -sf uas.xml
rem sipp.exe -sn uac 192.168.27.149:5060 -inf data.csv  -i 192.168.27.1  -s 1000 -sn uas
