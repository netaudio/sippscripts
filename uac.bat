#!/bin/bash



sipp  -sn uac -sf uac.xml -inf reg_uac.csv  -trace_err -s 7000 -deadcall_wait 0 -r 1 -rp 10 -p 6090 -i 192.168.101.89 -m 500 192.168.101.168:5060 -rtp_echo -trace_screen

