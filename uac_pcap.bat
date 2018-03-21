#!/bin/bash

sipp -sf uac_pcap.xml -inf reg_uac_all.csv -deadcall_wait 0 -trace_stat -trace_rtt -rtt_freq 50 -trace_err -trace_screen -rate_scale 10 -r 100 -rp 1000 -p 6090 -i 192.168.101.172 -m 300 192.168.101.168:5060
