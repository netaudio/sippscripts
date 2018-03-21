@echo off & title inside_reg
:start
if not exist sipp.exe (cd ..
goto start)

sipp -sf xml\auto_exec\register\common\regclient_reg.xml 172.32.225.220:5060 -i 172.32.231.180 -p 5088 -inf xml\auto_exec\basecall\normal_inside_audio_g711a\caller_inside_audio_g711a.csv -m 1

sipp -sf xml\auto_exec\register\common\regclient_reg.xml 172.32.225.220:5060 -i 172.32.231.181 -p 5099 -inf xml\auto_exec\basecall\normal_inside_audio_g711a\callee_inside_audio_g711a.csv -m 1

pause