#!/bin/bash

sipp -sf uac_pcap.xml -inf reg_uac.csv  -r 1 -rp 100 -p 6090 -i 192.168.101.113 -m 1 192.168.101.168:5060
