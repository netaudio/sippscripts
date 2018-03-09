#!/bin/bash

sipp -sf uac_pcap.xml -inf reg_uac.csv  -r 1 -rp 100 -p 6067 -i 192.168.101.89 -m 3000 192.168.101.168:5060
