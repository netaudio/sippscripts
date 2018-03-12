#!/bin/bash

sipp -sf uas_pcap.xml -inf reg_uas.csv -s 7000 -deadcall_wait 0 -p 6080 -i 192.168.101.40 192.168.101.168:5060
