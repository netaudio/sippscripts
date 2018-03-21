#!/bin/bash


sipp -sf uas_pcap.xml -inf reg_uas_all.csv -s 7000 -deadcall_wait 0 -p 6080 -i 192.168.101.172 192.168.101.168:5060
