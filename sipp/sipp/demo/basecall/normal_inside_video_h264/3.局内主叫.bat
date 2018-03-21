@echo off & title caller_inside_video_h264
:start
if not exist sipp.exe (cd ..
goto start)

sipp -sf xml\auto_exec\basecall\normal_inside_video_h264\caller_inside_video_h264.xml 172.32.225.220:5060 -i 172.32.231.180 -p 5088 -inf xml\auto_exec\basecall\normal_inside_video_h264\caller_inside_video_h264.csv -m 1 -d 2000

pause