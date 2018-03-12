#!/bin/bash

sipp -sf uac_pcap.xml -inf reg_uac.csv -deadcall_wait 0 -trace_err -r 1 -rp 1 -p 6090 -i 192.168.101.89 -m 300 192.168.101.168:5060
