#!/bin/bash

sipp -sf uas.xml -inf reg_uas.csv -s 7000 -deadcall_wait 0 -p 6080 -i 192.168.101.113 -m 5000 192.168.101.168:5060
