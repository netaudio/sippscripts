@echo off & title callee_inside_video_h264
:start
if not exist sipp.exe (cd ..
goto start)

sipp -sf xml\auto_exec\basecall\normal_inside_video_h264\callee_inside_video_h264.xml -i 172.32.231.181 -p 5099 -deadcall_wait 0 -m 1 

pause